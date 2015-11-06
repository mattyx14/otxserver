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
#include <iostream>
#include <iomanip>

#include "item.h"
#include "container.h"
#include "depot.h"

#include "teleport.h"
#include "trashholder.h"
#include "mailbox.h"

#include "luascript.h"
#include "combat.h"

#include "house.h"
#include "beds.h"

#include "actions.h"
#include "configmanager.h"
#include "game.h"
#include "movement.h"

extern Game g_game;
extern ConfigManager g_config;
extern MoveEvents* g_moveEvents;

Items Item::items;
Item* Item::CreateItem(const uint16_t type, uint16_t amount/* = 0*/)
{
	const ItemType& it = Item::items[type];
	if(it.group == ITEM_GROUP_DEPRECATED)
	{
		#ifdef __DEBUG__
		std::clog << "[Error - Item::CreateItem] Item " << it.id << " has been declared as deprecated" << std::endl;
		#endif
		return NULL;
	}

	if(!it.id)
		return NULL;

	Item* newItem = NULL;
	if(it.isDepot())
		newItem = new Depot(type);
	else if(it.isContainer())
		newItem = new Container(type);
	else if(it.isTeleport())
		newItem = new Teleport(type);
	else if(it.isMagicField())
		newItem = new MagicField(type);
	else if(it.isDoor())
		newItem = new Door(type);
	else if(it.isTrashHolder())
		newItem = new TrashHolder(type, it.magicEffect);
	else if(it.isMailbox())
		newItem = new Mailbox(type);
	else if(it.isBed())
		newItem = new BedItem(type);
	else if(it.id >= 2210 && it.id <= 2212)
		newItem = new Item(type - 3, amount);
	else if(it.id == 2215 || it.id == 2216)
		newItem = new Item(type - 2, amount);
	else if(it.id >= 2202 && it.id <= 2206)
		newItem = new Item(type - 37, amount);
	else if(it.id == 2640)
		newItem = new Item(6132, amount);
	else if(it.id == 6301)
		newItem = new Item(6300, amount);
	else
		newItem = new Item(type, amount);

	newItem->addRef();
	return newItem;
}

Item* Item::CreateItem(PropStream& propStream)
{
	uint16_t type;
	if(!propStream.getShort(type))
		return NULL;

	return Item::CreateItem(items.getRandomizedItem(type), 0);
}

bool Item::loadItem(xmlNodePtr node, Container* parent)
{
	if(!xmlStrcmp(node->name, (const xmlChar*)"item"))
		return false;

	int32_t intValue;
	std::string strValue;

	Item* item = NULL;
	if(readXMLInteger(node, "id", intValue))
		item = Item::CreateItem(intValue);

	if(!item)
		return false;

	if(readXMLString(node, "attributes", strValue))
	{
		StringVec v, attr = explodeString(strValue, ";");
		for(StringVec::iterator it = attr.begin(); it != attr.end(); ++it)
		{
			v = explodeString((*it), ",");
			if(v.size() < 2)
				continue;

			if(atoi(v[1].c_str()) || v[1] == "0")
				item->setAttribute(v[0].c_str(), atoi(v[1].c_str()));
			else
				item->setAttribute(v[0].c_str(), v[1]);
		}
	}

	//compatibility
	if(readXMLInteger(node, "subtype", intValue) || readXMLInteger(node, "subType", intValue))
		item->setSubType(intValue);

	if(readXMLInteger(node, "actionId", intValue) || readXMLInteger(node, "actionid", intValue)
		|| readXMLInteger(node, "aid", intValue))
		item->setActionId(intValue);

	if(readXMLInteger(node, "uniqueId", intValue) || readXMLInteger(node, "uniqueid", intValue)
		|| readXMLInteger(node, "uid", intValue))
		item->setUniqueId(intValue);

	if(readXMLString(node, "text", strValue))
		item->setText(strValue);

	if(item->getContainer())
		loadContainer(node, item->getContainer());

	if(parent)
		parent->addItem(item);

	return true;
}

bool Item::loadContainer(xmlNodePtr parentNode, Container* parent)
{
	for(xmlNodePtr node = parentNode->children; node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;

		if(!xmlStrcmp(node->name, (const xmlChar*)"item") && !loadItem(node, parent))
			return false;
	}

	return true;
}

