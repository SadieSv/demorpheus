#include <iostream>
#include <map>
#include <iterator>
#include <string.h>
#include "eifg.h"
#include "libdasm.h"
#include "state.h"
#define MAX_INSTRUCTION_LENGHT 16
#define MODE MODE_32
#define PUSH_CALL_THR 20

using namespace std;

EIFG::EIFG ( State* s  )
{
	push_call_cnt = 0;
	variablesState = s;
}

EIFG::~EIFG() {};

EIFG& EIFG::insertVertex ( Vertex* v )
{

	inst_Pair t;
	t.first = v->getOffset();
	t.second = v->getType();
	vertexes.insert ( std::pair <int, Vertex>  ( v->getOffset() , *v ) );
	InstructionArray.insert ( std::pair < unsigned char * , inst_Pair > ( v->getName(), t ) );
	return *this;		
}

void EIFG::printInformation ()
{
	cout << "Vertex number = " << vertexes.size() << std::endl;
}

void EIFG::printVertexes ()
{
	std::map < int, Vertex >::iterator iter;
	std::map< int, ParCh >::iterator ptr;

	for ( iter = vertexes.begin(); iter != vertexes.end(); ++iter )
	{
		cout << (*iter).first << "\t:\t";
		(*iter).second.printVertex();
		cout << std::endl;
		
		ptr = ParentChildArray.find( (*iter).first );
		if( ptr != ParentChildArray.end() )
		{
			cout << "parents("<< (*ptr).second.parent.size() << ")	";
			for (unsigned int i = 0; i < (*ptr).second.parent.size(); i++ )
			{
				cout << (*ptr).second.parent[i] << " , ";
			}
			cout<< endl;
			cout<< "children(" << (*ptr).second.child.size() << ")	";
			for (unsigned int i = 0; i < (*ptr).second.child.size(); i++ )
			{
				cout << (*ptr).second.child[i] << " , ";
			}
			cout<< endl;
			
		} 
	}
}


bool  EIFG::findVertex( int o )
{
	std::map < int, Vertex >::iterator iter;
	iter  = vertexes.find( o );
	if ( iter != vertexes.end() ) return true;
	return false;
}

bool EIFG::findVertex ( Vertex* v)
{
	std::map< int, Vertex >::iterator iter;

	for ( iter = vertexes.begin(); iter!= vertexes.end(); ++iter )
	{
		if ( (*iter).second.getName() == v->getName() && (*iter).first == v->getOffset()  )
		{
			return true;
			break;
		}
	}
	return false;
}

int EIFG::getVertexType ( int o )
{
	std::map < int, Vertex >::iterator iter;
	iter = vertexes.find( o );
	if ( iter == vertexes.end() ) return -1;
	return (*iter).second.getType();
}

int EIFG::getVertexOp1 ( int o )
{
	std::map< int, Vertex>::iterator iter;
	iter = vertexes.find( o );
	if ( iter == vertexes.end() ) return -1;
	return (*iter).second.getOp1();
}

Vertex* EIFG::getVertex( int o )
{
	std::map< int, Vertex>::iterator iter;
	Vertex* r = 0;
	
	iter = vertexes.find( o );
	if ( iter != vertexes.end() ) return &(*iter).second;
	else return 0;
}

EIFG& EIFG::removeVertex( int o )
{
	vertexes.erase( o );
	return *this;
}

bool EIFG::isExt ( int addr )
{
	std::map < int , std::vector< int > >::iterator iter;
	iter = ExtArray.find ( addr );
	if ( iter != ExtArray.end() ) return true;
	return false;
}

void EIFG::doJmp(int o, unsigned char* buffer )
{
	unsigned char op_byte[2];
        int addr;
	
	memcpy( op_byte, buffer, sizeof(char) );
        op_byte[1] = '\0';
        addr = op_byte[0]  - 0x00;
	
	dolibdasmJmp( o, addr );
	return;

}


