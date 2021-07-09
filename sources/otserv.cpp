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
#include "otsystem.h"
#include <signal.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#ifndef WINDOWS
#include <unistd.h>
#include <termios.h>
#else
#include <conio.h>
#endif

#include <boost/config.hpp>

#include "rsa.h"

#include "server.h"
#ifdef __LOGIN_SERVER__
#include "gameservers.h"
#endif
#include "networkmessage.h"

#include "game.h"
#include "chat.h"
#include "tools.h"

#include "protocollogin.h"
#include "protocolgame.h"
#include "protocolold.h"
#include "protocolhttp.h"

#include "status.h"
#include "manager.h"
#ifdef __OTADMIN__
#include "admin.h"
#endif

#include "configmanager.h"
#include "scriptmanager.h"
#include "databasemanager.h"

#include "iologindata.h"
#include "ioban.h"

#include "outfit.h"
#include "vocation.h"
#include "group.h"

#include "quests.h"
#include "raids.h"

#include "monsters.h"
#ifdef __OTSERV_ALLOCATOR__
#include "allocator.h"
#endif
#ifdef __EXCEPTION_TRACER__
#include "exception.h"
#endif
#ifndef __OTADMIN__
#include "textlogger.h"
#endif

#ifdef __NO_BOOST_EXCEPTIONS__
#include <exception>

inline void boost::throw_exception(std::exception const & e)
{
	std::clog << "Boost exception: " << e.what() << std::endl;
}
#endif

RSA g_RSA;
ConfigManager g_config;
Game g_game;
Chat g_chat;

Monsters g_monsters;
Npcs g_npcs;

boost::mutex g_loaderLock;
boost::condition_variable g_loaderSignal;
boost::unique_lock<boost::mutex> g_loaderUniqueLock(g_loaderLock);
std::list<std::pair<uint32_t, uint32_t> > serverIps;