Item::Item(const uint16_t type, uint16_t amount/* = 0*/):
	ItemAttributes(), id(type)
{
	raid = NULL;
	loadedFromMap = false;

	setItemCount(1);
	setDefaultDuration();

	const ItemType& it = items[type];
	if(it.isFluidContainer() || it.isSplash())
		setFluidType(amount);
	else if(it.stackable)
	{
		if(amount)
			setItemCount(amount);
		else if(it.charges)
			setItemCount(it.charges);
	}
	else if(it.charges)
		setCharges(amount ? amount : it.charges);

	if(it.armorRndMin > 0 && it.armorRndMax > it.armorRndMin)
		setAttribute("armor", it.armorRndMin + rand() % (it.armorRndMax+1 - it.armorRndMin));

	if(it.defenseRndMin > 0 && it.defenseRndMax > it.defenseRndMin)
		setAttribute("defense", it.defenseRndMin + rand() % (it.defenseRndMax+1 - it.defenseRndMin));

	if(it.extraDefenseRndMin > 0 && it.extraDefenseRndMax > it.extraDefenseRndMin)
	if(it.extraDefenseChance == 0 || (it.extraDefenseChance >= rand() % 101) )
		setAttribute("extradefense", it.extraDefenseRndMin + rand() % (it.extraDefenseRndMax+1 - it.extraDefenseRndMin));

	if(it.attackRndMin > 0 && it.attackRndMax > it.attackRndMin)
		setAttribute("attack", it.attackRndMin + rand() % (it.attackRndMax - it.attackRndMin));

	if(it.extraAttackRndMin > 0 && it.extraAttackRndMax > it.extraAttackRndMin)
	if(it.extraAttackChance == 0 || (it.extraAttackChance >= rand() % 101) )
		setAttribute("extraattack", it.extraAttackRndMin + rand() % (it.extraAttackRndMax+1 - it.extraAttackRndMin));

	if(it.attackSpeedRndMin > 0 && it.attackSpeedRndMax > it.attackSpeedRndMin)
	if(it.attackSpeedChance == 0 || (it.attackSpeedChance >= rand() % 101) )
		setAttribute("attackSpeed", it.attackSpeedRndMin + rand() % (it.attackSpeedRndMax+1 - it.attackSpeedRndMin));
}

Item* Item::clone() const
{
	Item* tmp = Item::CreateItem(id, count);
	if(!tmp)
		return NULL;

	if(!attributes || attributes->empty())
		return tmp;

	tmp->createAttributes();
	*tmp->attributes = *attributes;
	tmp->eraseAttribute("uid");
	return tmp;
}

void Item::copyAttributes(Item* item)
{
	if(item && item->attributes && !item->attributes->empty())
	{
		createAttributes();
		*attributes = *item->attributes;
		eraseAttribute("uid");
	}

	eraseAttribute("decaying");
	eraseAttribute("duration");
}

void Item::makeUnique(Item* parent)
{
	if(!parent || !parent->getUniqueId())
		return;

	ScriptEnviroment::removeUniqueThing(parent);
	setUniqueId(parent->getUniqueId());
	parent->eraseAttribute("uid");
}

void Item::onRemoved()
{
	if(raid)
	{
		raid->unRef();
		raid = NULL;
	}

	ScriptEnviroment::removeTempItem(this);
	if(getUniqueId())
		ScriptEnviroment::removeUniqueThing(this);
}

void Item::setDefaultSubtype()
{
	setItemCount(1);
	const ItemType& it = items[id];
	if(it.charges)
		setCharges(it.charges);
}

void Item::setID(uint16_t newId)
{
	const ItemType& it = Item::items[newId];
	const ItemType& pit = Item::items[id];
	id = newId;

	uint32_t newDuration = it.decayTime * 1000;
	if(!newDuration && !it.stopTime && it.decayTo == -1)
	{
		eraseAttribute("decaying");
		eraseAttribute("duration");
	}

	eraseAttribute("corpseowner");
	if(newDuration > 0 && (!pit.stopTime || !hasIntegerAttribute("duration")))
	{
		setDecaying(DECAYING_FALSE);
		setDuration(newDuration);
	}
}

bool Item::floorChange(FloorChange_t change/* = CHANGE_NONE*/) const
{
	if(change < CHANGE_NONE)
		return Item::items[id].floorChange[change];

	for(int32_t i = CHANGE_PRE_FIRST; i < CHANGE_LAST; ++i)
	{
		if(Item::items[id].floorChange[i])
			return true;
	}

	return false;
}

Player* Item::getHoldingPlayer()
{
	for(Cylinder* p = getParent(); p; p = p->getParent())
	{
		if(p->getCreature())
			return p->getCreature()->getPlayer();
	}

	return NULL;
}

const Player* Item::getHoldingPlayer() const
{
	return const_cast<Item*>(this)->getHoldingPlayer();
}

uint16_t Item::getSubType() const
{
	const ItemType& it = items[id];
	if(it.isFluidContainer() || it.isSplash())
		return getFluidType();

	if(it.charges)
		return getCharges();

	return count;
}

void Item::setSubType(uint16_t n)
{
	const ItemType& it = items[id];
	if(it.isFluidContainer() || it.isSplash())
		setFluidType(n);
	else if(it.charges)
		setCharges(n);
	else
		count = n;
}

