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
#include "raids.h"

#include "player.h"

#include "game.h"
#include "configmanager.h"

extern Game g_game;
extern ConfigManager g_config;

LuaInterface ScriptEvent::m_interface("Raid Interface");

Raids::Raids()
{
	running = NULL;
	loaded = started = false;
	lastRaidEnd = checkRaidsEvent = 0;
}

bool Raids::parseRaidNode(xmlNodePtr raidNode, bool checkDuplicate, FileType_t pathing)
{
	if(xmlStrcmp(raidNode->name, (const xmlChar*)"raid"))
		return false;

	int32_t intValue;
	std::string strValue;
	if(!readXMLString(raidNode, "name", strValue))
	{
		std::clog << "[Error - Raids::parseRaidNode] name tag missing for raid." << std::endl;
		return false;
	}

	std::string name = strValue;
	if(!readXMLInteger(raidNode, "interval2", intValue) || intValue <= 0)
	{
		std::clog << "[Error - Raids::parseRaidNode] interval2 tag missing or divided by 0 for raid " << name << std::endl;
		return false;
	}

	uint32_t interval = intValue * 60;
	std::string file;
	if(!readXMLString(raidNode, "file", strValue))
	{
		file = name + ".xml";
		std::clog << "[Warning - Raids::parseRaidNode] file tag missing for raid " << name << ", using default: " << file << std::endl;
	}
	else
		file = strValue;

	file = getFilePath(pathing, "raids/" + file);
	uint64_t margin = 0;
	if(!readXMLInteger(raidNode, "margin", intValue))
		std::clog << "[Warning - Raids::parseRaidNode] margin tag missing for raid " << name << ", using default: " << margin << std::endl;
	else
		margin = intValue * 60 * 1000;

	RefType_t refType = REF_NONE;
	if(readXMLString(raidNode, "reftype", strValue) || readXMLString(raidNode, "refType", strValue))
	{
		std::string tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "single")
			refType = REF_SINGLE;
		else if(tmpStrValue == "block")
			refType = REF_BLOCK;
		else if(tmpStrValue != "none")
			std::clog << "[Warning - Raids::parseRaidNode] Unknown reftype \"" << strValue << "\" for raid " << name << std::endl;
	}

	bool ref = false;
	if(readXMLString(raidNode, "ref", strValue))
		ref = booleanString(strValue);

	bool enabled = true;
	if(readXMLString(raidNode, "enabled", strValue))
		enabled = booleanString(strValue);

	Raid* raid = new Raid(name, interval, margin, refType, ref, enabled);
	if(!raid || !raid->loadFromXml(file))
	{
		delete raid;
		std::clog << "[Fatal - Raids::parseRaidNode] failed to load raid " << name << std::endl;
		return false;
	}

	if(checkDuplicate)
	{
		for(RaidList::iterator it = raidList.begin(); it != raidList.end(); ++it)
		{
			if((*it)->getName() == name)
				delete *it;
		}
	}

	raidList.push_back(raid);
	return true;
}

bool Raids::loadFromXml()
{
	if(isLoaded())
		return true;

	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_OTHER, "raids/raids.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Raids::loadFromXml] Could not load raids file."
			<< std::endl << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"raids"))
	{
		std::clog << "[Error - Raids::loadFromXml] Malformed raids file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
		parseRaidNode(p, false, FILE_TYPE_OTHER);

	xmlFreeDoc(doc);
	loaded = true;
	return true;
}

bool Raids::startup()
{
	if(!isLoaded() || isStarted())
		return false;

	setLastRaidEnd(OTSYS_TIME());
	checkRaidsEvent = Scheduler::getInstance().addEvent(createSchedulerTask(
		CHECK_RAIDS_INTERVAL * 1000, boost::bind(&Raids::checkRaids, this)));

	started = true;
	return true;
}