bool argumentsHandler(StringVec args)
{
	StringVec tmp;
	for(StringVec::iterator it = args.begin(); it != args.end(); ++it)
	{
		if((*it) == "--help")
		{
			std::clog << "Usage:\n"
			"\n"
			"\t--config=$1\t\tAlternate configuration file path.\n"
			"\t--data-directory=$1\tAlternate data directory path.\n"
			"\t--ip=$1\t\t\tIP address of the server.\n"
			"\t\t\t\tShould be equal to the global IP.\n"
			"\t--login-port=$1\tPort for login server to listen on.\n"
			"\t--game-port=$1\tPort for game server to listen on.\n"
			"\t--admin-port=$1\tPort for admin server to listen on.\n"
			"\t--manager-port=$1\tPort for manager server to listen on.\n"
			"\t--status-port=$1\tPort for status server to listen on.\n";
#ifndef WINDOWS
			std::clog << "\t--runfile=$1\t\tSpecifies run file. Will contain the pid\n"
			"\t\t\t\tof the server process as long as run status.\n";
#endif
			std::clog << "\t--log=$1\t\tWhole standard output will be logged to\n"
			"\t\t\t\tthis file.\n"
			"\t--closed\t\t\tStarts the server as closed.\n"
			"\t--no-script\t\t\tStarts the server without script system.\n";
			return false;
		}
		
	std::string luajit = "no";
	std::string groundCache = "no";
	std::string serverDiag = "no";
	std::string loginServer = "no";
	std::string useMySQL = "no";
	std::string useMySQLpp = "no";
	std::string useSQLite = "no";
	std::string usePostgreSQL = "no";

#ifdef __LUAJIT__
	luajit = "yes";
#endif

#ifdef __GROUND_CACHE__
	groundCache = "yes";
#endif

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	serverDiag = "yes";
#endif

#ifdef __LOGIN_SERVER__
	loginServer = "yes";
#endif

#ifdef __USE_MYSQL__
	useMySQL = "yes";
#endif

#ifdef __USE_MYSQLPP__
	useMySQLpp = "yes";
#endif

#ifdef __USE_SQLITE__
	useSQLite = "yes";
#endif

#ifdef __USE_PGSQL__
	usePostgreSQL = "yes";
#endif

		if((*it) == "--version" || (*it) == "-v")
		{
			std::clog << "The " << SOFTWARE_NAME << " Version: (" << SOFTWARE_VERSION << "." << MINOR_VERSION << ") - Codename: (" << SOFTWARE_CODENAME << ")\n"
			"Compiled with " << BOOST_COMPILER << " for arch "
			#if defined(__amd64__) || defined(_M_X64)
			"64 Bits"
			#elif defined(__i386__) || defined(_M_IX86) || defined(_X86_)
			"32 Bits"
			#else
			"unk"
			#endif
			" at " << __DATE__ << " " << __TIME__ << ".\n"

			"\n"
			"\t\tFEATURES ON THIS BUILD:\n"
				
				"\t\tLua Version: " << LUA_RELEASE << "\n" <<
				"\t\tXML Version: " << LIBXML_DOTTED_VERSION << "\n" <<
				"\t\tBOOST Version: " << BOOST_LIB_VERSION << "\n" <<
				"\t\tLuaJIT: " << luajit << "\n" <<
				"\t\tGroundCache: " << groundCache << "\n" <<
				"\t\tServerDiag: " << serverDiag << "\n" <<
				"\t\tLoginServer: " << loginServer << "\n" <<
				"\t\tUseMySQL: " << useMySQL << "\n" <<
				"\t\tUseSQLite: " << useSQLite << "\n" <<
				"\t\tUsePostgreSQL: " << usePostgreSQL << "\n\n" <<
				

			"A server developed by: " SOFTWARE_DEVELOPERS ".\n"
			"Visit for updates, support, and resources: " GIT_REPO "\n";
			return false;
		}

		tmp = explodeString((*it), "=");
		if(tmp[0] == "--config")
			g_config.setString(ConfigManager::CONFIG_FILE, tmp[1]);
		else if(tmp[0] == "--data-directory")
			g_config.setString(ConfigManager::DATA_DIRECTORY, tmp[1]);
		else if(tmp[0] == "--logs-directory")
			g_config.setString(ConfigManager::LOGS_DIRECTORY, tmp[1]);
		else if(tmp[0] == "--ip")
			g_config.setString(ConfigManager::IP, tmp[1]);
		else if(tmp[0] == "--login-port")
			g_config.setNumber(ConfigManager::LOGIN_PORT, atoi(tmp[1].c_str()));
		else if(tmp[0] == "--game-port")
			g_config.setNumber(ConfigManager::GAME_PORT, atoi(tmp[1].c_str()));
		else if(tmp[0] == "--admin-port")
			g_config.setNumber(ConfigManager::ADMIN_PORT, atoi(tmp[1].c_str()));
		else if(tmp[0] == "--manager-port")
			g_config.setNumber(ConfigManager::MANAGER_PORT, atoi(tmp[1].c_str()));
		else if(tmp[0] == "--status-port")
			g_config.setNumber(ConfigManager::STATUS_PORT, atoi(tmp[1].c_str()));
#ifndef WINDOWS
		else if(tmp[0] == "--runfile" || tmp[0] == "--run-file" || tmp[0] == "--pidfile" || tmp[0] == "--pid-file")
			g_config.setString(ConfigManager::RUNFILE, tmp[1]);
#endif
		else if(tmp[0] == "--log")
			g_config.setString(ConfigManager::OUTPUT_LOG, tmp[1]);
#ifndef WINDOWS
		else if(tmp[0] == "--daemon" || tmp[0] == "-d")
			g_config.setBool(ConfigManager::DAEMONIZE, true);
#endif
		else if(tmp[0] == "--closed")
			g_config.setBool(ConfigManager::START_CLOSED, true);
		else if(tmp[0] == "--no-script" || tmp[0] == "--noscript")
			g_config.setBool(ConfigManager::SCRIPT_SYSTEM, false);
	}

	return true;
}

#ifndef WINDOWS
int32_t OTSYS_getch()
{
	struct termios oldt;
	tcgetattr(STDIN_FILENO, &oldt);

	struct termios newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	int32_t ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

void signalHandler(int32_t sig)
{
	switch(sig)
	{
		case SIGHUP:
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::saveGameState, &g_game, (uint8_t)SAVE_PLAYERS | (uint8_t)SAVE_MAP | (uint8_t)SAVE_STATE)));
			break;

		case SIGTRAP:
			g_game.cleanMap();
			break;

		case SIGCHLD:
			g_game.proceduralRefresh();
			break;

		case SIGUSR1:
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::setGameState, &g_game, GAMESTATE_CLOSED)));
			break;

		case SIGUSR2:
			g_game.setGameState(GAMESTATE_NORMAL);
			break;

		case SIGCONT:
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::reloadInfo, &g_game, RELOAD_ALL, 0, false)));
			break;

		case SIGQUIT:
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::setGameState, &g_game, GAMESTATE_SHUTDOWN)));
			break;

		case SIGTERM:
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::shutdown, &g_game)));

			Dispatcher::getInstance().stop();
			Scheduler::getInstance().stop();
			break;

		default:
			break;
	}
}

