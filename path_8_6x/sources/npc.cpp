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
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <functional>
#include <iostream>
#include <fstream>

#include "npc.h"
#include "tools.h"

#include "luascript.h"
#include "position.h"

#include "spells.h"
#include "vocation.h"

#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;
extern Spells* g_spells;
extern Npcs g_npcs;

AutoList<Npc> Npc::autoList;
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t Npc::npcCount = 0;
#endif
NpcScript* Npc::m_interface = NULL;

Npcs::~Npcs()
{
	for(DataMap::iterator it = data.begin(); it != data.end(); ++it)
		delete it->second;

	data.clear();
}

bool Npcs::loadFromXml(bool reloading/* = false*/)
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_OTHER, "npc/npcs.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Npcs::loadFromXml] Cannot load npcs file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"npcs"))
	{
		std::clog << "[Error - Npcs::loadFromXml] Malformed npcs file." << std::endl;
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
	{
		if(p->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp(p->name, (const xmlChar*)"npc"))
		{
			std::clog << "[Warning - Npcs::loadFromXml] Unknown node name: " << p->name << "." << std::endl;
			continue;
		}
		else
			parseNpcNode(p, FILE_TYPE_OTHER, reloading);
	}

	return true;
}

bool Npcs::parseNpcNode(xmlNodePtr node, FileType_t path, bool reloading/* = false*/)
{
	std::string name;
	if(!readXMLString(node, "name", name))
	{
		std::clog << "[Warning - Npcs::parseNpcNode] Missing npc name!" << std::endl;
		return false;
	}

	bool new_nType = false;
	NpcType* nType = NULL;
	if(!(nType = getType(name)))
		new_nType = true;
	else if(reloading)
	{
		std::clog << "[Warning - Npcs::parseNpcNode] Duplicate registered npc with name: " << name << "." << std::endl;
		return false;
	}

	std::string strValue;
	if(!readXMLString(node, "file", strValue) && !readXMLString(node, "path", strValue))
	{
		std::clog << "[Warning - Npcs::loadFromXml] Missing file path for npc with name " << name << "." << std::endl;
		return false;
	}

	if(new_nType)
		nType = new NpcType();

	nType->name = name;
	toLowerCaseString(name);

	nType->file = getFilePath(path, "npc/" + strValue);
	if(readXMLString(node, "nameDescription", strValue) || readXMLString(node, "namedescription", strValue))
		nType->nameDescription = strValue;

	if(readXMLString(node, "script", strValue))
		nType->script = strValue;

	for(xmlNodePtr q = node->children; q; q = q->next)
	{
		if(!xmlStrcmp(q->name, (const xmlChar*)"look"))
		{
			int32_t intValue;
			if(readXMLInteger(q, "type", intValue))
			{
				nType->outfit.lookType = intValue;
				if(readXMLInteger(q, "head", intValue))
					nType->outfit.lookHead = intValue;

				if(readXMLInteger(q, "body", intValue))
					nType->outfit.lookBody = intValue;

				if(readXMLInteger(q, "legs", intValue))
					nType->outfit.lookLegs = intValue;

				if(readXMLInteger(q, "feet", intValue))
					nType->outfit.lookFeet = intValue;

				if(readXMLInteger(q, "addons", intValue))
					nType->outfit.lookAddons = intValue;
			}
			else if(readXMLInteger(q, "typeex", intValue))
				nType->outfit.lookTypeEx = intValue;
		}
	}

	if(new_nType)
		data[name] = nType;

	return true;
}

void Npcs::reload()
{
	for(AutoList<Npc>::iterator it = Npc::autoList.begin(); it != Npc::autoList.end(); ++it)
		it->second->closeAllShopWindows();

	delete Npc::m_interface;
	Npc::m_interface = NULL;

	if(fileExists(getFilePath(FILE_TYPE_OTHER, "npc/npcs.xml").c_str()))
	{
		DataMap tmp = data;
		if(!loadFromXml())
			data = tmp;

		tmp.clear();
	}

	for(AutoList<Npc>::iterator it = Npc::autoList.begin(); it != Npc::autoList.end(); ++it)
		it->second->reload();
}

NpcType* Npcs::getType(const std::string& name) const
{
	DataMap::const_iterator it = data.find(asLowerCaseString(name));
	if(it == data.end())
		return NULL;

	return it->second;
}

bool Npcs::setType(std::string name, NpcType* nType)
{
	toLowerCaseString(name);
	DataMap::const_iterator it = data.find(name);
	if(it != data.end())
		return false;

	data[name] = nType;
	return true;
}

Npc* Npc::createNpc(NpcType* nType)
{
	Npc* npc = new Npc(nType);
	if(!npc)
		return NULL;

	if(npc->load())
		return npc;

	delete npc;
	return NULL;
}

Npc* Npc::createNpc(const std::string& name)
{
	NpcType* nType = NULL;
	if(!(nType = g_npcs.getType(name)))
	{
		nType = new NpcType();
		nType->file = getFilePath(FILE_TYPE_OTHER, "npc/" + name + ".xml");
		if(!fileExists(nType->file.c_str()))
		{
			nType->file = getFilePath(FILE_TYPE_MOD, "npc/" + name + ".xml");
			if(!fileExists(nType->file.c_str()))
			{
				std::clog << "[Warning - Npc::createNpc] Cannot find npc with name: " << name << "." << std::endl;
				return NULL;
			}
		}

		nType->name = name;
		g_npcs.setType(name, nType);
	}

	return createNpc(nType);
}

Npc::Npc(NpcType* _nType) : Creature(), m_npcEventHandler(NULL)
{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	++npcCount;
#endif
	nType = _nType;

	m_npcEventHandler = NULL;
	reset();
}

Npc::~Npc()
{
	reset();
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	--npcCount;
#endif
}

void Npc::reset()
{
	loaded = false;
	walkTicks = 1500;
	floorChange = false;
	attackable = false;
	walkable = false;
	hasBusyReply = false;
	hasScriptedFocus = false;
	focusCreature = 0;
	isIdle = true;
	talkRadius = 2;
	idleTime = 0;
	idleInterval = 5 * 60;
	lastVoice = OTSYS_TIME();
	defaultPublic = true;
	baseDirection = SOUTH;
	if(m_npcEventHandler)
		delete m_npcEventHandler;

	m_npcEventHandler = NULL;
	for(ResponseList::iterator it = responseList.begin(); it != responseList.end(); ++it)
		delete *it;

	for(StateList::iterator it = stateList.begin(); it != stateList.end(); ++it)
		delete *it;

	responseList.clear();
	stateList.clear();
	queueList.clear();
	m_parameters.clear();
	itemListMap.clear();
	responseScriptMap.clear();
	shopPlayerList.clear();
	voiceList.clear();
}

bool Npc::load()
{
	if(isLoaded())
		return true;

	if(nType->file.empty())
	{
		std::clog << "[Warning - Npc::load] Cannot load npc with name: " << nType->name << "." << std::endl;
		return false;
	}

	if(!m_interface)
	{
		m_interface = new NpcScript();
		m_interface->loadDirectory(getFilePath(FILE_TYPE_OTHER, "npc/lib"), false, true, this);
	}

	loaded = loadFromXml();
	defaultOutfit = currentOutfit = nType->outfit;
	return isLoaded();
}

void Npc::reload()
{
	reset();
	load();
	//Simulate that the creature is placed on the map again.
	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureAppear(this);

	if(walkTicks)
		addEventWalk();
}

