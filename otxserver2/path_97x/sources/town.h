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

#ifndef __TOWN__
#define __TOWN__
#include "otsystem.h"

class Position;
class Town
{
	public:
		Town(uint32_t townId) {id = townId;}
		virtual ~Town() {}

		Position getPosition() const {return position;}
		std::string getName() const {return name;}

		void setPosition(const Position& pos) {position = pos;}
		void setName(const std::string& townName) {name = townName;}

		uint32_t getID() const {return id;}

	private:
		uint32_t id;
		std::string name;
		Position position;
};

typedef std::map<uint32_t, Town*> TownMap;
class Towns
{
	public:
		static Towns* getInstance()
		{
			static Towns instance;
			return &instance;
		}

		bool addTown(uint32_t townId, Town* town)
		{
			TownMap::iterator it = townMap.find(townId);
			if(it != townMap.end())
				return false;

			townMap[townId] = town;
			return true;
		}

		Town* getTown(const std::string& townName)
		{
			for(TownMap::iterator it = townMap.begin(); it != townMap.end(); ++it)
			{
				if(boost::algorithm::iequals(it->second->getName(), townName))
					return it->second;
			}

			return NULL;
		}

		Town* getTown(uint32_t townId)
		{
			TownMap::iterator it = townMap.find(townId);
			if(it != townMap.end())
				return it->second;

			return NULL;
		}

		TownMap::const_iterator getFirstTown() const {return townMap.begin();}
		TownMap::const_iterator getLastTown() const {return townMap.end();}

	private:
		TownMap townMap;
};
#endif
