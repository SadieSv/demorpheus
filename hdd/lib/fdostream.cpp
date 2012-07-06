#include "fdostream.h"

using namespace std;

fdoutbuf::fdoutbuf(int _fd) : fd(_fd) {
}
fdoutbuf::int_type fdoutbuf::overflow(int_type c) {
	if (c != EOF) {
		char z = c;
		if (write (fd, &z, 1) != 1) {
			return EOF;
		}
	}
	return c;
}
streamsize fdoutbuf::xsputn(const char* s, streamsize num) {
	return write(fd,s,num);
}

fdostream::fdostream(int fd) : ostream(0), buf(fd) {
	rdbuf(&buf);
}