Attr_ReadValue Item::readAttr(AttrTypes_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case ATTR_COUNT:
		{
			uint8_t _count;
			if(!propStream.getByte(_count))
				return ATTR_READ_ERROR;

			setSubType((uint16_t)_count);
			break;
		}

		case ATTR_ACTION_ID:
		{
			uint16_t aid;
			if(!propStream.getShort(aid))
				return ATTR_READ_ERROR;

			setAttribute("aid", aid);
			break;
		}

		case ATTR_UNIQUE_ID:
		{
			uint16_t uid;
			if(!propStream.getShort(uid))
				return ATTR_READ_ERROR;

			setUniqueId(uid);
			break;
		}

		case ATTR_NAME:
		{
			std::string name;
			if(!propStream.getString(name))
				return ATTR_READ_ERROR;

			setAttribute("name", name);
			break;
		}

		case ATTR_PLURALNAME:
		{
			std::string name;
			if(!propStream.getString(name))
				return ATTR_READ_ERROR;

			setAttribute("pluralname", name);
			break;
		}

		case ATTR_ARTICLE:
		{
			std::string article;
			if(!propStream.getString(article))
				return ATTR_READ_ERROR;

			setAttribute("article", article);
			break;
		}

		case ATTR_ATTACK:
		{
			int32_t attack;
			if(!propStream.getLong((uint32_t&)attack))
				return ATTR_READ_ERROR;

			setAttribute("attack", attack);
			break;
		}

		case ATTR_EXTRAATTACK:
		{
			int32_t attack;
			if(!propStream.getLong((uint32_t&)attack))
				return ATTR_READ_ERROR;

			setAttribute("extraattack", attack);
			break;
		}

		case ATTR_DEFENSE:
		{
			int32_t defense;
			if(!propStream.getLong((uint32_t&)defense))
				return ATTR_READ_ERROR;

			setAttribute("defense", defense);
			break;
		}

		case ATTR_EXTRADEFENSE:
		{
			int32_t defense;
			if(!propStream.getLong((uint32_t&)defense))
				return ATTR_READ_ERROR;

			setAttribute("extradefense", defense);
			break;
		}

		case ATTR_ARMOR:
		{
			int32_t armor;
			if(!propStream.getLong((uint32_t&)armor))
				return ATTR_READ_ERROR;

			setAttribute("armor", armor);
			break;
		}

		case ATTR_ATTACKSPEED:
		{
			int32_t attackSpeed;
			if(!propStream.getLong((uint32_t&)attackSpeed))
				return ATTR_READ_ERROR;

			setAttribute("attackspeed", attackSpeed);
			break;
		}

		case ATTR_HITCHANCE:
		{
			int32_t hitChance;
			if(!propStream.getLong((uint32_t&)hitChance))
				return ATTR_READ_ERROR;

			setAttribute("hitchance", hitChance);
			break;
		}

		case ATTR_SCRIPTPROTECTED:
		{
			uint8_t protection;
			if(!propStream.getByte(protection))
				return ATTR_READ_ERROR;

			setAttribute("scriptprotected", protection != 0);
			break;
		}

		case ATTR_DUALWIELD:
		{
			uint8_t wield;
			if(!propStream.getByte(wield))
				return ATTR_READ_ERROR;

			setAttribute("dualwield", wield != 0);
			break;
		}

		case ATTR_TEXT:
		{
			std::string text;
			if(!propStream.getString(text))
				return ATTR_READ_ERROR;

			setAttribute("text", text);
			break;
		}

		case ATTR_WRITTENDATE:
		{
			int32_t date;
			if(!propStream.getLong((uint32_t&)date))
				return ATTR_READ_ERROR;

			setAttribute("date", date);
			break;
		}

		case ATTR_WRITTENBY:
		{
			std::string writer;
			if(!propStream.getString(writer))
				return ATTR_READ_ERROR;

			setAttribute("writer", writer);
			break;
		}

		case ATTR_DESC:
		{
			std::string text;
			if(!propStream.getString(text))
				return ATTR_READ_ERROR;

			setAttribute("description", text);
			break;
		}

		case ATTR_RUNE_CHARGES:
		{
			uint8_t charges;
			if(!propStream.getByte(charges))
				return ATTR_READ_ERROR;

			setSubType((uint16_t)charges);
			break;
		}

		case ATTR_CHARGES:
		{
			uint16_t charges;
			if(!propStream.getShort(charges))
				return ATTR_READ_ERROR;

			setSubType(charges);
			break;
		}

		case ATTR_DURATION:
		{
			int32_t duration;
			if(!propStream.getLong((uint32_t&)duration))
				return ATTR_READ_ERROR;

			setAttribute("duration", duration);
			break;
		}

		case ATTR_DECAYING_STATE:
		{
			uint8_t state;
			if(!propStream.getByte(state))
				return ATTR_READ_ERROR;

			if((ItemDecayState_t)state != DECAYING_FALSE)
				setAttribute("decaying", (int32_t)DECAYING_PENDING);

			break;
		}

		//these should be handled through derived classes
		//if these are called then something has changed in the items.otb since the map was saved
		//just read the values

		//Depot class
		case ATTR_DEPOT_ID:
		{
			uint16_t depot;
			if(!propStream.getShort(depot))
				return ATTR_READ_ERROR;

			break;
		}

		//Door class
		case ATTR_HOUSEDOORID:
		{
			uint8_t door;
			if(!propStream.getByte(door))
				return ATTR_READ_ERROR;

			break;
		}

		//Teleport class
		case ATTR_TELE_DEST:
		{
			TeleportDest* dest;
			if(!propStream.getStruct(dest))
				return ATTR_READ_ERROR;

			break;
		}

		//Bed class
		case ATTR_SLEEPERGUID:
		{
			uint32_t sleeper;
			if(!propStream.getLong(sleeper))
				return ATTR_READ_ERROR;

			break;
		}

		case ATTR_SLEEPSTART:
		{
			uint32_t sleepStart;
			if(!propStream.getLong(sleepStart))
				return ATTR_READ_ERROR;

			break;
		}

		//Container class
		case ATTR_CONTAINER_ITEMS:
		{
			uint32_t _count;
			propStream.getLong(_count);
			return ATTR_READ_ERROR;
		}

		//ItemAttributes class
		case ATTR_ATTRIBUTE_MAP:
		{
			bool unique = hasIntegerAttribute("uid"), ret = unserializeMap(propStream);
			if(!unique && hasIntegerAttribute("uid")) // unfortunately we have to do this
				ScriptEnviroment::addUniqueThing(this);

			// this attribute has a custom behavior as well
			if(getDecaying() != DECAYING_FALSE)
				setDecaying(DECAYING_PENDING);

			if(ret)
				break;
		}

		default:
			return ATTR_READ_ERROR;
	}

	return ATTR_READ_CONTINUE;
}

