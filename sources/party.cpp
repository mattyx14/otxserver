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
#include "party.h"

#include "player.h"
#include "chat.h"
#include "game.h"
#include "configmanager.h"

extern Game g_game;
extern Chat g_chat;
extern ConfigManager g_config;

Party::Party(Player* _leader)
{
	sharedExpActive = sharedExpEnabled = false;
	if(_leader)
	{
		leader = _leader;
		leader->setParty(this);
		leader->sendPlayerIcons(leader);
	}
}

void Party::disband()
{
	leader->sendClosePrivate(CHANNEL_PARTY);
	leader->setParty(NULL);
	leader->sendTextMessage(MSG_PARTY, "Your party has been disbanded.");

	leader->sendPlayerIcons(leader);
	for(PlayerVector::iterator it = inviteList.begin(); it != inviteList.end(); ++it)
	{
		(*it)->removePartyInvitation(this);
		(*it)->sendPlayerIcons(leader);
		(*it)->sendPlayerIcons(*it);
		leader->sendPlayerIcons(*it);
	}

	inviteList.clear();
	for(PlayerVector::iterator it = memberList.begin(); it != memberList.end(); ++it)
	{
		(*it)->sendClosePrivate(CHANNEL_PARTY);
		(*it)->setParty(NULL);
		(*it)->sendTextMessage(MSG_PARTY, "Your party has been disbanded.");

		(*it)->sendPlayerIcons(*it);
		(*it)->sendPlayerIcons(leader);
		leader->sendPlayerIcons(*it);
	}

	memberList.clear();
	leader = NULL;
	delete this;
}

bool Party::leave(Player* player)
{
	if(!isPlayerMember(player) && player != leader)
		return false;

	bool missingLeader = false;
	if(leader == player)
	{
		if(!memberList.empty())
		{
			if(memberList.size() == 1 && inviteList.empty())
				missingLeader = true;
			else
				passLeadership(memberList.front());
		}
		else
			missingLeader = true;
	}

	//since we already passed the leadership, we remove the player from the list
	PlayerVector::iterator it = std::find(memberList.begin(), memberList.end(), player);
	if(it != memberList.end())
		memberList.erase(it);

	it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end())
		inviteList.erase(it);

	player->setParty(NULL);
	player->sendClosePrivate(CHANNEL_PARTY);

	player->sendTextMessage(MSG_PARTY, "You have left the party.");
	player->sendPlayerIcons(player);

	updateSharedExperience();
	updateIcons(player);
	clearPlayerPoints(player);

	char buffer[105];
	sprintf(buffer, "%s has left the party.", player->getName().c_str());

	broadcastMessage(MSG_PARTY, buffer);
	if(missingLeader || canDisband())
		disband();

	return true;
}

bool Party::passLeadership(Player* player)
{
	if(!isPlayerMember(player) || player == leader)
		return false;

	//Remove it before to broadcast the message correctly
	PlayerVector::iterator it = std::find(memberList.begin(), memberList.end(), player);
	if(it != memberList.end())
		memberList.erase(it);

	Player* oldLeader = leader;
	leader = player;
	memberList.insert(memberList.begin(), oldLeader);

	char buffer[125];
	sprintf(buffer, "%s is now the leader of the party.", player->getName().c_str());
	broadcastMessage(MSG_PARTY, buffer, true);

	player->sendTextMessage(MSG_PARTY, "You are now the leader of the party.");
	updateSharedExperience();

	updateIcons(oldLeader);
	updateIcons(player);
	return true;
}

bool Party::join(Player* player)
{
	if(isPlayerMember(player) || !isPlayerInvited(player))
		return false;

	memberList.push_back(player);
	player->setParty(this);

	player->removePartyInvitation(this);
	PlayerVector::iterator it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end())
		inviteList.erase(it);

	char buffer[200];
	sprintf(buffer, "%s has joined the party.", player->getName().c_str());
	broadcastMessage(MSG_PARTY, buffer);

	sprintf(buffer, "You have joined %s'%s party. Open the party channel to communicate with your companions.", leader->getName().c_str(), (leader->getName()[leader->getName().length() - 1] == 's' ? "" : "s"));
	player->sendTextMessage(MSG_PARTY, buffer);

	updateSharedExperience();
	updateIcons(player);
	return true;
}

