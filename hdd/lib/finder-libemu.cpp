#include "finder-libemu.h"

extern "C" {
	#include <emu/emu.h>
	#include <emu/emu_shellcode.h>
}

using namespace std;

#ifdef FINDER_LOG
	#define LOG (*log)
#else
	#define LOG if (false) cerr
#endif

FinderLibemu::FinderLibemu() : Finder(-1)
{
	Timer::start();
}

FinderLibemu::~FinderLibemu()
{
	Timer::stop();

	LOG	<< endl << endl
		<< "Time total: " << dec << Timer::secs() << " seconds." << endl
		<< "Time spent on load: " << dec << Timer::secs(TimeLoad) << " seconds." << endl
		<< "Time spent on find: " << dec << Timer::secs(TimeFind) << " seconds." << endl;
}

int FinderLibemu::find() {
	pos_dec.clear();
	Timer::start(TimeFind);
	struct emu *e = emu_new();

	long int offset = emu_shellcode_test(e, (uint8_t *) reader->pointer(), reader->size());
	if (offset >= 0) {
		pos_dec.push_back(offset);
		LOG << "Found shellcode at offset 0x" << hex << offset << endl;
	} else {
		LOG << "Did not find anything." << endl;
	}

	emu_free(e);
	Timer::stop(TimeFind);
	return pos_dec.size();
}
