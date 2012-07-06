//#define FINDER_LOG /// Write to logfile.
//#define FINDER_DUMP /// Dump passed data to disk
//#define FINDER_ONCE /// Stop after first found decryption routine.

#include "finder.h"
#ifdef BACKEND_GDBWINE
	#include "emulator_gdbwine.h"
#endif
#ifdef BACKEND_LIBEMU
	#include "emulator_libemu.h"
#endif
#ifdef BACKEND_QEMU
	#include "emulator_qemu.h"
#endif

using namespace std;

#ifdef FINDER_LOG
	#define LOG (*log)
#else
	#define LOG if (false) cerr
#endif

const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;

Finder::Command::Command(int a, INSTRUCTION i) {
	addr = a;
	inst = i;
}

Finder::Finder(int type)
{
	switch (type) {
#ifdef BACKEND_QEMU
		case 2:
			emulator = new Emulator_Qemu();
			break;
#endif
#ifdef BACKEND_LIBEMU
		case 1:
			emulator = new Emulator_LibEmu();
			break;
#endif
#ifdef BACKEND_GDBWINE
		case 0:
			emulator = new Emulator_GdbWine();
			break;
#endif
		case -1:
			emulator = NULL;
			break;
		default:
			cerr << "Unsupported emulation backend!" << endl;
			exit(0);
	}
	reader = NULL;
	log = NULL;
#ifdef FINDER_LOG
	log = new ofstream("../log/finder.txt");
#endif
	if (log) switch (type) {
		case 2:
			LOG << "### Using Qemu emulator. ###" << endl;
			break;
		case 1:
			LOG << "### Using LibEmu emulator. ###" << endl;
			break;
		case 0:
			LOG << "### Using GdbWine emulator. ###" << endl;
			break;
		default:
			LOG << "### Using unknown emulator. ###" << endl;
	}
}

Finder::~Finder()
{
	if (log) {
		log->close();
		delete log;
	}
	delete emulator;
	delete reader;
}
void Finder::load(string name, bool guessType) {
	Timer::start(TimeLoad);
	Reader *reader = new Reader();
	reader->load(name);
	LOG	<< endl << "Loaded file \'" << name << "\"."
		<< endl << "File size: 0x" << hex << reader->size() << "." << endl << endl;
	apply_reader(reader, guessType);
	Timer::stop(TimeLoad);
}
void Finder::link(const unsigned char *data, uint dataSize, bool guessType) {
	Timer::start(TimeLoad);
	Reader *reader = new Reader();
	reader->link(data, dataSize);
	LOG	<< endl << "Loaded data at 0x" << hex << (ulong) data << "."
		<< endl << "Data size: 0x" << hex << reader->size() << "." << endl << endl;
#ifdef FINDER_DUMP
	static int counter = 0;
	char *str = new char[50];
	sprintf(str,"../log/data%d.dat",counter++);
	ofstream datas(str);
	delete str;
	if (datas) {
		datas.write((const char *) data, dataSize);
		datas.close();
	}
#endif
	apply_reader(reader, guessType);
	Timer::stop(TimeLoad);
}
void Finder::apply_reader(Reader *reader, bool guessType) {
#ifdef TRY_READERS
	if (guessType) {
		if (Reader_PE::is_of_type(reader)) {
			reader = new Reader_PE(reader);
			LOG << "Looks like a PE file." << endl << endl;
		}
	}
#endif
	delete this->reader;
	this->reader = reader;
	if (emulator != NULL) {
		emulator->bind(reader);
	}
}


int Finder::instruction(INSTRUCTION *inst, int pos) {
	return get_instruction(inst, (BYTE*) (reader->pointer() + pos), mode);
}
string Finder::instruction_string(INSTRUCTION *inst, int pos) {
	if (!inst->ptr) {
		return "UNKNOWN";
	}
	char str[256];
	get_instruction_string(inst, format, (DWORD)pos, str, sizeof(str));
	return (string) str;
}
string Finder::instruction_string(int pos) {
	INSTRUCTION inst;
	instruction(&inst, pos);
	return instruction_string(&inst,pos);
}



void Finder::print_commands(vector <INSTRUCTION>* v, int start)
{
	int i=0;
	vector<INSTRUCTION>::iterator p;
	LOG << " Commands in queue:"<< endl;
	for (vector<INSTRUCTION>::iterator p=v->begin(); p!=v->end(); i++,p++) {
		if (i<start) {
			continue;
		}
		LOG << "  " << instruction_string(&(*p)) << endl;
	}
}

int Finder::int_to_reg(int code)
{
	switch (code) {
		case REG_EAX:
			return EAX;
		case REG_EBX:
			return EBX;
		case REG_ECX:
			return ECX;
		case REG_EDX:
			return EDX;
		case REG_EDI:
			return EDI;
		case REG_ESI:
			return ESI;
		case REG_ESP:
			return ESP;
		case REG_EBP:
			return EBP;
	}
	return -1;
}
bool Finder::get_write_indirect(INSTRUCTION *inst, int *reg)
{
	if (!is_write_indirect(inst)) {
		return false;
	}
	*reg = int_to_reg(inst->op1.basereg);
	return true;
}
bool Finder::is_write_indirect(INSTRUCTION *inst)
{
	if (inst->type == INSTRUCTION_TYPE_STOS) {
		return true;
	}
	if (inst->type == INSTRUCTION_TYPE_XCHG) {
		return	((inst->op1.type == OPERAND_TYPE_MEMORY) && (inst->op1.basereg != REG_NOP)) ||
			((inst->op2.type == OPERAND_TYPE_MEMORY) && (inst->op2.basereg != REG_NOP));
	}
	return is_write(inst) && (inst->op1.type == OPERAND_TYPE_MEMORY) && (inst->op1.basereg != REG_NOP);
}
bool Finder::is_write(INSTRUCTION *inst)
{
	switch (inst->type) {
		case INSTRUCTION_TYPE_XOR:
		case INSTRUCTION_TYPE_SUB:
		case INSTRUCTION_TYPE_SBB:
		case INSTRUCTION_TYPE_DIV:
		case INSTRUCTION_TYPE_IDIV:

		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:

		case INSTRUCTION_TYPE_INC:
		case INSTRUCTION_TYPE_DEC:
		
		case INSTRUCTION_TYPE_MOV:
		case INSTRUCTION_TYPE_POP:

		case INSTRUCTION_TYPE_XCHG:
			return true;
		default:
			return false;
	}
}

int Finder::get_start_list(int max_size, int* found_pos)
{
	/*return of found starting positions of decryptors*/
	list <int>:: iterator it;
	int i;
	for (it = pos_dec.begin(), i = 0; (it != pos_dec.end()) && (i<max_size); it++, i++)
		found_pos[i] = *it;

	return i;
}

int Finder::get_sizes_list(int max_size, int* found_pos)
{
	/*return of found starting positions of decryptors*/
	list <int>:: iterator it;
	int i;
	for (it = dec_sizes.begin(), i = 0; (it != dec_sizes.end()) && (i<max_size); it++, i++)
		found_pos[i] = *it;

	return i;
}

list <int> Finder::get_start_list()
{
	return pos_dec;
}

list <int> Finder::get_sizes_list()
{
	return dec_sizes;
}