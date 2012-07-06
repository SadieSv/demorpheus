#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "i386-dismod.h"
#include "bfd.h"
#include "racewalk.h"
#include <setjmp.h>


int c;
char *filename;
extern char *optarg;
extern int optind, optopt, opterr;


int main(int argc, char *argv[]){
    const char *option = "Usage:\n \
    racewalk  -c cmd -s sled_size [-f rawfile] [-m modelfile] [-r rangefile] [-t testfile]\
    [-i label] [-p pcapfilter]\n\
-c cmd\n\
    0 Learning. The next parameter is raw binary nop file's name. \n\
	For example,\n\
	  racewalk -c 0 -s 64 -f nopbinary\n\
    1 Detect on pcap using svm. The next parameters are svm_model_file, svm_scale_file  & pcap filter option. Note:\
svm must be learned first by using \"-c 0\"\n\
	For example,\n\
	  racewalk -c 0 -s 128 -f nopbinary\n\
	  racewalk -c 1 -s 128 -m nopbinary.128.freq.scale.model -r nopbinary.128.freq.range -p \"port 80\"\n\
    2 Experience for false positive and false negative. The next parameters are nopfile to learn & test_file.\
If test_file is nop file then the label parameter is 1, otherwise -1.\n\
	For example,\n\
	  racewalk -c 2 -s 64 -f nopbinary -t nopbinary.test -i 1\n\
	  racewalk -c 2 -s 64 -f nopbinary -t normal -i -1\n\
    3 Generate instruction frequency file from raw file\n\
	For example,\n\
	  racewalk -c 3 -s 64 -f nopbinary\n\
-l \n\
   lowerbound : default value -1, used with cmd = 0, 2\n\
-u \n\
   upperbound :default value 1, used with cmd = 0, 2\n\
-n \n\
   nu default :value 0.0078125, used with cmd = 0, 2\n\
-g \n\
   gamma :default value 0.0625, used with cmd = 0, 2\n";


    int type = -1;
    int sled_length = -1;
    char *svm_model_file = NULL;
    char *svm_range_file = NULL;
    char *raw_file = NULL;
    char *test_file = NULL;
    char *pcap_filter = "";
    char *svm_path = "/usr/local/bin";
    char freq_file[MAX_LENGTH];
    double nu = 0.0078125;
    double gamma = 0.0625;
    double lower = -1;
    double upper = 1;
    int label = 0;


    while ((c = getopt(argc, argv, ":c:s:f:l:r:m:t:n:g:l:u:p:i:h:")) != -1) {
        switch(c) {
        case 'c':
            type = atoi(optarg);
            break;
        case 's':
            sled_length = atoi(optarg);
            break;
        case 'm':
            svm_model_file = optarg;
            break;
        case 'r':
            svm_range_file = optarg;
            break;
        case 'f':
            raw_file = optarg;
            break;
        case 'i':
            label = atoi(optarg);
            break;
        case 't':
            test_file = optarg;
            break;
        case 'n':
            nu = atof(optarg);
            break;
        case 'g':
            gamma = atof(optarg);
            break;
        case 'l':
            lower = atof(optarg);
            break;
        case 'u':
            upper = atof(optarg);
            break;
        case 'p':
            pcap_filter = optarg;
            break;
        case 'h':
        case ':':
        case '?':
            printf("%s", option);
            return 1;
            break;
        }
    }
    if( sled_length == -1){
        printf("%s", option);
        return 1;
    }

    switch(type){
        case 0:
            if( raw_file == NULL){
                printf("%s", option);
                return 1;
            }
            racewalk_init(sled_length);
            racewalk_svm_config(nu, gamma, lower, upper, svm_path);
            racewalk_learn(raw_file);
            break;
        case 1:
            if(svm_model_file == NULL || svm_range_file == NULL){
                printf("%s", option);
                return 1;
            }
            racewalk_init(sled_length);
            racewalk_online_detect(svm_model_file, svm_range_file, pcap_filter);
            break;
        case 2:
            if( raw_file == NULL || test_file == NULL || label == 0){
                printf("%s", option);
                return 1;
            }
            racewalk_init(sled_length);
            racewalk_svm_config(nu, gamma, lower, upper, svm_path);
            racewalk_offline_exp(raw_file, test_file, label);
            break;
        case 3:
            racewalk_init(sled_length);

            sprintf(freq_file, "%s.%d.freq", raw_file, sled_length);
            racewalk_generate_freq(raw_file, freq_file, label);
            break;
        default:
            printf("%s", option);
            break;
    }
    return 0;
}
