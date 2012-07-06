#include "racewalk.h"
#include <time.h>

static u_int seq;
static u_int shift = 0;
static struct svm_node node[NUM_INSTRUCTION_TYPES + 1];
static u_char file_buf[MAX_SIZE_BUF];

void racewalk_init(u_int sled_length) {
    the_sled_length = sled_length;

    memset (&disasm_info, 0, sizeof (disasm_info));
    disasm_info.flavour = bfd_target_unknown_flavour;
    disasm_info.arch = bfd_arch_unknown;
    disasm_info.octets_per_byte = 1;
    disasm_info.mach = bfd_mach_i386_i386;
}

/**
 * We try to decode from all arrays code, we keep track "pos" in the array scan_cache.
 * @param code array we need to decode. This array has size length
 * @param pos the position of "code" in array cached
 * @param length size of code we want to decode.
 * @return SCANNED_VALID if scan from pos to the end succesfully, otherwise SCANNED_INVALID
 */
static int racewalk_go(const u_char *code, u_int pos, u_int length) {
    if ( scan_cache[pos] != NOT_SCANNED)
        return scan_cache[pos];
    if ( length < 1)
        return SCANNED_VALID;
    u_int npos = (pos + shift) >= the_sled_length ? (pos + shift - the_sled_length) : (pos + shift);
    bool jmp = false;

    if ( decode_cache[npos] != NOT_DECODED) {
        // We never decode an instruction twice
        jmp = decode_cache[npos] >= JMP ? 1 : 0;
    } else {
        int ret = print_insn((bfd_vma)code, &disasm_info);
        if ( ret < 0) {
            decode_cache[npos] = DECODED_INVALID;
        } else {
            int insn_size = (ret>>MOD);
            int insn_type = (ret & ((1<<MOD) - 1));
            switch (insn_type) {
            case INSTRUCTION_TYPE_JMP: //fall through
            case INSTRUCTION_TYPE_JMPC:
            case INSTRUCTION_TYPE_LOOP:
            case INSTRUCTION_TYPE_CALL:
            case INSTRUCTION_TYPE_ENTER:
            case INSTRUCTION_TYPE_JECXZ:
                jmp = true;
                break;
            default:
                jmp = false;
                break;
            }

            switch (insn_type) {
            case INSTRUCTION_TYPE_INVL: //fall through
            case INSTRUCTION_TYPE_PRIV:
                decode_cache[npos] = DECODED_INVALID;
                break;
            default:
                decode_cache[npos] = insn_size + JMP * (int)(jmp ? 1 : 0);
                break;
            }
        }
    }
    if ( decode_cache[npos] == DECODED_INVALID) {
        return scan_cache[pos] = SCANNED_INVALID;
    } else {
        if ( jmp) {
            return scan_cache[pos] = SCANNED_VALID;
        }
        u_int res = (u_int) decode_cache[npos];
        if ( res < length) {
            int cur = racewalk_go(code + res, pos + res, length - res);
            if ( cur == SCANNED_INVALID) {
                seq = seq & ~(1<<(pos & 0x3));
            }
            return scan_cache[pos] = cur;
        } else {
            return scan_cache[pos] = SCANNED_VALID;
        }
    }
}


bool racewalk_simple_check_sled(const u_char *code, bool skip) {
    disasm_info.buffer =(bfd_byte *) code;
    disasm_info.buffer_vma = (bfd_vma) code;
    disasm_info.buffer_length = the_sled_length + MAX_INSTR_SIZE;

    memset(scan_cache, NOT_SCANNED, sizeof(scan_cache));
    seq = 1 | (1<<1) | (1<<2) | (1<<3);
    for (u_int j = 0; j < 4; j++) {
        if ( skip  && j < 3)
            continue;
        if (( seq & ( 1<<j )) == 0)
            continue;
        bool valid = true;
        for (u_int i = j; i < the_sled_length; i += 4) {
            int res = racewalk_go(code + i, i, the_sled_length - i);
            if ( res == SCANNED_INVALID) {
                valid = false;
                break;
            }

        }
        if ( valid)
            return true;
    }
    return false;
}