void Raids::checkRaids()
{
	checkRaidsEvent = Scheduler::getInstance().addEvent(createSchedulerTask(
		CHECK_RAIDS_INTERVAL * 1000, boost::bind(&Raids::checkRaids, this)));
	if(running)
		return;

	uint64_t now = OTSYS_TIME();
	for(RaidList::iterator it = raidList.begin(); it != raidList.end(); ++it)
	{
		if((*it)->isEnabled() && !(*it)->hasRef() && now > (lastRaidEnd + (*it)->getMargin()) &&
			(MAX_RAND_RANGE * CHECK_RAIDS_INTERVAL / (*it)->getInterval()) >= (
			uint32_t)random_range(0, MAX_RAND_RANGE) && (*it)->startRaid())
			break;
	}
}

void Raids::clear()
{
	Scheduler::getInstance().stopEvent(checkRaidsEvent);
	checkRaidsEvent = lastRaidEnd = 0;
	loaded = started = false;

	running = NULL;
	for(RaidList::iterator it = raidList.begin(); it != raidList.end(); ++it)
		delete (*it);

	raidList.clear();
	ScriptEvent::m_interface.reInitState();
}

bool Raids::reload()
{
	clear();
	return loadFromXml();
}

Raid* Raids::getRaidByName(const std::string& name)
{
	RaidList::iterator it;
	for(it = raidList.begin(); it != raidList.end(); ++it)
	{
		if(boost::algorithm::iequals((*it)->getName(), name))
			return *it;
	}

	return NULL;
}

Raid::Raid(const std::string& _name, uint32_t _interval, uint64_t _margin,
	RefType_t _refType, bool _ref, bool _enabled)
{
	name = _name;
	interval = _interval;
	margin = _margin;
	refType = _refType;
	ref = _ref;
	enabled = _enabled;

	loaded = false;
	refCount = eventCount = nextEvent = 0;
}

Raid::~Raid()
{
	stopEvents();
	for(RaidEventVector::iterator it = raidEvents.begin(); it != raidEvents.end(); ++it)
		delete *it;

	raidEvents.clear();
}

bool Raid::loadFromXml(const std::string& _filename)
{
	if(isLoaded())
		return true;

	xmlDocPtr doc = xmlParseFile(_filename.c_str());
	if(!doc)
	{
		std::clog << "[Error - Raid::loadFromXml] Could not load raid file " << _filename << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"raid"))
	{
		std::clog << "[Error - Raid::loadFromXml] Malformed raid file " << _filename << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	std::string strValue;
	for(xmlNodePtr eventNode = root->children; eventNode; eventNode = eventNode->next)
	{
		RaidEvent* event = NULL;
		if(!xmlStrcmp(eventNode->name, (const xmlChar*)"announce"))
			event = new AnnounceEvent(this, ref);
		else if(!xmlStrcmp(eventNode->name, (const xmlChar*)"effect"))
			event = new EffectEvent(this, ref);
		else if(!xmlStrcmp(eventNode->name, (const xmlChar*)"itemspawn"))
			event = new ItemSpawnEvent(this, ref);
		else if(!xmlStrcmp(eventNode->name, (const xmlChar*)"singlespawn"))
			event = new SingleSpawnEvent(this, ref);
		else if(!xmlStrcmp(eventNode->name, (const xmlChar*)"areaspawn"))
			event = new AreaSpawnEvent(this, ref);
		else if(!xmlStrcmp(eventNode->name, (const xmlChar*)"script"))
			event = new ScriptEvent(this, ref);
		else
			continue;

		if(!event->configureRaidEvent(eventNode))
		{
			std::clog << "[Error - Raid::loadFromXml] Could not configure raid in file: " << _filename << ", eventNode: " << eventNode->name << std::endl;
			delete event;
		}
		else
			raidEvents.push_back(event);
	}

	//sort by delay time
	std::sort(raidEvents.begin(), raidEvents.end(), RaidEvent::compareEvents);
	xmlFreeDoc(doc);

	loaded = true;
	return true;
}

