#include <map>
#include <iostream>

#include "state.h"
#include "macros.h"


State::State(){};


//get last state of variable (register)
// input parameter - register
// @return:	-1 if there are no vector for certain register
// 		-2 if state vector of certain register is empty
//		last state otherwise
int State::getLastState( int t )
{
	std::map< int, std::vector< StateLink > >::iterator var_ptr;
        std::vector< StateLink >::iterator state_ptr;

        var_ptr = states.find( t );
        if ( var_ptr != states.end() )
        {
                int s = (*var_ptr).second.size();
                if ( s == 0 ) return -1;
                else return (*var_ptr).second[s-1].st;
        }
        else return -2;

}


void State::printStates()
{
	std::map< int, std::vector< StateLink > >::iterator iter;
	std::vector< StateLink >::iterator st_iter;

	for( iter = states.begin(); iter != states.end(); iter++ )
	{
		PRINT_DEBUG << "VARIABLE " << (*iter).first << std::endl;
		for ( int j = 0; j < (*iter).second.size(); j++ )
		{
			PRINT_DEBUG << (*iter).second[j].inst_off << "\t";
			switch ( (*iter).second[j].st )
			{
				case 1: PRINT_DEBUG << " D " << std::endl;
					break;
				case 2:
					PRINT_DEBUG << " U " << std::endl;
					break;
				case 3:
					PRINT_DEBUG << " R " << std::endl;
					break;
				case 4:
					PRINT_DEBUG << " UR " << std::endl;
					break;
				case 5:
					PRINT_DEBUG << " DD " << std::endl;
					break;
				case 6:
					PRINT_DEBUG << " DU " << std::endl;
					break;
				default:
					break;
			}
		}	
	}
	return;
}

void State::newChain()
{
	std::map< int, std::vector< StateLink > >::iterator iter;
	StateLink link;

	link.inst_off = -1;
	link.st = UNDEFINED;

	for ( iter = states.begin(); iter != states.end(); iter++ )
	{
		if ( getLastState( (*iter).first ) != UNDEFINED ) (*iter).second.push_back( link );
	}
	return;

}

