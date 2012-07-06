#include "finder-getpc.h"

using namespace std;

#ifdef FINDER_LOG
	#define LOG (*log)
#else
	#define LOG if (false) cerr
#endif

const uint FinderGetPC::maxEmulate = 50;
const uint FinderGetPC::maxUpGetPC = 20;

FinderGetPC::FinderGetPC(int type) : Finder(type)
{
	Timer::start();
}

FinderGetPC::~FinderGetPC()
{
	Timer::stop();

	LOG	<< endl << endl
		<< "Time total: " << dec << Timer::secs() << " seconds." << endl
		<< "Time spent on load: " << dec << Timer::secs(TimeLoad) << " seconds." << endl
		<< "Time spent on find: " << dec << Timer::secs(TimeFind) << " seconds." << endl
		<< "Time spent on bruteforcing: " << dec << Timer::secs(TimeBackwardsTraversal) << " seconds." << endl
		<< "Time spent on launches: " << dec << Timer::secs(TimeLaunches) << " seconds." << endl
		<< "Time spent on emulator launches (total): " << dec << Timer::secs(TimeEmulatorStart) << " seconds." << endl;
}
int FinderGetPC::launch(int pos)
{
	if (start_positions.count(pos)) {
		LOG << "Already checked 0x" << hex << pos << ", not running." << endl;
		return -3;
	}
	Timer::start(TimeLaunches);
	LOG << "Launching from position 0x" << hex << pos << endl;
	int num;
	INSTRUCTION inst;
	Timer::start(TimeEmulatorStart);
	emulator->begin(pos);
	Timer::stop(TimeEmulatorStart);
	char buff[10] = {0};
	uint last_fpu_ip = 0, saved_eip = 0;
	bool eip_saved = false, fpu_inst = false;
	uint len;
	uint sum_len = 0; //total length of emulated instructions
	for (uint strnum = 0; strnum < maxEmulate; strnum++, sum_len += len) {
		if ((sum_len >= maxUpGetPC) && (!eip_saved)) {
			Timer::stop(TimeLaunches);
			return -2;
		}

		if (!emulator->get_command(buff)) {
			LOG << " Execution error, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return -1;
		}

		num = emulator->get_register(EIP);
		if (!reader->is_valid(num)) {
			LOG << " Reached end of the memory block, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return -1;
		}
		if (num > pos) {
			start_positions.insert(num);
		}
		len = get_instruction(&inst, (BYTE *) buff, mode);
		LOG << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
		if (!emulator->step()) {
			LOG << " Execution error, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return -1;
		}

		if (eip_saved) {
			//check for registers
			if (emulator->get_register(EAX)==saved_eip || emulator->get_register(EBX)==saved_eip || 
				emulator->get_register(ECX)==saved_eip || emulator->get_register(EDX)==saved_eip ||
				emulator->get_register(ESI)==saved_eip || emulator->get_register(EDI)==saved_eip ||
				emulator->get_register(ESP)==saved_eip || emulator->get_register(EBP)==saved_eip) 
			{
				pos_dec.push_back(pos);
				LOG << " Shellcode found." << endl;
				Timer::stop(TimeLaunches);
#ifdef FINDER_LOG
		for (uint j = 0; j < 40; j++) {
			if (!emulator->get_command(buff)) {
				LOG << "  (extra) Execution error." << endl;
				break;
			}
			num = emulator->get_register(EIP);
			if (!reader->is_valid(num)) {
				LOG << "  (extra) Reached end of the memory block." << endl;
				break;
			}
			len = get_instruction(&inst, (BYTE *) buff, mode);
			LOG << "  (extra) Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
			if (!emulator->step()) {
				LOG << "  (extra) Execution error." << endl;
				break;
			}
		}
#endif
#ifdef FINDER_ONCE
				cout << "Shellcode found." << endl;
				exit(0);
#endif
				return 0;
			}
		}

		// reached seeding instruction while emulation
		switch ((unsigned char)buff[0]) {
			/// fsave/fnsave: 0x9bdd, 0xdd
			case 0x9b:
				if ((unsigned char)buff[1] != 0xdd) {
					break;
				}
			case 0xdd:
				if (fpu_inst) {
					LOG << "   EIP saved." << endl;
					eip_saved = true;
					saved_eip = last_fpu_ip;
				}
				break;
			/// fstenv/fnstenv: 0xf2d9, 0xd9
			case 0xf2:
				if ((unsigned char)buff[1]  != 0xd9) {
					break;
				}
			case 0xd9:
				if (fpu_inst) {
					LOG << "   EIP saved." << endl;
					eip_saved = true;
					saved_eip = last_fpu_ip;
				}
				break;
			/// call: 0xe8, 0xff, 0x9a
			case 0xe8:
			case 0xff:
			case 0x9a:
				LOG << "   EIP saved." << endl;
				eip_saved = true;
				saved_eip = num + len;
				break;
			default:
				;
		}

		// FPU instruction
		if (MASK_EXT(inst.flags) == EXT_CP)
		{
			fpu_inst = true;
			last_fpu_ip = num;
			LOG << "   Last FPU IP : " << last_fpu_ip << endl;
		}
	}
	Timer::stop(TimeLaunches);
	return 0;
}

