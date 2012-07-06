#include <iostream>
#include <algorithm>

#include "classifier.h"


//#define LOG 

const unsigned char* empty_str=reinterpret_cast<const unsigned char*>("");
int Classifier::data_len=0;
unsigned char* Classifier::_buffer= const_cast<unsigned char*>(empty_str);



//==================== CLASSIFIER PARENT CLASS=================================================//
Classifier::vector_type Classifier::information_vector[class_num];
void Classifier::init()
{
	for ( int i=0; i < class_num; i++ ){
		information_vector[i] = UNKNOWN;
	}
}

Classifier::Classifier()
{
	for(int i=0; i<class_num; i++) current_type[i] = false;
}
void Classifier::reload(const unsigned char *buffer, int length)
{
	_buffer = const_cast<unsigned char*>(buffer);
	//data_len = strlen(reinterpret_cast<const char*>(buffer));
	data_len = length;
}

void Classifier::updateInformationVector(vector_type type)
{
	for( int i=0; i< class_num; i++ ) {
		if(!current_type[i]) continue;
		if( information_vector[i] == UNKNOWN || information_vector[i] == MALWARE)
			information_vector[i] = type;			
	}
	
}

bool Classifier::checkInformationVector()
{
	for( int i=0; i< class_num; i++ )
		if( information_vector[i] == MALWARE ) return true;
	return false;
}

void Classifier::printInformationVector()
{
	std::cerr<< "Information vector:\n";
	for( int i=0; i< class_num; i++ ){
		std::cerr<< "Class " << i << " , value " << information_vector[i] << std::endl;
	}
}

void Classifier::featureScaling( double *f_scaled, const double f, 
								const double avr, const double range)
{
		*f_scaled = (double)((f-avr)/range);
}

//====================INITIAL LENGTH CLASSIFIER============================================//

InitialLength::InitialLength()
{
	for( int i=0; i<class_num; i++) current_type[i] = true;
}

bool InitialLength::check()
{
	PRINT_DEBUG << "in first classifier" << std::endl;
	PRINT_DEBUG << "data_len = " << data_len << " MIN_DATA_LEN =  " << MIN_DATA_LEN << std::endl; 
	return data_len >= MIN_DATA_LEN;
}

//=======================FLOW BASED PARENT CLASS==========================================//
void FlowBased::init()
{
	flow = boost::shared_ptr<Flow>(new Flow(_buffer, data_len));
}

void FlowBased::reload()
{
	init();
}

//=========================CFG BASED PARENT CLASS=========================================//
void CFGBased::init()
{
	s = new State();
	if (!s) {
		//ERROR<< "ERROR:cannot create State object";
		return;
	}
	ifg = new EIFG( s );
	if (!ifg) {
		//ERROR<< "ERROR:cannot create EIFG object";
		return;
	}
	
	ifg->makeGraph( _buffer, data_len, 0 );
}

void CFGBased::release()
{
	if(s) delete s;
	if(ifg) delete ifg;
}

CFGBased::~CFGBased()
{
}

void CFGBased::reload()
{
	if(s) delete s;
	if(ifg) delete ifg;
	init();
}

//========================DISAS LENGTH CLASSIFIER============================================//
DisasLength::DisasLength()
{
	//current_type.push_back(ALL);
	for( int i=0; i<class_num; i++) current_type[i] = true;
	
	fn = 0.05;
	fp_r = 0.67;
	fp_m = 0.95;
	complexity = 12;
}

int DisasLength::getChainSize( int i, int j ) const
{
		int size = flow->fl[i].size();
        int offset =  flow->fl[i][size-1].offset;
        // if last instruction is not reference, return chain length from reffered instruction
        // or in the case of quitely long instructions chain we don't need to compute the total
        // lenght ( in't enough already and we try to reduce time complexity)
        if ( size - j >= MIN_INSTR_CHAIN || offset < 0 || offset >= MAX_DATA_LEN ||
		  flow->ext_offset[ flow->fl[i][size-1].offset ]  == false ) return size - j;
        int f = flow->offsets[ offset ].first;
        int s = flow->offsets[ offset ].second;
        //infinite cycle
        if ( i == f ) return size-j;
        size += getChainSize( f, s );
        return size-(j+1);

}

bool DisasLength::check()
{
	if(!flow) return false;
	
	for (unsigned i = 0; i < flow->fl.size(); i++ ) {
		if ( getChainSize( i, 0 ) >= MIN_INSTR_CHAIN ) return true;
	}
	return false;
}

//=====================DISAS OFFSET CLASSIFIER=======================================================//
DisasOffset::DisasOffset()
{
	//current_type.push_back(NOP);
	current_type[NOP] = true;
	
	fn = 0.1;
	fp_r = 0.01;
	fp_m = 0.59;
	complexity = 13;
}

bool DisasOffset::check()
{
	if(!flow) return false;
	
	int prev_off = 0;
	
	for ( int i = 0; i < data_len; i++){
		if (i-prev_off>nop_len) return true;
		if ( flow->offsets[i].first == -1 )
			prev_off = i;
	}
	return false;
}


//======================PUSH-CALL CLASSIFIER======================================================//
PushCall::PushCall()
{
	for(int i=0; i<class_num; i++) current_type[i] = true;
	
	fn = 0.15;
	fp_r = 0.02;
	fp_m = 0.77;
	complexity = 790;
}