State& State::traverseExec( int o, INSTRUCTION *inst )
{
	std::map< int, std::vector<  StateLink  > > ::iterator var_ptr, var_ptr2;
        std::vector<  StateLink >::iterator state_ptr;
        std::vector<  StateLink > states_tmp;
	int t, t2, st_t, st_t2;
	StateLink link;

	//we are looking for anomaly in registers only
	if ( inst->op1.type != OPERAND_TYPE_REGISTER ) 
	{
		if ( inst->op2.type == OPERAND_TYPE_REGISTER)
		{
			t2 = inst->op2.reg;
                	var_ptr2 = states.find( t2 );
                	st_t2 = getLastState( t2 );

			if (st_t2 == -2 )
			{
				link.inst_off = -1;
				link.st = UNDEFINED;
				states_tmp.clear();
				states_tmp.push_back( link );
				link.inst_off = o;
				link.st = UR;
				states_tmp.push_back( link );
				states.insert( std::pair< int, std::vector< StateLink > > ( t2, states_tmp ) );
			}
			else if ( st_t2 == -1 )
			{
				link.inst_off = -1;
				link.st = UNDEFINED;
				(*var_ptr2).second.push_back( link );
				link.inst_off = o;
				link.st = UR;
				(*var_ptr2).second.push_back( link );
			}
			else if ( st_t2 == UNDEFINED || st_t2 == UR || st_t2 == DU )
			{
				link.inst_off = o;
				link.st = UR;
				(*var_ptr2).second.push_back( link );
			}
			else
			{
				link.inst_off = o;
				link.st = REFFERED;
				(*var_ptr2).second.push_back( link );
			}
		}


		return *this;
	}


	t = inst->op1.reg;
	var_ptr = states.find( t );
	st_t = getLastState( t );
	
	if ( st_t == -2 )
	{
		link.inst_off = -1;
		link.st = UNDEFINED;
		states_tmp.clear();
		states_tmp.push_back( link );
		states.insert( std::pair< int, std::vector< StateLink  > > ( t, states_tmp ) );
		var_ptr = states.find( t );
		st_t = UNDEFINED;
	}
	else if ( st_t == -1 ) 
	{
		link.inst_off = -1;
		link.st = UNDEFINED;
		(*var_ptr).second.push_back( link );
		st_t = UNDEFINED;
	}

	link.inst_off = o;

	if ( inst->op2.type == OPERAND_TYPE_REGISTER )
	{
		t2 = inst->op2.reg;
		var_ptr2 = states.find( t2 );
		st_t2 = getLastState( t2 );
		
		if ( st_t2 < 0 )
		{
			if ( st_t == DEFINED ) link.st = DU;
			else link.st = UNDEFINED;
			(*var_ptr).second.push_back( link );
			
			link.st = UNDEFINED;
			if( st_t2 == -2 )
			{
				states_tmp.clear();
				states_tmp.push_back( link );
				states.insert( std::pair< int, std::vector< StateLink > > ( t2, states_tmp) );
				var_ptr2 = states.find( t2 );
			}
			else (*var_ptr2).second.push_back( link );

			link.st = UR;
			(*var_ptr2).second.push_back( link );
			return *this;
		}
		else if( st_t2 == UNDEFINED || st_t2 == UR || st_t2 == DU )
		{
			if ( st_t == DEFINED ) link.st = DU;
			else link.st = UNDEFINED;
			(*var_ptr).second.push_back( link );
			
			link.st = UR;
			(*var_ptr2).second.push_back( link );
			return *this;
		}
		else
		{
			if ( st_t == DEFINED) link.st = DD;
			else link.st = DEFINED;
			(*var_ptr).second.push_back( link );
			link.st = REFFERED;
			(*var_ptr2).second.push_back( link );
			return *this;
		}
		
	}
	else
	{
		if ( st_t == DEFINED || st_t == DD ) link.st = DD;
		else link.st = DEFINED;
		(*var_ptr).second.push_back( link );
	}
	return *this;
}


State& State::setRegReffered( int o, int t )
{
	std::map< int, std::vector<  StateLink  > > ::iterator var_ptr;
        std::vector<  StateLink > states_tmp;
        int st_t;
        StateLink link;


	var_ptr = states.find( t );
        st_t = getLastState( t );

        if ( st_t == -2 )
        {
	        link.inst_off = -1;
                link.st = UNDEFINED;
                states_tmp.clear();
                states_tmp.push_back( link );
                link.inst_off = o;
                link.st = UR;
                states_tmp.push_back( link );
                states.insert( std::pair< int, std::vector< StateLink > > ( t, states_tmp ) );
        }
	else if ( st_t == -1 )
        {
	        link.inst_off = -1;
                link.st = UNDEFINED;
                (*var_ptr).second.push_back( link );
                link.inst_off = o;
                link.st  = UR;
                (*var_ptr).second.push_back( link );
        }
        else if( st_t == UNDEFINED || st_t == UR || st_t == DU )
        {
        	link.inst_off = o;
                link.st = UR;
                (*var_ptr).second.push_back( link );
        }
        else
        {
        	link.inst_off = o;
                link.st = REFFERED;
                (*var_ptr).second.push_back( link );
        }

	return *this;
}

State& State::traverseReffered( int o, INSTRUCTION * inst )
{
	int t;
	
	if ( inst->op1.type == OPERAND_TYPE_REGISTER) 
	{	
		t = inst->op1.reg;
		setRegReffered( o, t );
	}

	if ( inst->op2.type == OPERAND_TYPE_REGISTER )
	{
		t = inst->op2.reg;
		setRegReffered( o, t );
	}


	return *this;
}
