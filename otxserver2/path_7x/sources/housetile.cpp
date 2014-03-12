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
#include "housetile.h"

#include "house.h"
#include "game.h"
#include "configmanager.h"

extern ConfigManager g_config;
extern Game g_game;

HouseTile::HouseTile(int32_t x, int32_t y, int32_t z, House* _house):
	DynamicTile(x, y, z)
{
	house = _house;
	setFlag(TILESTATE_HOUSE);
}

void HouseTile::__addThing(Creature* actor, int32_t index, Thing* thing)
{
	Tile::__addThing(actor, index, thing);
	if(!thing->getParent())
		return;

	if(Item* item = thing->getItem())
		updateHouse(item);
}

void HouseTile::__internalAddThing(uint32_t index, Thing* thing)
{
	Tile::__internalAddThing(index, thing);
	if(!thing->getParent())
		return;

	if(Item* item = thing->getItem())
		updateHouse(item);
}

void HouseTile::updateHouse(Item* item)
{
	if(item->getTile() != this)
		return;

	if(Door* door = item->getDoor())
	{
		if(door->getDoorId() != 0)
			house->addDoor(door);
	}
	else if(BedItem* bed = item->getBed())
		house->addBed(bed);
}

ReturnValue HouseTile::__queryAdd(int32_t index, const Thing* thing, uint32_t count, uint32_t flags, Creature* actor/* = NULL*/) const
{
	if(const Creature* creature = thing->getCreature())
	{
		if(const Player* player = creature->getPlayer())
		{
			if(!house->isInvited(player) && !player->hasCustomFlag(PlayerCustomFlag_CanMoveAnywhere))
				return RET_PLAYERISNOTINVITED;
		}
		else
			return RET_NOTPOSSIBLE;
	}
	else if(thing->getItem())
	{
		const uint32_t itemLimit = g_config.getNumber(ConfigManager::HOUSE_TILE_LIMIT);
		if(itemLimit && getItemCount() > itemLimit)
			return RET_TILEISFULL;

		if(actor && g_config.getBool(ConfigManager::HOUSE_PROTECTION))
		{
			if(const Player* player = actor->getPlayer())
			{
				if(!house->isInvited(player) && !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
					return RET_PLAYERISNOTINVITED;
			}
		}
	}

	return Tile::__queryAdd(index, thing, count, flags, actor);
}

ReturnValue HouseTile::__queryRemove(const Thing* thing, uint32_t count, uint32_t flags, Creature* actor/* = NULL*/) const
{
	if(thing->getItem() && actor && g_config.getBool(ConfigManager::HOUSE_PROTECTION))
	{
		if(const Player* player = actor->getPlayer())
		{
			if(!house->isInvited(player) && !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
				return RET_PLAYERISNOTINVITED;
		}
	}

	return Tile::__queryRemove(thing, count, flags, actor);
}

Cylinder* HouseTile::__queryDestination(int32_t& index, const Thing* thing, Item** destItem, uint32_t& flags)
{
	if(const Creature* creature = thing->getCreature())
	{
		if(const Player* player = creature->getPlayer())
		{
			if(!house->isInvited(player) && !player->hasCustomFlag(PlayerCustomFlag_CanMoveAnywhere))
			{
				Tile* destTile = g_game.getTile(house->getEntry());
				if(!destTile)
				{
					std::clog << "[Error - HouseTile::__queryDestination] Tile at house entry position for house: "
						<< house->getName() << " (" << house->getId() << ") does not exist." << std::endl;
					destTile = g_game.getTile(player->getMasterPosition());
					if(!destTile)
						destTile = &(Tile::nullTile);
				}

				index = -1;
				*destItem = NULL;
				return destTile;
			}
		}
	}

	return Tile::__queryDestination(index, thing, destItem, flags);
}
