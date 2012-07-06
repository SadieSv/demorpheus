#ifndef FINDER_LIBEMU_H
#define FINDER_LIBEMU_H

#include <iostream>
#include <fstream>

#include "finder.h" 
#include "timer.h"

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class FinderLibemu : public Finder {
public:
	/**
	Constructor.
	*/
	FinderLibemu();
	/**
	Destructor of class FinderLibemu.
	*/
	~FinderLibemu();
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	int find();
};

#endif // FINDER_LIBEMU_H