bool Raid::startRaid()
{
	if(refCount)
		return true;

	RaidEvent* raidEvent = getNextRaidEvent();
	if(!raidEvent)
		return false;

	nextEvent = Scheduler::getInstance().addEvent(createSchedulerTask(
		raidEvent->getDelay(), boost::bind(&Raid::executeRaidEvent, this, raidEvent)));
	Raids::getInstance()->setRunning(this);
	return true;
}

bool Raid::executeRaidEvent(RaidEvent* raidEvent)
{
	if(!raidEvent->executeEvent(name))
		return !resetRaid(false);

	RaidEvent* newRaidEvent = getNextRaidEvent();
	if(!newRaidEvent)
		return !resetRaid(false);

	nextEvent = Scheduler::getInstance().addEvent(createSchedulerTask(
		std::max(RAID_MINTICKS, (int32_t)(newRaidEvent->getDelay() - raidEvent->getDelay())),
		boost::bind(&Raid::executeRaidEvent, this, newRaidEvent)));
	return true;
}

bool Raid::resetRaid(bool checkExecution)
{
	if(checkExecution && nextEvent)
		return true;

	stopEvents();
	if(refType == REF_BLOCK && refCount > 0)
		return false;

	if(refType != REF_SINGLE || refCount <= 0)
		eventCount = 0;

	if(Raids::getInstance()->getRunning() == this)
	{
		Raids::getInstance()->setRunning(NULL);
		Raids::getInstance()->setLastRaidEnd(OTSYS_TIME());
	}

	return true;
}

void Raid::stopEvents()
{
	if(!nextEvent)
		return;

	Scheduler::getInstance().stopEvent(nextEvent);
	nextEvent = 0;
}

RaidEvent* Raid::getNextRaidEvent()
{
	if(eventCount < raidEvents.size())
		return raidEvents[eventCount++];

	return NULL;
}

bool RaidEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	std::string strValue;
	if(readXMLString(eventNode, "ref", strValue))
		m_ref = booleanString(strValue);

	int32_t intValue;
	if(readXMLInteger(eventNode, "delay", intValue))
		m_delay = std::max((int32_t)m_delay, intValue);

	return true;
}

bool AnnounceEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	std::string strValue;
	if(!readXMLString(eventNode, "message", strValue))
	{
		std::clog << "[Error - AnnounceEvent::configureRaidEvent] Message tag missing for announce event." << std::endl;
		return false;
	}

	m_message = strValue;
	if(readXMLString(eventNode, "type", strValue))
	{
		std::string tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "warning")
			m_messageType = MSG_STATUS_WARNING;
		else if(tmpStrValue == "event")
			m_messageType = MSG_EVENT_ADVANCE;
		else if(tmpStrValue == "default")
			m_messageType = MSG_EVENT_DEFAULT;
		else if(tmpStrValue == "description")
			m_messageType = MSG_INFO_DESCR;
		else if(tmpStrValue == "status")
			m_messageType = MSG_STATUS_SMALL;
		else if(tmpStrValue == "blue")
			m_messageType = MSG_STATUS_CONSOLE_BLUE;
		else if(tmpStrValue == "red")
			m_messageType = MSG_STATUS_CONSOLE_RED;
		else
			std::clog << "[Notice - AnnounceEvent::configureRaidEvent] Unknown type tag for announce event, using default: "
				<< (int32_t)m_messageType << std::endl;
	}
	else
		std::clog << "[Notice - AnnounceEvent::configureRaidEvent] Missing type tag for announce event. Using default: "
			<< (int32_t)m_messageType << std::endl;

	return true;
}

bool AnnounceEvent::executeEvent(const std::string&) const
{
	g_game.broadcastMessage(m_message, m_messageType);
	return true;
}

