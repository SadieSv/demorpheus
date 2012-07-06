#include <iostream>
#include <vector>
#include <queue>

#include "macros.h"
#include "graphTopology.h"


using namespace std;

/**
 * Subtract the timeval values x and y, storing the result in result param
 * @param result	-	here the value of difference will be stored
 * @param x			-	parameter for subtraction
 * @param y			-	difference
 * @return	1 if the difference is negative
 * 			0 otherwise
 */
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

void GraphTopology::clearDetectedClasses()
{
		for(int i=0; i< class_num; i++)
		_detected_classes[i]=false;
}

GraphTopology::GraphTopology()
{
	_lastTime.tv_sec = 0;
	_lastTime.tv_usec = 0;
	
	Classifier::init();
	FlowBased::init();
	CFGBased::init();
	
	clearDetectedClasses();
	_topology.clear();
	mClassifier = new InitialLength();
	_elem_classifiers.push_back( mClassifier );
}

GraphTopology::~GraphTopology()
{
	for (int i=0; i<_elem_classifiers.size(); i++)
		if(_elem_classifiers[i]) delete _elem_classifiers[i];
	CFGBased::release();
}

void GraphTopology::recognizeShellcodeClasses()
{
	bool all = true;
	
	clearDetectedClasses();
	for( int i=1; i< _current_classifiers.size(); i++){
		if(!_current_classifiers[i]) continue;
		for(int j=0; j<class_num; j++){
			if(_current_classifiers[i]->current_type[j])
				_detected_classes[j] = true;
			else all=false;
		}

		if(all)	break;
		all = true;
	}
}

void GraphTopology::processNextLevel(std::vector< vector<Classifier*> > *levels)
{
	for( int i=0; i < _current_classifiers.size(); i++ ) {
		
	}
}

std::vector<Classifier*>selectOptimalLevel(std::vector< vector<Classifier*> > *levels)
{
	std::vector<Classifier*> level;
	
	return level;
}

std::vector<Classifier*> GraphTopology::createNextLevel(bool first_iteration)
{
	std::vector< vector<Classifier*> > levels;
	std::vector<Classifier*> empty;
	
	if(first_iteration) _current_classifiers = _elem_classifiers;
	if(_current_classifiers.empty()) return empty;
	
	recognizeShellcodeClasses();
	processNextLevel(&levels);
	return empty;
}

