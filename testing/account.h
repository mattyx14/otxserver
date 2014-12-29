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

#ifndef __ACCOUNT__
#define __ACCOUNT__
#include "otsystem.h"
#ifndef __LOGIN_SERVER__

typedef std::list<std::string> Characters;
#else
class GameServer;
struct Character
{
	Character(): server(NULL), status(0) {}
	Character(const std::string& _name, GameServer* _server, int8_t _status):
		name(_name), server(_server), status(_status) {}

	std::string name;
	GameServer* server;
	int8_t status;
};

typedef std::map<std::string, Character> Characters;
#endif

class Account
{
	public:
		Account() {premiumDays = warnings = number = lastDay = 0;}
		virtual ~Account() {charList.clear();}

		uint16_t premiumDays, warnings;
		uint32_t number, lastDay;
		std::string name, password, recoveryKey, salt;
		Characters charList;
};
#endif
