#include "emulator.h"
#include <sys/types.h>

void Emulator::bind(Reader* r) 
{
	reader = r;
}
unsigned int Emulator::memory_offset()
{
	return 0;
}
unsigned int Emulator::get_int(int addr, int size)
{
	u_int8_t memb = 0;
	u_int16_t memw = 0;
	u_int32_t memd = 0;
	switch (size)
	{
		case 1:
			return get_memory((char *) &memb, addr, size) ? memb : 0;
		case 2:
			return get_memory((char *) &memw, addr, size) ? memw : 0;
		case 4:
			return get_memory((char *) &memd, addr, size) ? memd : 0;
		default:;
	}
	return 0;
}
