#ifndef READER_PE_H
#define READER_PE_H

#include <string>
#include "reader.h"

using namespace std;

/**
@brief
Class working with PE-header.
*/

class Reader_PE : public Reader
{
	/**
	  Struct held in a tableof sections.
	*/
	struct entry
	{
		char name[8];///<name of section
		uint virt_addr;///<virtual address of section
		uint virt_size;///<virtual size of section
		uint raw_offset;///<raw offset of section
		uint raw_size;///<raw size of section
		uint max_size;///<max of raw and virtual size
	};
public:
	Reader_PE();
	Reader_PE(const Reader *reader);
	~Reader_PE();
	uint entrance();
	uint map(uint addr);
	bool is_valid(uint addr);
	bool is_within_one_block(uint a, uint b);
	static bool is_of_type(const Reader *reader);
protected:
	/**
	 Gets necessary information from header.
	*/
	void parse();
private:
	/**
	  Sorts table of sections by raw_offset
	*/
	void sort();
	/**
	  Prints table of sections
	*/
	void print_table();

	/**
	@return Return integer formed of @ref size bytes from the position pos from buffer @ref buff.
	*/
	static uint get(const unsigned char *buff, int pos, int size=4);
	/**
	@return Return integer formed of @ref size bytes from the position pos.
	*/
	uint get(int pos, int size=4);

	uint number_of_sections;///<Number of sections in input file
	uint entry_point;///< Entry point of input file
	entry* table;///< Table of sections
};
#endif