int PushCall::pushCallMax(int off, int pcn, bool flag, int* pocn)
{
	std::map< int, Vertex >::iterator v_iter;
	std::map< int, ParCh >::iterator p_iter;	
	int maxpc;

	visited.insert( off );
	
	// such vertex is not exist
	v_iter = ifg->vertexes.find( off );
	if ( v_iter == ifg->vertexes.end() ) return pcn;
	p_iter = ifg->ParentChildArray.find( off );
	if ( p_iter == ifg->ParentChildArray.end() ) return pcn;

	//instruction has no "children". That means that instruction
	// is last in the chain
	if ( (*p_iter).second.child.empty() ) return pcn;


	// PUSH instruction met
	if ( (*v_iter).second.getType() == INSTRUCTION_TYPE_PUSH ){
		flag = true;
		(*pocn)++;
	}
	else if ( (*v_iter).second.getType() == INSTRUCTION_TYPE_CALL )
	{
		if ( flag ) pcn++;
		flag = false;
		(*pocn)++;
	}
	maxpc = pcn;
	for ( unsigned int i=0; i < (*p_iter).second.child.size(); i++ ){
		if ( visited.find((*p_iter).second.child[i] ) != visited.end()) break;
		maxpc = std::max( maxpc,	pushCallMax((*p_iter).second.child[i], pcn, flag, pocn));
	}

	visited.erase( off );
	return maxpc;
}

bool PushCall::check()
{
	if(!ifg) return false;
	std::map< int, ParCh >::iterator iter;
	
	for ( iter = ifg->ParentChildArray.begin(); iter!= ifg->ParentChildArray.end(); ++iter){
		// if offset responds to first instruction in some chain
        if( (*iter).second.parent.empty() ){
			visited.clear();
			int pocn=0;
			int pc = pushCallMax((*iter).first, 0, false, &pocn);
			if ( pc >= pushCallThr ) return true;
			if(pocn >= pushOrCall) return true;
		}
	}
	return false;
}

//========================DATA FLOW ANOMALY CLASSIFIER==========================================//
DataFlowAnomaly::DataFlowAnomaly()
{
	//current_type.push_back(ALL);
	for( int i=0; i<class_num; i++) current_type[i] = true;
	
	fn = 0.21;
	fp_r = 0.02;
	fp_m = 0.77;
	complexity = 724;
	
	int global_range = (MAX_DATA_LEN - MIN_DATA_LEN)/(max_thr - min_thr);
	int local_range = (data_len - MIN_DATA_LEN)/global_range;
	int current_thr = min_thr + local_range; 
	
	if(ifg != NULL && s!=NULL) analyzer = new SGAnalyzer( ifg, s, current_thr);
}

DataFlowAnomaly::~DataFlowAnomaly()
{
	if(analyzer) delete analyzer;
}

bool DataFlowAnomaly::check()
{
	if(!analyzer) return false;
    analyzer->pruningUseless();
    return analyzer->isExecutable();
}

void DataFlowAnomaly::update()
{
	if( analyzer ) delete analyzer;
	
	int global_range = (MAX_DATA_LEN - MIN_DATA_LEN)/(max_thr - min_thr);
	int local_range = (data_len - MIN_DATA_LEN)/global_range;
	int current_thr = min_thr + local_range;
	
	if(ifg != NULL && s!=NULL) analyzer = new SGAnalyzer( ifg, s, current_thr);
	else {
		//ERROR<< "ERROR:cannot create analyzer! " << std::endl;
		return;
	}
}

//=============================CYCLE FINDER CLASSIFIER==========================================//
CycleFinder::CycleFinder()
{
	//current_type.push_back(DECRYPTOR);
	current_type[DECRYPTOR] = true;
	
	fn = 0.005;
	fp_r = 0.7;
	fp_m = 0.96;
	complexity = 18;
}

bool CycleFinder::check()
{
	if(!ifg) return false;
	std::map< int, ParCh >::iterator iter;

	for ( iter = ifg->ParentChildArray.begin(); iter!= ifg->ParentChildArray.end(); ++iter){
		// just simple assumption. If some instruction has "child" with 
		// lower offset, than alert cycle presence
		for ( unsigned int i = 0; 
				(*iter).second.child.size(); i++ )
			if ( (*iter).second.child[i] <= (*iter).first )
				return true;
	}
	return false;
}


//=============================RACEWALK CLASSIFIER=============================================//
RaceWalk::RaceWalk()
{
	//current_type.push_back(NOP);
	current_type[NOP] = true;
	
	fn = 0.21;
	fp_r = 0.35;
	fp_m = 0.25;
	complexity = 81;
	
	racewalk_init(nop_len);
}

bool RaceWalk::check()
{
	int l  = data_len;
	int res = racewalk_simple_find_sled( _buffer, l );
	return (res != l);
}

//==========================HDD CLASSIFIER======================================================//

HDD::HDD(int _finderType )
		: finderType(_finderType), emulatorType(1)
{
	//current_type.push_back(DECRYPTOR);
	current_type[DECRYPTOR] = true;
	
	findDecryptor = new FindDecryptor(finderType, emulatorType);
	
	switch (_finderType){
		case 0:
			fp_r = 0.00001;
			fp_m = 0.0;
			fn = 0.21;
			complexity = 489;
			break;
		case 1:
			fp_r = 0.0001;
			fp_m = 0.0;
			fn = 0.17;
			complexity = 371;
			break;
		default:
			fp_r = 0.00001;
			fn = 0.21;
			complexity = 489;
			break;
	}
}

HDD::~HDD()
{
	delete findDecryptor;
}

bool HDD::check()
{
	findDecryptor->link(_buffer, data_len);
	return (findDecryptor->find() > 0);
}