void runfileHandler(void)
{
	std::ofstream runfile(g_config.getString(ConfigManager::RUNFILE).c_str(), std::ios::trunc | std::ios::out);
	runfile.close();
}
#else
int32_t OTSYS_getch()
{
	return (int32_t)getchar();
}
#endif

void allocationHandler()
{
	puts("Allocation failed, server out of memory!\nDecrease size of your map or compile in a 64-bit mode.");
	OTSYS_getch();
	std::exit(-1);
}

void startupErrorMessage(std::string error = "")
{
	// we will get a crash here as the threads aren't going down smoothly
	if(error.length() > 0)
		std::clog << std::endl << "> ERROR: " << error << std::endl;

	OTSYS_getch();
	std::exit(-1);
}

void otserv(StringVec args, ServiceManager* services);
int main(int argc, char* argv[])
{
	std::srand((uint32_t)OTSYS_TIME());
	StringVec args = StringVec(argv, argv + argc);
	if(argc > 1 && !argumentsHandler(args))
		return 0;

	std::set_new_handler(allocationHandler);
	ServiceManager servicer;
	g_config.startup();

#ifndef WINDOWS
	// ignore sigpipe...
	struct sigaction sigh;
	sigh.sa_handler = SIG_IGN;
	sigh.sa_flags = 0;

	sigemptyset(&sigh.sa_mask);
	sigaction(SIGPIPE, &sigh, NULL);

	// register signals
	signal(SIGHUP, signalHandler); //save
	signal(SIGTRAP, signalHandler); //clean
	signal(SIGCHLD, signalHandler); //refresh
	signal(SIGUSR1, signalHandler); //close server
	signal(SIGUSR2, signalHandler); //open server
	signal(SIGCONT, signalHandler); //reload all
	signal(SIGQUIT, signalHandler); //save & shutdown
	signal(SIGTERM, signalHandler); //shutdown
#endif

	OutputHandler::getInstance();
	Dispatcher::getInstance().addTask(createTask(boost::bind(otserv, args, &servicer)));
	g_loaderSignal.wait(g_loaderUniqueLock);

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	if(servicer.is_running())
	{
		std::clog << ">> " << g_config.getString(ConfigManager::SERVER_NAME) << " server Online!" << std::endl << std::endl;
		servicer.run();
	}
	else
		std::clog << ">> " << g_config.getString(ConfigManager::SERVER_NAME) << " server Offline! No services available..." << std::endl << std::endl;

	Dispatcher::getInstance().exit();
	Scheduler::getInstance().exit();

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	exit(0);
	return 0;
}

