#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <string.h>

#include "flow.h"
#include "inst_types.h"
#include "libdasm.h"

#define MAX_INSTRUCTION_LENGTH 16
#define  MODE MODE_32

void Flow::init()
{
		for( int i=0; i < MAX_DATA_LEN; i++){
			offsets[i].first = -1;
			ext_offset[i] = false;
		}
}

Flow::Flow()
{
	init();
}
Flow::Flow( const unsigned char * buffer, int length )
{
	int o = 0;
	
	init();
	
	while ( length > 0 ){
		//if we didn't disassemble from such offset yet
                if( offsets[o].first == -1 )
					disassemble_flow( buffer+o, length, o );
				o += 1;
                length -= 1;

	}
	return;
}

void Flow::disassemble_flow( const unsigned char * buffer, unsigned length, int off )
{
	unsigned char ins_str[MAX_INSTRUCTION_LENGTH];
    INSTRUCTION inst;
	int o = 0;
	std::vector< instruct > tmp_chain;
	instruct ins;

	while( length > 0 && off + o < MAX_DATA_LEN )
	{
		//if we didn't dissasemble from such offset yet
        if ( offsets[off+o].first == -1 ){
			int ins_len = get_instruction(&inst, const_cast<BYTE*>( buffer+o ), MODE);
			// incorrect dissasembling
			if ( ins_len<=0 ) return;
						
			//filling instruction field
			ins.size = ins_len;
			ins.type = inst.type;
			ins.offset = off+o;
			ins.ext_off = false;
			ins.op1.type = inst.op1.type;
			ins.op2.type = inst.op2.type;
			ins.op1.reg = inst.op1.reg;
			ins.op2.reg = inst.op2.reg;
			ins.op1.immediate = inst.op1.immediate;
			ins.op2.immediate = inst.op2.immediate;
			
			int off_tmp = fl.size()-1; // the last chain position in the flow

			//if it's first instruction in the given buffer, create new chain in the flow
			//otherwise insert instruction in the end of last chain
			if ( o == 0 ){
				tmp_chain.clear();
				tmp_chain.push_back( ins );
				fl.push_back( tmp_chain );
				offsets[off+o].first = off_tmp+1;
				offsets[off+o].second = fl[ off_tmp+1 ].size()-1;
			}
			else {
				fl[ off_tmp ].push_back( ins );
				offsets[off+o].first = off_tmp;
				offsets[ off+o].second = fl[off_tmp].size()-1;
			}
			length -= ins_len;
			o += ins_len;
        }
		//if we've already met instruction with given offset then create reference element to other 
        // instruction in the other chain ( position of target instruction stored in the offsets vector)
        else {
			ins.offset = off+o;
            ins.ext_off = true;
            ext_offset[off+o] = true;
            fl[ fl.size() -1 ].push_back( ins );
            return;
        }
	}
    return ;

}