bool Npc::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(nType->file.c_str());
	if(!doc)
	{
		std::clog << "[Warning - Npc::loadFromXml] Cannot load npc file: " << nType->file << "." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"npc"))
	{
		std::clog << "[Warning - Npc::loadFromXml] Malformed npc file: " << nType->file << "." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	int32_t intValue;
	std::string strValue;
	if(readXMLString(root, "name", strValue))
		nType->name = strValue;

	if(readXMLString(root, "script", strValue))
		nType->script = strValue;

	if(readXMLString(root, "namedescription", strValue) || readXMLString(root, "nameDescription", strValue))
		nType->nameDescription = strValue;

	if(!nType->nameDescription.empty())
		replaceString(nType->nameDescription, "|NAME|", nType->name);
	else
		nType->nameDescription = nType->name;

	if(readXMLString(root, "hidename", strValue) || readXMLString(root, "hideName", strValue))
		hideName = booleanString(strValue);

	if(readXMLString(root, "hidehealth", strValue) || readXMLString(root, "hideHealth", strValue))
		hideHealth = booleanString(strValue);

	baseSpeed = 100;
	if(readXMLInteger(root, "speed", intValue))
		baseSpeed = intValue;

	if(readXMLString(root, "attackable", strValue))
		attackable = booleanString(strValue);

	if(readXMLString(root, "walkable", strValue))
		walkable = booleanString(strValue);

	if(readXMLInteger(root, "autowalk", intValue))
		std::clog << "[Notice - Npc::Npc] NPC: " << nType->name << " - autowalk attribute has been deprecated, use walkinterval instead." << std::endl;

	if(readXMLInteger(root, "walkinterval", intValue))
		walkTicks = intValue;

	if(readXMLInteger(root, "direction", intValue) && intValue >= NORTH && intValue <= WEST)
	{
		direction = (Direction)intValue;
		baseDirection = direction;
	}

	if(readXMLString(root, "floorchange", strValue))
		floorChange = booleanString(strValue);

	if(readXMLString(root, "skull", strValue))
		setSkull(getSkulls(strValue));

	if(readXMLString(root, "shield", strValue))
		setShield(getShields(strValue));

	if(readXMLString(root, "emblem", strValue))
		setEmblem(getEmblems(strValue));

	for(xmlNodePtr p = root->children; p; p = p->next)
	{
		if(!xmlStrcmp(p->name, (const xmlChar*)"health"))
		{
			if(readXMLInteger(p, "now", intValue))
				health = intValue;
			else
				health = 100;

			if(readXMLInteger(p, "max", intValue))
				healthMax = intValue;
			else
				healthMax = 100;
		}
		else if(!xmlStrcmp(p->name, (const xmlChar*)"look"))
		{
			if(readXMLInteger(p, "type", intValue))
			{
				nType->outfit.lookType = intValue;
				if(readXMLInteger(p, "head", intValue))
					nType->outfit.lookHead = intValue;

				if(readXMLInteger(p, "body", intValue))
					nType->outfit.lookBody = intValue;

				if(readXMLInteger(p, "legs", intValue))
					nType->outfit.lookLegs = intValue;

				if(readXMLInteger(p, "feet", intValue))
					nType->outfit.lookFeet = intValue;

				if(readXMLInteger(p, "addons", intValue))
					nType->outfit.lookAddons = intValue;
			}
			else if(readXMLInteger(p, "typeex", intValue))
				nType->outfit.lookTypeEx = intValue;
		}
		else if(!xmlStrcmp(p->name, (const xmlChar*)"voices"))
		{
			for(xmlNodePtr q = p->children; q != NULL; q = q->next)
			{
				if(!xmlStrcmp(q->name, (const xmlChar*)"voice"))
				{
					if(!readXMLString(q, "text", strValue))
						continue;

					Voice voice;
					voice.text = strValue;
					if(readXMLInteger(q, "interval2", intValue))
						voice.interval = intValue;
					else
						voice.interval = 60;

					if(readXMLInteger(q, "margin", intValue))
						voice.margin = intValue;
					else
						voice.margin = 0;

					voice.type = MSG_SPEAK_SAY;
					if(readXMLInteger(q, "type", intValue))
						voice.type = (MessageClasses)intValue;
					else if(readXMLString(q, "yell", strValue) && booleanString(strValue))
						voice.type = MSG_SPEAK_YELL;

					if(readXMLString(q, "randomspectator", strValue) || readXMLString(q, "randomSpectator", strValue))
						voice.randomSpectator = booleanString(strValue);
					else
						voice.randomSpectator = false;

					voiceList.push_back(voice);
				}
			}
		}
		else if(!xmlStrcmp(p->name, (const xmlChar*)"parameters"))
		{
			for(xmlNodePtr q = p->children; q != NULL; q = q->next)
			{
				if(!xmlStrcmp(q->name, (const xmlChar*)"parameter"))
				{
					std::string paramKey, paramValue;
					if(!readXMLString(q, "key", paramKey))
						continue;

					if(!readXMLString(q, "value", paramValue))
						continue;

					m_parameters[paramKey] = paramValue;
				}
			}
		}
		else if(!xmlStrcmp(p->name, (const xmlChar*)"interaction"))
		{
			if(readXMLInteger(p, "talkradius", intValue))
				talkRadius = intValue;

			if(readXMLInteger(p, "idletime", intValue))
				idleTime = intValue;

			if(readXMLInteger(p, "idleinterval", intValue))
				idleInterval = intValue;

			if(readXMLInteger(p, "defaultpublic", intValue))
				defaultPublic = intValue != 0;

			responseList = parseInteractionNode(p->children);
		}
	}

	xmlFreeDoc(doc);
	if(nType->script.empty())
		return true;

	if(nType->script.find("/") != std::string::npos)
	{
		replaceString(nType->script, "|DATA|", getFilePath(FILE_TYPE_OTHER, "npc/scripts"));
		replaceString(nType->script, "|MODS|", getFilePath(FILE_TYPE_MOD, "scripts"));
	}
	else
		nType->script = getFilePath(FILE_TYPE_OTHER, "npc/scripts/" + nType->script);

	m_npcEventHandler = new NpcEvents(nType->script, this);
	return m_npcEventHandler->isLoaded();
}

uint32_t Npc::parseParamsNode(xmlNodePtr node)
{
	std::string strValue;
	uint32_t params = RESPOND_DEFAULT;
	if(readXMLString(node, "param", strValue))
	{
		StringVec paramList = explodeString(strValue, ";");
		for(StringVec::iterator it = paramList.begin(); it != paramList.end(); ++it)
		{
			std::string tmpParam = asLowerCaseString(*it);
			if(tmpParam == "male")
				params |= RESPOND_MALE;
			else if(tmpParam == "female")
				params |= RESPOND_FEMALE;
			else if(tmpParam == "pzblock")
				params |= RESPOND_PZBLOCK;
			else if(tmpParam == "lowmoney")
				params |= RESPOND_LOWMONEY;
			else if(tmpParam == "noamount")
				params |= RESPOND_NOAMOUNT;
			else if(tmpParam == "lowamount")
				params |= RESPOND_LOWAMOUNT;
			else if(tmpParam == "premium")
				params |= RESPOND_PREMIUM;
			else if(tmpParam == "promoted")
				params |= RESPOND_PROMOTED;
			else if(tmpParam == "druid")
				params |= RESPOND_DRUID;
			else if(tmpParam == "knight")
				params |= RESPOND_KNIGHT;
			else if(tmpParam == "paladin")
				params |= RESPOND_PALADIN;
			else if(tmpParam == "sorcerer")
				params |= RESPOND_SORCERER;
			else if(tmpParam == "lowlevel")
				params |= RESPOND_LOWLEVEL;
			else
				std::clog << "[Warning - Npc::parseParamsNode] NPC Name: " << nType->name << " - Unknown param " << (*it) << std::endl;
		}
	}

	return params;
}

