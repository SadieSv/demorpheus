#ifndef RACEWALK_H_
#define RACEWALK_H_
//#include <sys/wait.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pcap.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "i386-dismod.h"

#include "svm.h"

#define MAX_INSTR_SIZE 16
#define LABEL_SLED 1
#define LABEL_NOTSLED - 1
#define MAX_LENGTH 1024
#define MAX_SIZE_BUF 100000


/**
 * This is "outside" improvement. With this improvment, we never decode an instruction twice
 * in the "whole" data. It means that the number of decoding instruction is racewalk of
 * "whole" data size. It does not depend on SLED_LENGTH anymore. However, the overall algorithm
 * still depends SLED_LENGTH. But decoding instruction is time consuming , therefore this improvement
 * can significantly improve running time. We're still working to reduce the complexity
 * of overall algorithm.
 * Suppose that in the previous round , we check sled in the following array:
 *      buf, buf + 1, buf + 2, ..., buf + SLED_LENGTH - 1.
 * In this round the array that we need to check for sled is:
 *      buf + 1, buf + 2, ... , buf + SLED_LENGTH
 * We observe that there is a overlaped segment between two rounds [buf + 1, buf + 2, ... , buf + SLED_LENGTH - 1].
 * Therefore, we can cache which position have been decoded from the previous round.
 * Let's consider the following array:
 *		int decode_cached[SLED_LENGTH];
 * decode_cached[i] can receive one of the 3 value
 *      -1 decoded but invalid
 *      0 not decoded
 *      x : 0 <x < JUMP  : the instruction size (decoded successfully), not jump instruction
 *      x : x >= JUMP : decoded successfully, jump instruction
 * Now, we will consider how to update array "decode_cached" from previous round to current round.
 * Note that the "buf + i" in the previous round is corresponding to "buf + i + 1" in current round. Therefore, the
 * value of decode_cached[i] in the previous round is decode_cached[i - 1], except for decode_cached[0] in previous
 * round is not valid in this current round anymore(the reason is "buf + 0" does not appear in current round anymore).
 */
#define NOT_DECODED		0
#define DECODED_INVALID -1
#define JMP			16

static int decode_cache[MAX_LENGTH];

/**
 * This is "inside" improvement.With this improvement, we never run scanning from
 * position i to the end of candiate sleds twice. It means that the complexity "inside" find_sled
 * function is linear of SLED_LENGTH.
 *  Array cached of size SLED_LENGTH, scan_cached[i] has one of three value
 *  	NOT_SCANNED: we have not scanned from position i.
 *  	SCANNED_VALID: we successully scan from i the the end.(SLED_LENGTH - i)
 *  	SCANNED_INVALID: we failed to scan from i to the end (SLED_LENGTH - i)
 *  Consider some position i each "find_sled" function(see STRIDE paper), we have following
 *	recursive relation:
 *		if instruction at position i is valid, not jmp, has size "length" then
 *			scan_cached[i] = scan_cached[i + length]
 * 		if instruction at position i is invalid scan_cached[i] = SCANNED_INVALID
 *		if instruction at position i is jmp instruction scan_cached[i] = SCANNED_VALID
 *  Array scan_cached is initialized NOT_SCANNED.
 *
 */
#define NOT_SCANNED		0
#define SCANNED_INVALID -1
#define SCANNED_VALID	2
static int scan_cache[MAX_LENGTH];
static u_int the_sled_length;
static disassemble_info disasm_info;

static char libsvm_path[MAX_LENGTH];
static char libsvm_trained_model[MAX_LENGTH];

static struct svm_model* model = NULL;
static char path_svm_scale[MAX_LENGTH];
static char path_svm_train[MAX_LENGTH];
static char path_svm_predict[MAX_LENGTH];
static double feature_min[NUM_INSTRUCTION_TYPES];
static double feature_max[NUM_INSTRUCTION_TYPES];
static double lower;
static double upper;
static double nu;
static double gamma_race;
#define MAX_VALUE 1e50

/* Initialize
    i/ Default disasemble_info which is used for decoding instruction.
    ii/ The sled length */
void racewalk_init(u_int);

/* Return true if valid sled otherwise false.
    Variable skip is just for optimization purpose */
bool racewalk_simple_check_sled(const u_char *, bool);

/* Return the position of sled in buf.
    If not found return length */
u_int racewalk_simple_find_sled(const u_char *, u_int32_t );

int racewalk_count_frequency(const u_char *, double *);

/* Load the svm model and scale file: used in detecting phase */
void racewalk_svm_load(const char *svm_model_file, const char *scale_file);

/* Configure nu,gamma_race ,lowerbound & upperbound parameter in one class svm . These parameter
    are used in learning phase to generate svm_model,  */
void racewalk_svm_config(double , double , double, double , char *svm_path);

/* Input is the binary file which contains only sled. After learning phase,
    it will create svm_model_file and scale_file which are used in detecting phase
    Return 0 if everything goes fine.
         -1 if the file contains only sled but racewalk does not detect them all.*/

void racewalk_learn(const char *file_learn);

/* Generate frequency file in the format of libsvm .
    If raw_file is nop then label = LABEL_SLED, otherwise LABEL_NOT_SLED*/

void racewalk_generate_freq(const char *raw_file, const char *freq_file, int label);

/* If already learn from learn_file then ignore learn_file, otherwise : learn from learn_file
    & test predicting with test_file which has test label*/
void racewalk_offline_exp(const char *learn_file, const char *test_file, int test_label);

/* Detect sled in network . It work until ctrl-c is pressed and print to stdout each time the NOP is
    detected: src IP:src port -> dst IP:dst port NOP (as string of hexadecimal) */
void racewalk_online_detect(const char *svm_model_file, const char *scale_file, char *filter_exp);



/* Detect sled with one-class svm(libsvm). It is considered sled iff
 i/ It is detected by racewalk_simple_find_sled(or racewalk_check_sled)
 ii/ The frequency of instructions is detected by one-class svm */
u_int racewalk_detect(u_char *, u_int);

static double insn_freq[NUM_INSTRUCTION_TYPES];

#endif