bool EffectEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	int32_t intValue;
	std::string strValue;
	if(!readXMLInteger(eventNode, "id", intValue))
	{
		if(!readXMLString(eventNode, "name", strValue))
		{
			std::clog << "[Error - EffectEvent::configureRaidEvent] id (or name) tag missing for effect event." << std::endl;
			return false;
		}
		else
			m_effect = getMagicEffect(strValue);
	}
	else
		m_effect = (MagicEffect_t)intValue;

	if(!readXMLString(eventNode, "pos", strValue))
	{
		if(!readXMLInteger(eventNode, "x", intValue))
		{
			std::clog << "[Error - EffectEvent::configureRaidEvent] x tag missing for effect event." << std::endl;
			return false;
		}

		m_position.x = intValue;
		if(!readXMLInteger(eventNode, "y", intValue))
		{
			std::clog << "[Error - EffectEvent::configureRaidEvent] y tag missing for effect event." << std::endl;
			return false;
		}

		m_position.y = intValue;
		if(!readXMLInteger(eventNode, "z", intValue))
		{
			std::clog << "[Error - EffectEvent::configureRaidEvent] z tag missing for effect event." << std::endl;
			return false;
		}

		m_position.z = intValue;
	}
	else
	{
		IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
		if(posList.size() < 3)
		{
			std::clog << "[Error - EffectEvent::configureRaidEvent] Malformed pos tag for effect event." << std::endl;
			return false;
		}

		m_position = Position(posList[0], posList[1], posList[2]);
	}

	return true;
}

bool EffectEvent::executeEvent(const std::string&) const
{
	g_game.addMagicEffect(m_position, m_effect);
	return true;
}

bool ItemSpawnEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	int32_t intValue;
	std::string strValue;
	if(!readXMLInteger(eventNode, "id", intValue))
	{
		if(!readXMLString(eventNode, "name", strValue))
		{
			std::clog << "[Error - ItemSpawnEvent::configureRaidEvent] id (or name) tag missing for itemspawn event." << std::endl;
			return false;
		}
		else
			m_itemId = Item::items.getItemIdByName(strValue);
	}
	else
		m_itemId = intValue;

	if(readXMLInteger(eventNode, "chance", intValue))
		m_chance = intValue;

	if(readXMLInteger(eventNode, "subType", intValue))
		m_subType = intValue;

	if(!readXMLString(eventNode, "pos", strValue))
	{
		if(!readXMLInteger(eventNode, "x", intValue))
		{
			std::clog << "[Error - ItemSpawnEvent::configureRaidEvent] x tag missing for itemspawn event." << std::endl;
			return false;
		}

		m_position.x = intValue;
		if(!readXMLInteger(eventNode, "y", intValue))
		{
			std::clog << "[Error - ItemSpawnEvent::configureRaidEvent] y tag missing for itemspawn event." << std::endl;
			return false;
		}

		m_position.y = intValue;
		if(!readXMLInteger(eventNode, "z", intValue))
		{
			std::clog << "[Error - ItemSpawnEvent::configureRaidEvent] z tag missing for itemspawn event." << std::endl;
			return false;
		}

		m_position.z = intValue;
	}
	else
	{
		IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
		if(posList.size() < 3)
		{
			std::clog << "[Error - ItemSpawnEvent::configureRaidEvent] Malformed pos tag for itemspawn event." << std::endl;
			return false;
		}

		m_position = Position(posList[0], posList[1], posList[2]);
	}

	return true;
}