bool Item::unserializeAttr(PropStream& propStream)
{
	uint8_t attrType = ATTR_END;
	while(propStream.getByte(attrType) && attrType != ATTR_END)
	{
		switch(readAttr((AttrTypes_t)attrType, propStream))
		{
			case ATTR_READ_ERROR:
				return false;

			case ATTR_READ_END:
				return true;

			default:
				break;
		}
	}

	return true;
}

bool Item::serializeAttr(PropWriteStream& propWriteStream) const
{
	if(isStackable() || isFluidContainer() || isSplash())
	{
		propWriteStream.addByte(ATTR_COUNT);
		propWriteStream.addByte((uint8_t)getSubType());
	}

	if(attributes && !attributes->empty())
	{
		propWriteStream.addByte(ATTR_ATTRIBUTE_MAP);
		serializeMap(propWriteStream);
	}

	return true;
}

bool Item::hasProperty(enum ITEMPROPERTY prop) const
{
	const ItemType& it = items[id];
	switch(prop)
	{
		case BLOCKSOLID:
			if(it.blockSolid)
				return true;

			break;

		case MOVABLE:
			if(it.movable && (!loadedFromMap || (!getUniqueId()
				&& (!getActionId() || !getContainer()))))
				return true;

			break;

		case HASHEIGHT:
			if(it.hasHeight)
				return true;

			break;

		case BLOCKPROJECTILE:
			if(it.blockProjectile)
				return true;

			break;

		case BLOCKPATH:
			if(it.blockPathFind)
				return true;

			break;

		case ISVERTICAL:
			if(it.isVertical)
				return true;

			break;

		case ISHORIZONTAL:
			if(it.isHorizontal)
				return true;

			break;

		case IMMOVABLEBLOCKSOLID:
			if(it.blockSolid && (!it.movable || (loadedFromMap &&
				(getUniqueId() || (getActionId() && getContainer())))))
				return true;

			break;

		case IMMOVABLEBLOCKPATH:
			if(it.blockPathFind && (!it.movable || (loadedFromMap &&
				(getUniqueId() || (getActionId() && getContainer())))))
				return true;

			break;

		case SUPPORTHANGABLE:
			if(it.isHorizontal || it.isVertical)
				return true;

			break;

		case IMMOVABLENOFIELDBLOCKPATH:
			if(!it.isMagicField() && it.blockPathFind && (!it.movable || (loadedFromMap &&
				(getUniqueId() || (getActionId() && getContainer())))))
				return true;

			break;

		case NOFIELDBLOCKPATH:
			if(!it.isMagicField() && it.blockPathFind)
				return true;

			break;

		case FLOORCHANGEDOWN:
			if(it.floorChange[CHANGE_DOWN])
				return true;

			break;

		case FLOORCHANGEUP:
			for(uint16_t i = CHANGE_FIRST; i <= CHANGE_PRE_LAST; ++i)
			{
				if(it.floorChange[i])
					return true;
			}

			break;

		default:
			break;
	}

	return false;
}

double Item::getWeight() const
{
	if(isStackable())
		return items[id].weight * std::max((int32_t)1, (int32_t)count);

	return items[id].weight;
}

