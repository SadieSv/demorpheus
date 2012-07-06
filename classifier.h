#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include <iostream>
#include <algorithm>
#include <memory>
#include <boost/shared_ptr.hpp>

#include "flow.h"
#include "state.h"
#include "analyzer.h"
#include "eifg.h"
#include "racewalk.h"
#include "finddecryptor.h"

#include "macros.h"
#define MIN_INSTR_CHAIN 14

using namespace std;

//TODO: create config for classifier features (weights)
// now parameters initialized in classes constructors

enum class_type {
	PLAIN,
	NOP,
	DECRYPTOR,
	ALL
};
const int class_num = 3;
static boost::shared_ptr<Flow> flow;
static State* s;
static EIFG* ifg;
/**
 * Common class for all elementary classifiers
 */
class Classifier {
protected:
	static const int max_instruction_length = 16;

//types and enums
public:
	enum vector_type {
		LEGITIMATE,
		MALWARE,
		UNKNOWN
	};
	bool current_type[class_num];//!< type of detected shellcodes
protected:

	static vector_type information_vector[ class_num ];
	
	/**
	 * @defgroup features important classifier features
	 * @{
	 */
	/** False positive rate for random data.*/
	double fp_r;
	/** False positive rate for multimedia.*/
	double fp_m;
	/** False negative rate*/
	double fn;
	/** Time complexity*/
	double complexity;
	/** @} */
	
	/**
	 * @defgroup scaled_features
	 * @{
	 * Scaled important classifier features.
	 * 
	 * Such group of parameters valuable after feature scaling procedure only.
	 * The purpose is to provide comparable weight of different parameters.
	 */
	double scaled_fp_r;
	double scaled_fp_m;
	double scaled_fn;
	double scaled_complexity;
	/** @} */
	
	static unsigned char *_buffer;
	static int data_len;
	
//member functions
public:	
	Classifier();
	virtual ~Classifier(){};
	
	/**
	 * Reloads classifier with new parameters (buffer, features, whatever...)
	 */
	static void reload(const unsigned char *buffer, int length);
	static void init();
	virtual bool check(){};
 	virtual void update(){};
	
	/** Updates bits of information_vector with respect of detected shellcode classes.
	 * Function sets bits of such classes to value of given parameter
	 * @param vector_type	if classes should be sets to malware value or nor
	 */
	void updateInformationVector(vector_type type);
	/**
	 * Checks if vector contains shellcode classes to which buffer is referred
	 */
	static bool checkInformationVector();
	static void printInformationVector();
	
	virtual void printInfo() const {};
	
	//parameters manipulation
	virtual double const getFPR() const {return fp_r;}
	virtual double const getFPM() const { return fp_m;}
	virtual double const getFN() const {return fn;}
	virtual double const getComplexity() const {return complexity;}
	
	virtual double const getScaledFPR() const { return scaled_fp_r; }
	virtual double const getScaledFPM() const { return scaled_fp_m; }
	virtual double const getScaledFN() const { return scaled_fn; }
	virtual double const getScaledComplexity() const {return scaled_complexity; }
	
	/** 
	 * Scaling of important classifier paramenters (weight). The purpose is to provide
	 * comparable weight of different paramenters.
	 * NOTE: Some classes may overload this function. See class declaration for more details.
	 * 
	 * @param f_scaled	result of scaling. This parameter will store scaled value of feature
	 * @param f			feature to scale
	 * @param avr		average feature meaning
	 * @param range		feature range
	 */
	virtual void featureScaling( double *f_scaled, const double f, 
								const double avr, const double range);
};

/**
 * Class filtering input data with length less that MIN_DATA_LEN.
 * NOTE: Class should be the first vertex in graph topology. There is no need to add such
 * classifier into topology manually. It will be presented there automatically.
 */
class InitialLength :public Classifier
{
public:
	InitialLength();
	~InitialLength(){};
	/** Checks whether buffer length more then MIN_DATA_LEN.
	 * @return	true if buffer length more than threshold
	 * 			false otherwise
	 */
	bool check();
	void printInfo() const { cout << " InitialLength "; }
};

/** Parent class for those derived, which uses information located in Flow to do their work. */
class FlowBased	:public Classifier
{
public:
	FlowBased(){};
	virtual ~FlowBased() {};
	
	static void init();
	static void reload();
	virtual bool check(){};
	virtual void printInfo(){};
};

