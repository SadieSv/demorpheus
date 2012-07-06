#include <iostream>
#include <cstdlib>
#include <cstring>
#include "finddecryptor.h"

/** @mainpage Description
This program is implementation of algorythm proposed by Qinghua Zhang, Douglas S. Reeves, Peng Ning, S. Purushothaman Iyer in the article "Analyzing Network Traffic To Detect Self-Decrypting Exploit Code".
They describe a method for detecting self-decrypting exploit codes. This method scans network traffic for the presence of a decryption routine, which is characteristic of such exploits. The proposed method uses static analysis and emulated instruction execution techniques. This improves the accuracy of determining the starting location and instructions of the decryption routine, even if self-modifying code is used. 
*/

/** 
 Function, running application.
 Makes an example of FindDecryptor class and uses it for finding necessary comand sequences.
 @param argc Parameter of command string. Definition not specified.
 @param argv One parameter - name of the input file.
 */
int main(int argc, char** argv)
{
	int finderType = 0, emulatorType = 1;
	switch (argc) {
		case 2:
			break;
		case 3:
			if (strcmp(argv[2],"GdbWine") == 0) {
				emulatorType = 0;
			} else if (strcmp(argv[2],"LibEmu") == 0) {
				emulatorType = 1;
			} else if (strcmp(argv[2],"Qemu") == 0) {
				emulatorType = 2;
			} else if (strcmp(argv[2],"GetPC") == 0) {
				finderType = 1;
			} else if (strcmp(argv[2],"FLibEmu") == 0) {
				finderType = 2;
			} else {
				cerr << "Unsupported argument." << endl;
				return 0;
			}
			break;
		default:
			cerr << "Wrong usage." << endl;
			return 0;
	}
	FindDecryptor find_decryptor(finderType, emulatorType);
	find_decryptor.load(argv[1], true);
	if (find_decryptor.find()) {
		cout << "Shellcode found!" << endl;
	}
	return 0;
}