ResponseList Npc::parseInteractionNode(xmlNodePtr node)
{
	std::string strValue;
	int32_t intValue;

	ResponseList _responseList;
	for(; node; node = node->next)
	{
		if(!xmlStrcmp(node->name, (const xmlChar*)"include"))
		{
			if(readXMLString(node, "file", strValue))
			{
				if(xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_OTHER, "npc/lib/" + strValue).c_str()))
				{
					xmlNodePtr root = xmlDocGetRootElement(doc);
					if(!xmlStrcmp(root->name,(const xmlChar*)"interaction"))
					{
						ResponseList includedResponses = parseInteractionNode(root->children);
						_responseList.insert(_responseList.end(), includedResponses.begin(), includedResponses.end());
					}
					else
						std::clog << "[Error - Npc::parseInteractionNode] Malformed interaction file (" << strValue << ")." << std::endl;

					xmlFreeDoc(doc);
				}
				else
				{
					std::clog << "[Warning - Npc::parseInteractionNode] Cannot load interaction file (" << strValue << ")." << std::endl;
					std::clog << getLastXMLError() << std::endl;
				}
			}
		}
		else if(!xmlStrcmp(node->name, (const xmlChar*)"itemlist"))
		{
			if(readXMLString(node, "listid", strValue))
			{
				ItemListMap::iterator it = itemListMap.find(strValue);
				if(it == itemListMap.end())
				{
					std::string listId = strValue;
					std::list<ListItem>& list = itemListMap[strValue];

					xmlNodePtr tmpNode = node->children;
					while(tmpNode)
					{
						if(!xmlStrcmp(tmpNode->name, (const xmlChar*)"item"))
						{
							ListItem li;
							if(!readXMLInteger(tmpNode, "id", intValue))
							{
								std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Missing list item itemId" << std::endl;
								tmpNode = tmpNode->next;
								continue;
							}

							li.itemId = intValue;
							const ItemType& it = Item::items[li.itemId];

							if(readXMLInteger(tmpNode, "sellprice", intValue))
								li.sellPrice = intValue;

							if(readXMLInteger(tmpNode, "buyprice", intValue))
								li.buyPrice = intValue;

							if(readXMLString(tmpNode, "keywords", strValue))
								li.keywords = strValue;
							else
							{
								std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Missing list item keywords" << std::endl;
								tmpNode = tmpNode->next;
								continue;
							}

							//optional
							if(readXMLInteger(tmpNode, "subtype", intValue))
								li.subType = intValue;
							else
							{
								if(it.stackable)
									li.subType = 1;
								else if(it.isFluidContainer() || it.isSplash())
									li.subType = 0;
							}

							if(readXMLString(tmpNode, "name", strValue))
								li.name = strValue;

							if(readXMLString(tmpNode, "pname", strValue))
								li.pluralName = strValue;

							list.push_back(li);
						}

						tmpNode = tmpNode->next;
					}
				}
				else
					std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Duplicate listId found: " << strValue << std::endl;
			}
		}
		else if(!xmlStrcmp(node->name, (const xmlChar*)"interact"))
		{
			NpcResponse::ResponseProperties prop;
			prop.publicize = defaultPublic;
			if(readXMLString(node, "keywords", strValue))
				prop.inputList.push_back(asLowerCaseString(strValue));
			else if(readXMLString(node, "event", strValue))
			{
				strValue = asLowerCaseString(strValue);
				if(strValue == "onbusy")
					hasBusyReply = true;

				prop.interactType = INTERACT_EVENT;
				prop.inputList.push_back(strValue);
			}

			if(readXMLInteger(node, "topic", intValue))
				prop.topic = intValue;

			if(readXMLInteger(node, "focus", intValue))
				prop.focusStatus = intValue;

			if(readXMLString(node, "storageId", strValue))
				prop.storageId = strValue;

			if(readXMLString(node, "storageValue", strValue))
				prop.storageValue = strValue;

			uint32_t interactParams = parseParamsNode(node);
			if(readXMLString(node, "storageComp", strValue))
			{
				std::string tmpStrValue = asLowerCaseString(strValue);
				if(tmpStrValue == "equal")
					prop.storageComp = STORAGE_EQUAL;
				if(tmpStrValue == "notequal")
					prop.storageComp = STORAGE_NOTEQUAL;
				if(tmpStrValue == "greaterorequal")
					prop.storageComp = STORAGE_GREATEROREQUAL;
				if(tmpStrValue == "greater")
					prop.storageComp = STORAGE_GREATER;
				if(tmpStrValue == "less")
					prop.storageComp = STORAGE_LESS;
				if(tmpStrValue == "lessorequal")
					prop.storageComp = STORAGE_LESSOREQUAL;
			}

			xmlNodePtr tmpNode = node->children;
			while(tmpNode)
			{
				if(!xmlStrcmp(tmpNode->name, (const xmlChar*)"keywords"))
				{
					//alternative input keywords
					xmlNodePtr altKeyNode = tmpNode->children;
					while(altKeyNode)
					{
						if(!xmlStrcmp(altKeyNode->name, (const xmlChar*)"text"))
						{
							if(readXMLContentString(altKeyNode, strValue))
								prop.inputList.push_back(asLowerCaseString(strValue));
						}
						altKeyNode = altKeyNode->next;
					}
				}
				else if(!xmlStrcmp(tmpNode->name, (const xmlChar*)"list"))
				{
					xmlNodePtr listNode = tmpNode->children;
					while(listNode)
					{
						if(!xmlStrcmp(listNode->name, (const xmlChar*)"text"))
						{
							if(readXMLContentString(listNode, strValue))
							{
								ItemListMap::iterator it = itemListMap.find(strValue);
								if(it != itemListMap.end())
									prop.itemList.insert(prop.itemList.end(), it->second.begin(), it->second.end());
								else
									std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Could not find a list id called: " << strValue << std::endl;
							}
						}

						listNode = listNode->next;
					}
				}

				tmpNode = tmpNode->next;
			}

			for(tmpNode = node->children; tmpNode; tmpNode = tmpNode->next)
			{
				if(!xmlStrcmp(tmpNode->name, (const xmlChar*)"response"))
				{
					prop.output = prop.knowSpell = "";
					prop.params = interactParams | parseParamsNode(tmpNode);

					ScriptVars scriptVars;
					if(readXMLString(tmpNode, "knowspell", strValue))
						prop.knowSpell = strValue;

					if(readXMLString(tmpNode, "text", strValue))
					{
						prop.responseType = RESPONSE_DEFAULT;
						prop.output = strValue;
					}
					else if(readXMLString(tmpNode, "function", strValue))
					{
						prop.responseType = RESPONSE_SCRIPT;
						prop.output = strValue;
					}

					if(readXMLInteger(tmpNode, "public", intValue))
						prop.publicize = (intValue == 1);

					if(readXMLInteger(tmpNode, "b1", intValue))
						scriptVars.b1 = (intValue == 1);

					if(readXMLInteger(tmpNode, "b2", intValue))
						scriptVars.b2 = (intValue == 1);

					if(readXMLInteger(tmpNode, "b3", intValue))
						scriptVars.b3 = (intValue == 1);

					ResponseList subResponseList;
					for(xmlNodePtr subNode = tmpNode->children; subNode; subNode = subNode->next)
					{
						if(!xmlStrcmp(subNode->name, (const xmlChar*)"action"))
						{
							ResponseAction action;
							if(readXMLString(subNode, "name", strValue))
							{
								std::string tmpStrValue = asLowerCaseString(strValue);
								if(tmpStrValue == "topic")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETTOPIC;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "price")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETPRICE;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "amount")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETAMOUNT;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "item")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETITEM;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "subtype")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETSUBTYPE;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "spell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETSPELL;
										action.strValue = strValue;
										if(strValue != "|SPELL|" && !g_spells->getInstantSpellByName(strValue))
											std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Could not find an instant spell called: " << strValue << std::endl;
									}
								}
								else if(tmpStrValue == "listname")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLISTNAME;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "listpname")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLISTPNAME;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "teachspell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TEACHSPELL;
										action.strValue = strValue;
										if(strValue != "|SPELL|" && !g_spells->getInstantSpellByName(strValue))
											std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Could not find an instant spell called: " << strValue << std::endl;
									}
								}
								else if(tmpStrValue == "unteachspell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_UNTEACHSPELL;
										action.strValue = strValue;
										if(strValue != "|SPELL|" && !g_spells->getInstantSpellByName(strValue))
											std::clog << "[Warning - Npc::parseInteractionNode] NPC Name: " << nType->name << " - Could not find an instant spell called: " << strValue << std::endl;
									}
								}
								else if(tmpStrValue == "sell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SELLITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "buy")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_BUYITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "takemoney")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TAKEMONEY;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "givemoney")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_GIVEMONEY;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "level")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLEVEL;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "giveitem")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_GIVEITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "takeitem")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TAKEITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "effect")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETEFFECT;
										action.intValue = getMagicEffect(strValue);
									}
								}
								else if(tmpStrValue == "idle")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETIDLE;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "script")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SCRIPT;
										action.strValue = strValue;
									}
									else if(parseXMLContentString(subNode->children, action.strValue))
										action.actionType = ACTION_SCRIPT;
								}
								else if(tmpStrValue == "scriptparam")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SCRIPTPARAM;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "storage")
								{
									if(readXMLString(subNode, "time", strValue))
									{
										action.actionType = ACTION_SETSTORAGE;
										std::stringstream s;

										s << time(NULL) + atoi(strValue.c_str());
										action.strValue = s.str();
									}
									else if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETSTORAGE;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "addqueue")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_ADDQUEUE;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "teleport")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETTELEPORT;
										action.strValue = strValue;
										action.pos = Position();

										IntegerVec posList = vectorAtoi(explodeString(strValue, ";"));
										if(posList.size() > 2)
											action.pos = Position(posList[0], posList[1], posList[2]);
									}
								}
								else
									std::clog << "[Warning - Npc::parseInteractionNode] Unknown action " << strValue << std::endl;
							}

							if(readXMLString(subNode, "key", strValue))
								action.key = strValue;

							if(action.actionType != ACTION_NONE)
								prop.actionList.push_back(action);
						}
						else if(!xmlStrcmp(subNode->name, (const xmlChar*)"interact"))
						{
							if(subResponseList.empty())
							{
								ResponseList nodeResponseList = parseInteractionNode(subNode);
								subResponseList.insert(subResponseList.end(),
									nodeResponseList.begin(), nodeResponseList.end());
							}
						}
					}

					//Check if this interaction has a |list| keyword
					bool hasListKeyword = false;
					for(std::list<std::string>::iterator it = prop.inputList.begin();
						it != prop.inputList.end(); ++it)
					{
						if(it->find("|list|") != std::string::npos)
						{
							hasListKeyword = true;
							break;
						}
					}

					//Iterate through all input keywords and replace all |LIST| with the item list
					if(hasListKeyword && !prop.itemList.empty())
					{
						for(std::list<ListItem>::iterator it = prop.itemList.begin(); it != prop.itemList.end(); ++it)
						{
							NpcResponse::ResponseProperties listItemProp = prop;
							for(std::list<std::string>::iterator iit = listItemProp.inputList.begin();
								iit != listItemProp.inputList.end(); ++iit)
							{
								std::string& input = (*iit);
								if(input.find("|list|") == std::string::npos)
									continue;

								//Replace |LIST| with the keyword in the list
								replaceString(input, "|list|", it->keywords);
								ResponseAction action;

								action.actionType = ACTION_SETITEM;
								action.intValue = it->itemId;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSELLPRICE;
								action.intValue = it->sellPrice;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETBUYPRICE;
								action.intValue = it->buyPrice;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSUBTYPE;
								action.intValue = it->subType;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSUBTYPE;
								action.intValue = it->subType;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETLISTNAME;
								if(it->name.empty())
								{
									const ItemType& itemType = Item::items[it->itemId];
									if(itemType.id != 0)
										action.strValue = itemType.article + " " + itemType.name;
								}
								else
									action.strValue = it->name;

								listItemProp.actionList.push_front(action);
								action.actionType = ACTION_SETLISTPNAME;
								if(it->pluralName.empty())
								{
									const ItemType& itemType = Item::items[it->itemId];
									if(itemType.id != 0)
										action.strValue = itemType.pluralName;
								}
								else
									action.strValue = it->pluralName;

								listItemProp.actionList.push_front(action);
								ResponseList list;
								for(ResponseList::iterator rit = subResponseList.begin();
									rit != subResponseList.end(); ++rit)
								{
										list.push_back(new NpcResponse(*(*rit)));
								}

								//Create a new response for this list item
								NpcResponse* response = new NpcResponse(listItemProp, list, scriptVars);
								_responseList.push_back(response);
							}
						}
					}
					else
					{
						NpcResponse* response = new NpcResponse(prop, subResponseList, scriptVars);
						_responseList.push_back(response);
					}
				}
			}
		}
	}

	return _responseList;
}

NpcState* Npc::getState(const Player* player, bool makeNew/* = true*/)
{
	for(StateList::iterator it = stateList.begin(); it != stateList.end(); ++it)
	{
		if((*it)->respondToCreature == player->getID())
			return *it;
	}

	if(!makeNew)
		return NULL;

	NpcState* state = new NpcState;
	state->prevInteraction = state->price = 0;
	state->sellPrice = state->buyPrice = -1;
	state->amount = 1;
	state->itemId = 0;
	state->subType = -1;
	state->ignore = state->inBackpacks = false;
	state->spellName = state->listName = "";
	state->listPluralName = "";
	state->level = state->topic = -1;
	state->isIdle = true;
	state->isQueued = false;
	state->respondToText = "";
	state->respondToCreature = 0;
	state->lastResponse = NULL;
	state->prevRespondToText = "";

	stateList.push_back(state);
	return state;
}

bool Npc::canSee(const Position& pos) const
{
	Position tmp = getPosition();
	if(pos.z != tmp.z)
		return false;

	return Creature::canSee(tmp, pos, Map::maxClientViewportX, Map::maxClientViewportY);
}

void Npc::onCreatureAppear(const Creature* creature)
{
	Creature::onCreatureAppear(creature);
	if(creature == this)
	{
		if(walkTicks)
			addEventWalk();

		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureAppear(creature);

		return;
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureAppear(creature);

	//only players for script events
	Player* player = const_cast<Player*>(creature->getPlayer());
	if(!player)
		return;

	if(NpcState* npcState = getState(player))
	{
		npcState->respondToCreature = player->getID();
		onPlayerEnter(player, npcState);
	}
}

void Npc::onCreatureDisappear(const Creature* creature, bool isLogout)
{
	Creature::onCreatureDisappear(creature, isLogout);
	if(creature == this)
	{
		closeAllShopWindows();
		return;
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureDisappear(creature);

	Player* player = const_cast<Player*>(creature->getPlayer());
	if(!player)
		return;

	if(NpcState* npcState = getState(player))
	{
		npcState->respondToCreature = player->getID();
		onPlayerLeave(player, npcState);
	}
}

void Npc::onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
		const Tile* oldTile, const Position& oldPos, bool teleport)
{
	Creature::onCreatureMove(creature, newTile, newPos, oldTile, oldPos, teleport);
	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureMove(creature, oldPos, newPos);

	Player* player = const_cast<Player*>(creature->getPlayer());
	if(!player)
		return;

	if(NpcState* npcState = getState(player))
	{
		bool canSeeNewPos = canSee(newPos), canSeeOldPos = canSee(oldPos);
		if(canSeeNewPos && !canSeeOldPos)
		{
			npcState->respondToCreature = player->getID();
			onPlayerEnter(player, npcState);
		}
		else if(!canSeeNewPos && canSeeOldPos)
		{
			npcState->respondToCreature = player->getID();
			onPlayerLeave(player, npcState);
		}
		else if(canSeeNewPos && canSeeOldPos)
		{
			npcState->respondToCreature = player->getID();
			const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_MOVE);
			executeResponse(player, npcState, response);
		}
	}
}