u_int racewalk_simple_find_sled(const u_char *buf, u_int length) {
    memset(decode_cache, NOT_DECODED, sizeof(decode_cache));
    shift = 0;
    if (racewalk_simple_check_sled(buf, false)) {
        return 0;
    }
    u_int i = 1;
    decode_cache[shift] = NOT_DECODED;
    shift++;
    while ( i + the_sled_length + MAX_INSTR_SIZE < length) {
        /**
          * The previous segment buf + i  - 1, buf + i + SLED_LENGTH - 2] is invalid.Therefore,
          * we can skip j = 0, 1, 2(see STRIDE paper to know what j means)
          */
        if (racewalk_simple_check_sled(buf + i,true) ) {
            return i;
        } else {
            i++;
            decode_cache[shift] = NOT_DECODED;
            shift = (shift + 1) % the_sled_length;
        }
    }

    //Have not found sled
    return length;
}

static bool counted[MAX_LENGTH];

bool stride_count_seq(const u_char *data, u_int length, double freq[], int pos){
    if( counted[pos]){
        return true;
    }
	u_int i = 0;
	while( i < length){
		int ret = print_insn((bfd_vma)(data + i), &disasm_info);
		if( ret < 0)
            return false;
		int insn_size = (ret>>MOD);
        int insn_type = (ret & ((1<<MOD) - 1));

        if( insn_type == INSTRUCTION_TYPE_PRIV || insn_type == INSTRUCTION_TYPE_INVL){
            return false;
        }

        if( counted[pos + i])
            return true;
        counted[pos + i] = true;
        freq[insn_type - 1]++;
         switch (insn_type) {
            case INSTRUCTION_TYPE_JMP: //fall through
            case INSTRUCTION_TYPE_JMPC:
            case INSTRUCTION_TYPE_LOOP:
            case INSTRUCTION_TYPE_CALL:
            case INSTRUCTION_TYPE_ENTER:
            case INSTRUCTION_TYPE_JECXZ:
                return true;
                break;
            default:
                break;
        }
        i += (u_int) insn_size;

	}
	return true;
}


int racewalk_count_frequency(const u_char *data, double freq[]){
    memset(freq, 0, sizeof(double) * NUM_INSTRUCTION_TYPES);
    memset(counted, 0, sizeof(counted));
    disasm_info.buffer =(bfd_byte *) data;
    disasm_info.buffer_vma = (bfd_vma) data;
    disasm_info.buffer_length = the_sled_length + MAX_INSTR_SIZE;

	for(u_int j = 0; j < 4; j++){
		bool is_valid = true;
		for(unsigned int i = j; i < the_sled_length; i += 4){
			if(!stride_count_seq(data + i, the_sled_length - i, freq, i)){
				is_valid = false;
				break;
			}
		}
		if( is_valid)
			return 0;
	}
	return -1;
}

void racewalk_svm_load(const char *svm_model_file, const char *scale_file){

    if( (model = svm_load_model(svm_model_file)) == NULL){
        fprintf(stderr,"Can not load the svm model file:%s\n", svm_model_file);
        exit(1);
    }
    printf("Loading svm_model from %s & range file from %s\n", svm_model_file, scale_file);
    FILE *finp = fopen(scale_file, "r");
    fgetc(finp);

    fscanf(finp, "%lf %lf\n", &lower, &upper);
    int idx;
    double fmin;
    double fmax;
    for(int i = 0; i < NUM_INSTRUCTION_TYPES; i++){
        feature_min[i] = MAX_VALUE;
        feature_max[i] = -MAX_VALUE;
    }
    while(fscanf(finp,"%d %lf %lf\n",&idx,&fmin,&fmax)==3) {
        feature_min[idx - 1] = fmin;
        feature_max[idx - 1] = fmax;
    }
    fclose(finp);
}

static void racewalk_svm_scale(double freq[]){
    for(int i = 0; i < NUM_INSTRUCTION_TYPES; i++){
        if( feature_min[i] > MAX_VALUE/2){
        //Not in stored range file, this feature never has never been seen in training phase
            if( freq[i] > 1e-9){
                freq[i] = MAX_VALUE;
            }
            continue;
        }
        if( freq[i] == feature_min[i]){
            freq[i] = lower;
        }else if( freq[i] == feature_max[i]){
            freq[i] = upper;
        }else{
            freq[i] = lower + (upper - lower) *
                (freq[i] - feature_min[i]) / (feature_max[i] - feature_min[i]);
        }
    }
}

