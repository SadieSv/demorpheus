#ifndef _STATE_H
#define _STATE_H

#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <string.h>
#include <string>

#include "libdasm.h"
//#include "eifg.h"

#define	DEFINED		1
#define UNDEFINED	2
#define REFFERED	3
#define UR		4
#define DD		5
#define DU 		6


struct StateLink
{
	int inst_off;
	int st;
};


class State
{
	
public:
	State();	

	//TODO: make it private
	std::map< int, std::vector< StateLink> > states;
	void printStates();
	void newChain();

	State& traverseExec( int o, INSTRUCTION * inst );
	State& traverseReffered( int o, INSTRUCTION * inst );
	State& setRegReffered( int o, int t );

	int getLastState( int reg );
};


#endif