// for vertex with offset "o" insert vertex "addr" as a child. 
// for vertex with offset "addr" insert "o" as parent
void EIFG::insertChildAndParent( int o, int addr )
{
	std::map< int, ParCh >::iterator iter;
	ParCh pc;
	bool exist = false;

	//insert child 
	iter = ParentChildArray.find( o );
	if ( iter != ParentChildArray.end() )
	{
		for (unsigned int i = 0; i < (*iter).second.child.size(); i++ )
		{
			if ( (*iter).second.child[i] == addr )
			{
				exist = true;
				break;
			}
		}
		if ( !exist ) (*iter).second.child.push_back( addr ); 
	}
	else
	{
		pc.child.clear();
		pc.parent.clear();
		pc.child.push_back( addr );
		ParentChildArray.insert( std::pair< int, ParCh > ( o, pc ) );
	}

	exist = false;

	//insert parent
	iter = ParentChildArray.find( addr );
	if ( iter != ParentChildArray.end() )
	{
		for (unsigned int i = 0; i < (*iter).second.parent.size(); i++ )
		{
			if ( (*iter).second.parent[i] == o )
			{
				exist = true;
				break;
			}
		}
		if ( !exist ) (*iter).second.parent.push_back( o );
	}
	else
	{
		pc.child.clear();
		pc.parent.clear();
		pc.parent.push_back( o );
		ParentChildArray.insert( std::pair< int, ParCh > ( addr, pc ) );
	}

	return;
}



//insert edges which corresponding to jmp instructions
void EIFG::dolibdasmJmp( int o, int addr )
{
	std::vector < int > from;
	std::map < int, std::vector < int > >::iterator ext_iter;
	std::vector< int >::iterator ptr_2;

	if ( findVertex( addr ) )
	{
		insertChildAndParent( o, addr );
	}
        else
        {
                ext_iter = ExtArray.find ( addr );

                if ( ext_iter != ExtArray.end() ) (*ext_iter).second.push_back( o );
                else
                {
                        from.clear();
                        from.push_back( o );
                        ExtArray.insert ( std::pair<int, std::vector <int> > ( addr, from ) );
                }
        }

        return;

}
//if there is control transfer to offset o, insert relative edges
void EIFG::doExt( int o )
{
	std::map< int, std::vector< int > >::iterator iter;

	iter = ExtArray.find( o );
	if ( iter!= ExtArray.end() )
	{
		for (unsigned int i = 0; i < (*iter).second.size(); ++i )
		{
			insertChildAndParent( (*iter).second[i], o );
		}
		ExtArray.erase ( iter );
	}
	return;
}

//we are looking for push-call patterns. 
//intput: current offset
//return value: number of vertex parents with "push" instruction type
int EIFG::pushNum( int o )
{
	std::map< int, ParCh >::iterator iter;
	int r = 0;
	
	iter = ParentChildArray.find( o );
	if ( iter == ParentChildArray.end() ) return 0;
	else
	{
		for (unsigned int i = 0; i < (*iter).second.parent.size(); i++ )
		{
			if ( getVertexType( (*iter).second.parent[i] ) == INSTRUCTION_TYPE_PUSH ) r++;
		}
	}
	return r;
}

/**
Function generates eifg from a certain offset by libdasm
**/

