/* opcode/i386.h -- Intel 80386 opcode macros
   Copyright 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler, and GDB, the GNU Debugger.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/* The SystemV/386 SVR3.2 assembler, and probably all AT&T derived
   ix86 Unix assemblers, generate floating point instructions with
   reversed source and destination registers in certain cases.
   Unfortunately, gcc and possibly many other programs use this
   reversed syntax, so we're stuck with it.

   eg. `fsub %st(3),%st' results in st = st - st(3) as expected, but
   `fsub %st,%st(3)' results in st(3) = st - st(3), rather than
   the expected st(3) = st(3) - st

   This happens with all the non-commutative arithmetic floating point
   operations with two register operands, where the source register is
   %st, and destination register is %st(i).

   The affected opcode map is dceX, dcfX, deeX, defX.  */
#ifndef I386_DIS_H_
#define I386_DIS_H_
#ifdef __cplusplus
    extern "C" {
#endif
#include <sys/types.h>
#include <string.h>
#include "dis-asm.h"
//#include "sysdep.h"
#include "opintl.h"
#include "inst_types.h"

#ifndef SYSV386_COMPAT
/* Set non-zero for broken, compatible instructions.  Set to zero for
   non-broken opcodes at your peril.  gcc generates SystemV/386
   compatible instructions.  */
#define SYSV386_COMPAT 1
#endif
#ifndef OLDGCC_COMPAT
/* Set non-zero to cater for old (<= 2.8.1) versions of gcc that could
   generate nonsense fsubp, fsubrp, fdivp and fdivrp with operands
   reversed.  */
#define OLDGCC_COMPAT SYSV386_COMPAT
#endif

#define MOV_AX_DISP32 0xa0
#define POP_SEG_SHORT 0x07
#define JUMP_PC_RELATIVE 0xeb
#define INT_OPCODE  0xcd
#define INT3_OPCODE 0xcc
/* The opcode for the fwait instruction, which disassembler treats as a
   prefix when it can.  */
#define FWAIT_OPCODE 0x9b
#define ADDR_PREFIX_OPCODE 0x67
#define DATA_PREFIX_OPCODE 0x66
#define LOCK_PREFIX_OPCODE 0xf0
#define CS_PREFIX_OPCODE 0x2e
#define DS_PREFIX_OPCODE 0x3e
#define ES_PREFIX_OPCODE 0x26
#define FS_PREFIX_OPCODE 0x64
#define GS_PREFIX_OPCODE 0x65
#define SS_PREFIX_OPCODE 0x36
#define REPNE_PREFIX_OPCODE 0xf2
#define REPE_PREFIX_OPCODE  0xf3

#define TWO_BYTE_OPCODE_ESCAPE 0x0f
#define NOP_OPCODE (char) 0x90

/* register numbers */
#define EBP_REG_NUM 5
#define ESP_REG_NUM 4

/* modrm_byte.regmem for twobyte escape */
#define ESCAPE_TO_TWO_BYTE_ADDRESSING ESP_REG_NUM
/* index_base_byte.index for no index register addressing */
#define NO_INDEX_REGISTER ESP_REG_NUM
/* index_base_byte.base for no base register addressing */
#define NO_BASE_REGISTER EBP_REG_NUM
#define NO_BASE_REGISTER_16 6

/* modrm.mode = REGMEM_FIELD_HAS_REG when a register is in there */
#define REGMEM_FIELD_HAS_REG 0x3/* always = 0x3 */
#define REGMEM_FIELD_HAS_MEM (~REGMEM_FIELD_HAS_REG)

/* x86-64 extension prefix.  */
#define REX_OPCODE	0x40

/* Indicates 64 bit operand size.  */
#define REX_W	8
/* High extension to reg field of modrm byte.  */
#define REX_R	4
/* High extension to SIB index field.  */
#define REX_X	2
/* High extension to base field of modrm or SIB, or reg field of opcode.  */
#define REX_B	1

/* max operands per insn */
#define MAX_OPERANDS 4

/* max immediates per insn (lcall, ljmp, insertq, extrq) */
#define MAX_IMMEDIATE_OPERANDS 2

/* max memory refs per insn (string ops) */
#define MAX_MEMORY_OPERANDS 2

/* max size of insn mnemonics.  */
#define MAX_MNEM_SIZE 16

/* max size of register name in insn mnemonics.  */
#define MAX_REG_NAME_SIZE 8

#define MOD 16
#define DIS_EINTERNAL -1
#define DIS_EINVL -2
#define DIS_ERANGE -3
#define DIS_EREPEATPREF -4

int print_insn(bfd_vma, disassemble_info *);

/* Copy "only defined" name from libdasm */
#define NUM_INSTRUCTION_TYPES 109
/*enum {
    INSTRUCTION_TYPE_INVL, //invalid instruction
    INSTRUCTION_TYPE_ASC,	// aaa, aam, etc.
    INSTRUCTION_TYPE_DCL,	// daa, das
    INSTRUCTION_TYPE_MOV,
    INSTRUCTION_TYPE_MOVSR,	// segment register
    INSTRUCTION_TYPE_ADD,
    INSTRUCTION_TYPE_XADD,
    INSTRUCTION_TYPE_ADC,
    INSTRUCTION_TYPE_SUB,
    INSTRUCTION_TYPE_SBB,
    INSTRUCTION_TYPE_INC,
    INSTRUCTION_TYPE_DEC,
    INSTRUCTION_TYPE_DIV,
    INSTRUCTION_TYPE_IDIV,
    INSTRUCTION_TYPE_NOT,
    INSTRUCTION_TYPE_NEG,
    INSTRUCTION_TYPE_STOS,
    INSTRUCTION_TYPE_LODS,
    INSTRUCTION_TYPE_SCAS,
    INSTRUCTION_TYPE_MOVS,
    INSTRUCTION_TYPE_MOVSX,
    INSTRUCTION_TYPE_MOVZX,
    INSTRUCTION_TYPE_CMPS,
    INSTRUCTION_TYPE_SHX,	// signed/unsigned shift left/right
    INSTRUCTION_TYPE_ROX,	// signed/unsigned rot left/right
    INSTRUCTION_TYPE_MUL,
    INSTRUCTION_TYPE_IMUL,
    INSTRUCTION_TYPE_EIMUL, // "extended" imul with 2-3 operands
    INSTRUCTION_TYPE_XOR,
    INSTRUCTION_TYPE_LEA,
    INSTRUCTION_TYPE_XCHG,
    INSTRUCTION_TYPE_CMP,
    INSTRUCTION_TYPE_TEST,
    INSTRUCTION_TYPE_PUSH,
    INSTRUCTION_TYPE_AND,
    INSTRUCTION_TYPE_OR,
    INSTRUCTION_TYPE_POP,
    INSTRUCTION_TYPE_JMP,
    INSTRUCTION_TYPE_JMPC,	// conditional jump
    INSTRUCTION_TYPE_JECXZ,
    INSTRUCTION_TYPE_SETC,	// conditional byte set
    INSTRUCTION_TYPE_MOVC,	// conditional mov
    INSTRUCTION_TYPE_LOOP,
    INSTRUCTION_TYPE_CALL,
    INSTRUCTION_TYPE_RET,
    INSTRUCTION_TYPE_ENTER,
    INSTRUCTION_TYPE_INT,	// interrupt
    INSTRUCTION_TYPE_BT,	// bit tests
    INSTRUCTION_TYPE_BTS,
    INSTRUCTION_TYPE_BTR,
    INSTRUCTION_TYPE_BTC,
    INSTRUCTION_TYPE_BSF,
    INSTRUCTION_TYPE_BSR,
    INSTRUCTION_TYPE_BSWAP,
    INSTRUCTION_TYPE_SGDT,
    INSTRUCTION_TYPE_SIDT,
    INSTRUCTION_TYPE_SLDT,
    INSTRUCTION_TYPE_LFP,
    INSTRUCTION_TYPE_CLD,
    INSTRUCTION_TYPE_STD,
    INSTRUCTION_TYPE_XLAT,
    INSTRUCTION_TYPE_FCMOVC, // float conditional mov
    INSTRUCTION_TYPE_FADD,
    INSTRUCTION_TYPE_FADDP,
    INSTRUCTION_TYPE_FIADD,
    INSTRUCTION_TYPE_FSUB,
    INSTRUCTION_TYPE_FSUBP,
    INSTRUCTION_TYPE_FISUB,
    INSTRUCTION_TYPE_FSUBR,
    INSTRUCTION_TYPE_FSUBRP,
    INSTRUCTION_TYPE_FISUBR,
    INSTRUCTION_TYPE_FMUL,
    INSTRUCTION_TYPE_FMULP,
    INSTRUCTION_TYPE_FIMUL,
    INSTRUCTION_TYPE_FDIV,
    INSTRUCTION_TYPE_FDIVP,
    INSTRUCTION_TYPE_FDIVR,
    INSTRUCTION_TYPE_FDIVRP,
    INSTRUCTION_TYPE_FIDIV,
    INSTRUCTION_TYPE_FIDIVR,
    INSTRUCTION_TYPE_FCOM,
    INSTRUCTION_TYPE_FCOMP,
    INSTRUCTION_TYPE_FCOMPP,
    INSTRUCTION_TYPE_FCOMI,
    INSTRUCTION_TYPE_FCOMIP,
    INSTRUCTION_TYPE_FUCOM,
    INSTRUCTION_TYPE_FUCOMP,
    INSTRUCTION_TYPE_FUCOMPP,
    INSTRUCTION_TYPE_FUCOMI,
    INSTRUCTION_TYPE_FUCOMIP,
    INSTRUCTION_TYPE_FST,
    INSTRUCTION_TYPE_FSTP,
    INSTRUCTION_TYPE_FIST,
    INSTRUCTION_TYPE_FISTP,
    INSTRUCTION_TYPE_FISTTP,
    INSTRUCTION_TYPE_FLD,
    INSTRUCTION_TYPE_FILD,
    INSTRUCTION_TYPE_FICOM,
    INSTRUCTION_TYPE_FICOMP,
    INSTRUCTION_TYPE_FFREE,
    INSTRUCTION_TYPE_FFREEP,
    INSTRUCTION_TYPE_FXCH,
    INSTRUCTION_TYPE_SYSENTER,
    INSTRUCTION_TYPE_FPU_CTRL, // FPU control instruction
    INSTRUCTION_TYPE_FPU,	// Other FPU instructions
    INSTRUCTION_TYPE_MMX,	// Other MMX instructions
    INSTRUCTION_TYPE_SSE,	// Other SSE instructions
    INSTRUCTION_TYPE_OTHER,	// Other instructions :-)
    INSTRUCTION_TYPE_PRIV,	// Privileged instruction
};*/

#ifdef __cplusplus
}
#endif

#endif