void Npc::onCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos/* = NULL*/)
{
	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureSay(creature, type, text, pos);

	const Player* player = creature->getPlayer();
	if(!player)
		return;

	if(type == MSG_SPEAK_SAY || type == MSG_NPC_TO)
	{
		Position destPos = creature->getPosition();
		if(pos)
			destPos = (*pos);

		const Position& myPos = getPosition();
		if(canSee(myPos) && (destPos.x >= myPos.x - talkRadius) && (destPos.x <= myPos.x + talkRadius)
			&& (destPos.y >= myPos.y - talkRadius) && (destPos.y <= myPos.y + talkRadius))
		{
			if(NpcState* npcState = getState(player))
			{
				npcState->respondToText = text;
				npcState->respondToCreature = player->getID();
			}
		}
	}
}

void Npc::onPlayerCloseChannel(const Player* player)
{
	if(NpcState* npcState = getState(player, true))
	{
		const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_CHATCLOSE);
		executeResponse(const_cast<Player*>(player), npcState, response);
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerCloseChannel(player);
}

void Npc::onPlayerEnter(Player* player, NpcState* state)
{
	const NpcResponse* response = getResponse(player, state, EVENT_PLAYER_ENTER);
	executeResponse(player, state, response);
}

void Npc::onPlayerLeave(Player* player, NpcState* state)
{
	if(player->getShopOwner() == this)
		player->closeShopWindow();

	const NpcResponse* response = getResponse(player, state, EVENT_PLAYER_LEAVE);
	executeResponse(player, state, response);
}

void Npc::onThink(uint32_t interval)
{
	Creature::onThink(interval);
	if(m_npcEventHandler)
		m_npcEventHandler->onThink();

	std::vector<Player*> list;
	Player* tmpPlayer = NULL;

	const SpectatorVec& tmpList = g_game.getSpectators(getPosition());
	if(tmpList.size()) //loop only if there's at least one spectator
	{
		for(SpectatorVec::const_iterator it = tmpList.begin(); it != tmpList.end(); ++it)
		{
			if((tmpPlayer = (*it)->getPlayer()) && !tmpPlayer->isRemoved())
				list.push_back(tmpPlayer);
		}
	}

	if(list.size()) //loop only if there's at least one player
	{
		int64_t now = OTSYS_TIME();
		for(VoiceList::iterator it = voiceList.begin(); it != voiceList.end(); ++it)
		{
			if(now < (lastVoice + it->margin))
				continue;

			if((uint32_t)(MAX_RAND_RANGE / it->interval) < (uint32_t)random_range(0, MAX_RAND_RANGE))
				continue;

			tmpPlayer = NULL;
			if(it->randomSpectator)
			{
				size_t random = random_range(0, (int32_t)list.size());
				if(random < list.size()) //1 slot chance to make it public
					tmpPlayer = list[random];
			}

			doSay(it->text, it->type, tmpPlayer);
			lastVoice = now;
			break;
		}
	}

	bool idleResponse = false;
	if((uint32_t)(MAX_RAND_RANGE / idleInterval) >= (uint32_t)random_range(0, MAX_RAND_RANGE))
		idleResponse = true;

	if(getTimeSinceLastMove() >= walkTicks)
		addEventWalk();

	isIdle = true;
	for(StateList::iterator it = stateList.begin(); it != stateList.end();)
	{
		NpcState* npcState = *it;
		const NpcResponse* response = NULL;
		bool closeConversation = false, idleTimeout = false;

		Player* player = g_game.getPlayerByID(npcState->respondToCreature);
		if(!npcState->isQueued)
		{
			if(!npcState->prevInteraction)
				npcState->prevInteraction = OTSYS_TIME();

			if(!queueList.empty() && npcState->isIdle && npcState->respondToText.empty())
				closeConversation = true;
			else if(idleTime > 0 && (OTSYS_TIME() - npcState->prevInteraction) > (uint64_t)(idleTime * 1000))
				idleTimeout = closeConversation = true;
		}

		if(idleResponse && player)
		{
			response = getResponse(player, EVENT_IDLE);
			executeResponse(player, npcState, response);
			idleResponse = false;
		}

		if(!player || closeConversation)
		{
			if(queueList.empty())
			{
				if(idleTimeout && player)
					onPlayerLeave(player, npcState);
			}
			else
			{
				Player* tmpPlayer = NULL;
				while(!queueList.empty())
				{
					if((tmpPlayer = g_game.getPlayerByID(*queueList.begin())))
					{
						if(NpcState* tmpPlayerState = getState(tmpPlayer, false))
						{
							tmpPlayerState->respondToText = tmpPlayerState->prevRespondToText;
							tmpPlayerState->isQueued = false;
							break;
						}
					}

					queueList.erase(queueList.begin());
				}
			}

			delete *it;
			it = stateList.erase(it);
			continue;
		}

		if(!npcState->respondToText.empty())
		{
			if(hasBusyReply && !isIdle)
			{
				//Check if we have a busy reply
				if((response = getResponse(player, npcState, EVENT_BUSY)))
					executeResponse(player, npcState, response);
			}
			else
			{
				if(npcState->lastResponse)
				{
					//Check previous response chain first
					const ResponseList& list = npcState->lastResponse->getResponseList();
					response = getResponse(list, player, npcState, npcState->respondToText);
				}

				if(!response)
					response = getResponse(player, npcState, npcState->respondToText);

				if(response)
				{
					setCreatureFocus(player);
					executeResponse(player, npcState, response);
				}
			}

			npcState->prevRespondToText = npcState->respondToText;
			npcState->respondToText = "";
		}

		response = getResponse(player, npcState, EVENT_THINK);
		executeResponse(player, npcState, response);
		if(!npcState->isIdle)
		{
			isIdle = false;
			if(hasBusyReply)
				setCreatureFocus(player);
		}

		++it;
	}

	if(isIdle && !hasScriptedFocus)
		setCreatureFocus(NULL);
}

