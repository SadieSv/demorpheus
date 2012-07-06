#ifndef _EIFG_H
#define _EIFG_H

#include <iostream>
#include <map>
#include <iterator>
#include <string.h>
#include <vector>
#include <utility>
#include "state.h"
#include "macros.h"

#define PUSH_CALL_THR 20

using namespace std;

typedef unsigned char* xbyte;

class Vertex
{
	int type:8;
	bool jmp;
	BYTE name;
	int size;
	int offset;
	bool external;
	int op1;

public:
	std::vector <int> parent;
	std::vector <int> child;

	Vertex():type(0), jmp(false), size(0), offset(0), external(false), op1(-1) {}
	~Vertex(){};

	int getType () const  { return type;  }
	bool isJmp () const { return jmp; }
	unsigned char* getName () const { return (unsigned char*)name; }
	int getSize () const { return size; }
	int getOffset () const { return offset; }
	bool getExternal () const { return external; }
	int getOp1() const { return op1; }

	Vertex& setType ( int t ) { type = t; return *this; }
	Vertex& setJmp ( bool j = true ) { jmp = j; return *this; }
	Vertex& setName ( BYTE  n ) 
	{
		name = n;
		return *this;
	}
	Vertex& setSize ( int s ) { size = s; return *this; }
	Vertex& setOffset ( int o ) { offset = o; return *this; }
	Vertex& setExternal ( bool fl = true ) { external = fl; return *this; }
	Vertex& setParent ( int o ) { parent.push_back ( o ); return *this; }
	Vertex& setOp1 ( int reg ) { op1 = reg; return *this; }

	void printVertex () 
	{
		cout << "valid = " << type << " jmp = " << jmp;
		cout << " name = " << *(&name) << " size = " << size << " offset = " << offset << std::endl;
	}


	friend class EIFG;  
};

typedef std::pair< int , int > Pair;
typedef std::pair< int, bool > inst_Pair;

static std::map < xbyte, inst_Pair > InstructionArray;

struct ParCh
{
	std::vector< int > parent;
	std::vector< int > child;
};

class EIFG;

class EIFG
{
	std::map < int , std::vector < int > > ExtArray;

	int push_call_cnt;
public:
	EIFG ( State* s );
	~EIFG();
	
	friend class Vertex;
	
	std::map< int, Vertex > vertexes;
	std::map< int, ParCh > ParentChildArray;
	State* variablesState; 
	
	//vertex
	EIFG& insertVertex( Vertex* v );
	Vertex& getExternal ( int o );
	bool isExt ( int addr );
	bool findVertex ( int o );
	bool findVertex ( Vertex* v);
	int getVertexType ( int o );
	int getVertexOp1 ( int o );
	Vertex* getVertex( int o );
	EIFG& removeVertex( int o );
	void deleteFamilyRelations( int o );

	//edge
	EIFG& removeRelativeEdges( int o );
	EIFG& removeEdge ( int o1, int o2 );

	//print information	
	void printInformation();
	void printVertexes();


	//eifg generation
	void doJmp ( int o, unsigned char* buffer);
	void dolibdasmJmp ( int o, int addr );
	void doExt ( int o );
	void insertJmp (int addr, Vertex* v);
	void removeJmp (int addr);
	bool isAddr (int addr);
	int getNodeOffset (int addr);
	EIFG& libdasmGenerator (const unsigned char* buffer, unsigned lenght, int o  );	
	EIFG& generator ( const unsigned char* buffer, unsigned lenght, int o  );
	EIFG& makeGraph ( const unsigned char* buffer, int length, int o );
	void clearGraph();
	void insertChildAndParent( int o, int addr );
	int pushNum ( int o );

	//useless prunning
	EIFG& addRelativeEdges( int o );

};


#endif