void otserv(StringVec, ServiceManager* services)
{
	std::srand((uint32_t)OTSYS_TIME());
#if defined(WINDOWS)
	SetConsoleTitle(SOFTWARE_NAME);
#endif

	g_game.setGameState(GAMESTATE_STARTUP);
#if !defined(WINDOWS) && !defined(__ROOT_PERMISSION__)
	if(!getuid() || !geteuid())
	{
		std::clog << "> WARNING: " "The " << SOFTWARE_NAME << " has been executed as super user! It is "
			<< "recommended to run as a normal user." << std::endl << "Continue? (y/N)" << std::endl;
		char buffer = OTSYS_getch();
		if(buffer != 121 && buffer != 89)
			startupErrorMessage("Aborted.");
	}
#endif

	std::string luajit = "no";
	std::string groundCache = "no";
	std::string serverDiag = "no";
	std::string loginServer = "no";
	std::string useMySQL = "no";
	std::string useMySQLpp = "no";
	std::string useSQLite = "no";
	std::string usePostgreSQL = "no";

#ifdef __LUAJIT__
	luajit = "yes";
#endif

#ifdef __GROUND_CACHE__
	groundCache = "yes";
#endif

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	serverDiag = "yes";
#endif

#ifdef __LOGIN_SERVER__
	loginServer = "yes";
#endif

#ifdef __USE_MYSQL__
	useMySQL = "yes";
#endif

#ifdef __USE_MYSQLPP__
	useMySQLpp = "yes";
#endif

#ifdef __USE_SQLITE__
	useSQLite = "yes";
#endif

#ifdef __USE_PGSQL__
	usePostgreSQL = "yes";
#endif

	std::clog << "The " << SOFTWARE_NAME << " Version: (" << SOFTWARE_VERSION << "." << MINOR_VERSION << ") - Codename: (" << SOFTWARE_CODENAME << ")" << std::endl
		<< "Compiled with " << BOOST_COMPILER << " for arch "
		#if defined(__amd64__) || defined(_M_X64)
		"64 Bits"
		#elif defined(__i386__) || defined(_M_IX86) || defined(_X86_)
		"32 Bits"
		#else
		"unk"
		#endif
			" at " << __DATE__ << " " << __TIME__ << std::endl << std::endl <<

			"\t\tFEATURES ON THIS BUILD: "<< std::endl << std::endl <<
				
				"\t\tLua Version: " << LUA_RELEASE << "\n" <<
				"\t\tXML Version: " << LIBXML_DOTTED_VERSION << "\n" <<
				"\t\tBOOST Version: " << BOOST_LIB_VERSION << "\n" <<
				"\t\tLuaJIT: " << luajit << "\n" <<
				"\t\tGroundCache: " << groundCache << "\n" <<
				"\t\tServerDiag: " << serverDiag << "\n" <<
				"\t\tLoginServer: " << loginServer << "\n" <<
				"\t\tUseMySQL: " << useMySQL << "\n" <<
				"\t\tUseSQLite: " << useSQLite << "\n" <<
				"\t\tUsePostgreSQL: " << usePostgreSQL << "\n\n" <<
				

			"A server developed by: " SOFTWARE_DEVELOPERS "." << std::endl<<
			"Visit for updates, support, and resources: " GIT_REPO << std::endl;
	std::ostringstream ss;
#ifdef __DEBUG__
	ss << " GLOBAL";
#endif
#ifdef __DEBUG_MOVESYS__
	ss << " MOVESYS";
#endif
#ifdef __DEBUG_CHAT__
	ss << " CHAT";
#endif
#ifdef __DEBUG_HOUSES__
	ss << " HOUSES";
#endif
#ifdef __DEBUG_LUASCRIPTS__
	ss << " LUA-SCRIPTS";
#endif
#ifdef __DEBUG_MAILBOX__
	ss << " MAILBOX";
#endif
#ifdef __DEBUG_NET__
	ss << " NET";
#endif
#ifdef __DEBUG_NET_DETAIL__
	ss << " NET-DETAIL";
#endif
#ifdef __DEBUG_RAID__
	ss << " RAIDS";
#endif
#ifdef __DEBUG_SCHEDULER__
	ss << " SCHEDULER";
#endif
#ifdef __DEBUG_SPAWN__
	ss << " SPAWNS";
#endif
#ifdef __SQL_QUERY_DEBUG__
	ss << " SQL-QUERIES";
#endif

	std::string debug = ss.str();
	if(!debug.empty())
		std::clog << ">> Debugging:" << debug << "." << std::endl;

	std::clog << ">> Loading config (" << g_config.getString(ConfigManager::CONFIG_FILE) << ")" << std::endl;
	if(!g_config.load())
		startupErrorMessage("Unable to load " + g_config.getString(ConfigManager::CONFIG_FILE) + "!");

#ifndef WINDOWS
	if(g_config.getBool(ConfigManager::DAEMONIZE))
	{
		std::clog << "> Daemonization... ";
		if(fork())
		{
			std::clog << "succeed, bye!" << std::endl;
			exit(0);
		}
		else
			std::clog << "failed, continuing." << std::endl;
	}

#endif
	// silently append trailing slash
	std::string path = g_config.getString(ConfigManager::DATA_DIRECTORY);
	g_config.setString(ConfigManager::DATA_DIRECTORY, path.erase(path.find_last_not_of("/") + 1) + "/");

	path = g_config.getString(ConfigManager::LOGS_DIRECTORY);
	g_config.setString(ConfigManager::LOGS_DIRECTORY, path.erase(path.find_last_not_of("/") + 1) + "/");

	std::clog << ">> Opening logs" << std::endl;
	Logger::getInstance()->open();

	IntegerVec cores = vectorAtoi(explodeString(g_config.getString(ConfigManager::CORES_USED), ","));
	if(cores[0] != -1)
	{
#ifdef WINDOWS
		int32_t mask = 0;
		for(IntegerVec::iterator it = cores.begin(); it != cores.end(); ++it)
			mask += 1 << (*it);

		SetProcessAffinityMask(GetCurrentProcess(), mask);
	}

	std::ostringstream mutexName;
	mutexName << "otxserver_" << g_config.getNumber(ConfigManager::WORLD_ID);

	CreateMutex(NULL, FALSE, mutexName.str().c_str());
	if(GetLastError() == ERROR_ALREADY_EXISTS)
		startupErrorMessage("Another instance of The OTX Server is already running with the same worldId.\nIf you want to run multiple servers, please change the worldId in configuration file.");

	std::string defaultPriority = asLowerCaseString(g_config.getString(ConfigManager::DEFAULT_PRIORITY));
	if(defaultPriority == "realtime" || defaultPriority == "real")
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	else if(defaultPriority == "high" || defaultPriority == "regular")
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	else if(defaultPriority == "higher" || defaultPriority == "above" || defaultPriority == "normal")
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

#else
#ifndef __APPLE__
		cpu_set_t mask;
		CPU_ZERO(&mask);
		for(IntegerVec::iterator it = cores.begin(); it != cores.end(); ++it)
			CPU_SET((*it), &mask);

		sched_setaffinity(getpid(), (int32_t)sizeof(mask), &mask);
	}