void Npc::executeResponse(Player* player, NpcState* npcState, const NpcResponse* response)
{
	if(response)
	{
		npcState->lastResponse = response;
		npcState->isIdle = !response->getFocusState();

		bool resetTopic = true;
		if(response->getAmount() != -1)
			npcState->amount = response->getAmount();

		for(ActionList::const_iterator it = response->getFirstAction(); it != response->getEndAction(); ++it)
		{
			switch(it->actionType)
			{
				case ACTION_SETTOPIC:
					npcState->topic = it->intValue;
					resetTopic = false;
					break;
				case ACTION_SETSELLPRICE:
					npcState->sellPrice = it->intValue;
					break;
				case ACTION_SETBUYPRICE:
					npcState->buyPrice = it->intValue;
					break;
				case ACTION_SETITEM:
					npcState->itemId = it->intValue;
					break;
				case ACTION_SETSUBTYPE:
					npcState->subType = it->intValue;
					break;
				case ACTION_SETEFFECT:
					g_game.addMagicEffect(player->getPosition(), it->intValue);
					break;
				case ACTION_SETPRICE:
				{
					if(it->strValue == "|SELLPRICE|")
						npcState->price = npcState->sellPrice;
					else if(it->strValue == "|BUYPRICE|")
						npcState->price = npcState->buyPrice;
					else
						npcState->price = it->intValue;

					break;
				}

				case ACTION_SETTELEPORT:
				{
					Position teleportTo = it->pos;
					if(it->strValue == "|TEMPLE|")
						teleportTo = player->getMasterPosition();

					g_game.internalTeleport(player, teleportTo, false);
					break;
				}

				case ACTION_SETIDLE:
				{
					npcState->isIdle = (it->intValue != 0);
					break;
				}

				case ACTION_SETLEVEL:
				{
					if(it->strValue == "|SPELLLEVEL|")
					{
						npcState->level = -1;
						if(InstantSpell* spell = g_spells->getInstantSpellByName(npcState->spellName))
							npcState->level = spell->getLevel();
					}
					else
						npcState->level = it->intValue;

					break;
				}

				case ACTION_SETSPELL:
				{
					npcState->spellName = "";
					if(g_spells->getInstantSpellByName(it->strValue))
						npcState->spellName = it->strValue;

					break;
				}

				case ACTION_SETLISTNAME:
				{
					npcState->listName = it->strValue;
					break;
				}

				case ACTION_SETLISTPNAME:
				{
					npcState->listPluralName = it->strValue;
					break;
				}

				case ACTION_SETAMOUNT:
				{
					int32_t amount = it->intValue;
					if(it->strValue == "|AMOUNT|")
						amount = npcState->amount;

					npcState->amount = amount;
					break;
				}

				case ACTION_TEACHSPELL:
				{
					std::string spellName = it->strValue;
					if(it->strValue == "|SPELL|")
						spellName = npcState->spellName;

					player->learnInstantSpell(spellName);
					break;
				}

				case ACTION_UNTEACHSPELL:
				{
					std::string spellName = it->strValue;
					if(it->strValue == "|SPELL|")
						spellName = npcState->spellName;

					player->unlearnInstantSpell(spellName);
					break;
				}

				case ACTION_SETSTORAGE:
				{
					if(!it->key.empty())
						player->setStorage(it->key, it->strValue);

					break;
				}

				case ACTION_ADDQUEUE:
				{
					if(std::find(queueList.begin(), queueList.end(), player->getID()) == queueList.end())
					{
						queueList.push_back(player->getID());
						npcState->isQueued = true;
					}

					break;
				}

				case ACTION_SELLITEM:
				{
					const ItemType& iit = Item::items[npcState->itemId];
					if(iit.id != 0)
					{
						uint32_t moneyCount = it->intValue;
						if(it->strValue == "|PRICE|")
							moneyCount = npcState->price * npcState->amount;

						int32_t subType = -1;
						if(iit.hasSubType())
							subType = npcState->subType;

						int32_t itemCount = player->__getItemTypeCount(iit.id, subType);
						if(itemCount >= npcState->amount)
						{
							g_game.removeItemOfType(player, iit.id, npcState->amount, subType);
							g_game.addMoney(player, moneyCount, FLAG_NOLIMIT);
						}
					}
					break;
				}

				case ACTION_BUYITEM:
				{
					const ItemType& iit = Item::items[npcState->itemId];
					if(iit.id != 0)
					{
						uint64_t moneyCount = it->intValue;
						if(it->strValue == "|PRICE|")
							moneyCount = npcState->price * npcState->amount;

						int32_t subType = -1;
						if(iit.hasSubType())
							subType = npcState->subType;

						if(g_game.getMoney(player) >= moneyCount)
						{
							int32_t amount = npcState->amount;
							if(iit.stackable)
							{
								while(amount > 0)
								{
									int32_t stack = std::min(100, amount);
									Item* item = Item::CreateItem(iit.id, stack);
									if(g_game.internalPlayerAddItem(this, player, item) != RET_NOERROR)
									{
										delete item;
										amount = npcState->amount - amount;
										break;
									}

									amount -= stack;
								}
							}
							else
							{
								for(int32_t i = 0; i < amount; ++i)
								{
									Item* item = Item::CreateItem(iit.id, subType);
									if(g_game.internalPlayerAddItem(this, player, item) != RET_NOERROR)
									{
										delete item;
										amount = i + 1;
										break;
									}
								}
							}

							if(it->strValue == "|PRICE|")
								moneyCount = npcState->price * amount;

							g_game.removeMoney(player, moneyCount);
						}
					}
					break;
				}

				case ACTION_TAKEITEM:
				{
					int32_t itemId = it->intValue;
					if(it->strValue == "|ITEM|")
						itemId = npcState->itemId;

					const ItemType& iit = Item::items[npcState->itemId];
					if(iit.id != 0)
					{
						int32_t subType = -1;
						if(iit.hasSubType())
							subType = npcState->subType;

						int32_t itemCount = player->__getItemTypeCount(itemId, subType);
						if(itemCount >= npcState->amount)
							g_game.removeItemOfType(player, itemId, npcState->amount, subType);
					}
					break;
				}

				case ACTION_GIVEITEM:
				{
					int32_t itemId = it->intValue;
					if(it->strValue == "|ITEM|")
						itemId = npcState->itemId;

					const ItemType& iit = Item::items[itemId];
					if(iit.id != 0)
					{
						int32_t subType = -1;
						if(iit.hasSubType())
							subType = npcState->subType;

						for(int32_t i = 0; i < npcState->amount; ++i)
						{
							Item* item = Item::CreateItem(iit.id, subType);
							if(g_game.internalPlayerAddItem(this, player, item) != RET_NOERROR)
								delete item;
						}
					}
					break;
				}

				case ACTION_TAKEMONEY:
				{
					uint32_t moneyCount = 0;
					if(it->strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = it->intValue;

					g_game.removeMoney(player, moneyCount);
					break;
				}

				case ACTION_GIVEMONEY:
				{
					uint32_t moneyCount = 0;
					if(it->strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = it->intValue;

					g_game.addMoney(player, moneyCount);
					break;
				}

				case ACTION_SCRIPT:
				{
					NpcScript interface;
					interface.loadDirectory(getFilePath(FILE_TYPE_OTHER, "npc/lib"), false, false, this);
					if(interface.reserveEnv())
					{
						ScriptEnviroment* env = m_interface->getEnv();
						std::stringstream scriptstream;

						//attach various variables that could be interesting
						scriptstream << "local cid = " << env->addThing(player) << std::endl;
						scriptstream << "local text = \"" << npcState->respondToText << "\"" << std::endl;
						scriptstream << "local name = \"" << player->getName() << "\"" << std::endl;
						scriptstream << "local idletime = " << idleTime << std::endl;
						scriptstream << "local idleinterval = " << idleInterval << std::endl;

						scriptstream << "local itemlist = {" << std::endl;
						uint32_t n = 0;
						for(std::list<ListItem>::const_iterator iit = response->prop.itemList.begin(); iit != response->prop.itemList.end(); ++iit)
						{
							scriptstream << "{id = " << iit->itemId
								<< ", subType = " << iit->subType
								<< ", buy = " << iit->buyPrice
								<< ", sell = " << iit->sellPrice
								<< ", name = '" << iit->name << "'}";

							++n;
							if(n != response->prop.itemList.size())
								scriptstream << "," << std::endl;
						}

						scriptstream << "}" << std::endl;
						scriptstream << "_state = {" << std::endl;
						scriptstream << "topic = " << npcState->topic << ',' << std::endl;
						scriptstream << "itemid = " << npcState->itemId << ',' << std::endl;
						scriptstream << "subtype = " << npcState->subType << ',' << std::endl;
						scriptstream << "ignore = " << npcState->ignore << ',' << std::endl;
						scriptstream << "ignorecapacity = ignore," << std::endl;
						scriptstream << "ignoreequipped = ignore," << std::endl;
						scriptstream << "inbackpacks = " << npcState->inBackpacks << ',' << std::endl;
						scriptstream << "amount = " << npcState->amount << ',' << std::endl;
						scriptstream << "price = " << npcState->price << ',' << std::endl;
						scriptstream << "level = " << npcState->level << ',' << std::endl;
						scriptstream << "spellname = \"" << npcState->spellName << "\"" << ',' << std::endl;
						scriptstream << "listname = \"" << npcState->listName << "\"" << ',' << std::endl;
						scriptstream << "listpname = \"" << npcState->listPluralName << "\"" << ',' << std::endl;

						scriptstream << "n1 = " << npcState->scriptVars.n1 << ',' << std::endl;
						scriptstream << "n2 = " << npcState->scriptVars.n2 << ',' << std::endl;
						scriptstream << "n3 = " << npcState->scriptVars.n3 << ',' << std::endl;

						scriptstream << "b1 = " << (npcState->scriptVars.b1 ? "true" : "false") << ',' << std::endl;
						scriptstream << "b2 = " << (npcState->scriptVars.b2 ? "true" : "false") << ',' << std::endl;
						scriptstream << "b3 = " << (npcState->scriptVars.b3 ? "true" : "false") << ',' << std::endl;

						scriptstream << "s1 = \"" << npcState->scriptVars.s1 << "\"" << ',' << std::endl;
						scriptstream << "s2 = \"" << npcState->scriptVars.s2 << "\"" << ',' << std::endl;
						scriptstream << "s3 = \"" << npcState->scriptVars.s3 << "\"" << std::endl;
						scriptstream << "}" << std::endl;

						scriptstream << it->strValue;
						if(interface.loadBuffer(scriptstream.str(), this))
						{
							lua_State* L = interface.getState();
							lua_getglobal(L, "_state");
							NpcScript::popState(L, npcState);
						}

						interface.releaseEnv();
					}

					break;
				}

				default:
					break;
			}
		}

		if(response->getResponseType() == RESPONSE_DEFAULT)
		{
			std::string responseString = formatResponse(player, npcState, response);
			if(!responseString.empty())
			{
				if(!response->publicize())
					doSay(responseString, MSG_NPC_FROM, player);
				else
					doSay(responseString, MSG_SPEAK_SAY, NULL);
			}
		}
		else
		{
			int32_t functionId = -1;
			ResponseScriptMap::iterator it = responseScriptMap.find(response->getText());
			if(it != responseScriptMap.end())
				functionId = it->second;
			else
			{
				functionId = m_interface->getEvent(response->getText());
				responseScriptMap[response->getText()] = functionId;
			}

			if(functionId != -1)
			{
				if(m_interface->reserveEnv())
				{
					ScriptEnviroment* env = m_interface->getEnv();
					lua_State* L = m_interface->getState();

					env->setScriptId(functionId, m_interface);
					env->setRealPos(getPosition());

					Npc* prevNpc = env->getNpc();
					env->setNpc(this);
					m_interface->pushFunction(functionId);

					int32_t paramCount = 0;
					for(ActionList::const_iterator it = response->getFirstAction(); it != response->getEndAction(); ++it)
					{
						if(it->actionType == ACTION_SCRIPTPARAM)
						{
							if(it->strValue == "|PLAYER|")
								lua_pushnumber(L, env->addThing(player));
							else if(it->strValue == "|TEXT|")
								lua_pushstring(L, npcState->respondToText.c_str());
							else
							{
								std::clog << "[Warning - Npc::executeResponse] Unknown script param: " << it->strValue << std::endl;
								break;
							}

							++paramCount;
						}
					}

					NpcScript::pushState(L, npcState);
					lua_setglobal(L, "_state");
					m_interface->callFunction(paramCount);
					lua_getglobal(L, "_state");

					NpcScript::popState(L, npcState);
					if(prevNpc)
					{
						env->setRealPos(prevNpc->getPosition());
						env->setNpc(prevNpc);
					}

					m_interface->releaseEnv();
				}
				else
					std::clog << "[Error] Call stack overflow." << std::endl;
			}
		}

		if(resetTopic && response->getTopic() == npcState->topic)
			npcState->topic = -1;

		npcState->prevInteraction = OTSYS_TIME();
	}
}

void Npc::doSay(const std::string& text, MessageClasses type, Player* player)
{
	if(!player)
	{
		std::string tmp = text;
		replaceString(tmp, "{", "");
		replaceString(tmp, "}", "");
		g_game.internalCreatureSay(this, type, tmp, player && player->isGhost());
	}
	else
	{
		player->sendCreatureSay(this, type, text);
		player->onCreatureSay(this, type, text);
	}
}

uint32_t Npc::getListItemPrice(uint16_t itemId, ShopEvent_t type)
{
	std::list<ListItem> itemList;
	for(ItemListMap::iterator it = itemListMap.begin(); it != itemListMap.end(); ++it)
	{
		itemList = it->second;
		for(std::list<ListItem>::iterator iit = itemList.begin(); iit != itemList.end(); ++iit)
		{
			if(iit->itemId != itemId)
				continue;

			if(type == SHOPEVENT_BUY)
				return iit->buyPrice;

			if(type == SHOPEVENT_SELL)
				return iit->sellPrice;
		}
	}

	return 0;
}

void Npc::onPlayerTrade(Player* player, ShopEvent_t type, int32_t callback, uint16_t itemId, uint8_t count,
	uint8_t amount, bool ignore/* = false*/, bool inBackpacks/* = false*/)
{
	if(type == SHOPEVENT_BUY)
	{
		if(NpcState* npcState = getState(player, true))
		{
			if(amount <= 0)
				amount = 1;

			npcState->amount = amount;
			npcState->subType = count;
			npcState->itemId = itemId;
			npcState->buyPrice = getListItemPrice(itemId, SHOPEVENT_BUY);
			npcState->ignore = ignore;
			npcState->inBackpacks = inBackpacks;

			const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPBUY);
			executeResponse(player, npcState, response);
		}
	}
	else if(type == SHOPEVENT_SELL)
	{
		if(NpcState* npcState = getState(player, true))
		{
			npcState->amount = amount;
			npcState->subType = count;
			npcState->itemId = itemId;
			npcState->sellPrice = getListItemPrice(itemId, SHOPEVENT_SELL);
			npcState->ignore = ignore;

			const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPSELL);
			executeResponse(player, npcState, response);
		}
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerTrade(player, callback, itemId, count, amount, ignore, inBackpacks);

	player->sendGoods();
}

void Npc::onPlayerEndTrade(Player* player, int32_t buyCallback, int32_t sellCallback)
{
	lua_State* L = getInterface()->getState();
	if(buyCallback != -1)
		luaL_unref(L, LUA_REGISTRYINDEX, buyCallback);
	if(sellCallback != -1)
		luaL_unref(L, LUA_REGISTRYINDEX, sellCallback);

	removeShopPlayer(player);
	if(NpcState* npcState = getState(player, true))
	{
		const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPCLOSE);
		executeResponse(player, npcState, response);
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerEndTrade(player);
}

bool Npc::getNextStep(Direction& dir, uint32_t& flags)
{
	if(Creature::getNextStep(dir, flags))
		return true;

	if(!walkTicks || !isIdle || focusCreature || getTimeSinceLastMove() < walkTicks)
		return false;

	return getRandomStep(dir);
}

bool Npc::canWalkTo(const Position& fromPos, Direction dir)
{
	if(cannotMove)
		return false;

	Position toPos = getNextPosition(dir, fromPos);
	if(!Spawns::getInstance()->isInZone(masterPosition, masterRadius, toPos))
		return false;

	Tile* tile = g_game.getTile(toPos);
	if(!tile || g_game.isSwimmingPool(NULL, getTile(), false) != g_game.isSwimmingPool(NULL, tile,
		false) || (!floorChange && (tile->floorChange() || tile->positionChange())))
		return false;

	return tile->__queryAdd(0, this, 1, FLAG_PATHFINDING) == RET_NOERROR;
}

bool Npc::getRandomStep(Direction& dir)
{
	std::vector<Direction> dirList;
	for(int32_t i = NORTH; i < SOUTHWEST; ++i)
	{
		if(canWalkTo(getPosition(), (Direction)i))
			dirList.push_back((Direction)i);
	}

	if(dirList.empty())
		return false;

	std::random_shuffle(dirList.begin(), dirList.end());
	dir = dirList[random_range(0, dirList.size() - 1)];
	return true;
}

void Npc::setCreatureFocus(Creature* creature)
{
	if(!creature)
	{
		if(!walkTicks)
			g_game.internalCreatureTurn(this, baseDirection);

		focusCreature = 0;
		return;
	}

	Position pos = creature->getPosition(), _pos = getPosition();
	int32_t dx = _pos.x - pos.x, dy = _pos.y - pos.y;

	float tan = 10;
	if(dx != 0)
		tan = dy / dx;

	Direction dir = SOUTH;
	if(std::abs(tan) < 1)
	{
		if(dx > 0)
			dir = WEST;
		else
			dir = EAST;
	}
	else if(dy > 0)
		dir = NORTH;

	g_game.internalCreatureTurn(this, dir);
	focusCreature = creature->getID();
}

const NpcResponse* Npc::getResponse(const ResponseList& list, const Player* player,
	NpcState* npcState, const std::string& text, bool exactMatch /*= false*/)
{
	std::string textString = asLowerCaseString(text);
	StringVec wordList = explodeString(textString, " ");
	int32_t bestMatchCount = 0, totalMatchCount = 0;

	NpcResponse* response = NULL;
	for(ResponseList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		int32_t matchCount = 0;
		if((*it)->getParams() != RESPOND_DEFAULT)
		{
			uint32_t params = (*it)->getParams();
			if(hasBitSet(RESPOND_MALE, params))
			{
				if(!player->getSex(false))
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_FEMALE, params))
			{
				if(player->getSex(false))
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_PZBLOCK, params))
			{
				if(!player->isPzLocked())
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_PREMIUM, params))
			{
				if(!player->isPremium())
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_PROMOTED, params))
			{
				Vocation* tmpVoc = player->vocation;

				if(tmpVoc->getId() == VOCATION_NONE ||
				tmpVoc->getId() == VOCATION_SORCERER ||
				tmpVoc->getId() == VOCATION_DRUID ||
				tmpVoc->getId() == VOCATION_KNIGHT ||
				tmpVoc->getId() == VOCATION_PALADIN)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_DRUID, params))
			{
				Vocation* tmpVoc = player->vocation;
				for(uint32_t i = 0; i <= player->promotionLevel; ++i)
					tmpVoc = Vocations::getInstance()->getVocation(tmpVoc->getFromVocation());

				if(tmpVoc->getId() != VOCATION_DRUID)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_KNIGHT, params))
			{
				Vocation* tmpVoc = player->vocation;
				for(uint32_t i = 0; i <= player->promotionLevel; ++i)
					tmpVoc = Vocations::getInstance()->getVocation(tmpVoc->getFromVocation());

				if(tmpVoc->getId() != VOCATION_KNIGHT)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_PALADIN, params))
			{
				Vocation* tmpVoc = player->vocation;
				for(uint32_t i = 0; i <= player->promotionLevel; ++i)
					tmpVoc = Vocations::getInstance()->getVocation(tmpVoc->getFromVocation());

				if(tmpVoc->getId() != VOCATION_PALADIN)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_SORCERER, params))
			{
				Vocation* tmpVoc = player->vocation;
				for(uint32_t i = 0; i <= player->promotionLevel; ++i)
					tmpVoc = Vocations::getInstance()->getVocation(tmpVoc->getFromVocation());

				if(tmpVoc->getId() != VOCATION_SORCERER)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWLEVEL, params))
			{
				if((int32_t)player->getLevel() >= npcState->level)
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWMONEY, params))
			{
				if(g_game.getMoney(player) >= (uint64_t)(npcState->price * npcState->amount))
					continue;

				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWAMOUNT, params) || hasBitSet(RESPOND_NOAMOUNT, params))
			{
				if((signed)player->__getItemTypeCount(npcState->itemId) >= npcState->amount)
					continue;

				if(hasBitSet(RESPOND_LOWAMOUNT, params))
				{
					if(npcState->amount == 1)
						continue;

					++matchCount;
				}

				if(hasBitSet(RESPOND_NOAMOUNT, params))
				{
					if(npcState->amount > 1)
						continue;

					++matchCount;
				}
			}
		}

		if((*it)->getKnowSpell() != "")
		{
			std::string spellName = (*it)->getKnowSpell();
			if(spellName == "|SPELL|")
				spellName = npcState->spellName;

			if(!player->hasLearnedInstantSpell(spellName))
				continue;

			++matchCount;
		}

		if((*it)->scriptVars.b1)
		{
			if(!npcState->scriptVars.b1)
				continue;

			++matchCount;
		}

		if((*it)->scriptVars.b2)
		{
			if(!npcState->scriptVars.b2)
				continue;

			++matchCount;
		}

		if((*it)->scriptVars.b3)
		{
			if(!npcState->scriptVars.b3)
				continue;

			++matchCount;
		}

		if(!(*it)->getStorageId().empty())
		{
			std::string value, storageValue = (*it)->getStorage();
			player->getStorage((*it)->getStorageId(), value);
			if(asLowerCaseString(storageValue) == "_time")
			{
				std::stringstream s;
				s << time(NULL);
				storageValue = s.str();
			}

			bool tmp = false;
			switch((*it)->getStorageComp())
			{
				case STORAGE_LESS:
				{
					int32_t v1 = atoi(value.c_str()), v2 = atoi(storageValue.c_str());
					tmp = v1 < v2;
					break;
				}
				case STORAGE_LESSOREQUAL:
				{
					int32_t v1 = atoi(value.c_str()), v2 = atoi(storageValue.c_str());
					tmp = v1 <= v2;
					break;
				}
				case STORAGE_EQUAL:
				{
					tmp = value == storageValue;
					break;
				}
				case STORAGE_NOTEQUAL:
				{
					tmp = value != storageValue;
					break;
				}
				case STORAGE_GREATEROREQUAL:
				{
					int32_t v1 = atoi(value.c_str()), v2 = atoi(storageValue.c_str());
					tmp = v1 >= v2;
					break;
				}
				case STORAGE_GREATER:
				{
					int32_t v1 = atoi(value.c_str()), v2 = atoi(storageValue.c_str());
					tmp = v1 > v2;
					break;
				}
				default:
					break;
			}

			if(!tmp)
				continue;

			++matchCount;
		}

		if((*it)->getInteractType() == INTERACT_TEXT || (*it)->getFocusState() != -1)
		{
			if(npcState->isIdle && (*it)->getFocusState() != 1) //We are idle, and this response does not activate the npc.
				continue;

			if(!npcState->isIdle && (*it)->getFocusState() == 1) //We are not idle, and this response would activate us again.
				continue;
		}

		if(npcState->topic == -1 && (*it)->getTopic() != -1) //Not the right topic
			continue;

		if(npcState->topic != -1 && npcState->topic == (*it)->getTopic()) //Topic is right
			matchCount += 1000;

		if((*it)->getInteractType() == INTERACT_EVENT)
		{
			if((*it)->getInputText() == asLowerCaseString(text))
				++matchCount;
			else
				matchCount = 0;
		}
		else if((*it)->getInteractType() == INTERACT_TEXT)
		{
			int32_t matchAllCount = 0, totalKeywordCount = 0, matchWordCount = getMatchCount(
				*it, wordList, exactMatch, matchAllCount, totalKeywordCount);
			if(matchWordCount> 0)
			{
				//Remove points for |*| matches
				matchWordCount -= matchAllCount;
				//Remove points for not full match
				matchWordCount -= (totalKeywordCount - matchAllCount - matchWordCount);
				//Total "points" for this response, word matches are worth more
				matchCount += matchWordCount * 100000;
			}
			else
				matchCount = 0;
		}

		if(matchCount > bestMatchCount)
		{
			totalMatchCount = 0;
			response = (*it);
			bestMatchCount = matchCount;
		}
		else if(bestMatchCount > 0 && matchCount == bestMatchCount)
			++totalMatchCount;
	}

	if(totalMatchCount > 1)
		return NULL;

	return response;
}

