#ifndef _GRAPH_TOPOLOGY_H_
#define _GRAPH_TOPOLOGY_H_

#include <iostream>
#include <vector>
#include <set>
#include <sys/time.h>

#include "classifier.h"


/**
 * Subtract the timeval values x and y, storing the result in result param
 * @param result	-	here the value of difference will be stored
 * @param x			-	parameter for subtraction
 * @param y			-	difference
 * @return	1 if the difference is negative
 * 			0 otherwise
 */
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
int timeval_addition(struct timeval *result, struct timeval *x, struct timeval *y);


/**
 * Struct for elementary classifier representation inside graph.
 */
struct classifierVertex
{
	Classifier *cl; //!< Current classifier.
	std::vector<Classifier*> outcomings; //!< List of classifiers those have edge from current.
};

class GraphTopology {
private:
	bool _detected_classes[class_num];//!< Indicates classes detected by input classifiers
	std::vector<Classifier*> _current_classifiers;
public:
	std::vector<Classifier*> _elem_classifiers;//!< List of input classifiers
	std::map<Classifier*, std::vector<Classifier* > > _topology;
	
//member functions
public:
	/**
	 * Constructor function.
	 * Automatically adds InitialLength classifier to topology.
	 */
	GraphTopology();
	~GraphTopology();
	
	void makeHybridTopology();
	
	/**
	 * Makes linear combination of input classifiers.
	 * Fills _topology with _elem_classifiers elements.
	 */
	void makeLinearTopology();
	
	bool executeTopology( const unsigned char* buffer, int length, bool reduce = false);
	struct timeval getLastTime() const { return _lastTime; };
	
	void printDetectedClasses();
	void printTopology();
	
	//===========to be private after testing===========================
	/**
	 * Recognizes shellcode classes which can be detected by input elementary classifiers.
	 * Fuction fills _detected_classes array.
	 */
	void recognizeShellcodeClasses();
	void processNextLevel( std::vector< vector<Classifier*> > *levels);
	std::vector<Classifier*> selectOptimalLevel(std::vector< vector<Classifier*> > *levels);
	std::vector<Classifier*> createNextLevel(bool first_iteration);
private:
	/** makes all elements of _detected_classes to be false. */
	void clearDetectedClasses();
	Classifier *mClassifier; //!< Initial vertex
	
	std::map<Classifier*, bool > markedClassifier;
	void clearMarkedClassifier();
	
	struct timeval _lastTime;
};

#endif