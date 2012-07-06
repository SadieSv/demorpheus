#ifndef _MACROS_H_
#define _MACROS_H_

#include <iostream>
#include <sstream>
#include <sys/time.h>

#define DEBUG 0
#define ERROR 1
#define LOG 1

using namespace std;


#define PRINT_ERROR(X)  cout << " [Demorpheus error message] " << X << endl; abort();
#define PRINT_WARNING(X) cout << " [Demorpheus warning message] " << X << endl;
#define PRINT_DEBUG if(DEBUG) cout << " [Demorpheus] "

#endif