EIFG& EIFG::libdasmGenerator (const unsigned char* buffer, unsigned lenght, int o )
{
	unsigned char ins_str[MAX_INSTRUCTION_LENGHT];
        Vertex* v1 = new Vertex();
        Vertex* v2 = new Vertex();
        Vertex* temp;
        bool break_flow = false;
        bool tmp_break_fl = false;
	INSTRUCTION inst;
	POPERAND op;
	char string[256];
	unsigned int len, op_len;
	std::vector< int > parent_vector;
	std::map< int, std::vector< int > >::iterator par_ptr;


	//first iteration
	len = get_instruction (&inst, const_cast<unsigned char*>(buffer), MODE);
	if ( !len )
	{
		if ( isExt ( o ) ) ExtArray.erase( o );
		delete v1;
		delete v2;
                return *this;

	}
//	get_instruction_string( &inst, FORMAT_INTEL , o, string, 256 );


	variablesState->newChain();

	// if there is control transfer to current offset, insert relative edges 
	doExt( o );

	//fill instruction fields
        v1->setType ( inst.type );
        v1->setJmp ( ( inst.type == INSTRUCTION_TYPE_JMP || inst.type == INSTRUCTION_TYPE_JMPC) );
        v1->setSize ( len );
	v1->setOffset ( o );
	v1->setName (  inst.opcode  );
	insertVertex( v1 );

	//fill operand field
	if ( inst.op1.type == OPERAND_TYPE_REGISTER )
	{
		v1->setOp1 ( inst.op1.reg );
	}

	//Jmp instruction
	if ( inst.type == INSTRUCTION_TYPE_JMP || inst.type == INSTRUCTION_TYPE_JMPC )
	{
		if ( get_operand_type(&inst.op1) == OPERAND_TYPE_IMMEDIATE )
		{
			dolibdasmJmp( o, inst.op1.immediate );
		}	
		if ( inst.type == INSTRUCTION_TYPE_JMP ) break_flow = true;
	}
	
	//Define instructions
	if ( inst.type == INSTRUCTION_TYPE_MOV || inst.type == INSTRUCTION_TYPE_MOVSR || inst.type == INSTRUCTION_TYPE_MOVS ||
		inst.type == INSTRUCTION_TYPE_MOVC || inst.type == INSTRUCTION_TYPE_MOVZX || inst.type == INSTRUCTION_TYPE_MOVSX )
	{
		variablesState->traverseExec( o, &inst );	
	}
	else variablesState->traverseReffered( o, &inst );

		
	if ( lenght <= len ) 
	{
		delete v1;
		delete v2;
		return *this;
	}

        buffer += len;
        lenght -= len;


	while ( lenght > 0 )
	{
		o += len;
		if ( findVertex( o ) )
		{
			if ( !break_flow )
			{
				insertChildAndParent( v1->offset, o );
				if ( inst.type == INSTRUCTION_TYPE_MOV || inst.type == INSTRUCTION_TYPE_MOVSR || inst.type == INSTRUCTION_TYPE_MOVS ||
                inst.type == INSTRUCTION_TYPE_MOVC || inst.type == INSTRUCTION_TYPE_MOVZX || inst.type == INSTRUCTION_TYPE_MOVSX )
                		{
                        		variablesState->traverseExec( o, &inst );
                		}
			}
			delete v1;
			delete v2;
			return *this;		
		}
		len = get_instruction ( &inst, const_cast<unsigned char*>(buffer), MODE);
                if ( !len )
                {
                        if ( isExt ( o ) ) ExtArray.erase( o );
			delete v1;
			delete v2;
                        return *this;
                }

                doExt( o );

		//fill vertex fields
                v2->setType ( inst.type );
                v2->setJmp ( ( inst.type == INSTRUCTION_TYPE_JMP || inst.type == INSTRUCTION_TYPE_JMPC) );
                v2->setSize ( len );
                v2->setOffset ( o );
                v2->setName ( inst.opcode  );
                insertVertex( v2 );
		
		if ( inst.type == INSTRUCTION_TYPE_JMP )
        	{
                	tmp_break_fl = true;
                	if ( get_operand_type(&inst.op1) == OPERAND_TYPE_IMMEDIATE )
                	{
                        	dolibdasmJmp( o, inst.op1.immediate );
                	}
        	}
        	if ( inst.type == INSTRUCTION_TYPE_JMPC )
        	{
                	if ( get_operand_type(&inst.op1) == OPERAND_TYPE_IMMEDIATE )
                	{
                        	dolibdasmJmp( o, inst.op1.immediate );
                	}
        	}

		//Define instructions
        	if ( inst.type == INSTRUCTION_TYPE_MOV || inst.type == INSTRUCTION_TYPE_MOVSR || inst.type == INSTRUCTION_TYPE_MOVS ||
                inst.type == INSTRUCTION_TYPE_MOVC || inst.type == INSTRUCTION_TYPE_MOVZX || inst.type == INSTRUCTION_TYPE_MOVSX )
        	{
                	variablesState->traverseExec( o, &inst );
        	}
		else variablesState->traverseReffered( o, &inst );


                if ( !break_flow ) 
		{
			insertChildAndParent( v1->getOffset(), v2->getOffset() );
		}


		//push-call pattern
		if ( inst.type == INSTRUCTION_TYPE_CALL ) push_call_cnt += pushNum( o );

                temp = v1;
                v1 = v2;
                v2 = temp;
                break_flow = tmp_break_fl;
                tmp_break_fl = false;
                buffer += len;
		if (lenght<= len) 
		{
			delete v1;
			delete v2;
			return *this;
		}
                else lenght -= len;

	}
	delete v1;
	delete v2;
	return *this;	

}


