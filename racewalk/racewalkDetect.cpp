#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include "i386-dismod.h"
#include "bfd.h"
#include "racewalk.h"
#include <setjmp.h>

#define MAX_LEN 256

void printHelp()
{
	std::cerr<< "usage: ./racewalkDetect <path_to file> MODE <nop_length> \n"
		<< "MODE: \n\tfn stands for nop files\n\tfp stands for normal files"
		<< std::endl;

	return;
}


int main ( int argc, char *argv[])
{
	u_int res;
	bool fp = false;
	u_int slen;
	//unsigned char buffer[MAX_LEN];
	u_int total_cnt = 0;
	u_int cnt = 0;
	u_int ncnt = 0;
	const char* file_name, *flag;
	FILE* pFile;

	if ( argc < 4 )
        {
		printHelp();
                return 1;
        }

        file_name = argv[1];
        flag = argv[2];
        if ( flag[1] == 'p' )
        {
                fp = true;
        }
	slen = atoi( argv[3] );


	pFile = fopen(file_name, "rb");
        if ( pFile == NULL )
        {
                std::cerr<< "cannot open file with NOPs" << std::endl;
                return 1;
        }

	unsigned char buffer[slen];

	racewalk_init(slen);

	while( !feof( pFile ) )
	{
		cnt = fread( buffer, sizeof(unsigned char), slen*sizeof(unsigned char) , pFile );
		if ( cnt == 0 || cnt < slen ) continue;
		total_cnt++;		
		u_int l = sizeof(buffer);

		res = racewalk_simple_find_sled(buffer,  l);
		if ( res != l ) ncnt++;
		
		
	}	

	std::cerr<< total_cnt << " traces have been analyzed\n" <<
		ncnt << " nops have been found\n";

	double rate;
	if ( fp )
	{
		rate  = (double)ncnt/(double)total_cnt;
		std::cerr<< "false positive rate = ";
	}
	else
	{
		rate = (double)(total_cnt - ncnt)/(double)total_cnt;
		std::cerr<<"false negative rate = ";
	}
	std::cerr << rate << std::endl;
	
	return 0;
}
