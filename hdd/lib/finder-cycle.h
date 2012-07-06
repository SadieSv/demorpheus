#ifndef FINDER_CYCLE_H
#define FINDER_CYCLE_H

#include <map>
#include <cstring>
#include <set>

#include "finder.h" 

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class FinderCycle : public Finder {
public:
	/**
	@param type Type of the emulator. Possible values: 0(GdbWine), 1(LibEmu).
	*/
	FinderCycle(int type=0);
	/**
	Destructor of class FinderCycle.
	*/
	~FinderCycle();
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	int find();
protected:
	/**
	Finds instructions writing to memory and indirect jumps (via disassembling sequence of bytes starting from pos).
	@param pos Position in binary file from which to start finding (number of byte).
	*/
	void find_memory_and_jump(int pos);
	/**
	Implements techniques of backwards traversal.
	Disassembles bytes in reverse order from pos. Founds the most appropriate chain using special rules (all the variables of target instruction should be defined within that chain) and prints it. 
	@param pos Starting point of the process.
	*/
	int backwards_traversal(int pos);
	/**
	Gets operands of given instruction (registers used in it).
	Saves this information in regs_target.
	@param inst Given instruction
	*/
	void get_operands(INSTRUCTION *inst);
	/**
	Checks every instruction in vector instructions. Changes regs_target and regs_known respectively.
	@param instructions Vector of instructions to be checked.
	*/
	void check(vector <INSTRUCTION>* instructions);
	/**
	Checks whether instruction defines one of the registers that need to be defined before the emulation and changes regs_target regs_known in corresponding way.
	@param inst Instruction to check.
	*/
	void check(INSTRUCTION *inst);
	/**
	  Adds new dependencies in regs_target (if any registers make influence on given operand).
	  @param op Operand of some instruction.
	*/
	void add_target(OPERAND *op);
	/**
	 @return Returns true if all dependencies are found and false vice versa.
	*/
	bool regs_closed();
	/**
	 Function which works with emulator. Makes emulator emulate found chain of instruction and looks for the loop. If neccessary restarts the process of finding dependencies and restarts emulator.
	 @return pos Position in input file from which emulation is started.
	*/
	void launch(int pos=0);
	/**
	  Checks the cycle found for the presence of decription routine.
	  @param cycle Cycle found (represents sequence of entities named Command).
	  @param size A number of lines in cycle.
	  @return Returns the number of line where indirect write happens.
	*/
	int verify(Command *cycle, int size);
	/**
	  Checks the cycle found for the presence of instructions changing register in target instruction.
	  @param inst The target instruction to check.
	  @param cycle Cycle found (represents sequence of entities named Command).
	  @param size A number of lines in cycle.
	  @return true if such instruction is found and false vice versa.
	  */
	bool verify_changing_reg(INSTRUCTION *inst, Command *cycle, int size);
	/**
	  Write registers to log.
	  */
	void dump_regs();

	vector <INSTRUCTION> instructions_after_getpc;///<instructions between seeding and target instruction
	int pos_getpc; ///<position of seeding instruction in the inputfile
	
	bool *regs_target; ///<registers to be defined (array which size is number of registers, regs_target[i]=true if register is to be defined and regs_target[i]=false vice versa)
	bool *regs_known; ///registers which are already defined (array which size is number of registers, regs_known[i]=true if register was defined and regs_target[i]=false vice versa)

	set<uint> start_positions;///<postions which were already checked
	set<uint> targets_found;///<positions where target instructions are alredy found
	static const uint maxBackward; ///<limit for backwards traversal
	static const uint maxEmulate; ///<limit for emulating
	static const uint maxForward; ///<limit for amount of instructions checked after GetPC to find target instruction
	int am_back; ///<amount of commands found by backwards traversal
	Command cycle[256]; // TODO: fix. It should be a member of the Finder::launch(). Here because of qemu lags.
};

#endif // FINDER_CYCLE_H