bool Party::removeInvite(Player* player)
{
	if(!isPlayerInvited(player))
		return false;

	PlayerVector::iterator it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end())
		inviteList.erase(it);

	leader->sendPlayerIcons(player);
	player->sendPlayerIcons(leader);

	player->removePartyInvitation(this);
	if(canDisband())
		disband();

	return true;
}

void Party::revokeInvitation(Player* player)
{
	if(!player || player->isRemoved())
		return;

	char buffer[150];
	sprintf(buffer, "%s has revoked %s invitation.", leader->getName().c_str(), (leader->getSex(false) ? "his" : "her"));
	player->sendTextMessage(MSG_PARTY, buffer);

	sprintf(buffer, "Invitation for %s has been revoked.", player->getName().c_str());
	leader->sendTextMessage(MSG_PARTY, buffer);
	removeInvite(player);
}

bool Party::invitePlayer(Player* player)
{
	if(isPlayerInvited(player, true))
		return false;

	inviteList.push_back(player);
	player->addPartyInvitation(this);

	char buffer[150];
	sprintf(buffer, "%s has been invited.%s", player->getName().c_str(), (!memberList.size() ? " Open the party channel to communicate with your members." : ""));
	leader->sendTextMessage(MSG_PARTY, buffer);

	sprintf(buffer, "%s has invited you to %s party.", leader->getName().c_str(), (leader->getSex(false) ? "his" : "her"));
	player->sendTextMessage(MSG_PARTY, buffer);

	leader->sendPlayerIcons(player);
	player->sendPlayerIcons(leader);
	return true;
}

void Party::updateIcons(Player* player)
{
	if(!player || player->isRemoved())
		return;

	PlayerVector::iterator it;
	for(it = memberList.begin(); it != memberList.end(); ++it)
	{
		(*it)->sendPlayerIcons(player);
		player->sendPlayerIcons((*it));
	}

	for(it = inviteList.begin(); it != inviteList.end(); ++it)
	{
		(*it)->sendPlayerIcons(player);
		player->sendPlayerIcons((*it));
	}

	leader->sendPlayerIcons(player);
	player->sendPlayerIcons(leader);
}

void Party::updateAllIcons()
{
	PlayerVector::iterator it;
	for(it = memberList.begin(); it != memberList.end(); ++it)
	{
		for(PlayerVector::iterator iit = memberList.begin(); iit != memberList.end(); ++iit)
			(*it)->sendPlayerIcons((*iit));

		(*it)->sendPlayerIcons(leader);
		leader->sendPlayerIcons((*it));
	}

	leader->sendPlayerIcons(leader);
	for(it = inviteList.begin(); it != inviteList.end(); ++it)
		(*it)->sendPlayerIcons(leader);
}

void Party::broadcastMessage(MessageClasses messageClass, const std::string& text, bool sendToInvitations/* = false*/)
{
	PlayerVector::iterator it;
	if(!memberList.empty())
	{
		for(it = memberList.begin(); it != memberList.end(); ++it)
			(*it)->sendTextMessage(messageClass, text);
	}

	leader->sendTextMessage(messageClass, text);
	if(!sendToInvitations || inviteList.empty())
		return;

	for(it = inviteList.begin(); it != inviteList.end(); ++it)
		(*it)->sendTextMessage(messageClass, text);
}

void Party::updateSharedExperience()
{
	if(!sharedExpActive)
		return;

	bool result = canEnableSharedExperience();
	if(result == sharedExpEnabled)
		return;

	sharedExpEnabled = result;
	updateAllIcons();
}

bool Party::setSharedExperience(Player* player, bool _sharedExpActive)
{
	if(!player || player->isRemoved() || player != leader)
		return false;

	if(sharedExpActive == _sharedExpActive)
		return true;

	sharedExpActive = _sharedExpActive;
	if(sharedExpActive)
	{
		sharedExpEnabled = canEnableSharedExperience();
		if(sharedExpEnabled)
			leader->sendTextMessage(MSG_PARTY_MANAGEMENT, "Shared Experience is now active.");
		else
			leader->sendTextMessage(MSG_PARTY_MANAGEMENT, "Shared Experience has been activated, but some members of your party are inactive.");
	}
	else
		leader->sendTextMessage(MSG_PARTY_MANAGEMENT, "Shared Experience has been deactivated.");

	updateAllIcons();
	return true;
}