uint32_t Npc::getMatchCount(NpcResponse* response, StringVec wordList,
	bool exactMatch, int32_t& matchAllCount, int32_t& totalKeywordCount)
{
	int32_t bestMatchCount = matchAllCount = totalKeywordCount = 0;
	const std::list<std::string>& inputList = response->getInputList();
	for(std::list<std::string>::const_iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		std::string keywords = (*it), tmpKit;
		StringVec::iterator lastWordMatch = wordList.begin();

		int32_t matchCount = 0;
		StringVec keywordList = explodeString(keywords, ";");
		for(StringVec::iterator kit = keywordList.begin(); kit != keywordList.end(); ++kit)
		{
			tmpKit = asLowerCaseString(*kit);
			if(!exactMatch && (*kit) == "|*|") //Match anything.
				matchAllCount++;
			else if(tmpKit == "|amount|")
			{
				//TODO: Should iterate through each word until a number or a new keyword is found.
				int32_t amount = atoi((*lastWordMatch).c_str());
				if(amount <= 0)
				{
					response->setAmount(1);
					continue;
				}

				response->setAmount(amount);
			}
			else
			{
				StringVec::iterator wit = wordList.end();
				for(wit = lastWordMatch; wit != wordList.end(); ++wit)
				{
					size_t pos = (*wit).find_first_of("!\"#�%&/()=?`{[]}\\^*><,.-_~");
					if(pos == std::string::npos)
						pos = 0;

					if((*wit).find((*kit), pos) == pos)
						break;
				}

				if(wit == wordList.end())
					continue;

				lastWordMatch = wit + 1;
			}

			++matchCount;
			if(matchCount > bestMatchCount)
			{
				bestMatchCount = matchCount;
				totalKeywordCount = keywordList.size();
			}

			if(lastWordMatch == wordList.end())
				break;
		}
	}

	return bestMatchCount;
}

