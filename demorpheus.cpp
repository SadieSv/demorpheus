#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/time.h>
#include <pcap.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>




#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>

#include "macros.h"
#include "graphTopology.h"

using namespace std;

unsigned char * payload;
static int size_payload = 0;
static char * ip_saddr;
static char * ip_daddr;
static uint tcp_source;
static uint tcp_dest;
static struct iphdr ip_hdr;
static struct tcphdr * tcp_hdr;


string ts(){
	std::stringstream strm;
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	strm << "time:" <<  current_time.tv_sec << ":" << current_time.tv_usec << ";";
	return strm.str();
};

//#define PRINT_IP_FROM inet_ntoa(*(struct in_addr * )&ip_hdr->saddr) << ":" << ntohs(tcp_hdr->source)
#define PRINT_IP_FROM ip_saddr << ":" << tcp_source
//#define PRINT_IP_TO inet_ntoa(*(struct in_addr * )&ip_hdr->daddr) << ":" << ntohs(tcp_hdr->dest)
#define PRINT_IP_TO ip_daddr << ":" << tcp_dest
#define PRINT_LOG if(LOG) cout << " [Demorpheus] ["<<ts()<<"] Shellcode found in traffic from " << PRINT_IP_FROM << " to " << PRINT_IP_TO << " shellcode types : " << endl; 

static struct option long_options[] = 
{
	{"error-rate", required_argument, 0, 'e'},
	{"topology", required_argument, 0, 't'},
	{"interface", required_argument, 0, 'i'},
	{"file", required_argument, 0, 'f'},
	{"help", no_argument, 0, 'h'}
};

void printHelp()
{
	cout << "usage: ./demorpheus OPTION [file name or interface] [--error-rate fp|fn] [--topology linear|hybrid]\n" <<
			"OPTIONS: \n"<< 
			"-i \t\t interface\n"<<
			"-f \t\t file\n"<<
			"-h \t\t help message\n\n" << endl;
	cout << "error-rate: \n fp calculates false positives rate \n fn calculates false negatives rate" << endl;
	cout << " topology is hybrid by default" << endl << endl;
}

bool process_tcp_pkt( const u_char * raw_tcp_part, int remaining_len, struct iphdr * hdr_ip, int size_ip )
{
	if( remaining_len < (int)sizeof(struct tcphdr ) ) {
		PRINT_DEBUG << "partial_fill_chunk_tcp: remaining_len < sizeof(struct tcphdr )" << endl;
		return false;
	}

	struct tcphdr * hdr_tcp = (struct tcphdr * )( raw_tcp_part );
	int size_tcp = hdr_tcp->doff * 4;

	if ( size_tcp < 20 ) {
		PRINT_DEBUG << "partial_fill_chunk_tcp: Invalid TCP header length: " << size_tcp << " bytes, less than 20 bytes" << endl;
		return false;
	}

	//ip_hdr.saddr = hdr_ip->saddr;
	//ip_hdr.daddr = hdr_ip->daddr;
	tcp_hdr = hdr_tcp;
	ip_saddr = inet_ntoa(*(struct in_addr * )&hdr_ip->saddr);
	ip_daddr = inet_ntoa(*(struct in_addr * )&hdr_ip->daddr);
	tcp_source = ntohs(hdr_tcp->source);
	tcp_dest = ntohs(hdr_tcp->dest);

	size_payload = ntohs(hdr_ip->tot_len) - (size_ip + size_tcp);
	payload = (unsigned char *)(raw_tcp_part + size_tcp);
	
	PRINT_DEBUG << "payload size: " << size_payload << endl;

	return true;
}

bool process_udp_pkt( const u_char * raw_udp_part, int remaining_len, struct iphdr * hdr_ip, int size_ip  )
{
	if( remaining_len < (int)sizeof(struct udphdr ) ) {
		PRINT_DEBUG << "partial_fill_chunk_udp: remaining_len < sizeof(struct tcphdr )" << endl;
		return false;
	}

	struct udphdr * hdr_udp = (struct udphdr * )( raw_udp_part );
	int size_udp = sizeof(udphdr);

	size_payload = ntohs(hdr_ip->tot_len) - (size_ip + size_udp);
	payload = (u_char*)(raw_udp_part + size_udp);

	return true;
}