std::string Item::getDescription(const ItemType& it, int32_t lookDistance, const Item* item/* = NULL*/,
	int32_t subType/* = -1*/, bool addArticle/* = true*/)
{
	std::stringstream s;
	s << getNameDescription(it, item, subType, addArticle);
	if(item)
		subType = item->getSubType();

	bool dot = true;
	if(it.isRune())
	{
		if(!it.runeSpellName.empty())
			s << "(\"" << it.runeSpellName << "\")";

		if(it.runeLevel > 0 || it.runeMagLevel > 0 || (it.vocationString != "" && it.wieldInfo == 0))
		{
			s << "." << std::endl << "It can only be used";
			if(it.vocationString != "" && it.wieldInfo == 0)
				s << " by " << it.vocationString;

			bool begin = true;
			if(g_config.getBool(ConfigManager::USE_RUNE_REQUIREMENTS) && it.runeLevel > 0)
			{
				begin = false;
				s << " with level " << it.runeLevel;
			}

			if(g_config.getBool(ConfigManager::USE_RUNE_REQUIREMENTS) && it.runeMagLevel > 0)
			{
				begin = false;
				s << " " << (begin ? "with" : "and") << " magic level " << it.runeMagLevel;
			}

			if(g_config.getBool(ConfigManager::USE_RUNE_REQUIREMENTS) && !begin)
				s << " or higher";
		}
	}
	else if(it.weaponType != WEAPON_NONE)
	{
		bool begin = true;
		if(it.weaponType == WEAPON_DIST && it.ammoType != AMMO_NONE)
		{
			begin = false;
			s << " (Range:" << int32_t(item ? item->getShootRange() : it.shootRange);
			if(it.attack || it.extraAttack || (item && (item->getAttack() || item->getExtraAttack())))
			{
				s << ", Atk " << std::showpos << int32_t(item ? item->getAttack() : it.attack);
				if(it.extraAttack || (item && item->getExtraAttack()))
					s << " " << std::showpos << int32_t(item ? item->getExtraAttack() : it.extraAttack) << std::noshowpos;
			}

			if(it.hitChance != -1 || (item && item->getHitChance() != -1))
				s << ", Hit% " << std::showpos << (item ? item->getHitChance() : it.hitChance) << std::noshowpos;

			if(it.attackSpeed || (item && item->getAttackSpeed()))
				s << ", AS: " << (item ? item->getAttackSpeed() : it.attackSpeed);
		}
		else if(it.weaponType != WEAPON_AMMO && it.weaponType != WEAPON_WAND)
		{
			if(it.attack || it.extraAttack || (item && (item->getAttack() || item->getExtraAttack())))
			{
				begin = false;
				s << " (Atk:";
				if(it.hasAbilities() && it.abilities->elementType != COMBAT_NONE)
				{
					s << std::max((int32_t)0, int32_t((item ? item->getAttack() : it.attack) - it.abilities->elementDamage));
					if(it.extraAttack || (item && item->getExtraAttack()))
						s << " " << std::showpos << int32_t(item ? item->getExtraAttack() : it.extraAttack) << std::noshowpos;

					s << " physical + " << it.abilities->elementDamage << " " << getCombatName(it.abilities->elementType);
				}
				else
				{
					s << int32_t(item ? item->getAttack() : it.attack);
					if(it.extraAttack || (item && item->getExtraAttack()))
						s << " " << std::showpos << int32_t(item ? item->getExtraAttack() : it.extraAttack) << std::noshowpos;
				}
			}

			if(it.defense || it.extraDefense || (item && (item->getDefense() || item->getExtraDefense())))
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "Def:" << int32_t(item ? item->getDefense() : it.defense);
				if(it.extraDefense || (item && item->getExtraDefense()))
					s << " " << std::showpos << int32_t(item ? item->getExtraDefense() : it.extraDefense) << std::noshowpos;
			}
		}

		if(it.attackSpeed || (item && item->getAttackSpeed()))
		{
			if(begin)
			{
				begin = false;
				s << " (";
			}
			else
				s << ", ";

			s << "AS: " << (item ? item->getAttackSpeed() : it.attackSpeed);
		}

		if(it.hasAbilities())
		{
			for(uint16_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
			{
				if(!it.abilities->skills[i])
					continue;

				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << getSkillName(i) << " " << std::showpos << (int32_t)it.abilities->skills[i] << std::noshowpos;
			}

			if(it.abilities->stats[STAT_MAGICLEVEL])
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "magic level " << std::showpos << (int32_t)it.abilities->stats[STAT_MAGICLEVEL] << std::noshowpos;
			}

			int32_t show = it.abilities->absorb[COMBAT_ALL];
			if(!show)
			{
				bool tmp = true;
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
				{
					if(!it.abilities->absorb[i])
						continue;

					if(tmp)
					{
						tmp = false;
						if(begin)
						{
							begin = false;
							s << " (";
						}
						else
							s << ", ";

						s << "protection ";
					}
					else
						s << ", ";

					s << getCombatName((CombatType_t)i) << " " << std::showpos << it.abilities->absorb[i] << std::noshowpos << "%";
				}
			}
			else
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "protection all " << std::showpos << show << std::noshowpos << "%";
			}

			show = it.abilities->fieldAbsorb[COMBAT_ALL];
			if(!show)
			{
				bool tmp = true;
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
				{
					if(!it.abilities->fieldAbsorb[i])
						continue;

					if(tmp)
					{
						tmp = false;
						if(begin)
						{
							begin = false;
							s << " (";
						}
						else
							s << ", ";

						s << "protection ";
					}
					else
						s << ", ";

					s << getCombatName((CombatType_t)i) << " field " << std::showpos << it.abilities->absorb[i] << std::noshowpos << "%";
				}
			}
			else
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "protection all fields " << std::showpos << show << std::noshowpos << "%";
			}

			show = it.abilities->reflect[REFLECT_CHANCE][COMBAT_ALL];
			if(!show)
			{
				bool tmp = true;
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
				{
					if(!it.abilities->reflect[REFLECT_CHANCE][i] || !it.abilities->reflect[REFLECT_PERCENT][i])
						continue;

					if(tmp)
					{
						tmp = false;
						if(begin)
						{
							begin = false;
							s << " (";
						}
						else
							s << ", ";

						s << "reflect: ";
					}
					else
						s << ", ";

					s << it.abilities->reflect[REFLECT_CHANCE][i] << "% for ";
					if(it.abilities->reflect[REFLECT_PERCENT][i] > 99)
						s << "whole";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 75)
						s << "huge";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 50)
						s << "medium";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 25)
						s << "small";
					else
						s << "tiny";

					s << getCombatName((CombatType_t)i);
				}

				if(!tmp)
					s << " damage";
			}
			else
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				int32_t tmp = it.abilities->reflect[REFLECT_PERCENT][COMBAT_ALL];
				s << "reflect: " << show << "% for ";
				if(tmp)
				{
					if(tmp > 99)
						s << "whole";
					else if(tmp >= 75)
						s << "huge";
					else if(tmp >= 50)
						s << "medium";
					else if(tmp >= 25)
						s << "small";
					else
						s << "tiny";
				}
				else
					s << "mixed";

				s << " damage";
			}

			if(it.abilities->speed)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "speed " << std::showpos << (int32_t)(it.abilities->speed / 2) << std::noshowpos;
			}

			if(it.abilities->invisible)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "invisibility";
			}

			if(it.abilities->regeneration)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "faster regeneration";
			}

			if(it.abilities->manaShield)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "mana shield";
			}

			if(hasBitSet(CONDITION_DRUNK, it.abilities->conditionSuppressions))
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "hard drinking";
			}
		}

		if(it.dualWield || (item && item->isDualWield()))
		{
			if(begin)
			{
				begin = false;
				s << " (";
			}
			else
				s << ", ";

			s << "dual wielding";
		}

		if(!begin)
			s << ")";
	}
	else if(it.armor || (item && item->getArmor()) || it.showAttributes)
	{
		int32_t tmp = it.armor;
		if(item)
			tmp = item->getArmor();

		bool begin = true;
		if(tmp)
		{
			s << " (Arm:" << tmp;
			begin = false;
		}

		if(it.hasAbilities())
		{
			for(uint16_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
			{
				if(!it.abilities->skills[i])
					continue;

				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << getSkillName(i) << " " << std::showpos << (int32_t)it.abilities->skills[i] << std::noshowpos;
			}

			if(it.abilities->stats[STAT_MAGICLEVEL])
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "magic level " << std::showpos << (int32_t)it.abilities->stats[STAT_MAGICLEVEL] << std::noshowpos;
			}

			int32_t show = it.abilities->absorb[COMBAT_ALL];
			if(!show)
			{
				bool tmp = true;
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
				{
					if(!it.abilities->absorb[i])
						continue;

					if(tmp)
					{
						tmp = false;
						if(begin)
						{
							begin = false;
							s << " (";
						}
						else
							s << ", ";

						s << "protection ";
					}
					else
						s << ", ";

					s << getCombatName((CombatType_t)i) << " " << std::showpos << it.abilities->absorb[i] << std::noshowpos << "%";
				}
			}
			else
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "protection all " << std::showpos << show << std::noshowpos << "%";
			}

			show = it.abilities->reflect[REFLECT_CHANCE][COMBAT_ALL];
			if(!show)
			{
				bool tmp = true;
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
				{
					if(!it.abilities->reflect[REFLECT_CHANCE][i] || !it.abilities->reflect[REFLECT_PERCENT][i])
						continue;

					if(tmp)
					{
						tmp = false;
						if(begin)
						{
							begin = false;
							s << " (";
						}
						else
							s << ", ";

						s << "reflect: ";
					}
					else
						s << ", ";

					s << it.abilities->reflect[REFLECT_CHANCE][i] << "% for ";
					if(it.abilities->reflect[REFLECT_PERCENT][i] > 99)
						s << "whole";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 75)
						s << "huge";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 50)
						s << "medium";
					else if(it.abilities->reflect[REFLECT_PERCENT][i] >= 25)
						s << "small";
					else
						s << "tiny";

					s << getCombatName((CombatType_t)i);
				}

				if(!tmp)
					s << " damage";
			}
			else
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				int32_t tmp = it.abilities->reflect[REFLECT_PERCENT][COMBAT_ALL];
				s << "reflect: " << show << "% for ";
				if(tmp)
				{
					if(tmp > 99)
						s << "whole";
					else if(tmp >= 75)
						s << "huge";
					else if(tmp >= 50)
						s << "medium";
					else if(tmp >= 25)
						s << "small";
					else
						s << "tiny";
				}
				else
					s << "mixed";

				s << " damage";
			}

			if(it.abilities->speed)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "speed " << std::showpos << (int32_t)(it.abilities->speed / 2) << std::noshowpos;
			}

			if(it.abilities->invisible)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "invisibility";
			}

			if(it.abilities->regeneration)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "faster regeneration";
			}

			if(it.abilities->manaShield)
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "mana shield";
			}

			if(hasBitSet(CONDITION_DRUNK, it.abilities->conditionSuppressions))
			{
				if(begin)
				{
					begin = false;
					s << " (";
				}
				else
					s << ", ";

				s << "hard drinking";
			}

			if(!begin)
				s << ")";
		}
	}
	else if(it.isContainer())
		s << " (Vol:" << (int32_t)it.maxItems << ")";
	else if(it.isKey())
		s << " (Key:" << (item ? (int32_t)item->getActionId() : 0) << ")";
	else if(it.isFluidContainer())
	{
		if(subType > 0)
			s << " of " << (items[subType].name.length() ? items[subType].name : "unknown");
		else
			s << ". It is empty";
	}
	else if(it.isSplash())
	{
		s << " of ";
		if(subType > 0 && items[subType].name.length())
			s << items[subType].name;
		else
			s << "unknown";
	}
	else if(it.allowDistRead)
	{
		s << "." << std::endl;
		if(item && !item->getText().empty())
		{
			if(lookDistance <= 4)
			{
				if(!item->getWriter().empty())
				{
					s << item->getWriter() << " wrote";
					time_t date = item->getDate();
					if(date > 0)
						s << " on " << formatDate(date);

					s << ": ";
				}
				else
					s << "You read: ";

				std::string text = item->getText();
				s << text;

				char end = *text.rbegin();
				if(end == '?' || end == '!' || end == '.')
					dot = false;
			}
			else
				s << "You are too far away to read it";
		}
		else
			s << "Nothing is written on it";
	}
	else if(it.levelDoor && item && item->getActionId() >= (int32_t)it.levelDoor && item->getActionId()
		<= ((int32_t)it.levelDoor + g_config.getNumber(ConfigManager::MAXIMUM_DOOR_LEVEL)))
		s << " for level " << item->getActionId() - it.levelDoor;

	if(it.showCharges)
		s << " that has " << subType << " charge" << (subType != 1 ? "s" : "") << " left";

	if(it.showDuration)
	{
		if(item && item->hasIntegerAttribute("duration"))
		{
			int32_t duration = item->getDuration() / 1000;
			s << " that will expire in ";
			if(duration >= 86400)
			{
				uint16_t days = duration / 86400;
				uint16_t hours = (duration % 86400) / 3600;
				s << days << " day" << (days > 1 ? "s" : "");
				if(hours > 0)
					s << " and " << hours << " hour" << (hours > 1 ? "s" : "");
			}
			else if(duration >= 3600)
			{
				uint16_t hours = duration / 3600;
				uint16_t minutes = (duration % 3600) / 60;
				s << hours << " hour" << (hours > 1 ? "s" : "");
				if(hours > 0)
					s << " and " << minutes << " minute" << (minutes > 1 ? "s" : "");
			}
			else if(duration >= 60)
			{
				uint16_t minutes = duration / 60;
				uint16_t seconds = duration % 60;
				s << minutes << " minute" << (minutes > 1 ? "s" : "");
				if(seconds > 0)
					s << " and " << seconds << " second" << (seconds > 1 ? "s" : "");
			}
			else
				s << duration << " second" << (duration > 1 ? "s" : "");
		}
		else
			s << " that is brand-new";
	}

	if(dot)
		s << ".";

	if(it.wieldInfo)
	{
		s << std::endl << "It can only be wielded properly by ";
		if(it.wieldInfo & WIELDINFO_PREMIUM)
			s << "premium ";

		if(it.wieldInfo & WIELDINFO_VOCREQ)
			s << it.vocationString;
		else
			s << "players";

		if(it.wieldInfo & WIELDINFO_LEVEL)
			s << " of level " << (int32_t)it.minReqLevel << " or higher";

		if(it.wieldInfo & WIELDINFO_MAGLV)
		{
			if(it.wieldInfo & WIELDINFO_LEVEL)
				s << " and";
			else
				s << " of";

			s << " magic level " << (int32_t)it.minReqMagicLevel << " or higher";
		}

		s << ".";
	}

	if(lookDistance <= 1 && it.pickupable)
	{
		std::string tmp;
		if(!item)
			tmp = getWeightDescription(it.weight, it.stackable && it.showCount, subType);
		else
			tmp = item->getWeightDescription();

		if(!tmp.empty())
			s << std::endl << tmp;
	}

	if(item && !item->getSpecialDescription().empty())
		s << std::endl << item->getSpecialDescription();
	else if(!it.description.empty() && lookDistance <= 1)
		s << std::endl << it.description;

	std::string str = s.str();
	if(str.find("|PLAYERNAME|") != std::string::npos)
	{
		std::string tmp = "You";
		if(item)
		{
			if(const Player* player = item->getHoldingPlayer())
				tmp = player->getName();
		}

		replaceString(str, "|PLAYERNAME|", tmp);
	}

	if(str.find("|TIME|") != std::string::npos || str.find("|DATE|") != std::string::npos || str.find(
		"|DAY|") != std::string::npos || str.find("|MONTH|") != std::string::npos || str.find(
		"|YEAR|") != std::string::npos || str.find("|HOUR|") != std::string::npos || str.find(
		"|MINUTES|") != std::string::npos || str.find("|SECONDS|") != std::string::npos ||
		str.find("|WEEKDAY|") != std::string::npos || str.find("|YEARDAY|") != std::string::npos)
	{
		time_t now = time(NULL);
		tm* ts = localtime(&now);

		std::stringstream ss;
		ss << ts->tm_sec;
		replaceString(str, "|SECONDS|", ss.str());

		ss.str("");
		ss << ts->tm_min;
		replaceString(str, "|MINUTES|", ss.str());

		ss.str("");
		ss << ts->tm_hour;
		replaceString(str, "|HOUR|", ss.str());

		ss.str("");
		ss << ts->tm_mday;
		replaceString(str, "|DAY|", ss.str());

		ss.str("");
		ss << (ts->tm_mon + 1);
		replaceString(str, "|MONTH|", ss.str());

		ss.str("");
		ss << (ts->tm_year + 1900);
		replaceString(str, "|YEAR|", ss.str());

		ss.str("");
		ss << ts->tm_wday;
		replaceString(str, "|WEEKDAY|", ss.str());

		ss.str("");
		ss << ts->tm_yday;
		replaceString(str, "|YEARDAY|", ss.str());

		ss.str("");
		ss << ts->tm_hour << ":" << ts->tm_min << ":" << ts->tm_sec;
		replaceString(str, "|TIME|", ss.str());

		ss.str("");
		replaceString(str, "|DATE|", formatDateEx(now));
	}

	return str;
}