bool racewalk_svm_predict(u_char *data){
    int ret = racewalk_count_frequency(data, insn_freq);
    if( ret != 0)
        return false;

    racewalk_svm_scale(insn_freq);
    for(int j = 0; j < NUM_INSTRUCTION_TYPES; j++){
        node[j].index = j + 1;
        node[j].value = insn_freq[j];
    }
    node[NUM_INSTRUCTION_TYPES].index = -1;
    int predict_label = svm_predict(model, node);

    return predict_label == LABEL_SLED;
}

u_int racewalk_detect(u_char *data, u_int length){

    memset(decode_cache, NOT_DECODED, sizeof(decode_cache));
    shift = 0;
    bool skip = false;
    if (racewalk_simple_check_sled(data, false)) {
        if( racewalk_svm_predict(data))
            return 0;
    }else{
        skip = true;
    }
    u_int i = 1;
    decode_cache[shift] = NOT_DECODED;
    shift++;
    while ( i + the_sled_length + MAX_INSTR_SIZE < length) {
        /**
          * if the previous segment buf + i  - 1, buf + i + SLED_LENGTH - 2] is invalid then
          * we can skip j = 0, 1, 2(see STRIDE paper to know what j means)
          */

        if (racewalk_simple_check_sled(data + i, skip) ) {
            if( racewalk_svm_predict(data + i)){
                return i;
            }
            skip = false;
        }else{
            skip = true;
        }
        i++;
        decode_cache[shift] = NOT_DECODED;
        shift = (shift + 1) % the_sled_length;
    }

    //Have not found sled
    return length;
}

void racewalk_svm_config(double n, double g, double lowerbound, double upperbound,
    char *p){
    nu = n;
    gamma_race = g;
    lower = lowerbound;
    upper = upperbound;
    sprintf(path_svm_scale, "%s/svm-scale", p);
    sprintf(path_svm_train, "%s/svm-train", p);
    sprintf(path_svm_predict, "%s/svm-predict", p);
    printf("path_svm_scale:%s\n", path_svm_scale);
}

void racewalk_generate_freq(const char *raw_file, const char *freq_file, int label){
    double freq[NUM_INSTRUCTION_TYPES];
    FILE *finp = fopen(raw_file, "rb");
    FILE *ffreq = fopen(freq_file, "w");
    if( finp == NULL || ffreq == NULL){
        printf("Error reading file '%s' or '%s'...\n", raw_file, freq_file);
        exit(1);
    }

    fprintf(stdout, "Generating frequency file '%s' from raw file '%s' with label '%s'\n", freq_file, raw_file,
    (label == LABEL_SLED? "SLED" : "NOT_SLED") );
    do{
        int n = fread(file_buf, 1, MAX_SIZE_BUF,finp);
        if( ferror(finp))
            break;
        u_int i = 0;
        u_int length = n;
        while( i  + the_sled_length + MAX_INSTR_SIZE  < length){
            u_int pos = racewalk_simple_find_sled(file_buf + i, length - i);
            if( pos != length - i){
                int ret = racewalk_count_frequency(file_buf + i + pos, freq);
                if( ret == 0){
                    fprintf(ffreq,"%d ", label);
                    for(int j= 0; j < NUM_INSTRUCTION_TYPES; j++){
                        if( freq[j] > 1e-9)
                        fprintf(ffreq, "%d:%lf   ",j + 1, freq[j]);
                    }
                    fprintf(ffreq, "\n");
                }
                i += pos + the_sled_length;
            }else{
                break;
            }
        }
    }while(!feof(finp));
    fclose(finp);
    fclose(ffreq);
}