void EIFG::clearGraph()
{
        push_call_cnt = 0;
        vertexes.clear();
        ExtArray.clear();
	ParentChildArray.clear();
        return;
}


EIFG& EIFG::makeGraph(const unsigned char* buffer, int len, int o  )
{
	push_call_cnt = 0;

	while ( len >0 )
	{
		if ( !findVertex( o ) ) libdasmGenerator( buffer + o, len, o);

		if ( push_call_cnt >= PUSH_CALL_THR )
		{
			throw push_call_cnt;
			clearGraph();
			return *this;
		}
		o += 1;
		len -= 1;
		push_call_cnt = 0;
	}
	return *this;
}


//insert edges between parents and childrens of input vertex
EIFG& EIFG::addRelativeEdges( int o )
{
	std::map< int, ParCh >::iterator iter;

	
	iter = ParentChildArray.find( o );
	if ( iter == ParentChildArray.end() ) return *this;

	for (unsigned int i = 0; i < (*iter).second.parent.size(); i++ )
	{
		for (unsigned int j =0; j < (*iter).second.child.size(); j++ )
		{
			insertChildAndParent( (*iter).second.parent[i], (*iter).second.child[j]);
		}
	}
	return *this;
}


EIFG& EIFG::removeEdge( int o1, int o2 )
{
	std::map< int, ParCh >::iterator iter;

	iter = ParentChildArray.find( o1 );
	if ( iter != ParentChildArray.end() )
	{
		for (unsigned int j = 0; j < (*iter).second.child.size(); j++ )
		{
			if ( (*iter).second.child[j] == o2 )
			{
				(*iter).second.child.erase( (*iter).second.child.begin() + j );
				break;
			}
		}
	} 
	iter = ParentChildArray.find( o2 );
	if ( iter != ParentChildArray.end() )
	{
		for (unsigned int j = 0; j < (*iter).second.parent.size(); j++ )
		{
			if ( (*iter).second.parent[j] == o1 )
			{
				(*iter).second.parent.erase( (*iter).second.parent.begin() + j );
				break;
			}
		}
	}
	return *this;
}

void EIFG::deleteFamilyRelations( int o )
{
	std::map< int, ParCh >::iterator iter, iter2;

	iter = ParentChildArray.find( o );
	if ( iter == ParentChildArray.end() ) return ;


	//for all parents delete current vertex from their childrens      
	for (unsigned int i = 0; i < (*iter).second.parent.size(); i++ )
	{
		iter2 = ParentChildArray.find( (*iter).second.parent[i] );
		if ( iter2 != ParentChildArray.end() )
		{
			for (unsigned int j = 0; j < (*iter2).second.child.size(); j++)
			{
				if ( (*iter2).second.child[j] == o )
				{
					(*iter2).second.child.erase( (*iter2).second.child.begin()+j  );
					break;
				}
			}
		}
	}

	//for all childrens delete current vertex from their parrents
	for (unsigned int i = 0; i < (*iter).second.child.size(); i++ )
	{
		iter2 = ParentChildArray.find( (*iter).second.child[i] );
		if ( iter2 != ParentChildArray.end() )
		{
			for (unsigned int j = 0; j < (*iter2).second.parent.size(); j++ )
			{
				if ( (*iter2).second.parent[j] == o )
				{
					(*iter2).second.parent.erase( (*iter2).second.parent.begin() + j );
					break;
				}
			}
		}
	}

	//delete Vertex from ParentChildArray
	ParentChildArray.erase ( iter );	
	
	return;
}


