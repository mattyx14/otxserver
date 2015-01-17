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
#include "waitlist.h"

#include "player.h"

#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;

WaitList::iterator WaitingList::find(const Player* player, uint32_t& slot)
{
	slot = 1;
	for(WaitList::iterator it = waitList.begin(); it != waitList.end(); ++it)
	{
		if((*it)->ip == player->getIP() && boost::algorithm::iequals((*it)->name, player->getName()))
			return it;

		++slot;
	}

	return waitList.end();
}

int32_t WaitingList::getTime(int32_t slot)
{
	if(slot < 5)
		return 5;
	else if(slot < 10)
		return 10;
	else if(slot < 20)
		return 20;
	else if(slot < 50)
		return 60;

	return 120;
}

bool WaitingList::login(const Player* player)
{
	uint32_t online = g_game.getPlayersOnline(), max = g_config.getNumber(ConfigManager::MAX_PLAYERS);
	if(player->hasFlag(PlayerFlag_CanAlwaysLogin) || player->isAccountManager() || (waitList.empty()
		&& online < max) || (g_config.getBool(ConfigManager::PREMIUM_SKIP_WAIT) && player->isPremium()))
		return true;

	cleanup();
	uint32_t slot = 0;

	WaitList::iterator it = find(player, slot);
	if(it != waitList.end())
	{
		if((online + slot) > max)
		{
			//let them wait a bit longer
			(*it)->timeout = OTSYS_TIME() + getTimeout(slot) * 1000;
			return false;
		}

		//should be able to login now
		delete *it;
		waitList.erase(it);
		return true;
	}

	Wait* wait = new Wait();
	if(player->isPremium())
	{
		slot = 1;
		WaitList::iterator it = waitList.end();
		for(WaitList::iterator wit = waitList.begin(); wit != it; ++wit)
		{
			if(!(*wit)->premium)
			{
				it = wit;
				break;
			}

			++slot;
		}

		waitList.insert(it, wait);
	}
	else
	{
		waitList.push_back(wait);
		slot = waitList.size();
	}

	wait->name = player->getName();
	wait->ip = player->getIP();
	wait->premium = player->isPremium();

	wait->timeout = OTSYS_TIME() + getTimeout(slot) * 1000;
	return false;
}

int32_t WaitingList::getSlot(const Player* player)
{
	uint32_t slot = 0;
	WaitList::iterator it = find(player, slot);
	if(it != waitList.end())
		return slot;

	return -1;
}

void WaitingList::cleanup()
{
	for(WaitList::iterator it = waitList.begin(); it != waitList.end();)
	{
		if(((*it)->timeout - OTSYS_TIME()) <= 0)
		{
			delete *it;
			it = waitList.erase(it);
		}
		else
			++it;
	}
}
