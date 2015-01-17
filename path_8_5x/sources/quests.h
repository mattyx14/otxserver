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

#ifndef __QUESTS__
#define __QUESTS__
#include "otsystem.h"

#include "networkmessage.h"
#include "player.h"

typedef std::map<uint32_t, std::string> StateMap;
class Mission
{
	public:
		Mission(std::string _name, std::string _state, std::string _storageId, int32_t _startValue, int32_t _endValue, bool _notify)
		{
			name = _name;
			state = _state;
			startValue = _startValue;
			endValue = _endValue;
			storageId = _storageId;
			notify = _notify;
		}
		virtual ~Mission() {states.clear();}

		void newState(uint32_t id, const std::string& description) {states[id] = description;}

		bool isStarted(Player* player);
		bool isCompleted(Player* player);
		bool isNotifying() const {return notify;}

		std::string getName(Player* player) {return (isCompleted(player) ? (name + " (completed)") : name);}
		std::string getDescription(Player* player);

		int32_t getStartValue() {return startValue;}
		int32_t getEndValue() {return endValue;}

		const std::string& getStorageId() const {return storageId;}

	private:
		std::string parseStorages(std::string state, std::string value, Player* player);

		std::string name, state;
		StateMap states;

		bool notify;
		int32_t startValue, endValue;
		std::string storageId;
};

typedef std::list<Mission*> MissionList;
class Quest
{
	public:
		Quest(std::string _name, uint16_t _id, std::string _storageId, int32_t _storageValue)
		{
			name = _name;
			id = _id;
			storageValue = _storageValue;
			storageId = _storageId;
		}
		virtual ~Quest();

		void newMission(Mission* mission) {missions.push_back(mission);}

		bool isStarted(Player* player);
		bool isCompleted(Player* player) const;

		uint16_t getId() const {return id;}
		const std::string& getName() const {return name;}
		const std::string& getStorageId() const {return storageId;}
		int32_t getStorageValue() const {return storageValue;}
		uint16_t getMissionCount(Player* player);

		inline MissionList::const_iterator getFirstMission() const {return missions.begin();}
		inline MissionList::const_iterator getLastMission() const {return missions.end();}

	private:
		std::string name;
		MissionList missions;

		uint16_t id;
		int32_t storageValue;
		std::string storageId;
};

typedef std::list<Quest*> QuestList;
class Quests
{
	public:
		virtual ~Quests() {clear();}
		static Quests* getInstance()
		{
			static Quests instance;
			return &instance;
		}

		void clear();
		bool reload();

		bool loadFromXml();
		bool parseQuestNode(xmlNodePtr p, bool checkDuplicate);

		bool isQuestStorage(const std::string& key, const std::string& value, bool notification) const;
		uint16_t getQuestCount(Player* player);

		inline QuestList::const_iterator getFirstQuest() const {return quests.begin();}
		inline QuestList::const_iterator getLastQuest() const {return quests.end();}

		Quest* getQuestById(uint16_t id) const;

	private:
		Quests() {m_lastId = 1;}

		QuestList quests;
		uint16_t m_lastId;
};
#endif
