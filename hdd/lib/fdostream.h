#ifndef FDOSTREAM_H
#define FDOSTREAM_H

#include <iostream>
#include <streambuf>
#include <cstdio>
 
using namespace std;


class fdoutbuf : public streambuf {
	public:
		fdoutbuf (int _fd);
	protected:
		virtual int_type overflow(int_type c);
		virtual streamsize xsputn(const char* s, streamsize num);
	
		int fd;
};

class fdostream : public ostream {
	public:
		fdostream(int fd);
	protected:
		fdoutbuf buf;
};

#endif