std::string Item::getNameDescription(const ItemType& it, const Item* item/* = NULL*/, int32_t subType/* = -1*/, bool addArticle/* = true*/)
{
	if(item)
		subType = item->getSubType();

	std::stringstream s;
	if(it.loaded || (item && !item->getName().empty()))
	{
		if(subType > 1 && it.stackable && it.showCount)
			s << subType << " " << (item ? item->getPluralName() : it.pluralName);
		else
		{
			if(addArticle)
			{
				if(item && !item->getArticle().empty())
					s << item->getArticle() << " ";
				else if(!it.article.empty())
					s << it.article << " ";
			}

			s << (item ? item->getName() : it.name);
		}
	}
	else if(it.name.empty())
		s << "an item of type " << it.id << ", please report it to gamemaster";
	else
		s << "an item '" << it.name << "', please report it to gamemaster";

	return s.str();
}

std::string Item::getWeightDescription(double weight, bool stackable, uint32_t count/* = 1*/)
{
	if(weight <= 0)
		return "";

	std::stringstream s;
	if(stackable && count > 1)
		s << "They weigh " << std::fixed << std::setprecision(2) << weight << " oz.";
	else
		s << "It weighs " << std::fixed << std::setprecision(2) << weight << " oz.";

	return s.str();
}