void GraphTopology::makeHybridTopology()
{
	std::vector<Classifier*> v;
	std::map<Classifier*, std::vector< Classifier* > >::iterator ptr, ptr1;
	Classifier* current;
	
	
	//STEP0 while-loop here
	//STEP1:recognizeShellcodeClasses();
	//STEP2: next layer
	//STEP3: link classifiers
	//STEP4: remove classifiers
	//STEP5: check for return
	
	//==========MANUAL TOPOLOGY==================//
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(mClassifier, v) );
	ptr = _topology.find( mClassifier );
	if( ptr == _topology.end() ) {
		PRINT_ERROR( " Something wrong with topology graph ");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(mClassifier, false));
	
	// InitialLength --> DisasLength
	current = new DisasLength();
	_elem_classifiers.push_back( current );
	(*ptr).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr = _topology.find( current );
	if( ptr == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//DisasLength --> DisasOffset
	current = new DisasOffset();
	_elem_classifiers.push_back( current );
	(*ptr).second.push_back( current ); //ptr is in DisasLength now
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr1 = _topology.find( current ); //ptr1 is in DisasOffset
	if( ptr1 == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//DisasLength --> CycleFinder
	current = new CycleFinder();
	_elem_classifiers.push_back( current );
	(*ptr).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr = _topology.find( current ); // ptr is in CycleFinder
	if( ptr == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//DisasOffset --> Racewalk
	current = new RaceWalk();
	_elem_classifiers.push_back( current );
	(*ptr1).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr1 = _topology.find( current ); //ptr1 is in Racewalk
	if( ptr1 == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//CycleFinder->HDD
	current = new HDD(0);
	_elem_classifiers.push_back( current );
	(*ptr).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr = _topology.find( current ); // ptr is in HDD
	if( ptr == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//Racewalk --> DataFlowAnomaly
	current = new DataFlowAnomaly();
	_elem_classifiers.push_back( current );
	(*ptr1).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	ptr1 = _topology.find( current ); //ptr1 is in DataFlowAnomaly
	if( ptr1 == _topology.end() ) {
		PRINT_ERROR(" Something wrong with topology graph");
	}
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
	
	//HDD --> DataFlowAnomaly
	(*ptr).second.push_back( current );
	
	//DataFlowAnomaly --> PushCall
	current  = new PushCall();
	_elem_classifiers.push_back( current );
	(*ptr1).second.push_back( current );
	_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(current, v));
	markedClassifier.insert(std::pair<Classifier*, bool>(current, false));
}

void GraphTopology::makeLinearTopology()
{
	classifierVertex vertex;
	std::vector<Classifier*> v;
	for( int i=0; i< _elem_classifiers.size(); i++){
		//fills structure for the next execution
		markedClassifier.insert(std::pair<Classifier*, bool>(_elem_classifiers[i], false));
		
		//if first classifier, add it to initial classifier, 
		//which is already in _topology
		v.clear();
		if( i == _elem_classifiers.size()-1 ) 
			_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(_elem_classifiers[i], v) );
		else {
			v.push_back( _elem_classifiers[i+1] );
			_topology.insert(std::pair<Classifier*, std::vector<Classifier*> >(_elem_classifiers[i], v) );
		}
	}
}

void GraphTopology::clearMarkedClassifier()
{
	std::map<Classifier*, bool>::iterator ptr;
	for(ptr = markedClassifier.begin(); ptr != markedClassifier.end(); ptr++)
		(*ptr).second = false;
}


//uses BFS graph algorithm
bool GraphTopology::executeTopology( const unsigned char* buffer, int length, bool reduce )
{
	std::map<Classifier*, std::vector<Classifier*> >::iterator ptr;
	std::queue<Classifier*> executionOrder;
	std::map<Classifier*, bool>::iterator marked_ptr;
	Classifier *current;
	struct timeval startTime, endTime, diff, totalTime;
	
	Classifier::reload( buffer, length );
	FlowBased::reload();
	CFGBased::reload();


	//initialize first classifier
	clearMarkedClassifier();
	executionOrder.push( mClassifier );
	
	//mark first classifier as explored
	marked_ptr = markedClassifier.find( mClassifier );
	if( marked_ptr == markedClassifier.end() ) {
		PRINT_WARNING(" Something wrong with markedClassifier structure");
		return false;
	}
	(*marked_ptr).second = true;
	totalTime.tv_sec = 0;
	totalTime.tv_usec = 0;
	
	while( !executionOrder.empty() ){
		//remove first classifier from the queue
		current = executionOrder.front();
		executionOrder.pop();
			
		ptr = _topology.find( current );
		if ( ptr == _topology.end() ) {
			PRINT_WARNING(" Something wrong with _topology structure");
			return false;
		}
			
		bool mal = false;
		gettimeofday(&startTime, NULL);
		//execute first classifier
		current->update();
		mal  = current->check();
			
		gettimeofday(&endTime, NULL);
		timeval_subtract(&diff, &endTime, &startTime);
		timeval_addition(&totalTime, &totalTime, &diff);
			
		if( mal ) {
			current->updateInformationVector( Classifier::MALWARE );
		}
		else {
			current->updateInformationVector( Classifier::LEGITIMATE );
			//if we are in hybrid topology, i.e. if we want to reduce the flow, we 
			//should break connection between current classifier and it's childrens
			if( reduce )
				(*ptr).second.clear();
		}
			
		//look for a children
		for( int i=0; i< (*ptr).second.size(); i++ ) {
			marked_ptr = markedClassifier.find( (*ptr).second[i] );
			if( marked_ptr == markedClassifier.end())  {
				PRINT_WARNING(" Something wrong with markedClassifier structure");
				return false;
			}
			if( (*marked_ptr).second ) continue;
			(*marked_ptr).second = true;
			executionOrder.push((*marked_ptr).first);
		}
	}
	
	_lastTime = totalTime;
	PRINT_DEBUG << "check information vector " << Classifier::checkInformationVector() << std::endl;
	return Classifier::checkInformationVector();
}

void GraphTopology::printDetectedClasses()
{
	for( int i=0; i<class_num; i++ )
		cout <<i << ":" << _detected_classes[i]<< std::endl;
}

void GraphTopology::printTopology()
{
	std::map<Classifier*, std::vector<Classifier*> >::iterator ptr; 
	
	
	if (!_topology.size() ) return;

	for( ptr = _topology.begin(); ptr != _topology.end(); ptr++) {
		for( int j=0; j< (*ptr).second.size(); j++ ){
			(*ptr).first->printInfo();
			cout<< " ---> ";
			(*ptr).second[j]->printInfo();
			cout<< std::endl;
		}
	}
	

}