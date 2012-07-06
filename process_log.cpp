#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/time.h>

using namespace std; 

int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
	// Perform the carry for the later subtraction by updating y. 
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }
     
    // Compute the time remaining to wait. tv_usec is certainly positive.
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;
     
    // Return 1 if result is negative.
    return x->tv_sec < y->tv_sec;
}

int timeval_addition(struct timeval *result, struct timeval *x, struct timeval *y)
{
	result->tv_sec = x->tv_sec + y->tv_sec;
	result->tv_usec = x->tv_usec + y->tv_usec;
	if (result->tv_usec > 1000000){
		result->tv_sec++;
		result->tv_usec = result->tv_usec-1000000;
	}
	return 0;
}
//first parameter  - log file
//second - fp or fn
int main( int argc, char * argv[] )
{
	ifstream pFile;
	//processing program parameters
	if ( argc < 3 ){
		return 1;
	}
	const char* file_name = argv[1];
	pFile.open(file_name);
	if (!pFile){
		std::cerr<< "cannot open log file" << std::endl;
		return 1;
	}
	
	bool fp = false;
	bool fn = false;
	if(argv[2][1] =='p') fp=true;
	else if(argv[2][1] == 'n' ) fn = true;
	
	int total=0, total_tmp=0, s_cnt = 0, s_temp_cnt=0;
	struct timeval totalTime;
	long sec, usec;
	
	totalTime.tv_sec = 0;
	totalTime.tv_usec = 0;
	
	while(!pFile.eof())
	{
		pFile >> total_tmp;
		pFile >> s_temp_cnt;
		pFile >> sec;
		pFile >> usec;
		//std::cerr<< total_tmp <<"/"<<s_temp_cnt<< "processed in " << sec << ":" <<usec<< std::endl;
		total += total_tmp;
		s_cnt += s_temp_cnt;
		
		struct timeval diff;
		diff.tv_sec = sec;
		diff.tv_usec = usec;
		timeval_addition(&totalTime, &totalTime, &diff);
		
	}
	
	std::cerr<< "total: " << total << " shellcodes: " << s_cnt <<std::endl;
	std::cerr<< "Total time: " << totalTime.tv_sec << ":" << totalTime.tv_usec << std::endl;
	
	if (fp){
		double FP = (double)((double)(s_cnt)/(double)total);
		std::cerr<< "FP rate: " << FP <<std::endl;;
	}
	if(fn){
		double FN = (double)(((double)total - (double)s_cnt)/(double)total);
		std::cerr<< "FN rate: " << FN << std::endl;;
	}
	long time = (totalTime.tv_sec * 1000000 + totalTime.tv_usec)/total;
	sec = time/1000000;
	usec = time%1000000;
	
	std::cerr<< "unity time: " << sec << ":" << usec <<std::endl;
	
	return 0;
	
	
}