#ifndef __FIND_DECRYPTOR_PLUGIN_H
#define __FIND_DECRYPTOR_PLUGIN_H

#include <aura/plugin> 
#include <aura/log>
#include <aura/config>

using namespace Aura;
using namespace std;

/**
* @author Anastasiya Shcherbinina <nastya@lvk.cs.msu.su>
*/
class FindDecryptorPlugin : public Plugin
{
public:
	DECLARE_INTERFACE(FindDecryptor)

	FindDecryptorPlugin() {}

	virtual bool Load(InitIface * i);
	virtual bool Start();
	virtual bool Stop();
	virtual int BindInterface(const vector<Provider*>& p); 

	~FindDecryptorPlugin() {}

	bool RegisterSymbols(IDSLangInitInterface * lii);
	bool UnregisterSymbols( IDSLangInitInterface * lii );
};

#endif // #ifndef __FIND_DECRYPTOR_PLUGIN_H
