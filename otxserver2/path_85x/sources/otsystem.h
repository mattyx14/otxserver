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

#ifndef __OTSYSTEM__
#define __OTSYSTEM__
#include "definitions.h"

#include <string>
#include <algorithm>
#include <bitset>
#include <queue>
#include <set>
#include <vector>
#include <list>
#include <map>
#include <limits>

#include <boost/version.hpp>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <cstddef>
#include <cstdlib>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	#include <cstdint>
#else
	#include <stdint.h>
#endif

#ifndef __x86_64__
	#ifdef _M_X64 // msvc
		#define __x86_64__ 1
	#else
		#define __x86_64__ 0
	#endif
#endif

#include <ctime>
#include <cassert>
#ifdef WINDOWS
	#include <windows.h>
	#include <sys/timeb.h>

	#ifndef access
	#define access _access
	#endif

	#ifndef timeb
	#define timeb _timeb
	#endif

	#ifndef ftime
	#define ftime _ftime
	#endif

	#ifndef EWOULDBLOCK
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#endif

	#ifndef errno
	#define errno WSAGetLastError()
	#endif

	#ifndef OTSYS_SLEEP
		#define OTSYS_SLEEP(n) Sleep(n)
	#endif
#else
	#include <sys/timeb.h>
	#include <sys/types.h>
	#include <sys/socket.h>

	#include <unistd.h>
	#include <netdb.h>
	#include <errno.h>

	#include <arpa/inet.h>
	#include <netinet/in.h>

	#ifndef SOCKET
	#define SOCKET int32_t
	#endif

	#ifndef closesocket
	#define closesocket close
	#endif

	#ifndef SOCKADDR
	#define SOCKADDR sockaddr
	#endif

	#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
	#endif

	inline void OTSYS_SLEEP(int32_t n)
	{
		timespec tv;
		tv.tv_sec  = n / 1000;
		tv.tv_nsec = (n % 1000) * 1000000;
		nanosleep(&tv, NULL);
	}
#endif

inline int64_t OTSYS_TIME()
{
	timeb t;
	ftime(&t);
	return ((int64_t)t.millitm) + ((int64_t)t.time) * 1000;
}

inline uint32_t swap_uint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

#if BOOST_VERSION < 104400
#define BOOST_DIR_ITER_FILENAME(iterator) (iterator)->path().filename()
#else
#define BOOST_DIR_ITER_FILENAME(iterator) (iterator)->path().filename().string()
#endif

#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH
#endif