const NpcResponse* Npc::getResponse(const Player* player, NpcState* npcState, const std::string& text)
{
	return getResponse(responseList, player, npcState, text);
}

const NpcResponse* Npc::getResponse(const Player*, NpcEvent_t eventType)
{
	std::string eventName = getEventResponseName(eventType);
	if(eventName.empty())
		return NULL;

	std::vector<NpcResponse*> result;
	for(ResponseList::const_iterator it = responseList.begin(); it != responseList.end(); ++it)
	{
		if((*it)->getInteractType() != INTERACT_EVENT)
			continue;

		if((*it)->getInputText() == asLowerCaseString(eventName))
			result.push_back(*it);
	}

	if(result.empty())
		return NULL;

	return result[random_range(0, result.size() - 1)];
}

const NpcResponse* Npc::getResponse(const Player* player, NpcState* npcState, NpcEvent_t eventType)
{
	std::string eventName = getEventResponseName(eventType);
	if(eventName.empty())
		return NULL;

	return getResponse(responseList, player, npcState, eventName, true);
}

std::string Npc::getEventResponseName(NpcEvent_t eventType)
{
	switch(eventType)
	{
		case EVENT_BUSY:
			return "onBusy";
		case EVENT_THINK:
			return "onThink";
		case EVENT_IDLE:
			return "onIdle";
		case EVENT_PLAYER_ENTER:
			return "onPlayerEnter";
		case EVENT_PLAYER_MOVE:
			return "onPlayerMove";
		case EVENT_PLAYER_LEAVE:
			return "onPlayerLeave";
		case EVENT_PLAYER_SHOPSELL:
			return "onPlayerShopSell";
		case EVENT_PLAYER_SHOPBUY:
			return "onPlayerShopBuy";
		case EVENT_PLAYER_SHOPCLOSE:
			return "onPlayerShopClose";
		case EVENT_PLAYER_CHATCLOSE:
			return "onPlayerChatClose";
		default:
			break;
	}

	return "";
}

