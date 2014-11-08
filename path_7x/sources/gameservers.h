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

#ifndef __GAMESERVER__
#define __GAMESERVER__
#include "otsystem.h"
#include "const.h"
#include "resources.h"

class GameServer
{
	public:
		GameServer(): name("TheOTXServer"), address(LOCALHOST),
			versionMin(CLIENT_VERSION_MIN), versionMax(CLIENT_VERSION_MAX) {}
		GameServer(std::string _name, uint32_t _versionMin, uint32_t _versionMax, uint32_t _address, std::vector<int32_t> _ports):
			name(_name), address(_address), versionMin(_versionMin), versionMax(_versionMax), ports(_ports) {}
		virtual ~GameServer() {}

		std::string getName() const {return name;}
		uint32_t getVersionMin() const {return versionMin;}
		uint32_t getVersionMax() const {return versionMax;}

		uint32_t getAddress() const {return address;}
		std::vector<int32_t> getPorts() const {return ports;}

	protected:
		std::string name;
		uint32_t address, versionMin, versionMax;
		std::vector<int32_t> ports;
};

typedef std::map<uint32_t, GameServer*> GameServersMap;
class GameServers
{
	public:
		GameServers() {}
		virtual ~GameServers() {clear();}

		static GameServers* getInstance()
		{
			static GameServers instance;
			return &instance;
		}

		bool loadFromXml(bool result);
		bool reload();

		GameServer* getServerById(uint32_t id) const;

		GameServersMap::const_iterator getFirstServer() const {return serverList.begin();}
		GameServersMap::const_iterator getLastServer() const {return serverList.end();}

	protected:
		void clear();

		GameServersMap serverList;
};
#endif