void Party::shareExperience(double experience, Creature* target, bool multiplied)
{
	double shareExperience = experience;
	if(experience >= (double)g_config.getNumber(ConfigManager::EXTRA_PARTY_LIMIT))
		shareExperience += (experience * ((double)g_config.getNumber(ConfigManager::EXTRA_PARTY_PERCENT) / 100));

	shareExperience /= memberList.size() + 1;
	double tmpExperience = shareExperience; //we need this, as onGainSharedExperience increases the value

	leader->onGainSharedExperience(tmpExperience, target, multiplied);
	for(PlayerVector::iterator it = memberList.begin(); it != memberList.end(); ++it)
	{
		tmpExperience = shareExperience;
		(*it)->onGainSharedExperience(tmpExperience, target, multiplied);
	}
}

bool Party::canUseSharedExperience(const Player* player, uint32_t highestLevel/* = 0*/) const
{
	if(!player || player->isRemoved() || !memberList.size())
		return false;

	if(!highestLevel)
	{
		highestLevel = leader->getLevel();
		for(PlayerVector::const_iterator it = memberList.begin(); it != memberList.end(); ++it)
		{
			if((*it)->getLevel() > highestLevel)
				highestLevel = (*it)->getLevel();
		}
	}

	if(player->getLevel() < (uint32_t)std::ceil((double)highestLevel * g_config.getDouble(
		ConfigManager::PARTY_DIFFERENCE)) || !Position::areInRange(Position(
		g_config.getNumber(ConfigManager::PARTY_RADIUS_X), g_config.getNumber(
		ConfigManager::PARTY_RADIUS_Y), g_config.getNumber(ConfigManager::PARTY_RADIUS_Z)),
		leader->getPosition(), player->getPosition()))
		return false;

	CountMap::const_iterator it = pointMap.find(player->getID());
	return it != pointMap.end() && (OTSYS_TIME() - it->second.ticks) <= g_config.getNumber(
		ConfigManager::EXPERIENCE_SHARE_ACTIVITY);
}

bool Party::canEnableSharedExperience()
{
	if(!memberList.size())
		return false;

	uint32_t highestLevel = leader->getLevel();
	for(PlayerVector::iterator it = memberList.begin(); it != memberList.end(); ++it)
	{
		if((*it)->getLevel() > highestLevel)
			highestLevel = (*it)->getLevel();
	}

	for(PlayerVector::iterator it = memberList.begin(); it != memberList.end(); ++it)
	{
		if(!canUseSharedExperience((*it), highestLevel))
			return false;
	}

	return canUseSharedExperience(leader, highestLevel);
}

void Party::addPlayerHealedMember(Player* player, uint32_t points)
{
	CountMap::iterator it = pointMap.find(player->getID());
	if(it != pointMap.end())
	{
		it->second.totalHeal += points;
		it->second.ticks = OTSYS_TIME();
	}
	else
		pointMap[player->getID()] = CountBlock_t(points, 0);

	updateSharedExperience();
}

void Party::addPlayerDamageMonster(Player* player, uint32_t points)
{
	CountMap::iterator it = pointMap.find(player->getID());
	if(it != pointMap.end())
	{
		it->second.totalDamage += points;
		it->second.ticks = OTSYS_TIME();
	}
	else
		pointMap[player->getID()] = CountBlock_t(0, points);

	updateSharedExperience();
}

void Party::clearPlayerPoints(Player* player)
{
	CountMap::iterator it = pointMap.find(player->getID());
	if(it == pointMap.end())
		return;

	pointMap.erase(it);
	updateSharedExperience();
}

bool Party::isPlayerMember(const Player* player, bool result/* = false*/) const
{
	if(!player || player->isRemoved())
		return result;

	return std::find(memberList.begin(), memberList.end(), player) != memberList.end();
}

bool Party::isPlayerInvited(const Player* player, bool result/* = false*/) const
{
	if(!player || player->isRemoved())
		return result;

	return std::find(inviteList.begin(), inviteList.end(), player) != inviteList.end();
}

bool Party::canOpenCorpse(uint32_t ownerId)
{
	return leader->getGUID() == ownerId || isPlayerMember(g_game.getPlayerByGuid(ownerId));
}