/** Parent class for derived which based on CFG or IFG.*/
class CFGBased
	:public Classifier
{
public:
	CFGBased(){};
	virtual ~CFGBased();
	
	static void init();
	static void release();
	static void reload();
	virtual bool check(){};
	virtual void printInfo(){};
};


//=========================FLOW BASED CLASSES===============================================//

/** Class filtering input data with dissasembled chain length less than threshold. */
class DisasLength
	:public FlowBased {
private:
	/**
	 * function calculates chain size of given variation of instruction chain (in flow class)
	 * and given offset in this chain
	 * @param i	-	chain number
	 * @param j -	offset in chain
	 * @return chain size
	 */
	int getChainSize( int, int ) const;
public:
	DisasLength();

	bool check();
	void printInfo() const { cout << " DisasLength "; }
};

/** 
 * Class filtering input data which isn't dissasembling correctly from each and every offset
 * inside at least nop_len bytes.
 */
class DisasOffset
	:public FlowBased {
private:
	static const int nop_len = 64;
public:
	DisasOffset();
	bool check();
	void printInfo() const { cout << " DisasOffset "; }
};

//=====================CFG BASED CLASSES======================================================//

/** Class based on SigFree heuristic (for more details, see SigFree schema 1). */
class PushCall
	:public CFGBased
{
private:
	/** Push_call patterns threshold. */
	static const int pushCallThr = 1;
	/**
	 * Push or Call threshold 
	 * Sometimes programs avoids explicit push-call pattern, but therefore they still use
	 * push or call instruction.
	 */
	static const int pushOrCall = 2;
	/**
	 * counts maximum number of push-call patterns in the chain which starts
	 * from given offset.
	 * @param off		given offset from which instruction chain starts
	 * @param pcn		number of push-call patterns which are already met
	 * @param flag		indicates if push instruction was already met
	 * 
	 * @return int		max number of push-call patterns
	 */
	int pushCallMax(int off, int pcn, bool flag, int*);
	
	//structure helps to avoid infinite cycles when calculating the number of push-call patterns
	std::set< int > visited;
public:
	PushCall();
	bool check();
	void printInfo() const { cout << " PushCall "; }
};

/** Class based on SigFree heuristic (for more details, see SigFree schema 2).
 * TODO: fix dissasembling for this class
 * TODO: fix SGAnalyzer, EIFG and State logic
 */
class DataFlowAnomaly
	:public CFGBased
{
private:
	/**
	 * @defgroup thresholds Thresholds for DataFlowAnomaly
	 * @{
	 * Used for threshold range calculation.
	 * 
	 * Important issue in the case of buffer of "non-maximum" size. For example, 
	 * if we have just "half-full" buffer, it's needed to decrement threshold range.
	 * Nevertheless, it's still important to have minimum threshold as significant 
	 * heuristic of executable chain.
	 */
	static const int max_thr = 14;
	static const int min_thr = 8;
	/**
	 * @} End of thresholds
	 */
	
	SGAnalyzer* analyzer;
public:
	DataFlowAnomaly();
	virtual ~DataFlowAnomaly();
	bool check();
	void update();
	void printInfo() const { cout << " DataFlowAnomaly "; }
};

/** Class find presense of cycles in flow. */
class CycleFinder
	:public CFGBased
{
public:
	CycleFinder();
	bool check();
	void printInfo() const { cout << " CycleFinder "; }
};

//======================OTHER CLASSES========================================================//
//TODO: make them dependent from CFGBased or FlowBased

/** Class of NOP-sled detecting
 * TODO: fix disassembling for this class
 */
class RaceWalk
	:public Classifier
{
private:
	static const int nop_len = 64;
public:
	RaceWalk();
	
	/**
	 * Analyze buffer for NOP-sled presence
	 * @return true if racewalk analyzer concludes buffer is malware
	 */
	bool check();
	void printInfo() const { cout << " RaceWalk " ;}
};

/** TODO: */
class HDD
	:public Classifier
{
private:
	FindDecryptor *findDecryptor;
	int finderType;
	int emulatorType;
public:
	HDD(int _finderType);
	virtual ~HDD();
	
	bool check();
	void printInfo() const { cout << " HDD ";}
	
	//setters
	void setFinderType(const int type) { finderType = type; }
	void setEmulatorType(const int type) { emulatorType = type;}
	
	//getters
	int getFinderType() const {return finderType;}
	int getEmulatorType() const {return emulatorType;}
};
#endif