void Item::setActionId(int32_t aid, bool callEvent/* = true*/)
{
	Tile* tile = NULL;
	if(callEvent)
		tile = getTile();

	if(tile && getActionId())
		g_moveEvents->onRemoveTileItem(tile, this);

	setAttribute("aid", aid);
	if(tile)
		g_moveEvents->onAddTileItem(tile, this);
}

void Item::resetActionId(bool callEvent/* = true*/)
{
	if(!getActionId())
		return;

	Tile* tile = NULL;
	if(callEvent)
		tile = getTile();

	eraseAttribute("aid");
	if(tile)
		g_moveEvents->onAddTileItem(tile, this);
}

void Item::setUniqueId(int32_t uid)
{
	if(getUniqueId())
		return;

	setAttribute("uid", uid);
	ScriptEnviroment::addUniqueThing(this);
}

bool Item::canDecay()
{
	if(isRemoved())
		return false;

	if(loadedFromMap && (getUniqueId() || (getActionId() && getContainer())))
		return false;

	const ItemType& it = Item::items[id];
	return it.decayTo >= 0 && it.decayTime;
}

void Item::getLight(LightInfo& lightInfo)
{
	const ItemType& it = items[id];
	lightInfo.color = it.lightColor;
	lightInfo.level = it.lightLevel;
}

void Item::__startDecaying()
{
	g_game.startDecay(this);
}

void Item::generateSerial()
{
	std::string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	std::string serial = "";
	for(int32_t i = 1; i < 6; i++)
	{
		int32_t l = rand() % (letters.length() - 1) + 1;
		serial += letters.substr(l, 1);
	}
	serial += "-";
	for(int32_t i = 1; i < 6; i++)
	{
		int32_t l = rand() % (letters.length() - 1) + 1;
		serial += letters.substr(l, 1);
	}
	serial += "-";
	for(int32_t i = 1; i < 6; i++)
	{
		int32_t l = rand() % (letters.length() - 1) + 1;
		serial += letters.substr(l, 1);
	}
	serial += "-";
	for(int32_t i = 1; i < 6; i++)
	{
		int32_t l = rand() % (letters.length() - 1) + 1;
		serial += letters.substr(l, 1);
	}

	std::string key = "serial";
	this->setAttribute(key.c_str(), serial);
	serial = "";
}
