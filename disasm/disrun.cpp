#include "i386-dismod.h"
#include "dis-asm.h"

unsigned char buf[] ="\x0f\x7a\xd8\x1d\xb2\x48\xb6\x79\x73\x2c\x48\xA1\x00\x00\x00\x00\x00\x00\x00\x80";
unsigned char buf1[] ="\x79\x4b\x92\xb9\x8d\xb1\x35\xa9\x97\x1c\x6b\xf9\xb3\xa8\x4e";

struct disassemble_info disasm_info;
void init() {
    memset (&disasm_info, 0, sizeof (disasm_info));
    disasm_info.flavour = bfd_target_unknown_flavour;
    disasm_info.arch = bfd_arch_unknown;
    disasm_info.octets_per_byte = 1;
    //This is 32-bit address-mode. Please use disasm_info.mach = bfd_mach_x86_64 "instead" for 64-bit address-mode
    disasm_info.mach = bfd_mach_i386_i386;
//    disasm_info.mach = bfd_mach_x86_64;
}

void disassemble(unsigned char *buf, int length){
    disasm_info.buffer =(bfd_byte *) buf;
    disasm_info.buffer_vma = (bfd_vma) buf;
    disasm_info.buffer_length = length;

    u_int i = 0;
    while( i < length){
        /* The modified print_insn version return as follows:
        a/ Return < 0 : if out of range or internal error
        b/ Return (instruction_size<<MOD) + instruction_type where
            MOD is defined in i386-dismod.h*/

        int ret = print_insn((bfd_vma)(buf + i), &disasm_info);
        //Out of range or internal error
        if( ret < 0){
            printf("\n\n");
            return;
        }


        int insn_size = (ret>>MOD);
        int insn_type = (ret & ((1<<MOD) - 1));
        printf("code: ");
        for(int k = 0; k < insn_size; k++){
            printf("\\x%x", *(buf +i + k));
        }
        printf("    size:%d     type:%d\n", insn_size, insn_type);
        i += insn_size;
    }



}


int main(){
    //Call init only 1 time
	init();

	disassemble(buf, sizeof(buf));
	disassemble(buf1, sizeof(buf1));
	return 0;
}

