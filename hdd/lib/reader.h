#ifndef READER_H
#define READER_H

#include <string>

using namespace std;

typedef unsigned int uint;

/**
@brief
Class for reading input.
*/

class Reader
{
public:
	Reader();
	Reader(const Reader *reader);
	~Reader();

	/**
	  Load file.
	  @param name Name of input file
	*/
	void load(string name);
	/**
	  Link to data.
	  @param data Pointer to memory area.
	  @param dataSize Size of memory area.
	*/
	void link(const unsigned char *data, uint dataSize);
	/**
	@return Name of the input file.
	*/
	string name();
	/**
	@return Pointer to a buffer helding an input file
	*/
	const unsigned char *pointer(bool nohead=false) const;
	/**
	@return Size of input file.
	*/
	uint size(bool nohead=false) const;
	/**
	  @return The position of the first instruction in file.
	*/
	uint start();
	/**
	  @return Entry point of the program.
	*/
	virtual uint entrance();
	/**
	  Translates address of instruction in input file into its address when program is loaded into memory.
	  @param addr Address the of instruction in input file.
	*/
	virtual uint map(uint addr);
	/**
	  Tells if the memory address if valid.
	  @param addr Address the of instruction in memory.
	*/
	virtual bool is_valid(uint addr);
	/**
	@param a First address.
	@param b Second address.
	@return Returns true if these adresses are within one section
	*/
	virtual bool is_within_one_block(uint a, uint b);
protected:
	/**
	Reads input binary file into buffer.
	*/
	void read();
	/**
	 Gets necessary information from header.
	*/
	virtual void parse();
	
	bool indirect; ///<do we need to delete data at the end?
	string filename; ///<input file name
	const unsigned char *data; ///<buffer containing binary file
	uint dataSize; ///<size of buffer data
	uint dataStart; ///<start of the actual data in buffer
	uint base;///< Base of addresses in memory
};
#endif