void racewalk_learn(const char *input){
/*    char param[3][MAX_LENGTH];

    char input_freq[MAX_LENGTH];
    char input_scale[MAX_LENGTH];
    char input_scale_model[MAX_LENGTH];
    char range[MAX_LENGTH];

    sprintf(input_freq, "%s.%d.freq", input, the_sled_length);
    sprintf(input_scale,"%s.scale", input_freq);
    sprintf(input_scale_model, "%s.scale.model", input_freq);
    sprintf(range, "%s.range", input_freq);

    racewalk_generate_freq(input, input_freq, LABEL_SLED);

    fprintf(stdout, "Scale frequency file, output to '%s', save range file to '%s'...\n",input_scale, range);
    pid_t pid;

    if( (pid = fork()) == 0){//child process
        int fileid = open(input_scale, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        sprintf(param[0], "%lf", lower);
        sprintf(param[1], "%lf", upper);

        dup2(fileid, STDOUT_FILENO);

        execl(path_svm_scale, path_svm_scale, "-l", param[0], "-u", param[1], "-s" ,range,
        input_freq,(char *) 0);
        exit(1);
    }else if( pid < 0){
        fprintf(stdout, "Error at fork(). Exit now...\n");
        exit(1);
    }else{
        waitpid(pid, 0, 0);
    }


    fprintf(stdout, "Training with one-class svm, nu = %lf, gamma_race = %lf\n\
Saving model file to '%s'...\n\n", nu, gamma_race, input_scale_model);
    if( (pid = fork()) == 0){//child process
        sprintf(param[0], "%lf", nu);
        sprintf(param[1], "%lf", gamma_race);
        sprintf(param[2], "%d", 2);

        execl(path_svm_train, path_svm_train, "-s", param[2], "-n", param[0], "-g" ,param[1],
        input_scale,(char *) 0);
        exit(1);
    }else if( pid < 0){
        fprintf(stdout, "Error at fork(). Exit now...\n");
        exit(1);
    }else{
        waitpid(pid, 0, 0);
    }
*/
}

void racewalk_offline_exp(const char *learn_file, const char *test_file, int test_label){
    char param[3][MAX_LENGTH];
    char test_freq[MAX_LENGTH];
    char test_scale[MAX_LENGTH];
    char range[MAX_LENGTH];
    char learn_file_model[MAX_LENGTH];
    char test_result[MAX_LENGTH];

    sprintf(test_freq, "%s.%d.freq",  test_file, the_sled_length);
    sprintf(test_scale,"%s.scale", test_freq);
    sprintf(range, "%s.%d.freq.range", learn_file, the_sled_length);
    sprintf(learn_file_model, "%s.%d.freq.scale.model", learn_file, the_sled_length);
    FILE *f;

    if( (f = fopen(learn_file_model, "r")) == NULL){ // have not learn
        fprintf(stdout, "Learning...\n");
        racewalk_learn(learn_file);
    }else{
        printf("Already have learning model '%s'... \n", learn_file_model);
        fclose(f);
    }

    racewalk_svm_load(learn_file_model, range);


    double freq[NUM_INSTRUCTION_TYPES];
    FILE *finp = fopen(test_file, "rb");
    if( finp == NULL){
        printf("Error reading file '%s'...\n", test_file);
        exit(1);
    }
    int num_predict = 0;
    int num_correct = 0;
    printf("Predicting...\n");
    do{
        int n = fread(file_buf, 1, MAX_SIZE_BUF,finp);
        if( ferror(finp))
            break;
        u_int i = 0;
        u_int length = n;
        while( i  + the_sled_length + MAX_INSTR_SIZE  < length){
            u_int pos = racewalk_simple_find_sled(file_buf + i, length - i);
            if( pos != length - i){
                num_predict++;
                if( racewalk_svm_predict(file_buf + i + pos)){
                    if( test_label == LABEL_SLED){
                        num_correct++;
                    }
                }else if( test_label == LABEL_NOTSLED){
                    num_correct++;
                }
                i += pos + the_sled_length;
            }else{
                break;
            }
        }
    }while(!feof(finp));
    fclose(finp);
    printf("Accuracy=%0.2lf\%(%d/%d)\n", 100.0 * (num_correct + 0.0) / num_predict, num_correct, num_predict);
}