bool process_ethernet_pkt( pcap_pkthdr & header, const u_char * raw_packet )
{
	int caplen = header.caplen; /* length of portion present from bpf  */
	int length = header.len;    /* length of this packet off the wire  */

	if( caplen < length ) {
		PRINT_DEBUG << "init_packet: caplen " << caplen << " is less than packet size " << length << endl;
		return false;
	}

	if( caplen < 14 ) { 
		PRINT_DEBUG << "init_packet: caplen " << caplen << " is less than header size, total packet size " << length << endl;
		return false;
	}

	struct ether_header * hdr_ethernet = (struct ether_header*)( raw_packet );
	u_int16_t type = ntohs( hdr_ethernet->ether_type );
 
	if( type != ETHERTYPE_IP ) {
		PRINT_DEBUG << "got packet with non-IPv4 header , type = " << type << endl;
		return false;
	}

	struct iphdr * hdr_ip = (struct iphdr * )(raw_packet + sizeof(struct ether_header) );
	int size_ip = hdr_ip->ihl * 4;

	if (size_ip < 20) {
		PRINT_DEBUG << "init_packet: Invalid IP header length: " << size_ip << " bytes, less than 20 bytes" << endl;
		return false;
	}

	//tcp packet has been received
	if( hdr_ip->protocol == 6 ) {
		return process_tcp_pkt( raw_packet + sizeof(ether_header) + size_ip, length - sizeof(ether_header) + size_ip, hdr_ip, size_ip );
	}
	//udp packet
	else if( hdr_ip->protocol == 17 ) {
		return process_udp_pkt( raw_packet + sizeof(ether_header) + size_ip, length - sizeof(ether_header) + size_ip, hdr_ip, size_ip );
	}

	return false;
}



bool process_raw_pkt( pcap_pkthdr & header, const u_char * raw_packet )
{
	size_t caplen = header.caplen; /* length of portion present from bpf  */
	size_t length = header.len;    /* length of this packet off the wire  */

	if( caplen < length ) {
		PRINT_DEBUG << "init_packet: caplen " << caplen << " is less than packet size " << length << endl;
		return false;
	}

	struct iphdr * hdr_ip = (struct iphdr * )(raw_packet);
	int size_ip = hdr_ip->ihl * 4;

	if (size_ip < 20) {
		PRINT_DEBUG << "init_packet: Invalid IP header length: " << size_ip << " bytes, less than 20 bytes" << endl;
		return false;
	}

	if( hdr_ip->protocol == 6 ) {
		return process_tcp_pkt( raw_packet + size_ip, length - size_ip, hdr_ip, size_ip );
	}
	else if( hdr_ip->protocol == 17 ) {
		return process_udp_pkt( raw_packet + size_ip, length - size_ip, hdr_ip, size_ip );
	}
 
	return false;
}

