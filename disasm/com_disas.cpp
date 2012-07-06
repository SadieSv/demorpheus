#include <map>
#include <iterator>
#include <string.h>
#include <vector>

#include "libdasm.h"
#include "com_disas.h"

#define PRINT_OPERAND( op ) \
	switch ( flow->fl[i][j].op.type) { \
	case OPERAND_TYPE_REGISTER: std::cerr<< " register = " << flow->fl[i][j].op.reg << std::endl; break;\
        case OPERAND_TYPE_IMMEDIATE: std::cerr<< " immediate = " << flow->fl[i][j].op.immediate << std::endl; break;\
        default: std::cerr<< "no op " << std::endl; break; }




#ifndef LIBDASM
struct disassemble_info disasm_info;
void init() {
    memset (&disasm_info, 0, sizeof (disasm_info));
    disasm_info.flavour = bfd_target_unknown_flavour;
    disasm_info.arch = bfd_arch_unknown;
    disasm_info.octets_per_byte = 1;
    //This is 32-bit address-mode. Please use disasm_info.mach = bfd_mach_x86_64 "instead" for 64-bit address-mode
    disasm_info.mach = bfd_mach_i386_i386;
}
#endif

void disassemble_flow( unsigned char* buffer, unsigned length, int off,  Flow* flow )
{
	#ifdef LIBDASM
	unsigned char ins_str[MAX_INSTRUCTION_LENGTH];
	INSTRUCTION inst;
	#ifdef DEBUG
	char string[256];
	#endif
	#else
	disasm_info.buffer =(bfd_byte *) buffer;
    	disasm_info.buffer_vma = (bfd_vma) buffer;
    	disasm_info.buffer_length = length;
	#endif
	std::vector< instruct > tmp_chain;
	int ins_len, ret;
	int len = length;
	instruct ins;
	int o = 0;

	while ( len > 0 )
	{
		//if we didn't dissasemble from such offset yet
		if ( flow->offsets[off+o].first == -1 )
		{
			#ifdef LIBDASM
			ins_len = get_instruction(&inst, buffer+o, MODE);
			#else
			ret = print_insn((bfd_vma)(buffer + o), &disasm_info);
			ins_len = (ret >> MOD); 
			#endif


			// filling instruction fields
			if ( ins_len<=0 )
			{
				return;			
			}
			ins.size = ins_len;
			#ifdef LIBDASM
			ins.type = inst.type;
			#else
			ins.type = (ret & ((1<<MOD) - 1));
			#endif
			ins.offset = off+o;
			ins.ext_off = false;
			
			#ifdef LIBDASM		
			ins.op1.type = inst.op1.type;
			ins.op2.type = inst.op2.type;
			ins.op1.reg = inst.op1.reg;
			ins.op2.reg = inst.op2.reg;
			ins.op1.immediate = inst.op1.immediate;
			ins.op2.immediate = inst.op2.immediate;
			#endif
		
			int off_tmp = flow->fl.size()-1; // the last chain position in the flow

			//if it's first instruction in the given buffer, create new chain in the flow
			//otherwise insert instruction in the end of last chain
			if ( o == 0 )
			{
				tmp_chain.clear();
				tmp_chain.push_back( ins );
				flow->fl.push_back( tmp_chain );
				flow->offsets[off+o].first = off_tmp+1; 
				flow->offsets[off+o].second = flow->fl[ off_tmp+1 ].size()-1;
			}
			else
			{
				flow->fl[ off_tmp ].push_back( ins );
				flow->offsets[off+o].first = off_tmp;
				flow->offsets[ off+o].second = flow->fl[off_tmp].size()-1;
			}

			len -= ins_len;
			o += ins_len;
		}
		//if we've already met instruction with given offset create reference element to other 
		// instruction in the other chain ( position of target instruction stored in the offsets vector)
		else
		{
			ins.offset = off+o;
			ins.ext_off = true;
			flow->ext_offset[off+o] = true;
			flow->fl[ flow->fl.size() -1 ].push_back( ins );
			return;
		}
			
	}
	return ;
};

void make_flow(unsigned char* buffer, unsigned length, Flow* flow )
{
	int o = 0;

	while( length > 0 )
	{
		//if we didn't disassemble from such offset yet
		if( flow->offsets[o].first == -1 )
			disassemble_flow( buffer+o, length, o, flow );
		o += 1;
		length -= 1;
	}
	return;
}

void print_flow_info(Flow* flow)
{
	for(unsigned int i=0; i< flow->fl.size(); i++ )
	{
		for(unsigned int j=0; j< flow->fl[i].size(); j++ )
		{
			if ( flow->fl[i][j].ext_off )
				continue;
			#ifdef LIBDASM
			#ifdef DEBUG
			PRINT_OPERAND( op1 );
			PRINT_OPERAND( op2 );
			#endif 
			#endif
		}
	}

};
