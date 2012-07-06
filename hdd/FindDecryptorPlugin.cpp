#include <iostream>
#include <gcc_macros>

#include <stdio.h>
//#include "mediator.h"
#include "finddecryptor.h"

#include <idsnet.h>

#include "FindDecryptorPlugin.h"

AURA_LOG_COMPONENT(FindDecryptorPlugin)

using namespace Aura;

static void findDecryptorFree();
static void findDecryptorInit();

/////////////////////////////////////////////////////////////////////////////
// Plugin standard methods
/////////////////////////////////////////////////////////////////////////////

bool FindDecryptorPlugin::Load(InitIface * i)
{
	DEBUG << "Load";
	init = i;
	return true;
}

bool FindDecryptorPlugin::Start()
{
	DEBUG << "Start";
	bool success = 	RegisterSymbols(lang_iface);
	findDecryptorInit();
	return success;
}

bool FindDecryptorPlugin::Stop()
{
	DEBUG << "Stop";
	findDecryptorFree();
	bool success = UnregisterSymbols( lang_iface );
	return success;
}

int FindDecryptorPlugin::BindInterface(const vector<Provider*>& p)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Implementations of wrapper functions.
/////////////////////////////////////////////////////////////////////////////

//static Mediator *mediator = NULL;
static FindDecryptor *findDecryptor = NULL;


static void findDecryptorFree()
{
	DEBUG << "FindDecryptor_Free";
	//delete mediator;
	//mediator = NULL;
	delete findDecryptor;
	findDecryptor = NULL;
}
static void findDecryptorInit()
{
	DEBUG << "FindDecryptor_Init";
	//delete mediator;
	//mediator = new Mediator;
	delete findDecryptor;
	findDecryptor = new FindDecryptor;
}

static void _ids_findDecryptorReset(int finderType, int emulatorType)
{
       DEBUG << "FindDecryptor_Reset";
       delete findDecryptor;
       findDecryptor = new FindDecryptor(finderType, emulatorType);
}

static u_int_32 _ids_findDecryptorFind(ids_string data, ids_vector<u_int_32> * start_ofs, ids_vector<u_int_32> *end_ofs)
{
	DEBUG << "FindDecryptor_Find";
        start_ofs->clear();
        end_ofs->clear();
//	mediator->link(data.data(), data.length());
//	return mediator->find() > 0;

	findDecryptor->link(data.data(), data.length());
        if(findDecryptor->find() > 0){
            std::list<int> starts = findDecryptor->get_start_list();
            std::list<int> sizes = findDecryptor->get_sizes_list();
            int num = sizes.size();
            while(!sizes.empty()){
                start_ofs->push_back(starts.front());
                end_ofs->push_back(starts.front()+sizes.front());
                starts.erase(starts.begin());
                sizes.erase(sizes.begin());
            } 
            return num;
        }else{
            return 0;
        }
}

/////////////////////////////////////////////////////////////////////////////
// Import symbols to AURA language
/////////////////////////////////////////////////////////////////////////////
#define _F(X) ((void *)(X))
bool FindDecryptorPlugin::RegisterSymbols(IDSLangInitInterface * lii)
{
	bool success = lii->RegisterSymbol ("findDecryptorFind", _F(_ids_findDecryptorFind));
	success = lii->RegisterSymbol ("findDecryptorReset", _F(_ids_findDecryptorReset)) && success;
	return success;
}

bool FindDecryptorPlugin::UnregisterSymbols( IDSLangInitInterface * lii )
{
	bool success = lii->UnregisterSymbol("findDecryptorFind", _F(_ids_findDecryptorFind));
	success = lii->UnregisterSymbol("findDecryptorReset", _F(_ids_findDecryptorReset)) && success;
        return success;
}

/////////////////////////////////////////////////////////////////////////////
// Allocator
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
__declspec(dllexport) 
#else
extern "C" 
#endif
DLL_PUBLIC Plugin *allocPlugin()
{
	Plugin *p = new FindDecryptorPlugin();
	return p;
}
