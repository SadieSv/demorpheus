#pragma once
#include <vector>
#include <map>
#include <set>

#define MAX_DATA_LEN 256
#define MIN_DATA_LEN 64

//#define LIBDASM
#ifdef LIBDASM
#include "libdasm.h"
#endif


#ifdef LIBDASM
struct oper
{
        enum Operand type;
        int reg;
        DWORD immediate;
};
#endif

struct instruct
{
        int size;
        int type:8;
        int offset;
	//if in chain is instruction with ext_off== true, that means reference to other chain
	// some kind of "chain merging"
	bool ext_off;

#ifdef LIBDASM
        oper op1;
        oper op2;
#endif

};


typedef std::vector< std::vector< instruct > > fl_type;

//TODO: fix Flow class ( make data fields private )
class Flow
{
private:
	/**function dissasemles flow and fills fl field
	 * @param buffer - given source buffer
	 * @param length - length of buffer
	 * @param offset - offset from with to start disassembling
	 */
	void disassemble_flow( const unsigned char *buffer , unsigned length, int off);
	
	/** initialize offsets and ext_offset array */
	void init();
public:
    Flow();
	Flow(const unsigned char* buffer, int length );
	fl_type fl;
	
	//first item in pair stands for chain number, second for offset in the chain
	std::pair< int, int > offsets[MAX_DATA_LEN];
	bool ext_offset[MAX_DATA_LEN];
//	std::map< int, ParCh > ParentChildArray;

//additional function
	void insertChildAndParent( int o, int addr );
	bool doJmp( int o, int addr );
	Flow& generateEIFG();

//dubugging function
	void printFamilyArray();
};


