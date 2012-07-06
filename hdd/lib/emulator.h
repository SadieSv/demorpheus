#ifndef EMULATOR_H
#define EMULATOR_H

#include "reader.h"
#include "data.h"

using namespace std;

/**
	@brief
	Interface for emulators
*/

class Emulator : protected Data {
public:
	/**
	  Binds reader to emulator.
	  @param r Pointer to an examplar of Reader class which is used for reading the file and taking interesting information out of the file header (if present).
	*/
	void bind(Reader *r);
	/**
	  Runs emulation from the instruction situated on specified position in input file.
	  @param pos Position to run emulation from.
	*/
	virtual void begin(uint pos=0) = 0;
	/**
	  Passes emulation to the next instruction.
	*/
	virtual bool step() = 0;
	/**
	  Copies @ref size bytes of current emulated instruction into buffer @ref buff.
	*/
	virtual bool get_command(char *buff, uint size=10) = 0;
	/**
	  Copies @ref size bytes from address @ref addr into buffer @ref buff.
	*/
	virtual bool get_memory(char *buff, int addr, uint size=1) = 0;
	/**
	  Gets dword/word/byte (@ref size) from address @ref addr.
	*/
	virtual unsigned int get_int(int addr, int size=4);
	/**
	  Returns current state of register @ref reg.
	*/
	virtual unsigned int get_register(Register reg) = 0;
	/**
	  Returns memory offset for translating constant values from registers to memory pointers.
	*/
	virtual unsigned int memory_offset();
	
protected:
	Reader *reader; ///<Pointer to an examplar of Reader class which is used for reading the file and taking interesting information out of the file header (if present).
};

#endif 
