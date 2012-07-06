#pragma once

#include <iostream>
#include <map>
#include <iterator>
#include <string.h>
#include <vector>


#define  MODE MODE_32
#define  MAX_CHAIN_LEN 14
#define MAX_INSTRUCTION_LENGTH 16


#ifdef LIBDASM
#include "libdasm.h"
#else
#include "dis-asm.h"
#include "i386-dismod.h"
#endif

#include "flow.h"

static bool forceMode;

//for fast dissasemler purposes
void init();


void disassemble_flow( unsigned char* buffer, unsigned length, int off,  Flow* flow );
void make_flow( unsigned char* buffer, unsigned length, Flow* flow);
void print_flow_info(Flow* flow);



