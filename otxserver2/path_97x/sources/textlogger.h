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

#ifndef __TEXTLOGGER__
#define __TEXTLOGGER__
#include "otsystem.h"

#if defined(WINDOWS) && !defined(_CONSOLE)
#include <ostream>
#include <fstream>
#endif

enum LogFile_t
{
	LOGFILE_FIRST = 0,
	LOGFILE_ADMIN = LOGFILE_FIRST,
	LOGFILE_OUTPUT = 1,
	LOGFILE_ASSERTIONS = 2,
	LOGFILE_CLIENT_ASSERTION = 3,
	LOGFILE_LAST = LOGFILE_CLIENT_ASSERTION
};

enum LogType_t
{
	LOGTYPE_EVENT,
	LOGTYPE_NOTICE,
	LOGTYPE_WARNING,
	LOGTYPE_ERROR,
};

class Logger
{
	public:
		virtual ~Logger() {close();}
		static Logger* getInstance()
		{
			static Logger instance;
			return &instance;
		}

		void open();
		void close();

		void iFile(LogFile_t file, std::string output, bool newLine);
		void eFile(std::string file, std::string output, bool newLine);

		void log(const char* func, LogType_t type, std::string message, std::string channel = "", bool newLine = true);

	private:
		Logger() {}
		void internal(FILE* file, std::string output, bool newLine);

		FILE* m_files[LOGFILE_LAST + 1];
};

#define LOG_MESSAGE(type, message, channel) \
	Logger::getInstance()->log(__PRETTY_FUNCTION__, type, message, channel);

#if defined(WINDOWS) && !defined(_CONSOLE)
class GUILogger : public std::streambuf
{
	public:
		GUILogger();
		virtual ~GUILogger();

		std::streambuf* err;
		std::streambuf* log;
		std::streambuf* out;

	protected:
		int32_t overflow(int32_t c);

		bool m_displayDate;
		std::string m_cache;
};
#endif
#endif
