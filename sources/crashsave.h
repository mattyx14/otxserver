////////////////////////////////////////////////////////////////////////
// Emergency save when the server crashes.
// On a fatal error (access violation / SIGSEGV, illegal instruction, division by zero, stack overflow, abort,
// std::terminate, CRT invalid parameter / pure virtual call) the players, the map with the houses and the global
// storages are saved once before the process dies, as on a normal shutdown save. Windows and Linux.
////////////////////////////////////////////////////////////////////////

#ifndef __CRASHSAVE__
#define __CRASHSAVE__

namespace CrashSave
{
	// installs the handlers; call once, first thing in main
	void install();
}

#endif