bool ItemSpawnEvent::executeEvent(const std::string&) const
{
	if(m_chance < (uint32_t)random_range(0, (int32_t)MAX_ITEM_CHANCE))
		return true;

	Tile* tile = g_game.getTile(m_position);
	if(!tile)
	{
		std::clog << "[Fatal - ItemSpawnEvent::executeEvent] Missing tile at position " << m_position << std::endl;
		return false;
	}

	const ItemType& it = Item::items[m_itemId];
	if(it.stackable && m_subType > 100)
	{
		int32_t subCount = m_subType;
		while(subCount > 0)
		{
			int32_t stackCount = std::min(100, (int32_t)subCount);
			subCount -= stackCount;

			Item* newItem = Item::CreateItem(m_itemId, stackCount);
			if(!newItem)
			{
				std::clog << "[Error - ItemSpawnEvent::executeEvent] Cannot create item with id " << m_itemId << std::endl;
				return false;
			}

			ReturnValue ret = g_game.internalAddItem(NULL, tile, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT);
			if(ret != RET_NOERROR)
			{
				std::clog << "[Error - ItemSpawnEvent::executeEvent] Cannot spawn item with id " << m_itemId << std::endl;
				return false;
			}

			if(m_raid->usesRef() && m_ref)
			{
				newItem->setRaid(m_raid);
				m_raid->addRef();
			}
		}
	}
	else
	{
		Item* newItem = Item::CreateItem(m_itemId, m_subType);
		if(!newItem)
		{
			std::clog << "[Error - ItemSpawnEvent::executeEvent] Cannot create item with id " << m_itemId << std::endl;
			return false;
		}

		ReturnValue ret = g_game.internalAddItem(NULL, tile, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT);
		if(ret != RET_NOERROR)
		{
			std::clog << "[Error - ItemSpawnEvent::executeEvent] Cannot spawn item with id " << m_itemId << std::endl;
			return false;
		}

		if(m_raid->usesRef() && m_ref)
		{
			newItem->setRaid(m_raid);
			m_raid->addRef();
		}
	}

	return true;
}

bool SingleSpawnEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	std::string strValue;
	if(!readXMLString(eventNode, "name", strValue))
	{
		std::clog << "[Error - SingleSpawnEvent::configureRaidEvent] name tag missing for singlespawn event." << std::endl;
		return false;
	}

	m_monsterName = strValue;
	if(!readXMLString(eventNode, "pos", strValue))
	{
		int32_t intValue;
		if(!readXMLInteger(eventNode, "x", intValue))
		{
			std::clog << "[Error - SingleSpawnEvent::configureRaidEvent] x tag missing for singlespawn event." << std::endl;
			return false;
		}

		m_position.x = intValue;
		if(!readXMLInteger(eventNode, "y", intValue))
		{
			std::clog << "[Error - SingleSpawnEvent::configureRaidEvent] y tag missing for singlespawn event." << std::endl;
			return false;
		}

		m_position.y = intValue;
		if(!readXMLInteger(eventNode, "z", intValue))
		{
			std::clog << "[Error - SingleSpawnEvent::configureRaidEvent] z tag missing for singlespawn event." << std::endl;
			return false;
		}

		m_position.z = intValue;
	}
	else
	{
		IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
		if(posList.size() < 3)
		{
			std::clog << "[Error - SingleSpawnEvent::configureRaidEvent] Malformed pos tag for singlespawn event." << std::endl;
			return false;
		}

		m_position = Position(posList[0], posList[1], posList[2]);
	}

	return true;
}

bool SingleSpawnEvent::executeEvent(const std::string&) const
{
	Monster* monster = Monster::createMonster(m_monsterName);
	if(!monster)
	{
		std::clog << "[Error - SingleSpawnEvent::executeEvent] Cannot create monster " << m_monsterName << std::endl;
		return false;
	}

	if(!g_game.placeCreature(monster, m_position, false, true))
	{
		delete monster;
		std::clog << "[Error - SingleSpawnEvent::executeEvent] Cannot spawn monster " << m_monsterName << std::endl;
		return false;
	}

	if(m_raid->usesRef() && m_ref)
	{
		monster->setRaid(m_raid);
		m_raid->addRef();
	}

	return true;
}

