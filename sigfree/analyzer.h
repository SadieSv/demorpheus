
#ifndef _ANALYZER_H
#define _ANALYZER_H

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <string.h>
#include <string>

//#include "libdasm.h"
#include "state.h"
#include "eifg.h"

#define SCHEME2_THR 14

class Comp
{
public:
	bool operator() ( int o1, int o2 )
	{
		if ( o1 < o2 ) return true;
		else return false;
	}
};


class SGAnalyzer
{
	EIFG* ifg;
	State* states;
	std::set< int, Comp > visited;
	
	int last_offset;
	std::map < int, std::set<int> > border;	
private:
	/**
	 * threshold for schema 2.
	 */
	int threshold;

public:
	SGAnalyzer( EIFG* _ifg, State* _states )
			:ifg(_ifg), states(_states), threshold(SCHEME2_THR){};
	SGAnalyzer( EIFG* _ifg, State* _states, int thr)
			:ifg(_ifg), states(_states), threshold(thr){};

	int parentAnomalyDD( int o, int reg );
	
	void  pruningUseless();

	/** calculate the maximum length of instructuion chain 
	 * @param o - given instruction offset
	 * @return maximum length of instruction chain
	 */
	int checkUsefull( int o );
	/**
	 * calculate the maximum length of instruction chain function fills 
	 * the startOff and the endOff vectors with first and last offset of
	 * chain that exeeds given threshold
	 * @param length		-	given threshold for chain length
	 * @param o				-	given instruction offset
	 * @param first			-	the start offset of chain
	 * @param max_offset	-	maximum offset in the chain ( that helps to 
	 * 							escape cycles ( when offset of child instruction less than of given)
	 * @return maximum length of instruction chain
	 */
	int checkUsefull( int length, int o, int first, int max_offset );
	/**
	 * fills the border map
	 * @param first			-	offset of first instruction in the chain
	 * @param o				-	offset of the last instruction in the chain
	 * @param max_offset	-	offset of the instruction before jmp back ( by default = -1 )
	 */
	void fill_border( int first, int o, int max_offset );
	/**
	 * Calculates maximum executable chain length  
	 * @return "0" for non-executable code; max executable chain length otherwise
	 */
	int isExecutable( );
	
	std::map< int, std::set< int > > * getBorder();

};

#endif