#endif

	std::string runPath = g_config.getString(ConfigManager::RUNFILE);
	if(!runPath.empty() && runPath.length() > 2)
	{
		std::ofstream runFile(runPath.c_str(), std::ios::trunc | std::ios::out);
		runFile << getpid();
		runFile.close();
		atexit(runfileHandler);
	}

	if(!nice(g_config.getNumber(ConfigManager::NICE_LEVEL))) {}
#endif

	std::clog << ">> Loading Password encryption:" << std::endl;
	std::string encryptionType = asLowerCaseString(g_config.getString(ConfigManager::ENCRYPTION_TYPE));
	if(encryptionType == "sha1")
	{
		g_config.setNumber(ConfigManager::ENCRYPTION, ENCRYPTION_SHA1);
		std::clog << ">>> Using (SHA1) encryption ... (done)." << std::endl;
	}
	else
	{
		g_config.setNumber(ConfigManager::ENCRYPTION, ENCRYPTION_PLAIN);
		std::clog << ">>> Using plaintext encryption" << std::endl << std::endl
			<< ">>> WARNING: This method is completely unsafe!" << std::endl
			<< ">>> Please set encryptionType = \"sha1\" (or any other available method) in config.lua" << std::endl;
		boost::this_thread::sleep(boost::posix_time::seconds(15));
	}

	std::clog << ">> Loading RSA key" << std::endl;
	const char* p("14299623962416399520070177382898895550795403345466153217470516082934737582776038882967213386204600674145392845853859217990626450972452084065728686565928113");
	const char* q("7630979195970404721891201847792002125535401292779123937207447574596692788513647179235335529307251350570728407373705564708871762033017096809910315212884101");
	const char* d("46730330223584118622160180015036832148732986808519344675210555262940258739805766860224610646919605860206328024326703361630109888417839241959507572247284807035235569619173792292786907845791904955103601652822519121908367187885509270025388641700821735345222087940578381210879116823013776808975766851829020659073");

	g_RSA.initialize(p, q, d);

	std::clog << ">> Starting SQL connection" << std::endl;
	Database* db = Database::getInstance();
	if(db && db->connect())
	{
		std::clog << ">> Running Database Manager" << std::endl;
		if(DatabaseManager::getInstance()->isDatabaseSetup())
		{
			uint32_t version = 0;
			do
			{
				version = DatabaseManager::getInstance()->updateDatabase();
				if(version == 0)
					break;

				std::clog << "> Database has been updated to version: " << version << "." << std::endl;
			}
			while(version < VERSION_DATABASE);
		}
		else
			startupErrorMessage("The database you have specified in config.lua is empty, please import schemas/<engine>.sql to the database.");

		DatabaseManager::getInstance()->checkTriggers();
		DatabaseManager::getInstance()->checkEncryption();
		if(g_config.getBool(ConfigManager::OPTIMIZE_DATABASE) && !DatabaseManager::getInstance()->optimizeTables())
			std::clog << "> No tables were optimized." << std::endl;
	}
	else
		startupErrorMessage("Couldn't estabilish connection to SQL database!");

	std::clog << ">> Checking for duplicated items" << std::endl;
	std::ostringstream query;
	query << "SELECT unitedItems.serial, COUNT(1) AS duplicatesCount FROM (SELECT serial FROM `player_items` UNION ALL SELECT serial FROM `player_depotitems` UNION ALL SELECT serial FROM `tile_items`) unitedItems GROUP BY unitedItems.serial HAVING COUNT(1) > 1;";
	std::string logText = "";

	DBResult* result;
	bool duplicated = false;

	if((result = db->storeQuery(query.str())))
	{
		do
		{
			std::string serial = result->getDataString("serial");
			if(serial != "" && serial.length() > 1)
			{
				DBResult* result_;
				std::ostringstream query_playeritems;
				query_playeritems << "SELECT `player_id`, `pid`, `sid`, `itemtype`, `count`, `attributes`, `serial` FROM `player_items` WHERE `serial` = " << db->escapeString(serial) << ";";
				if((result_ = db->storeQuery(query_playeritems.str())))
				{
					duplicated = true;
					do
					{
						std::string name;
						IOLoginData::getInstance()->getNameByGuid((uint32_t)result_->getDataInt("player_id"), name, false);
						std::clog << ">> Deleted item from 'player_items' with SERIAL: [" << serial.c_str() << "] PLAYER: [" << result_->getDataInt("player_id") << "] PLAYER NAME: [" << name.c_str() << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]" << std::endl;
						std::ostringstream logText;
						logText << "Deleted item from 'player_items' with SERIAL: [" << serial << "] PLAYER: [" << result_->getDataInt("player_id") << "] PLAYER NAME: [" << name << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]";
						Logger::getInstance()->eFile("anti_dupe.log", logText.str(), true);
					}
					while(result_->next());
					result_->free();
				}

				query_playeritems.clear();
				std::ostringstream query_playerdepotitems;
				query_playerdepotitems << "SELECT `player_id`, `sid`, `pid`, `itemtype`, `count`, `attributes`, `serial` FROM `player_depotitems` WHERE `serial` = " << db->escapeString(serial) << ";";
				if((result_ = db->storeQuery(query_playerdepotitems.str())))
				{
					duplicated = true;
					do
					{
						std::string name;
						IOLoginData::getInstance()->getNameByGuid((uint32_t)result_->getDataInt("player_id"), name, false);
						std::clog << ">> Deleted item from 'player_depotitems' with SERIAL: [" << serial.c_str() << "] PLAYER: [" << result_->getDataInt("player_id") << "] PLAYER NAME: [" << name.c_str() << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]" << std::endl;
						std::ostringstream logText;
						logText << "Deleted item from 'player_depotitems' with SERIAL: [" << serial << "] PLAYER: [" << result_->getDataInt("player_id") << "] PLAYER NAME: [" << name << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]";
						Logger::getInstance()->eFile("anti_dupe.log", logText.str(), true);
					}
					while(result_->next());
					result_->free();
				}

				query_playerdepotitems.clear();
				std::ostringstream query_tileitems;
				query_tileitems << "SELECT `tile_id`, `world_id`, `sid`, `pid`, `itemtype`, `count`, `attributes`, `serial` FROM `tile_items` WHERE `serial` = " << db->escapeString(serial) << ";";
				if((result_ = db->storeQuery(query_tileitems.str())))
				{
					duplicated = true;
					do
					{
						std::clog << ">> Deleted item from 'tile_items' with SERIAL: [" << serial.c_str() << "] TILE ID: [" << result_->getDataInt("tile_id") << "] WORLD ID: [" << result_->getDataInt("world_id") << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]" << std::endl;
						std::ostringstream logText;
						logText << "Deleted item from 'tile_items' with SERIAL: [" << serial << "] TILE ID: [" << result_->getDataInt("tile_id") << "] WORLD ID: [" << result_->getDataInt("world_id") << "] ITEM: [" << result_->getDataInt("itemtype") << "] COUNT: [" << result_->getDataInt("count") << "]";
						Logger::getInstance()->eFile("anti_dupe.log", logText.str(), true);
					}
					while(result_->next());
					result_->free();
				}

				query_tileitems.clear();
				std::ostringstream query_deletepi;
				query_deletepi << "DELETE FROM `player_items` WHERE `serial` = " << db->escapeString(serial) << ";";
				if(!db->query(query_deletepi.str()))
					std::clog << ">> Cannot delete duplicated items from 'player_items'!" << std::endl;

				query_deletepi.clear();
				std::ostringstream query_deletedi;
				query_deletedi << "DELETE FROM `player_depotitems` WHERE `serial` = " << db->escapeString(serial) << ";";
				if(!db->query(query_deletedi.str()))
					std::clog << ">> Cannot delete duplicated items from 'player_depotitems'!" << std::endl;

				query_deletedi.clear();
				std::ostringstream query_deleteti;
				query_deleteti << "DELETE FROM `tile_items` WHERE `serial` = " << db->escapeString(serial) << ";";
				if(!db->query(query_deleteti.str()))
					std::clog << ">> Cannot delete duplicated items from 'tile_items'!" << std::endl;

				query_deleteti.clear();
			}
		}
		while(result->next());
		result->free();

		if(duplicated)
			std::clog << ">> Duplicated items successfully removed." << std::endl;
	} else {
		std::clog << ">> There wasn't duplicated items in the server." << std::endl;
	}

	std::clog << ">> Loading items (OTB)" << std::endl;
	if(Item::items.loadFromOtb(getFilePath(FILE_TYPE_OTHER, "items/items.otb")))
		startupErrorMessage("Unable to load items (OTB)!");

	std::clog << ">> Loading items (XML)" << std::endl;
	if(!Item::items.loadFromXml())
	{
		std::clog << "Unable to load items (XML)! Continue? (y/N)" << std::endl;
		char buffer = OTSYS_getch();
		if(buffer != 121 && buffer != 89)
			startupErrorMessage("Unable to load items (XML)!");
	}

	std::clog << ">> Loading groups" << std::endl;
	if(!Groups::getInstance()->loadFromXml())
		startupErrorMessage("Unable to load groups!");

	std::clog << ">> Loading outfits" << std::endl;
	if(!Outfits::getInstance()->loadFromXml())
		startupErrorMessage("Unable to load outfits!");

	std::clog << ">> Loading quests" << std::endl;
	if(!Quests::getInstance()->loadFromXml())
		startupErrorMessage("Unable to load quests!");

	std::clog << ">> Loading raids" << std::endl;
	if(!Raids::getInstance()->loadFromXml())
		startupErrorMessage("Unable to load raids!");

	std::clog << ">> Loading vocations" << std::endl;
	if(!Vocations::getInstance()->loadFromXml())
		startupErrorMessage("Unable to load vocations!");

	std::clog << ">> Loading chat channels" << std::endl;
	if(!g_chat.loadFromXml())
		startupErrorMessage("Unable to load chat channels!");

	std::clog << ">> Loading experience stages" << std::endl;
	if(!g_game.loadExperienceStages())
		startupErrorMessage("Unable to load experience stages!");

	std::clog << ">> Loading mods:" << std::endl;
	if(!ScriptManager::getInstance()->loadMods())
		startupErrorMessage();

	if(g_config.getBool(ConfigManager::SCRIPT_SYSTEM))
	{
		std::clog << ">> Loading script systems:" << std::endl;
		if(!ScriptManager::getInstance()->loadSystem())
			startupErrorMessage();
	}
	else
		ScriptManager::getInstance();

	#ifdef __LOGIN_SERVER__
	std::clog << ">> Loading game servers" << std::endl;
	if(!GameServers::getInstance()->loadFromXml(true))
		startupErrorMessage("Unable to load game servers!");
	#endif

	std::clog << ">> Loading monsters" << std::endl;
	if(!g_monsters.loadFromXml())
	{
		std::clog << "Unable to load monsters! Continue? (y/N)" << std::endl;
		char buffer = OTSYS_getch();
		if(buffer != 121 && buffer != 89)
			startupErrorMessage("Unable to load monsters!");
	}

	if(fileExists(getFilePath(FILE_TYPE_OTHER, "npc/npcs.xml")))
	{
		std::clog << ">> Loading npcs" << std::endl;
		if (!g_npcs.loadFromXml())
		{
			std::clog << "Unable to load npcs! Continue? (y/N)" << std::endl;
			char buffer = OTSYS_getch();
			if (buffer != 121 && buffer != 89)
				startupErrorMessage("Unable to load npcs!");
		}
	}

	std::clog << ">> Loading raids" << std::endl;
	Raids::getInstance()->loadFromXml();

	std::clog << ">> Loading map and spawns..." << std::endl;
	if(!g_game.loadMap(g_config.getString(ConfigManager::MAP_NAME)))
		startupErrorMessage();

	std::clog << ">> Checking world type... ";
	std::string worldType = asLowerCaseString(g_config.getString(ConfigManager::WORLD_TYPE));
	if(worldType == "open" || worldType == "2" || worldType == "openpvp")
	{
		g_game.setWorldType(WORLDTYPE_OPEN);
		std::clog << "Open PvP" << std::endl;
	}
	else if(worldType == "optional" || worldType == "1" || worldType == "optionalpvp")
	{
		g_game.setWorldType(WORLDTYPE_OPTIONAL);
		std::clog << "Optional PvP" << std::endl;
	}
	else if(worldType == "hardcore" || worldType == "3" || worldType == "hardcorepvp")
	{
		g_game.setWorldType(WORLDTYPE_HARDCORE);
		std::clog << "Hardcore PvP" << std::endl;
	}
	else
	{
		std::clog << std::endl;
		startupErrorMessage("Unknown world type: " + g_config.getString(ConfigManager::WORLD_TYPE));
	}

	std::clog << ">> Starting to dominate the world... done." << std::endl;

	std::clog << ">> Initializing game state and binding services:" << std::endl;
	g_game.setGameState(GAMESTATE_INIT);

	services->add<ProtocolStatus>(g_config.getNumber(ConfigManager::STATUS_PORT));
	services->add<ProtocolManager>(g_config.getNumber(ConfigManager::MANAGER_PORT));