std::string Npc::formatResponse(Creature* creature, const NpcState* npcState, const NpcResponse* response) const
{
	std::string responseString = response->getText();

	std::stringstream ss;
	ss << npcState->price * npcState->amount;
	replaceString(responseString, "|PRICE|", ss.str());

	ss.str("");
	ss << npcState->amount;
	replaceString(responseString, "|AMOUNT|", ss.str());

	ss.str("");
	ss << npcState->level;
	replaceString(responseString, "|LEVEL|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n1;
	replaceString(responseString, "|N1|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n2;
	replaceString(responseString, "|N2|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n3;
	replaceString(responseString, "|N3|", ss.str());

	replaceString(responseString, "|S1|", npcState->scriptVars.s1);
	replaceString(responseString, "|S2|", npcState->scriptVars.s2);
	replaceString(responseString, "|S3|", npcState->scriptVars.s3);

	ss.str("");
	if(npcState->itemId != -1)
	{
		const ItemType& it = Item::items[npcState->itemId];
		if(npcState->amount <= 1)
		{
			ss << it.article + " " + it.name;
			replaceString(responseString, "|ITEMNAME|", ss.str());
		}
		else
		{
			ss << it.pluralName;
			replaceString(responseString, "|ITEMNAME|", ss.str());
		}
	}

	replaceString(responseString, "|NAME|", creature->getName());
	replaceString(responseString, "|NPCNAME|", nType->name);
	return responseString;
}

void Npc::addShopPlayer(Player* player)
{
	ShopPlayerList::iterator it = std::find(shopPlayerList.begin(), shopPlayerList.end(), player);
	if(it == shopPlayerList.end())
		shopPlayerList.push_back(player);
}

void Npc::removeShopPlayer(const Player* player)
{
	ShopPlayerList::iterator it = std::find(shopPlayerList.begin(), shopPlayerList.end(), player);
	if(it != shopPlayerList.end())
		shopPlayerList.erase(it);
}

void Npc::closeAllShopWindows()
{
	ShopPlayerList closeList = shopPlayerList;
	for(ShopPlayerList::iterator it = closeList.begin(); it != closeList.end(); ++it)
		(*it)->closeShopWindow();
}

NpcScript* Npc::getInterface()
{
	return m_interface;
}

NpcScript::NpcScript():
	LuaInterface("NpcScript Interface")
{
	initState();
}

void NpcScript::registerFunctions()
{
	LuaInterface::registerFunctions();
	lua_register(m_luaState, "selfFocus", NpcScript::luaActionFocus);
	lua_register(m_luaState, "selfSay", NpcScript::luaActionSay);
	lua_register(m_luaState, "selfFollow", NpcScript::luaActionFollow);

	lua_register(m_luaState, "getNpcId", NpcScript::luaGetNpcId);
	lua_register(m_luaState, "getNpcParameter", NpcScript::luaGetNpcParameter);

	lua_register(m_luaState, "getNpcState", NpcScript::luaGetNpcState);
	lua_register(m_luaState, "setNpcState", NpcScript::luaSetNpcState);

	lua_register(m_luaState, "openShopWindow", NpcScript::luaOpenShopWindow);
	lua_register(m_luaState, "closeShopWindow", NpcScript::luaCloseShopWindow);
	lua_register(m_luaState, "getShopOwner", NpcScript::luaGetShopOwner);
}

int32_t NpcScript::luaActionFocus(lua_State* L)
{
	//selfFocus(cid)
	ScriptEnviroment* env = getEnv();
	Npc* npc = env->getNpc();
	if(!npc)
		return 0;

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(creature)
		npc->hasScriptedFocus = true;
	else
		npc->hasScriptedFocus = false;

	npc->setCreatureFocus(creature);
	return 0;
}

int32_t NpcScript::luaActionSay(lua_State* L)
{
	//selfSay(words[, target[, type]])
	int32_t params = lua_gettop(L), target = 0;
	MessageClasses type = MSG_NONE;
	if(params > 2)
		type = (MessageClasses)popNumber(L);

	if(params > 1)
		target = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Npc* npc = env->getNpc();
	if(!npc)
		return 0;

	Player* player = env->getPlayerByUID(target);
	if(type == MSG_NONE)
	{
		if(player)
			type = MSG_NPC_FROM;
		else
			type = MSG_SPEAK_SAY;
	}

	npc->doSay(popString(L), (MessageClasses)type, player);
	return 0;
}

int32_t NpcScript::luaActionFollow(lua_State* L)
{
	//selfFollow(cid)
	uint32_t cid = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(cid);
	if(cid && !creature)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	Npc* npc = env->getNpc();
	if(!npc)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, npc->setFollowCreature(creature, true));
	return 1;
}

int32_t NpcScript::luaGetNpcId(lua_State* L)
{
	//getNpcId()
	ScriptEnviroment* env = getEnv();
	if(Npc* npc = env->getNpc())
		lua_pushnumber(L, env->addThing(npc));
	else
		lua_pushnil(L);

	return 1;
}

int32_t NpcScript::luaGetNpcParameter(lua_State* L)
{
	//getNpcParameter(key)
	ScriptEnviroment* env = getEnv();
	if(Npc* npc = env->getNpc())
	{
		Npc::ParametersMap::iterator it = npc->m_parameters.find(popString(L));
		if(it != npc->m_parameters.end())
			lua_pushstring(L, it->second.c_str());
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);

	return 1;
}

int32_t NpcScript::luaGetNpcState(lua_State* L)
{
	//getNpcState(cid)
	ScriptEnviroment* env = getEnv();
	Npc* npc = env->getNpc();

	const Player* player = env->getPlayerByUID(popNumber(L));
	if(player && npc)
		NpcScript::pushState(L, npc->getState(player));
	else
		lua_pushnil(L);

	return 1;
}

int32_t NpcScript::luaSetNpcState(lua_State* L)
{
	//setNpcState(state, cid)
	ScriptEnviroment* env = getEnv();
	Npc* npc = env->getNpc();

	const Player* player = env->getPlayerByUID(popNumber(L));
	if(player && npc)
	{
		NpcState* tmp = npc->getState(player);
		NpcScript::popState(L, tmp);
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

void NpcScript::pushState(lua_State* L, NpcState* state)
{
	lua_newtable(L);
	setField(L, "price", state->price);
	setField(L, "amount", state->amount);
	setField(L, "itemid", state->itemId);
	setField(L, "subtype", state->subType);
	setFieldBool(L, "ignore", state->ignore);
	setFieldBool(L, "ignorecapacity", state->ignore);
	setFieldBool(L, "ignoreequipped", state->ignore);
	setFieldBool(L, "inbackpacks", state->inBackpacks);
	setField(L, "topic", state->topic);
	setField(L, "level", state->level);
	setField(L, "spellname", state->spellName);
	setField(L, "listname", state->listName);
	setField(L, "listpname", state->listPluralName);
	setFieldBool(L, "isidle", state->isIdle);

	setField(L, "n1", state->scriptVars.n1);
	setField(L, "n2", state->scriptVars.n2);
	setField(L, "n3", state->scriptVars.n3);

	setFieldBool(L, "b1", state->scriptVars.b1);
	setFieldBool(L, "b2", state->scriptVars.b2);
	setFieldBool(L, "b3", state->scriptVars.b3);

	setField(L, "s1", state->scriptVars.s1);
	setField(L, "s2", state->scriptVars.s2);
	setField(L, "s3", state->scriptVars.s3);
}

void NpcScript::popState(lua_State* L, NpcState* &state)
{
	state->price = getField(L, "price");
	state->amount = getField(L, "amount");
	state->itemId = getField(L, "itemid");
	state->subType = getField(L, "subtype");
	state->ignore = getFieldBool(L, "ignore") || getFieldBool(L,
		"ignorecapacity") || getFieldBool(L, "ignoreequipped");
	state->inBackpacks = getFieldBool(L, "inbackpacks");
	state->topic = getField(L, "topic");
	state->level = getField(L, "level");
	state->spellName = getFieldString(L, "spellname");
	state->listName = getFieldString(L, "listname");
	state->listPluralName = getFieldString(L, "listpname");
	state->isIdle = getFieldBool(L, "isidle");

	state->scriptVars.n1 = getField(L, "n1");
	state->scriptVars.n2 = getField(L, "n2");
	state->scriptVars.n3 = getField(L, "n3");

	state->scriptVars.b1 = getFieldBool(L, "b1");
	state->scriptVars.b2 = getFieldBool(L, "b2");
	state->scriptVars.n3 = getFieldBool(L, "b3");

	state->scriptVars.s1 = getFieldString(L, "s1");
	state->scriptVars.s2 = getFieldString(L, "s2");
	state->scriptVars.s3 = getFieldString(L, "s3");
}

int32_t NpcScript::luaOpenShopWindow(lua_State* L)
{
	//openShopWindow(cid, items, onBuy callback, onSell callback)
	ScriptEnviroment* env = getEnv();
	Npc* npc = env->getNpc();
	if(!npc)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	int32_t sellCallback = -1;
	if(!lua_isfunction(L, -1))
		lua_pop(L, 1); // skip it - use default value
	else
		sellCallback = popCallback(L);

	int32_t buyCallback = -1;
	if(!lua_isfunction(L, -1))
		lua_pop(L, 1);
	else
		buyCallback = popCallback(L);

	if(!lua_istable(L, -1))
	{
		error(__FUNCTION__, "Item list is not a table");
		lua_pushboolean(L, false);
		return 1;
	}

	ShopInfoList itemList;
	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		ShopInfo item;
		item.itemId = getField(L, "id");
		item.subType = getField(L, "subType");
		item.buyPrice = getField(L, "buy");
		item.sellPrice = getField(L, "sell");
		item.itemName = getFieldString(L, "name");

		itemList.push_back(item);
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(player->getShopOwner() == npc)
	{
		lua_pushboolean(L, true);
		return 1;
	}

	player->closeShopWindow(false);
	npc->addShopPlayer(player);

	player->setShopOwner(npc, buyCallback, sellCallback, itemList);
	player->openShopWindow(npc);

	lua_pushboolean(L, true);
	return 1;
}

int32_t NpcScript::luaCloseShopWindow(lua_State* L)
{
	//closeShopWindow(cid)
	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	player->closeShopWindow();
	lua_pushboolean(L, true);
	return 1;
}

int32_t NpcScript::luaGetShopOwner(lua_State* L)
{
	//getShopOwner(cid)
	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(Npc* npc = player->getShopOwner())
		lua_pushnumber(L, env->addThing(npc));
	else
		lua_pushnil(L);

	return 1;
}

NpcEvents::NpcEvents(std::string file, Npc* npc):
	m_npc(npc), m_interface(npc->getInterface())
{
	if(!m_interface->loadFile(file, npc))
	{
		std::clog << "[Warning - NpcEvents::NpcEvents] Cannot load script: " << file
			<< std::endl << m_interface->getLastError() << std::endl;
		m_loaded = false;
		return;
	}

	m_onCreatureSay = m_interface->getEvent("onCreatureSay");
	m_onCreatureDisappear = m_interface->getEvent("onCreatureDisappear");
	m_onCreatureAppear = m_interface->getEvent("onCreatureAppear");
	m_onCreatureMove = m_interface->getEvent("onCreatureMove");
	m_onPlayerCloseChannel = m_interface->getEvent("onPlayerCloseChannel");
	m_onPlayerEndTrade = m_interface->getEvent("onPlayerEndTrade");
	m_onThink = m_interface->getEvent("onThink");
	m_loaded = true;
}

void NpcEvents::onCreatureAppear(const Creature* creature)
{
	if(m_onCreatureAppear == -1)
		return;

	//onCreatureAppear(cid)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onCreatureAppear, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onCreatureAppear);
		lua_pushnumber(L, env->addThing(const_cast<Creature*>(creature)));

		m_interface->callFunction(1);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onCreatureAppear] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onCreatureDisappear(const Creature* creature)
{
	if(m_onCreatureDisappear == -1)
		return;

	//onCreatureDisappear(cid)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onCreatureDisappear, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onCreatureDisappear);
		lua_pushnumber(L, env->addThing(const_cast<Creature*>(creature)));

		m_interface->callFunction(1);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onCreatureDisappear] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onCreatureMove(const Creature* creature, const Position& oldPos, const Position& newPos)
{
	if(m_onCreatureMove == -1)
		return;

	//onCreatureMove(cid, oldPos, newPos)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onCreatureMove, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onCreatureMove);
		lua_pushnumber(L, env->addThing(const_cast<Creature*>(creature)));

		LuaInterface::pushPosition(L, oldPos, 0);
		LuaInterface::pushPosition(L, newPos, 0);

		m_interface->callFunction(3);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onCreatureMove] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* /*pos = NULL*/)
{
	if(m_onCreatureSay == -1)
		return;

	//onCreatureSay(cid, type, msg)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onCreatureSay, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onCreatureSay);
		lua_pushnumber(L, env->addThing(const_cast<Creature*>(creature)));

		lua_pushnumber(L, type);
		lua_pushstring(L, text.c_str());

		m_interface->callFunction(3);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onCreatureSay] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onPlayerTrade(const Player* player, int32_t callback, uint16_t itemid,
	uint8_t count, uint8_t amount, bool ignore, bool inBackpacks)
{
	if(callback == -1)
		return;

	//on"Buy/Sell"(cid, itemid, count, amount, "ignore", inBackpacks)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(-1, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Player*>(player));
		LuaInterface::pushCallback(L, callback);

		lua_pushnumber(L, cid);
		lua_pushnumber(L, itemid);
		lua_pushnumber(L, count);
		lua_pushnumber(L, amount);

		lua_pushboolean(L, ignore);
		lua_pushboolean(L, inBackpacks);

		m_interface->callFunction(6);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onPlayerTrade] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onPlayerEndTrade(const Player* player)
{
	if(m_onPlayerEndTrade == -1)
		return;

	//onPlayerEndTrade(cid)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onPlayerEndTrade, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onPlayerEndTrade);
		lua_pushnumber(L, env->addThing(const_cast<Player*>(player)));

		m_interface->callFunction(1);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onPlayerEndTrade] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onPlayerCloseChannel(const Player* player)
{
	if(m_onPlayerCloseChannel == -1)
		return;

	//onPlayerCloseChannel(cid)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		lua_State* L = m_interface->getState();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onPlayerCloseChannel, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onPlayerCloseChannel);
		lua_pushnumber(L, env->addThing(const_cast<Player*>(player)));

		m_interface->callFunction(1);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onPlayerCloseChannel] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcEvents::onThink()
{
	if(m_onThink == -1)
		return;

	//onThink()
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEvent(desc.str());
		#endif

		env->setScriptId(m_onThink, m_interface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_interface->pushFunction(m_onThink);

		m_interface->callFunction(0);
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - NpcEvents::onThink] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}