bool AreaSpawnEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	int32_t intValue;
	std::string strValue;
	if(readXMLInteger(eventNode, "radius", intValue))
	{
		int32_t radius = intValue;
		Position centerPos;
		if(readXMLString(eventNode, "centerPosition", strValue) || readXMLString(eventNode, "centerpos", strValue))
		{
			IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
			if(posList.size() < 3)
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] Malformed centerPosition tag for areaspawn event." << std::endl;
				return false;
			}

			centerPos = Position(posList[0], posList[1], posList[2]);
		}
		else
		{
			if(!readXMLInteger(eventNode, "centerx", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] centerx tag missing for areaspawn event." << std::endl;
				return false;
			}

			centerPos.x = intValue;
			if(!readXMLInteger(eventNode, "centery", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] centery tag missing for areaspawn event." << std::endl;
				return false;
			}

			centerPos.y = intValue;
			if(!readXMLInteger(eventNode, "centerz", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] centerz tag missing for areaspawn event." << std::endl;
				return false;
			}

			centerPos.z = intValue;
		}

		m_fromPos.x = centerPos.x - radius;
		m_fromPos.y = centerPos.y - radius;
		m_fromPos.z = centerPos.z;

		m_toPos.x = centerPos.x + radius;
		m_toPos.y = centerPos.y + radius;
		m_toPos.z = centerPos.z;
	}
	else
	{
		if(readXMLString(eventNode, "fromPosition", strValue) || readXMLString(eventNode, "frompos", strValue))
		{
			IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
			if(posList.size() < 3)
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] Malformed fromPosition tag for areaspawn event." << std::endl;
				return false;
			}

			m_fromPos = Position(posList[0], posList[1], posList[2]);
		}
		else
		{
			if(!readXMLInteger(eventNode, "fromx", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] fromx tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_fromPos.x = intValue;
			if(!readXMLInteger(eventNode, "fromy", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] fromy tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_fromPos.y = intValue;
			if(!readXMLInteger(eventNode, "fromz", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] fromz tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_fromPos.z = intValue;
		}

		if(readXMLString(eventNode, "toPosition", strValue) || readXMLString(eventNode, "topos", strValue))
		{
			IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
			if(posList.size() < 3)
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] Malformed toPosition tag for areaspawn event." << std::endl;
				return false;
			}

			m_toPos = Position(posList[0], posList[1], posList[2]);
		}
		else
		{
			if(!readXMLInteger(eventNode, "tox", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] tox tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_toPos.x = intValue;
			if(!readXMLInteger(eventNode, "toy", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] toy tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_toPos.y = intValue;
			if(!readXMLInteger(eventNode, "toz", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] toz tag missing for areaspawn event." << std::endl;
				return false;
			}

			m_toPos.z = intValue;
		}
	}

	for(xmlNodePtr monsterNode = eventNode->children; monsterNode; monsterNode = monsterNode->next)
	{
		if(xmlStrcmp(monsterNode->name, (const xmlChar*)"monster"))
			continue;

		if(!readXMLString(monsterNode, "name", strValue))
		{
			std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] name tag missing for monster node." << std::endl;
			return false;
		}

		std::string name = strValue;
		int32_t min = 0, max = 0;
		if(readXMLInteger(monsterNode, "min", intValue) || readXMLInteger(monsterNode, "minamount", intValue))
			min = intValue;

		if(readXMLInteger(monsterNode, "max", intValue) || readXMLInteger(monsterNode, "maxamount", intValue))
			max = intValue;

		if(!min && !max)
		{
			if(!readXMLInteger(monsterNode, "amount", intValue))
			{
				std::clog << "[Error - AreaSpawnEvent::configureRaidEvent] amount tag missing for monster node." << std::endl;
				return false;
			}

			min = max = intValue;
		}

		addMonster(name, min, max);
	}

	return true;
}

AreaSpawnEvent::~AreaSpawnEvent()
{
	for(MonsterSpawnList::iterator it = m_spawnList.begin(); it != m_spawnList.end(); ++it)
		delete *it;

	m_spawnList.clear();
}

void AreaSpawnEvent::addMonster(MonsterSpawn* _spawn)
{
	m_spawnList.push_back(_spawn);
}

void AreaSpawnEvent::addMonster(const std::string& name, uint32_t min, uint32_t max)
{
	MonsterSpawn* monsterSpawn = new MonsterSpawn();
	monsterSpawn->min = min;
	monsterSpawn->max = max;

	monsterSpawn->name = name;
	addMonster(monsterSpawn);
}