#ifdef __OTADMIN__
	services->add<ProtocolAdmin>(g_config.getNumber(ConfigManager::ADMIN_PORT));
#endif
	//services->add<ProtocolHTTP>(8080);
	if (
#ifdef __LOGIN_SERVER__
		true
#else
		!g_config.getBool(ConfigManager::LOGIN_ONLY_LOGINSERVER)
#endif
		)
	{
		services->add<ProtocolLogin>(g_config.getNumber(ConfigManager::LOGIN_PORT));
	}

	services->add<ProtocolOld>(g_config.getNumber(ConfigManager::LOGIN_PORT));
	IntegerVec games = vectorAtoi(explodeString(g_config.getString(ConfigManager::GAME_PORT), ","));
	for (IntegerVec::const_iterator it = games.begin(); it != games.end(); ++it)
	{
		services->add<ProtocolGame>(*it);
		break; // CRITICAL: more ports are causing crashes- either find the issue or drop the "feature"
	}

	std::pair<uint32_t, uint32_t> IpNetMask;
	IpNetMask.first = inet_addr("127.0.0.1");
	IpNetMask.second = 0xFFFFFFFF;
	serverIps.push_back(IpNetMask);

	char szHostName[128];
	if (gethostname(szHostName, 128) == 0) {
		hostent* he = gethostbyname(szHostName);
		if (he) {
			unsigned char** addr = (unsigned char**)he->h_addr_list;
			while (addr[0] != nullptr) {
				IpNetMask.first = *(uint32_t*)(*addr);
				IpNetMask.second = 0xFFFFFFFF;
				serverIps.push_back(IpNetMask);
				addr++;
			}
		}
	}

	std::string ip = g_config.getString(ConfigManager::IP);

	uint32_t resolvedIp = inet_addr(ip.c_str());
	if (resolvedIp == INADDR_NONE) {
		struct hostent* he = gethostbyname(ip.c_str());
		if (!he) {
			std::ostringstream ss;
			ss << "ERROR: Cannot resolve " << ip << "!" << std::endl;
			startupErrorMessage(ss.str());
			return;
		}
		resolvedIp = *(uint32_t*)he->h_addr;
	}

	IpNetMask.first = resolvedIp;
	IpNetMask.second = 0;
	serverIps.push_back(IpNetMask);

	std::clog << std::endl << ">> Everything smells good, server is starting up..." << std::endl;
	g_game.start(services);
	g_game.setGameState(g_config.getBool(ConfigManager::START_CLOSED) ? GAMESTATE_CLOSED : GAMESTATE_NORMAL);
	g_loaderSignal.notify_all();
}
