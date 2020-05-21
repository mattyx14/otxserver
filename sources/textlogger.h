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

enum LogFile_t
{
	LOGFILE_FIRST = 0,
	LOGFILE_ADMIN = LOGFILE_FIRST,
	LOGFILE_OUTPUT = 1,
	LOGFILE_ASSERTIONS = 2,
	LOGFILE_LAST = LOGFILE_ASSERTIONS
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

		bool isLoaded() const {return m_loaded;}

		void iFile(LogFile_t file, std::string output, bool newLine);
		void eFile(std::string file, std::string output, bool newLine);

		void log(const char* func, LogType_t type, std::string message, std::string channel = "", bool newLine = true);

	private:
		Logger() {m_loaded = false;}
		void internal(FILE* file, std::string output, bool newLine);

		FILE* m_files[LOGFILE_LAST + 1];
		bool m_loaded;
};

#define LOG_MESSAGE(type, message, channel) \
	Logger::getInstance()->log(__PRETTY_FUNCTION__, type, message, channel);

class OutputHandler : public std::streambuf
{
	public:
		virtual ~OutputHandler();
		static OutputHandler* getInstance()
		{
			static OutputHandler instance;
			return &instance;
		}

	protected:
		OutputHandler();
		std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof());

		std::streambuf* log;
		std::streambuf* err;
		std::string m_cache;
};
#endif