int main( int argc, char **argv)
{
	FILE* pFile = NULL;
	pcap_t *handle = NULL;
    unsigned char buffer[MAX_DATA_LEN];
 	char errbuf[PCAP_ERRBUF_SIZE];
	Classifier *cl;
	GraphTopology *topology;
	struct timeval lastTime, totalTime;
	string dev, file_name;
	bool fp = false, fn = false, hybrid = true, interface = false, file = false;
	
	int opt;
	
	while (1) {
		int option_index = 0;
		opt = getopt_long(argc, argv, "i:f:e:t:h", long_options, &option_index);
		
		if(opt == -1) break;
		
		switch (opt) {
			case 'i':
				dev = optarg;
				interface = true;
				break;
			case 'f':
				file = true;
				file_name = optarg;
				pFile = fopen( file_name.c_str(), "rb" );
				if ( pFile == NULL )
					PRINT_ERROR(" Cannot open file with binary data " );
				break;
			case 'e':
				if( !strcmp(optarg, "fp") ) fp = true;
				else if( !strcmp(optarg, "fn")) fn = true;
				break;
			case 't':
				if( !strcmp(optarg, "hybrid")) hybrid = true;
				else if( !strcmp(optarg, "linear")) hybrid = false;
				break;
			case 'h':
				printHelp();
				break;
			default:
				abort();
		}
	}
	
	if(!interface && !file) {
		PRINT_ERROR("No interface or filename is given");
	}
	else if(interface && file) {
		PRINT_ERROR( " Interface and file options are incompatible");
	}
	
	//total number of processed pieces of data and total number of detected shellcodes
	int total_cnt = 0;
	int s_cnt = 0;
	totalTime.tv_sec = 0;
	totalTime.tv_usec = 0;
	
	topology = new GraphTopology();
	
	//set of elementary classifiers added to topology. 
	//comment those you don't want to use
	
	topology->_elem_classifiers.push_back(new DisasLength());
	topology->_elem_classifiers.push_back(new PushCall());
	topology->_elem_classifiers.push_back(new DataFlowAnomaly());
	topology->_elem_classifiers.push_back(new DisasOffset());
	topology->_elem_classifiers.push_back(new RaceWalk());
	topology->_elem_classifiers.push_back(new HDD(0));
	topology->_elem_classifiers.push_back(new CycleFinder());

	topology->recognizeShellcodeClasses();
	
	//select Hybrid or Linear topology
	if(hybrid) topology->makeHybridTopology();
	else topology->makeLinearTopology();
	
	//catch the packets, retrieve payload and process it
	if (interface) {
		handle = pcap_open_live(dev.c_str(), BUFSIZ, 1, 1000, errbuf);
		if (handle == NULL ) {
			PRINT_ERROR( " Cannot open interface");
		}
		
		int index = 0;
		while(1) {
			pcap_pkthdr header;
			const u_char * packet = pcap_next( handle, &header );
			if( !packet )
				continue;

			if( pcap_datalink(handle) == DLT_EN10MB ) {
				process_ethernet_pkt( header, packet);
			}
			else if( pcap_datalink(handle) == DLT_RAW ) 
				process_raw_pkt( header, packet );
			
			if ( size_payload <= MIN_DATA_LEN) continue;
			
			struct timeval current_time;
			while (size_payload - index > 0) {
				int cnt = 0;
				if ( size_payload < MAX_DATA_LEN )
					cnt = size_payload;
				else cnt = MAX_DATA_LEN;
				
				memcpy(buffer, payload + index*sizeof(unsigned char), cnt); 
				buffer[cnt]='\0';
				index+=cnt;
				
				//cout << "size of payload: " << size_payload << endl;
				//cout << "cnt: " << cnt << endl;
				//cout << "index: " << index << endl;
				
				if( topology->executeTopology(buffer, cnt, false)) {
					PRINT_LOG;
					s_cnt++;
				}
				total_cnt++;	
			}
			index = 0;
		}
		pcap_close(handle);
	}
	
	if (pFile){
		while(!feof(pFile)){
			int cnt = fread( buffer, sizeof( unsigned char ), MAX_DATA_LEN*sizeof( unsigned char ), pFile );
			if ( cnt == 0 ) continue;
			if ( cnt < MIN_DATA_LEN ) continue;
			if ( cnt < MAX_DATA_LEN ) buffer[cnt]='\0';
		
			if( topology->executeTopology(buffer, cnt, false)) 
				s_cnt++;
			total_cnt++;
			lastTime = topology->getLastTime();
			timeval_addition(&totalTime, &totalTime, &lastTime);
		}
		
		fclose(pFile);
	}
	
	delete topology;

	PRINT_DEBUG << total_cnt<< " " << s_cnt << std::endl;
	PRINT_DEBUG << totalTime.tv_sec << " " << totalTime.tv_usec <<std::endl;

	return 0;	
}