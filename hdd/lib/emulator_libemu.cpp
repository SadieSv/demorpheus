#include "emulator_libemu.h"

#include <fstream>
#include <algorithm>

extern "C" {
    #include <emu/emu.h>
    #include <emu/emu_cpu.h>
    #include <emu/emu_memory.h>
}

using namespace std;

const int Emulator_LibEmu::mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int Emulator_LibEmu::mem_after = 80*1024; //80 KiB, min 8k instructions

Emulator_LibEmu::Emulator_LibEmu() {
	//ofstream log("../log/libemu.txt");
	//log.close();
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
}
Emulator_LibEmu::~Emulator_LibEmu() {
	emu_free(e);
}
void Emulator_LibEmu::begin(uint pos) {
	if (pos==0) {
		pos = reader->start();
	}
	offset = reader->map(pos) - pos;
	uint start = max((int) reader->start(), (int) pos - mem_before), end = min(reader->size(), pos + mem_after);

	for (int i=0; i<8; i++) {
		emu_cpu_reg32_set(cpu, (emu_reg32) i, 0);
	}
	emu_cpu_reg32_set(cpu, esp, 0x1000000);

	emu_memory_clear(mem);
	//emu_memory_write_block(mem, offset + reader->start(), reader->pointer(true), reader->size(true));
	emu_memory_write_block(mem, offset + start, reader->pointer() + start, end - start);

	jump(pos);
}
void Emulator_LibEmu::jump(uint pos) {
	emu_cpu_eip_set(cpu, offset + pos);
}
/*bool Emulator_LibEmu::step() {
	ofstream log("../log/libemu.txt",ios_base::out|ios_base::app);
	bool ok = true;
	if (emu_cpu_parse(cpu) != 0) {
		ok = false;
	}
	log << "Command: " << cpu->instr_string << endl;
	if (ok && (emu_cpu_step(cpu) != 0)) {
		ok = false;
	}
	if (!ok) {
		log << "ERROR: " << emu_strerror(cpu->emu) << endl;
	}
	log.close();
	return ok;
}*/
bool Emulator_LibEmu::step() {
	if (emu_cpu_parse(cpu) != 0) {
		return false;
	}
	if (emu_cpu_step(cpu) != 0) {
		return false;
	}
	return true;
}
bool Emulator_LibEmu::get_command(char *buff, uint size) {
	return get_memory(buff, emu_cpu_eip_get(cpu), size);
}
bool Emulator_LibEmu::get_memory(char *buff, int addr, uint size)
{
	emu_memory_read_block(mem, addr, buff, size);
	return true;
}
unsigned int Emulator_LibEmu::get_int(int addr, int size)
{
	uint8_t memb = 0;
	uint16_t memw = 0;
	uint32_t memd = 0;
	switch (size)
	{
		case 1:
			emu_memory_read_byte(mem, addr, &memb);
			return memb;
		case 2:
			emu_memory_read_word(mem, addr, &memw);
			return memw;
		case 4:
			emu_memory_read_dword(mem, addr, &memd);
			return memd;
		default:;
	}
	return 0;
}
unsigned int Emulator_LibEmu::get_register(Register reg) {
	switch (reg) {
		case EAX:
			return emu_cpu_reg32_get(cpu, eax);
		case EBX:
			return emu_cpu_reg32_get(cpu, ebx);
		case ECX:
			return emu_cpu_reg32_get(cpu, ecx);
		case EDX:
			return emu_cpu_reg32_get(cpu, edx);
		case ESI:
			return emu_cpu_reg32_get(cpu, esi);
		case EDI:
			return emu_cpu_reg32_get(cpu, edi);
		case ESP:
			return emu_cpu_reg32_get(cpu, esp);
		case EBP:
			return emu_cpu_reg32_get(cpu, ebp);
		case EIP:
			return emu_cpu_eip_get(cpu);
		default:;
	}
	return 0;
}