bool AreaSpawnEvent::executeEvent(const std::string&) const
{
	MonsterSpawn* spawn = NULL;
	for(MonsterSpawnList::const_iterator it = m_spawnList.begin(); it != m_spawnList.end(); ++it)
	{
		if(!(spawn = *it))
			continue;

		uint32_t amount = (uint32_t)random_range(spawn->min, spawn->max);
		for(uint32_t i = 0; i < amount; ++i)
		{
			Monster* monster = Monster::createMonster(spawn->name);
			if(!monster)
			{
				std::clog << "[Error - AreaSpawnEvent::executeEvent] Cannot create monster " << spawn->name << std::endl;
				return false;
			}

			bool success = false;
			for(int32_t t = 0; t < MAXIMUM_TRIES_PER_MONSTER; ++t)
			{
				if(!g_game.placeCreature(monster, Position(random_range(m_fromPos.x, m_toPos.x),
					random_range(m_fromPos.y, m_toPos.y), random_range(m_fromPos.z, m_toPos.z)), true))
					continue;

				if(m_raid->usesRef() && m_ref)
				{
					monster->setRaid(m_raid);
					m_raid->addRef();
				}

				success = true;
				break;
			}

			if(!success)
				delete monster;
		}
	}

	return true;
}

bool ScriptEvent::configureRaidEvent(xmlNodePtr eventNode)
{
	if(!RaidEvent::configureRaidEvent(eventNode))
		return false;

	std::string scriptsName = Raids::getInstance()->getScriptBaseName();
	if(!m_interface.getState())
	{
		m_interface.initState();
		std::string path = getFilePath(FILE_TYPE_OTHER, std::string(scriptsName + "/lib/"));
		if(!m_interface.loadDirectory(path, false, true))
			std::clog << "[Warning - ScriptEvent::configureRaidEvent] Cannot load " << path << std::endl;
	}

	std::string strValue;
	if(readXMLString(eventNode, "file", strValue))
	{
		std::string path = getFilePath(FILE_TYPE_OTHER, std::string(scriptsName + "/scripts/" + strValue));
		if(!fileExists(path.c_str()))
			path = getFilePath(FILE_TYPE_MOD, std::string("/scripts/" + strValue));

		if(!fileExists(path.c_str()))
		{
			std::clog << "[Error - ScriptEvent::configureRaidEvent] Cannot find script file " << strValue << std::endl;
			return false;
		}

		if(checkScript(scriptsName, path, true) && loadScript(path, true))
			return true;

		std::clog << "[Error - ScriptEvent::configureRaidEvent] Cannot load script file " << path << std::endl;
		return false;
	}
	else if(parseXMLContentString(eventNode->children, strValue) &&
		checkBuffer(scriptsName, strValue) && loadBuffer(strValue))
		return true;

	std::clog << "[Error - ScriptEvent::configureRaidEvent] Cannot load script buffer." << std::endl;
	return false;
}

bool ScriptEvent::executeEvent(const std::string& name) const
{
	//onRaid(name)
	if(m_interface.reserveEnv())
	{
		ScriptEnviroment* env = m_interface.getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			std::stringstream scriptstream;
			scriptstream << "local name = \"" << name << "\"" << std::endl;

			bool result = true;
			if(m_scriptData && m_interface.loadBuffer(*m_scriptData))
			{
				lua_State* L = m_interface.getState();
				result = m_interface.getGlobalBool(L, "_result", true);
			}

			m_interface.releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			env->setEvent("Raid event");
			#endif
			env->setScriptId(m_scriptId, &m_interface);
			lua_State* L = m_interface.getState();

			m_interface.pushFunction(m_scriptId);
			lua_pushstring(L, name.c_str());

			bool result = m_interface.callFunction(1);
			m_interface.releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - ScriptEvent::executeEvent] Call stack overflow." << std::endl;
		return false;
	}
}
