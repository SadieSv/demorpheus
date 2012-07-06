#ifndef FINDER_GETPC_H
#define FINDER_GETPC_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <set>

#include "finder.h" 
#include "libdasm.h" 
#include "data.h"
#include "timer.h"
#include "emulator.h"
#include "reader_pe.h"

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class FinderGetPC : public Finder {
public:
	/**
	@param type Type of the emulator. Possible values: 0(GdbWine), 1(LibEmu).
	*/
	FinderGetPC(int type=0);
	/**
	Destructor of class FinderGetPC.
	*/
	~FinderGetPC();
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	int find();
protected:
	/**
	 Function which works with emulator. Makes emulator emulate found chain of instruction and looks for the loop. If neccessary restarts the process of finding dependencies and restarts emulator.
	 @param pos Position in input file from which emulation is started.
	 @return
	*/
	int launch(int pos=0);

	/**
	 New
	 @param pos
	 */
	void find_dependence(uint pos);

	set<uint> start_positions;///<positions where target instructions are alredy found	
	static const uint maxEmulate; ///<limit for emulating
	static const uint maxUpGetPC; ///< New
	Command cycle[256]; // TODO: fix. It should be a member of the Finder::launch(). Here because of qemu lags.	
};

#endif // FINDER_GETPC_H