int FinderGetPC::find() {
	pos_dec.clear();
	Timer::start(TimeFind);
	INSTRUCTION inst;
	for (uint i=reader->start(); i<reader->size(); i++) {
		/// TODO: check opcodes
		switch (reader->pointer()[i]) {
			/// fsave/fnsave: 0x9bdd, 0xdd
			case 0x9b:
				if ((reader->pointer()[i+1]) != 0xdd) { /// TODO: check if i+1 is present
					continue;
				}
			case 0xdd:
				break;
			/// fstenv/fnstenv: 0xf2d9, 0xd9
			case 0xf2:
				if ((reader->pointer()[i+1]) != 0xd9) { /// TODO: check if i+1 is present
					continue;
				}
			case 0xd9:
				break;
			/// call: 0xe8, 0xff, 0x9a
			case 0xe8:
			case 0xff:
			case 0x9a:
				break;
			default:
				continue;
		}
		uint len = instruction(&inst, i);
		if (!len || (len + i > reader->size())) {
			continue;
		}
		switch (inst.type) {
			case INSTRUCTION_TYPE_FPU_CTRL:
				if (	(strcmp(inst.ptr->mnemonic,"fstenv") == 0) ||
					(strcmp(inst.ptr->mnemonic,"fsave") == 0)) {
					LOG << "Seeding instruction \"" << instruction_string(i) << "\" on position 0x" << hex << i << "." << endl;
					find_dependence(i);
					break;
				}
				continue;
			case INSTRUCTION_TYPE_CALL:
				if (	(strcmp(inst.ptr->mnemonic,"call") == 0) &&
					(inst.op1.type == OPERAND_TYPE_IMMEDIATE)) {
					LOG << "Seeding instruction \"" << instruction_string(i) << "\" on position 0x" << hex << i << "." << endl;
					if ((i + len + inst.op1.immediate) < reader->size()) {
						launch(i);
					}
					break;
				}
				continue;
			default:
				continue;
		}
	}
	Timer::stop(TimeFind);
	return pos_dec.size();
}

void FinderGetPC::find_dependence(uint pos)
{
	Timer::start(TimeBackwardsTraversal);

	for (std::set<uint>::iterator it = start_positions.begin(); it != start_positions.end();) {
		std::set<uint>::iterator current = it++;
		if (*current < pos - maxUpGetPC) {
			start_positions.erase(current);
		}
	}
	
	INSTRUCTION inst;
	for (int i=pos-1; i >= (int) (pos - maxUpGetPC) && i >=0; i--) {
		instruction(&inst, i);
		if (MASK_EXT(inst.flags) != EXT_CP) {
			continue;
		}
		uint end = i;
		for (end += inst.length; end < pos; end += inst.length) {
			if (!instruction(&inst, end)) {
				break;
			}
		}
		if (end == pos) {
			if (launch(i) == -2) {
				Timer::stop(TimeBackwardsTraversal);
				return;
			}
		}
	}
	Timer::stop(TimeBackwardsTraversal);
}
