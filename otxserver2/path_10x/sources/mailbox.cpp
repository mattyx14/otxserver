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
#include "mailbox.h"

#include "player.h"
#include "iologindata.h"
#include "town.h"

#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;

ReturnValue Mailbox::canSend(const Item* item, Creature* actor) const
{
	if(item->getID() != ITEM_PARCEL && item->getID() != ITEM_LETTER)
		return RET_NOTPOSSIBLE;

	if(actor)
	{
		if(Player* player = actor->getPlayer())
		{
			if(player->hasCondition(CONDITION_MUTED, 2))
				return RET_YOUAREEXHAUSTED;

			if(player->getMailAttempts() >= g_config.getNumber(ConfigManager::MAIL_ATTEMPTS))
			{
				if(Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT,
					CONDITION_MUTED, g_config.getNumber(ConfigManager::MAIL_BLOCK), 0, false, 2))
				{
					player->addCondition(condition);
					player->setLastMail(1); // auto erase
				}

				return RET_YOUAREEXHAUSTED;
			}

			player->setLastMail(OTSYS_TIME());
			player->addMailAttempt();
		}
	}

	return RET_NOERROR;
}

ReturnValue Mailbox::__queryAdd(int32_t, const Thing* thing, uint32_t,
	uint32_t, Creature* actor/* = NULL*/) const
{
	if(const Item* item = thing->getItem())
		return canSend(item, actor);

	return RET_NOTPOSSIBLE;
}

ReturnValue Mailbox::__queryMaxCount(int32_t, const Thing*, uint32_t count, uint32_t& maxQueryCount,
	uint32_t) const
{
	maxQueryCount = std::max((uint32_t)1, count);
	return RET_NOERROR;
}

void Mailbox::__addThing(Creature* actor, int32_t, Thing* thing)
{
	Item* item = thing->getItem();
	if(!item)
		return;

	if(canSend(item, actor) == RET_NOERROR)
		sendItem(actor, item);
}

bool Mailbox::sendItem(Creature* actor, Item* item)
{
	std::string name;
	if(!getReceiver(item, name) || name.empty())
		return false;

	return IOLoginData::getInstance()->playerMail(actor, name, item);
}

bool Mailbox::getReceiver(Item* item, std::string& name)
{
	if(!item)
		return false;

	if(item->getID() == ITEM_PARCEL) /**We need to get the text from the label incase its a parcel**/
	{
		Container* parcel = item->getContainer();
		if(parcel)
		{
			for(ItemList::const_iterator cit = parcel->getItems(); cit != parcel->getEnd(); ++cit)
			{
				if((*cit)->getID() == ITEM_LABEL)
				{
					item = (*cit);
					if(item->getText().empty())
						break;
				}
			}
		}
	}
	else if(item->getID() != ITEM_LETTER) // The item is somehow not a parcel or letter
	{
		std::clog << "[Error - Mailbox::getReciver] Trying to get receiver from unkown item with id: " << item->getID() << "!" << std::endl;
		return false;
	}

	if(!item || item->getText().empty()) // No label or letter found or its empty
		return false;

	name = getFirstLine(item->getText());
	trimString(name);
	return true;
}
