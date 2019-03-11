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
#include "otpch.h"
#include "textlogger.h"

#include "manager.h"
#include "dispatcher.h"

#include "configmanager.h"
#include "game.h"
#include "tools.h"

extern ConfigManager g_config;
extern Game g_game;

void Logger::open()
{
	std::string path = g_config.getString(ConfigManager::OUTPUT_LOG);
	if(path.length() < 3)
		path = "";
	else if(path[0] != '/' && path[1] != ':')
		path = getFilePath(FILE_TYPE_LOG, path);

	m_files[LOGFILE_ADMIN] = fopen(getFilePath(FILE_TYPE_LOG, "admin.log").c_str(), "a");
	if(!path.empty())
		m_files[LOGFILE_OUTPUT] = fopen(path.c_str(), (g_config.getBool(ConfigManager::TRUNCATE_LOG) ? "w" : "a"));

	m_files[LOGFILE_ASSERTIONS] = fopen(getFilePath(FILE_TYPE_LOG, "client_assertions.log").c_str(), "a");
	m_loaded = true;
}

void Logger::close()
{
	m_loaded = false;
	for(uint8_t i = 0; i <= LOGFILE_LAST; ++i)
	{
		if(m_files[i])
			fclose(m_files[i]);
	}
}

void Logger::iFile(LogFile_t file, std::string output, bool newLine)
{
	if(!m_loaded || !m_files[file])
		return;

	internal(m_files[file], output, newLine);
	fflush(m_files[file]);
}

void Logger::eFile(std::string file, std::string output, bool newLine)
{
	FILE* f = fopen(getFilePath(FILE_TYPE_LOG, file).c_str(), "a");
	if(!f)
		return;

	internal(f, "[" + formatDate() + "] " + output, newLine);
	fclose(f);
}

void Logger::internal(FILE* file, std::string output, bool newLine)
{
	if(!file)
		return;

	if(newLine)
		output += "\n";

	fprintf(file, "%s", output.c_str());
}

void Logger::log(const char* func, LogType_t type, std::string message, std::string channel/* = ""*/, bool newLine/* = true*/)
{
	if(!m_loaded)
		return;

	std::ostringstream ss;
	ss << "[" << formatDate() << "]" << " (";
	switch(type)
	{
		case LOGTYPE_EVENT:
			ss << "Event";
			break;

		case LOGTYPE_NOTICE:
			ss << "Notice";
			break;

		case LOGTYPE_WARNING:
			ss << "Warning";
			break;

		case LOGTYPE_ERROR:
			ss << "Error";
			break;

		default:
			break;
	}

	ss << " - " << func << ") ";
	if(!channel.empty())
		ss << channel << ": ";

	ss << message;
	iFile(LOGFILE_ADMIN, ss.str(), newLine);
}

OutputHandler::OutputHandler()
{
	log = std::clog.rdbuf(this);
	err = std::cerr.rdbuf(this);
}

OutputHandler::~OutputHandler()
{
	std::clog.rdbuf(log);
	std::cerr.rdbuf(err);
}

std::streambuf::int_type OutputHandler::overflow(std::streambuf::int_type c/* = traits_type::eof()*/)
{
	m_cache += c;
	if(c != '\n' && c != '\r')
		return c;

	if(m_cache.size() > 1)
		std::cout << "[" << formatTime(0, true) << "] ";

	std::cout.write(m_cache.c_str(), m_cache.size());
	if(Logger::getInstance()->isLoaded())
	{
		std::ostringstream s;
		if(m_cache.size() > 1)
			s << "[" << formatDate() << "] ";

		s.write(m_cache.c_str(), m_cache.size());
		Logger::getInstance()->iFile(LOGFILE_OUTPUT, s.str(), false);
		if(g_game.isRunning())
			Dispatcher::getInstance().addTask(createTask(boost::bind(&Manager::output, Manager::getInstance(), m_cache)));
	}

	m_cache.clear();
	return c;
}
