////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#ifdef __EXCEPTION_TRACER__
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "otsystem.h"
#ifdef WINDOWS
	#include <winsock2.h>
	#include <windows.h>
	#include <dbghelp.h>
#endif

class ExceptionHandler
{
	public:
		ExceptionHandler();
		~ExceptionHandler();

		bool InstallHandler();
		bool RemoveHandler();

	private:
		bool isInstalled;
#ifdef WINDOWS

		static long WINAPI MiniDumpExceptionHandler(EXCEPTION_POINTERS* exceptionPointers = NULL);
		static int ref_counter;
#endif
};

#endif
#endif