static void racewalk_process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
    const struct ether_header* petherhdr;
    const struct ip* pip;


    int size_ip;

    petherhdr = (struct ether_header *) packet;
    if( !(ntohs(petherhdr->ether_type) == ETHERTYPE_IP)){
        //Only process ip packet
        return ;

    }

    pip =(struct ip *) (packet + ETH_HLEN);
    size_ip = ((pip->ip_hl) & 0x0f) * 4;
    if( size_ip < 20){
        return;
    }
    int sport = -1;
    int dport = -1;
    int size_tcp = 0;
    int size_udp = 0;
    struct tcphdr* ptcphdr;
    struct udphdr* pudphdr;

    u_char *data = (u_char *)(packet + ETH_HLEN + size_ip);
    int data_length = ntohs(pip->ip_len) - size_ip;

    switch(pip->ip_p){
        case IPPROTO_TCP:
            ptcphdr = (struct tcphdr *) (packet + ETH_HLEN + size_ip);
            size_tcp = ((ptcphdr->doff) &0x0f) * 4;

            sport = ntohs(ptcphdr->source);
            dport = ntohs(ptcphdr->dest);
            data = (u_char *)(packet + ETH_HLEN + size_ip + size_tcp);
            data_length = ntohs(pip->ip_len) - size_ip - size_tcp;
            break;
        case IPPROTO_UDP:

            pudphdr = (struct udphdr *)(packet + ETH_HLEN + size_ip);
            size_udp = sizeof(struct udphdr);
            data = (u_char *)(packet + ETH_HLEN + size_ip + size_udp);
            data_length = ntohs(pip->ip_len) - size_ip - size_udp;
            sport = ntohs(pudphdr->source);
            dport = ntohs(ptcphdr->dest);
            break;
        case IPPROTO_IP:
            data = (u_char *)(packet + ETH_HLEN + size_ip);
            data_length = ntohs(pip->ip_len) - size_ip;
            break;
        default:
            return;
    }

    int pos = racewalk_detect(data, data_length);
    if( pos != data_length){
        char src[INET_ADDRSTRLEN];
        char dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(pip->ip_src), src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(pip->ip_dst), dst, INET_ADDRSTRLEN);
        if( sport == -1){
            printf("From %s to %s :\n", src, dst);
        }else{
            printf("From %s:%d to %s:%d\n", src, sport, dst, dport);
        }

        printf("NOP: ", pos);
        for(unsigned int i = 0; i < the_sled_length + MAX_INSTR_SIZE && pos + i < data_length; i++){
            printf("\\x%x",(unsigned)(u_char) *(data + pos + i));
        }
        printf("\n");
    }
}

static void racewalk_init_pcap(char *filter_exp){
    char *dev;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    bpf_u_int32 mask;
    bpf_u_int32 net;
    struct in_addr addr;
    struct pcap_pkthdr header;
    const u_char *packet;
    struct ether_header *ehdr;

    if( (dev = pcap_lookupdev(errbuf)) == NULL ){
        fprintf(stderr, "Could not find default device: %s\n", errbuf);
        exit(1);
    }

    if(pcap_lookupnet(dev, &net, &mask, errbuf) == -1){
        fprintf(stderr, "Can't get netmask for device %s\n", dev);
        net = 0;
        mask = 0;
    }
    handle = pcap_open_live(dev, BUFSIZ, 0, 10000, errbuf);
    if( handle == NULL){
        fprintf(stderr, "Couldn't open device %s : %s\n", dev, errbuf);
        exit(1);
    }

    if( pcap_compile(handle, &fp, filter_exp, 0, net) == -1){
        fprintf(stderr, "Couldn't parse filter %s : %s\n", filter_exp, pcap_geterr(handle));
        exit(1);
    }

    if( pcap_setfilter(handle, &fp) == -1){
        fprintf(stderr, "Couldn't install filter %s : %s\n", filter_exp, pcap_geterr(handle));
        exit(1);
    }

    pcap_loop(handle, -1, racewalk_process_packet, NULL);
    pcap_freecode(&fp);
    pcap_close(handle);
}


void racewalk_online_detect(const char *svm_model_file, const char *scale_file, char *filter){
    racewalk_svm_load(svm_model_file, scale_file);
    printf("Running racewalk on pcap using one class svm...\n");
    racewalk_init_pcap(filter);
}

