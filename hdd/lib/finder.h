#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <libdasm.h>

#include "data.h"
#include "timer.h"
#include "emulator.h"
#include "reader_pe.h"

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class Finder : protected Data {
public:
	/**
	  Struct to keep information about instruction (its address and instruction itself)
	*/
	struct Command {
		Command(int a = 0, INSTRUCTION inst = INSTRUCTION());
		int addr;
		INSTRUCTION inst;
	};

	/**
	@param type Type of the emulator. Possible values: 0(GdbWine), 1(LibEmu).
	*/
	Finder(int type=0);
	/**
	Destructor of class Finder.
	*/
	virtual ~Finder();
	/**
	Loads a file.
	@param name Name of input file.
	@param guessType Try to guess binary type.
	*/
	void load(string name, bool guessType=false);
	/**
	Links to data.
	@param data Pointer to memory area.
	@param dataSize Size of memory area.
	@param guessType Try to guess binary type.
	*/
	void link(const unsigned char *data, uint dataSize, bool guessType=false);
	/**
	Applies a reader. Common part of load() and link() functions.
	@param reader Reader to apply.
	@param guessType Try to guess binary type.
	*/
	void apply_reader(Reader *reader, bool tryTypes=false);
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	virtual int find() = 0;
	int get_start_list(int max_size, int* list);
	list <int> get_start_list();
	int get_sizes_list(int max_size, int* list);
	list <int> get_sizes_list();
protected:
	/**
	  Translates registers from libdasm format to the neccessary format used here. 
	  @param code register in libdasm format (it means its number as it is a presented in enum format)
	*/
	int int_to_reg(int code);
	/**
	 @return Returns true if all dependencies are found and false vice versa.
	*/
	bool is_write(INSTRUCTION *inst);
	/**
	@param inst Given instruction.
	@return Returns true if given instruction rewrites at least one of its operands and used write is indirect.
	@sa is_write
	*/
	bool is_write_indirect(INSTRUCTION *inst);
	/**
	@param inst Given instruction.
	@param reg Pointer to register which is used in indirect addressing.
	@sa is_write_indirect
	@return Return value is the same as in is_write_indirect function. Additionally saves information about register in second parameter.
	*/
	bool get_write_indirect(INSTRUCTION *inst, int *reg);

	Reader *reader; ///<saves neccessary information about structure of input from its header 
	Emulator *emulator; ///<emulator used
	static const Mode mode; ///<mode of disassembling (here it is MODE_32)
	static const Format format; ///<format of commands (here it is Intel)
	list <int> pos_dec; ///<starting positions of found decryptors
	list <int> dec_sizes; ///<sizes of found decryptors

	/**
	  @param pos Position in input file from which we get instruction.
	  @param inst Pointer instruction the function gets.
	  @return Length of instruction.
	*/
	int instruction(INSTRUCTION *inst, int pos=0); 
	/**
	  @param pos Position in input file from which we get instruction.
	  @param inst Pointer to instruction the function gets.
	  @return String containing the instruction.
	*/
	string instruction_string(INSTRUCTION *inst, int pos=0);
	/**
	  @param pos Position in input file from which we get instruction.
	  @return String containing the instruction.
	*/
	string instruction_string(int pos);

	/** Debug **/
	/**
	Prints commands from vector of instructions v.
	@param start Position from which to print commands.
	*/
	void print_commands(vector <INSTRUCTION>* v, int start=0);
	ofstream *log;///<stream used to write all service information.
	/** /Debug **/
};

#endif