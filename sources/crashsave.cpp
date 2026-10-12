////////////////////////////////////////////////////////////////////////
// Emergency save when the server crashes (see crashsave.h)
////////////////////////////////////////////////////////////////////////
#include "otpch.h"
#include "crashsave.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>

#include "game.h"

#ifdef WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
	#include <cstring>
	#include <cstdlib>
#endif

extern Game g_game;

namespace
{
	// only the first fatal error saves; a crash during the save itself just ends the process
	std::atomic<bool> saving(false);

	const uint8_t SAVE_ALL = (uint8_t)SAVE_PLAYERS | (uint8_t)SAVE_MAP | (uint8_t)SAVE_STATE; // the map save includes the houses

	void doSave()
	{
		try
		{
			g_game.saveGameState(SAVE_ALL);
			std::clog << "[CrashSave] Server saved." << std::endl;
		}
		catch(...)
		{
			std::clog << "[CrashSave] The save failed with an exception." << std::endl;
		}
	}

#ifdef WINDOWS
	DWORD WINAPI saveThread(LPVOID)
	{
		doSave();
		return 0;
	}
#endif

	// saves once. With freshStack, on another thread (the crashing thread may have no stack left: stack
	// overflow); otherwise on the crashing thread, so the locks it may hold (the database mutex is recursive)
	// don't block the save.
	void emergencySave(const char* reason, bool freshStack)
	{
		bool expected = false;
		if(!saving.compare_exchange_strong(expected, true))
			return;

		std::clog << std::endl << "[CrashSave] Fatal error: " << reason << ". Saving the server before it closes..." << std::endl;
#ifdef WINDOWS
		if(freshStack)
		{
			HANDLE thread = CreateThread(NULL, 8 * 1024 * 1024, saveThread, NULL, 0, NULL);
			if(thread)
			{
				// a lock held by the crashing thread would block it forever: give up after two minutes
				if(WaitForSingleObject(thread, 120000) == WAIT_TIMEOUT)
					std::clog << "[CrashSave] The save did not finish in 2 minutes, giving up." << std::endl;

				CloseHandle(thread);
			}
			std::clog.flush();
			return;
		}
#else
		(void)freshStack;
#endif
		doSave();
		std::clog.flush();
	}

#ifdef WINDOWS
	const char* exceptionName(DWORD code)
	{
		switch(code)
		{
			case EXCEPTION_ACCESS_VIOLATION: return "access violation";
			case EXCEPTION_STACK_OVERFLOW: return "stack overflow";
			case EXCEPTION_INT_DIVIDE_BY_ZERO: return "integer division by zero";
			case EXCEPTION_ILLEGAL_INSTRUCTION: return "illegal instruction";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "array bounds exceeded";
			case EXCEPTION_IN_PAGE_ERROR: return "in-page error";
			case EXCEPTION_PRIV_INSTRUCTION: return "privileged instruction";
			case EXCEPTION_DATATYPE_MISALIGNMENT: return "datatype misalignment";
			default: return "unhandled exception";
		}
	}

	LONG WINAPI unhandledFilter(EXCEPTION_POINTERS* info)
	{
		DWORD code = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0;
		emergencySave(exceptionName(code), code == EXCEPTION_STACK_OVERFLOW);
		return EXCEPTION_CONTINUE_SEARCH; // then the default handling (crash dialog / WER dump)
	}

	void invalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
	{
		emergencySave("CRT invalid parameter", false);
		std::abort();
	}

	void pureCall()
	{
		emergencySave("pure virtual function call", false);
		std::abort();
	}
#else
	// the handlers run on their own stack (a stack overflow leaves none on the thread)
	char* altStack = NULL;

	const char* signalName(int sig)
	{
		switch(sig)
		{
			case SIGSEGV: return "segmentation fault (SIGSEGV)";
			case SIGBUS: return "bus error (SIGBUS)";
			case SIGFPE: return "floating point exception (SIGFPE)";
			case SIGILL: return "illegal instruction (SIGILL)";
			case SIGABRT: return "abort (SIGABRT)";
			default: return "fatal signal";
		}
	}

	void fatalSignal(int sig)
	{
		emergencySave(signalName(sig), false);
		// back to the default action and raise it again: the process dies as it would have (and dumps core)
		signal(sig, SIG_DFL);
		raise(sig);
	}
#endif

	void terminateHandler()
	{
		const char* reason = "std::terminate (uncaught C++ exception)";
		emergencySave(reason, false);
		std::abort();
	}
}

void CrashSave::install()
{
#ifdef WINDOWS
	SetUnhandledExceptionFilter(unhandledFilter);
	_set_invalid_parameter_handler(invalidParameter);
	_set_purecall_handler(pureCall);
	// abort() normally exits through the CRT without the filter: SIGABRT gets the save too
	signal(SIGABRT, [](int) {emergencySave("abort", false);});
#else
	const size_t size = 8 * 1024 * 1024; // the save (MySQL, file writes) needs room
	altStack = (char*)malloc(size);
	if(altStack)
	{
		stack_t ss;
		memset(&ss, 0, sizeof(ss));
		ss.ss_sp = altStack;
		ss.ss_size = size;
		sigaltstack(&ss, NULL);
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = fatalSignal;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	const int signals[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT};
	for(int sig : signals)
		sigaction(sig, &sa, NULL);
#endif
	std::set_terminate(terminateHandler);
}
