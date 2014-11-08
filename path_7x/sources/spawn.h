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

#ifndef __SPAWN__
#define __SPAWN__
#include "otsystem.h"

#include "templates.h"
#include "position.h"

#include "tile.h"
#include "monster.h"

class Spawn;
typedef std::list<Spawn*> SpawnList;

class Spawns
{
	public:
		virtual ~Spawns();
		static Spawns* getInstance()
		{
			static Spawns instance;
			return &instance;
		}

		bool isInZone(const Position& centerPos, int32_t radius, const Position& pos);

		bool loadFromXml(const std::string& _filename);
		bool parseSpawnNode(xmlNodePtr p, bool checkDuplicate);

		void startup();
		void clear();

		bool isLoaded() const {return loaded;}
		bool isStarted() const {return started;}

	private:
		Spawns();
		SpawnList spawnList;

		typedef std::list<Npc*> NpcList;
		NpcList npcList;

		bool loaded, started;
		std::string filename;
};

struct spawnBlock_t
{
	MonsterType* mType;
	Position pos;
	Direction direction;

	uint32_t interval;
	int64_t lastSpawn;
};

class Spawn
{
	public:
		Spawn(const Position& _pos, int32_t _radius);
		virtual ~Spawn();

		bool addMonster(const std::string& _name, const Position& _pos, Direction _dir, uint32_t _interval);
		void removeMonster(Monster* monster);

		Position getPosition() const {return centerPos;}
		uint32_t getInterval() const {return interval;}

		void startEvent();
		void stopEvent();

		void startup();
		bool isInSpawnZone(const Position& pos) {return Spawns::getInstance()->isInZone(centerPos, radius, pos);}

	private:
		uint32_t interval, checkSpawnEvent;

		Position centerPos;
		int32_t radius, despawnRange, despawnRadius;

		void checkSpawn();
		bool spawnMonster(uint32_t spawnId, MonsterType* mType, const Position& pos, Direction dir, bool startup = false);

		bool findPlayer(const Position& pos);

		//map of creatures in the spawn
		typedef std::map<uint32_t, spawnBlock_t> SpawnMap;
		SpawnMap spawnMap;

		//map of the spawned creatures
		typedef std::multimap<uint32_t, Monster*, std::less<uint32_t> > SpawnedMap;
		typedef SpawnedMap::value_type SpawnedPair;
		SpawnedMap spawnedMap;
};
#endif
