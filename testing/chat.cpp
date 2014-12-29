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
#include "chat.h"

#include "player.h"
#include "iologindata.h"
#include "manager.h"

#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;
extern Chat g_chat;

uint16_t ChatChannel::staticFlags = CHANNELFLAG_ENABLED | CHANNELFLAG_ACTIVE;

PrivateChatChannel::PrivateChatChannel(uint16_t id, std::string name, uint16_t flags):
	ChatChannel(id, name, flags), m_owner(0) {}

bool PrivateChatChannel::isInvited(const Player* player)
{
	if(player->getGUID() == m_owner)
		return true;

	return std::find(m_invites.begin(), m_invites.end(), player->getGUID()) != m_invites.end();
}

bool PrivateChatChannel::addInvited(Player* player)
{
	if(std::find(m_invites.begin(), m_invites.end(), player->getGUID()) != m_invites.end())
		return false;

	m_invites.push_back(player->getGUID());
	return true;
}

bool PrivateChatChannel::removeInvited(Player* player)
{
	InviteList::iterator it = std::find(m_invites.begin(), m_invites.end(), player->getGUID());
	if(it == m_invites.end())
		return false;

	m_invites.erase(it);
	return true;
}

void PrivateChatChannel::invitePlayer(Player* player, Player* invitePlayer)
{
	if(player == invitePlayer || !addInvited(invitePlayer))
		return;

	std::stringstream msg;
	msg << player->getName() << " invites you to " << (player->getSex(false) ? "his" : "her") << " private chat channel.";
	invitePlayer->sendTextMessage(MSG_INFO_DESCR, msg.str().c_str());

	msg.str("");
	msg << invitePlayer->getName() << " has been invited.";
	player->sendTextMessage(MSG_INFO_DESCR, msg.str().c_str());
}

void PrivateChatChannel::excludePlayer(Player* player, Player* excludePlayer)
{
	if(player == excludePlayer || !removeInvited(excludePlayer))
		return;

	std::string msg = excludePlayer->getName();
	msg += " has been excluded.";
	player->sendTextMessage(MSG_INFO_DESCR, msg.c_str());

	removeUser(excludePlayer, true);
	excludePlayer->sendClosePrivate(getId());
}

void PrivateChatChannel::closeChannel()
{
	for(UsersMap::iterator it = m_users.begin(); it != m_users.end(); ++it)
		it->second->sendClosePrivate(m_id);
}

ChatChannel::ChatChannel(uint16_t id, const std::string& name, uint16_t flags, uint32_t access/* = 0*/,
	uint32_t level/* = 1*/, Condition* condition/* = NULL*/, int32_t conditionId/* = -1*/,
	const std::string& conditionMessage/* = ""*/, VocationMap* vocationMap/* = NULL*/):
	m_id(id), m_flags(flags), m_conditionId(conditionId), m_access(access), m_level(level),
		m_name(name), m_conditionMessage(conditionMessage), m_condition(condition),
		m_vocationMap(vocationMap)
{
	if(hasFlag(CHANNELFLAG_LOGGED))
	{
		m_file.reset(new std::ofstream(getFilePath(FILE_TYPE_LOG, (std::string)"chat/" + g_config.getString(
			ConfigManager::PREFIX_CHANNEL_LOGS) + m_name + (std::string)".log").c_str(), std::ios::app | std::ios::out));
		if(!m_file->is_open())
			m_flags &= ~CHANNELFLAG_LOGGED;
	}
}

bool ChatChannel::addUser(Player* player)
{
	if(!player)
		return false;

	if(m_users.find(player->getID()) != m_users.end())
		return true;

	ChatChannel* channel = g_chat.getChannel(player, m_id);
	if(!channel)
	{
		#ifdef __DEBUG_CHAT__
		std::clog << "ChatChannel::addUser - failed retrieving channel." << std::endl;
		#endif
		return false;
	}

	m_users[player->getID()] = player;
	CreatureEventList joinEvents = player->getCreatureEvents(CREATURE_EVENT_CHANNEL_JOIN);
	for(CreatureEventList::iterator it = joinEvents.begin(); it != joinEvents.end(); ++it)
		(*it)->executeChannel(player, m_id, m_users);

	Manager::getInstance()->addUser(player->getID(), m_id);
	return true;
}

bool ChatChannel::removeUser(Player* player, bool/* exclude = false*/)
{
	if(!player)
		return false;

	UsersMap::iterator it = m_users.find(player->getID());
	if(it == m_users.end())
		return true;

	m_users.erase(it);
	CreatureEventList leaveEvents = player->getCreatureEvents(CREATURE_EVENT_CHANNEL_LEAVE);
	for(CreatureEventList::iterator it = leaveEvents.begin(); it != leaveEvents.end(); ++it)
		(*it)->executeChannel(player, m_id, m_users);

	Manager::getInstance()->removeUser(player->getID(), m_id);
	return true;
}

bool ChatChannel::talk(Player* player, MessageClasses type, const std::string& text, uint32_t statementId)
{
	UsersMap::iterator it = m_users.find(player->getID());
	if(it == m_users.end())
		return false;

	if(m_condition && !player->hasFlag(PlayerFlag_CannotBeMuted))
	{
		if(Condition* condition = m_condition->clone())
			player->addCondition(condition);
	}

	for(it = m_users.begin(); it != m_users.end(); ++it)
		it->second->sendCreatureChannelSay(player, type, text, m_id, statementId);

	if(hasFlag(CHANNELFLAG_LOGGED) && m_file->is_open())
		*m_file << "[" << formatDate() << "] " << player->getName() << ": " << text << std::endl;

	return true;
}

bool ChatChannel::talk(std::string nick, MessageClasses type, const std::string& text)
{
	for(UsersMap::iterator it = m_users.begin(); it != m_users.end(); ++it)
		it->second->sendChannelMessage(nick, text, type, m_id);

	if(hasFlag(CHANNELFLAG_LOGGED) && m_file->is_open())
		*m_file << "[" << formatDate() << "] " << nick << ": " << text << std::endl;

	return true;
}

Chat::~Chat()
{
	for(GuildChannelMap::iterator it = m_guildChannels.begin(); it != m_guildChannels.end(); ++it)
		delete it->second;

	m_guildChannels.clear();
	clear();
}

void Chat::clear()
{
	for(NormalChannelMap::iterator it = m_normalChannels.begin(); it != m_normalChannels.end(); ++it)
		delete it->second;

	m_normalChannels.clear();
	for(PartyChannelMap::iterator it = m_partyChannels.begin(); it != m_partyChannels.end(); ++it)
		delete it->second;

	m_partyChannels.clear();
	for(PrivateChannelMap::iterator it = m_privateChannels.begin(); it != m_privateChannels.end(); ++it)
		delete it->second;

	m_privateChannels.clear();
	delete dummyPrivate;
}

bool Chat::reload()
{
	clear();
	return loadFromXml();
}

bool Chat::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML, "channels.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Chat::loadFromXml] Cannot load channels file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"channels"))
	{
		std::clog << "[Error - Chat::loadFromXml] Malformed channels file" << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
		parseChannelNode(p);

	xmlFreeDoc(doc);
	return true;
}

bool Chat::parseChannelNode(xmlNodePtr p)
{
	int32_t intValue;
	if(xmlStrcmp(p->name, (const xmlChar*)"channel"))
		return false;

	if(!readXMLInteger(p, "id", intValue) || intValue <= CHANNEL_GUILD)
	{
		std::clog << "[Warning - Chat::loadFromXml] Invalid or not specified channel id." << std::endl;
		return false;
	}

	uint16_t id = intValue;
	std::string strValue;
	if(m_normalChannels.find(id) != m_normalChannels.end() && (!readXMLString(p, "override", strValue) || !booleanString(strValue)))
	{
		std::clog << "[Warning - Chat::loadFromXml] Duplicated channel with id: " << id << "." << std::endl;
		return false;
	}

	if(!readXMLString(p, "name", strValue))
	{
		std::clog << "[Warning - Chat::loadFromXml] Missing name for channel with id: " << id << "." << std::endl;
		return false;
	}

	std::string name = strValue;
	uint16_t flags = ChatChannel::staticFlags;
	if(readXMLString(p, "enabled", strValue) && !booleanString(strValue))
		flags &= ~CHANNELFLAG_ENABLED;

	if(readXMLString(p, "active", strValue) && !booleanString(strValue))
		flags &= ~CHANNELFLAG_ACTIVE;

	if((readXMLString(p, "logged", strValue) || readXMLString(p, "log", strValue)) && booleanString(strValue))
		flags |= CHANNELFLAG_LOGGED;

	uint32_t access = 0;
	if(readXMLInteger(p, "access", intValue))
		access = intValue;

	uint32_t level = 1;
	if(readXMLInteger(p, "level", intValue))
		level = intValue;

	int32_t conditionId = -1;
	std::string conditionMessage = "You are muted.";

	Condition* condition = NULL;
	if(readXMLInteger(p, "muted", intValue))
	{
		conditionId = 3;
		int32_t tmp = intValue * 1000;
		if(readXMLInteger(p, "conditionId", intValue))
		{
			conditionId = intValue;
			if(conditionId < 3)
				std::clog << "[Warning - Chat::parseChannelNode] Using reserved muted condition sub id (" << conditionId << ")" << std::endl;
		}

		if(readXMLString(p, "conditionMessage", strValue))
			conditionMessage = strValue;

		if(tmp && !(condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MUTED, tmp, 0, false, conditionId)))
			conditionId = -1;
	}

	StringVec vocStringVec;
	VocationMap vocMap;

	std::string error;
	for(xmlNodePtr tmpNode = p->children; tmpNode; tmpNode = tmpNode->next)
	{
		if(!parseVocationNode(tmpNode, vocMap, vocStringVec, error))
			std::clog << "[Warning - Chat::loadFromXml] " << error << std::endl;
	}

	VocationMap* vocationMap = NULL;
	if(!vocMap.empty())
		vocationMap = new VocationMap(vocMap);

	switch(id)
	{
		case CHANNEL_PARTY:
		{
			partyName = name;
			break;
		}

		case CHANNEL_PRIVATE:
		{
			if(ChatChannel* newChannel = new PrivateChatChannel(CHANNEL_PRIVATE, name, flags))
				dummyPrivate = newChannel;

			break;
		}

		default:
		{
			if(ChatChannel* newChannel = new ChatChannel(id, name, flags, access, level,
				condition, conditionId, conditionMessage, vocationMap))
				m_normalChannels[id] = newChannel;

			break;
		}
	}

	return true;
}

ChatChannel* Chat::createChannel(Player* player, uint16_t channelId)
{
	if(!player || player->isRemoved() || getChannel(player, channelId))
		return NULL;

	switch(channelId)
	{
		case CHANNEL_GUILD:
		{
			ChatChannel* newChannel = NULL;
			if((newChannel = new ChatChannel(channelId, player->getGuildName(), ChatChannel::staticFlags)))
				m_guildChannels[player->getGuildId()] = newChannel;

			return newChannel;
		}

		case CHANNEL_PARTY:
		{
			ChatChannel* newChannel = NULL;
			if(player->getParty() && (newChannel = new ChatChannel(channelId, partyName, ChatChannel::staticFlags)))
				m_partyChannels[player->getParty()] = newChannel;

			return newChannel;
		}

		case CHANNEL_PRIVATE:
		{
			//only 1 private channel for each premium player
			if(!player->isPremium() || getPrivateChannel(player))
				return NULL;

			//find a free private channel slot
			for(uint16_t i = 100; i < 10000; ++i)
			{
				if(m_privateChannels.find(i) != m_privateChannels.end())
					continue;

				uint16_t flags = 0;
				if(dummyPrivate)
					flags = dummyPrivate->getFlags();

				PrivateChatChannel* newChannel = NULL;
				if((newChannel = new PrivateChatChannel(i, player->getName() + "'s Channel", flags)))
				{
					newChannel->setOwner(player->getGUID());
					m_privateChannels[i] = newChannel;
				}

				return newChannel;
			}
		}

		default:
			break;
	}

	return NULL;
}

bool Chat::deleteChannel(Player* player, uint16_t channelId)
{
	switch(channelId)
	{
		case CHANNEL_GUILD:
		{
			GuildChannelMap::iterator it = m_guildChannels.find(player->getGuildId());
			if(it == m_guildChannels.end())
				return false;

			delete it->second;
			m_guildChannels.erase(it);
			return true;
		}

		case CHANNEL_PARTY:
		{
			PartyChannelMap::iterator it = m_partyChannels.find(player->getParty());
			if(it == m_partyChannels.end())
				return false;

			delete it->second;
			m_partyChannels.erase(it);
			return true;
		}

		default:
		{
			PrivateChannelMap::iterator it = m_privateChannels.find(channelId);
			if(it == m_privateChannels.end())
				return false;

			it->second->closeChannel();
			delete it->second;

			m_privateChannels.erase(it);
			return true;
		}
	}

	return false;
}

ChatChannel* Chat::addUserToChannel(Player* player, uint16_t channelId)
{
	ChatChannel* channel = getChannel(player, channelId);
	if(channel && channel->addUser(player))
		return channel;

	return NULL;
}

void Chat::reOpenChannels(Player* player)
{
	if(!player || player->isRemoved())
		return;

	for(NormalChannelMap::iterator it = m_normalChannels.begin(); it != m_normalChannels.end(); ++it)
	{
		if(it->second->hasUser(player))
			player->sendChannel(it->second->getId(), it->second->getName());
	}

	for(PartyChannelMap::iterator it = m_partyChannels.begin(); it != m_partyChannels.end(); ++it)
	{
		if(it->second->hasUser(player))
			player->sendChannel(it->second->getId(), it->second->getName());
	}

	for(GuildChannelMap::iterator it = m_guildChannels.begin(); it != m_guildChannels.end(); ++it)
	{
		if(it->second->hasUser(player))
			player->sendChannel(it->second->getId(), it->second->getName());
	}

	for(PrivateChannelMap::iterator it = m_privateChannels.begin(); it != m_privateChannels.end(); ++it)
	{
		if(it->second->hasUser(player))
			player->sendChannel(it->second->getId(), it->second->getName());
	}
}

bool Chat::removeUserFromChannel(Player* player, uint16_t channelId)
{
	ChatChannel* channel = getChannel(player, channelId);
	if(!channel || !channel->removeUser(player))
		return false;

	if(channel->getOwner() == player->getGUID())
		deleteChannel(player, channelId);

	return true;
}

void Chat::removeUserFromChannels(Player* player)
{
	if(!player || player->isRemoved())
		return;

	for(NormalChannelMap::iterator it = m_normalChannels.begin(); it != m_normalChannels.end(); ++it)
		it->second->removeUser(player);

	for(PartyChannelMap::iterator it = m_partyChannels.begin(); it != m_partyChannels.end(); ++it)
		it->second->removeUser(player);

	for(GuildChannelMap::iterator it = m_guildChannels.begin(); it != m_guildChannels.end(); ++it)
		it->second->removeUser(player);

	for(PrivateChannelMap::iterator it = m_privateChannels.begin(); it != m_privateChannels.end(); ++it)
	{
		it->second->removeUser(player);
		if(it->second->getOwner() == player->getGUID())
			deleteChannel(player, it->second->getId());
	}
}

bool Chat::talk(Player* player, MessageClasses type, const std::string& text, uint16_t channelId,
	uint32_t statementId, bool anonymous/* = false*/)
{
	if(text.empty())
		return false;

	ChatChannel* channel = getChannel(player, channelId);
	if(!channel)
		return false;

	if(!player->hasFlag(PlayerFlag_CannotBeMuted))
	{
		if(!channel->hasFlag(CHANNELFLAG_ACTIVE))
		{
			player->sendTextMessage(MSG_STATUS_SMALL, "You may not speak into this channel.");
			return true;
		}

		if(player->getLevel() < channel->getLevel())
		{
			char buffer[100];
			sprintf(buffer, "You may not speak into this channel as long as you are on level %d.", channel->getLevel());
			player->sendCancel(buffer);
			return true;
		}

		if(channel->getConditionId() >= 0 && player->hasCondition(CONDITION_MUTED, channel->getConditionId()))
		{
			player->sendCancel(channel->getConditionMessage().c_str());
			return true;
		}
	}

	if(isPublicChannel(channelId))
		Manager::getInstance()->talk(player->getID(), channelId, type, text);

	if(channelId != CHANNEL_GUILD || !g_config.getBool(ConfigManager::INGAME_GUILD_MANAGEMENT)
		|| (text[0] != '!' && text[0] != '/'))
	{
		if(channelId == CHANNEL_GUILD)
		{
			switch(player->getGuildLevel())
			{
				case GUILDLEVEL_VICE:
					return channel->talk(player, MSG_CHANNEL_HIGHLIGHT, text, statementId);
				case GUILDLEVEL_LEADER:
					return channel->talk(player, MSG_GAMEMASTER_CHANNEL, text, statementId);
				default:
					break;
			}
		}

		if(anonymous)
			return channel->talk("", type, text);

		return channel->talk(player, type, text, statementId);
	}

	if(!player->getGuildId())
	{
		player->sendCancel("You are not in a guild.");
		return true;
	}

	if(!IOGuild::getInstance()->guildExists(player->getGuildId()))
	{
		player->sendCancel("It seems like your guild does not exist anymore.");
		return true;
	}

	char buffer[350];
	if(text.substr(1) == "disband")
	{
		if(player->getGuildLevel() == GUILDLEVEL_LEADER)
		{
			IOGuild::getInstance()->disbandGuild(player->getGuildId());
			channel->talk("", MSG_GAMEMASTER_CHANNEL, "The guild has been disbanded.");
		}
		else
			player->sendCancel("You are not the leader of your guild.");
	}
	else if(text.substr(1, 6) == "invite")
	{
		if(player->getGuildLevel() > GUILDLEVEL_MEMBER)
		{
			if(text.length() > 7)
			{
				std::string param = text.substr(8);
				trimString(param);

				Player* paramPlayer = NULL;
				if(g_game.getPlayerByNameWildcard(param, paramPlayer) == RET_NOERROR)
				{
					if(paramPlayer->getGuildId() == 0)
					{
						if(!paramPlayer->isGuildInvited(player->getGuildId()))
						{
							sprintf(buffer, "%s has invited you to join the guild, %s. You may join this guild by writing: !joinguild %s", player->getName().c_str(), player->getGuildName().c_str(), player->getGuildName().c_str());
							paramPlayer->sendTextMessage(MSG_EVENT_GUILD, buffer);

							sprintf(buffer, "%s has invited %s to the guild.", player->getName().c_str(), paramPlayer->getName().c_str());
							channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
							paramPlayer->invitationsList.push_back(player->getGuildId());
						}
						else
							player->sendCancel("A player with that name has already been invited to your guild.");
					}
					else
						player->sendCancel("A player with that name is already in a guild.");
				}
				else if(IOLoginData::getInstance()->playerExists(param))
				{
					uint32_t guid;
					IOLoginData::getInstance()->getGuidByName(guid, param);
					if(!IOGuild::getInstance()->hasGuild(guid))
					{
						if(!IOGuild::getInstance()->isInvited(player->getGuildId(), guid))
						{
							if(IOGuild::getInstance()->guildExists(player->getGuildId()))
							{
								IOGuild::getInstance()->invitePlayer(player->getGuildId(), guid);
								sprintf(buffer, "%s has invited %s to the guild.", player->getName().c_str(), param.c_str());
								channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
							}
							else
								player->sendCancel("Your guild does not exist anymore.");
						}
						else
							player->sendCancel("A player with that name has already been invited to your guild.");
					}
					else
						player->sendCancel("A player with that name is already in a guild.");
				}
				else
					player->sendCancel("A player with that name does not exist.");
			}
			else
				player->sendCancel("Invalid guildcommand parameters.");
		}
		else
			player->sendCancel("You don't have rights to invite players to your guild.");
	}
	else if(text.substr(1, 5) == "leave")
	{
		if(player->getGuildLevel() < GUILDLEVEL_LEADER)
		{
			if(!player->hasEnemy())
			{
				sprintf(buffer, "%s has left the guild.", player->getName().c_str());
				channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
				player->leaveGuild();
			}
			else
				player->sendCancel("Your guild is currently at war, you cannot leave it right now.");
		}
		else
			player->sendCancel("You cannot leave your guild because you are the leader of it, you have to pass the leadership to another member of your guild or disband the guild.");
	}
	else if(text.substr(1, 6) == "revoke")
	{
		if(player->getGuildLevel() > GUILDLEVEL_MEMBER)
		{
			if(text.length() > 7)
			{
				std::string param = text.substr(8);
				trimString(param);

				Player* paramPlayer = NULL;
				if(g_game.getPlayerByNameWildcard(param, paramPlayer) == RET_NOERROR)
				{
					if(paramPlayer->getGuildId() == 0)
					{
						InvitationsList::iterator it = std::find(paramPlayer->invitationsList.begin(), paramPlayer->invitationsList.end(), player->getGuildId());
						if(it != paramPlayer->invitationsList.end())
						{
							sprintf(buffer, "%s has revoked your invite to %s guild.", player->getName().c_str(), (player->getSex(false) ? "his" : "her"));
							paramPlayer->sendTextMessage(MSG_EVENT_GUILD, buffer);

							sprintf(buffer, "%s has revoked the guildinvite of %s.", player->getName().c_str(), paramPlayer->getName().c_str());
							channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);

							paramPlayer->invitationsList.erase(it);
							return true;
						}
						else
							player->sendCancel("A player with that name is not invited to your guild.");
					}
					else
						player->sendCancel("A player with that name is already in a guild.");
				}
				else if(IOLoginData::getInstance()->playerExists(param))
				{
					uint32_t guid;
					IOLoginData::getInstance()->getGuidByName(guid, param);
					if(IOGuild::getInstance()->isInvited(player->getGuildId(), guid))
					{
						if(IOGuild::getInstance()->guildExists(player->getGuildId()))
						{
							sprintf(buffer, "%s has revoked the guildinvite of %s.", player->getName().c_str(), param.c_str());
							channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
							IOGuild::getInstance()->revokeInvite(player->getGuildId(), guid);
						}
						else
							player->sendCancel("It seems like your guild does not exist anymore.");
					}
					else
						player->sendCancel("A player with that name is not invited to your guild.");
				}
				else
					player->sendCancel("A player with that name does not exist.");
			}
			else
				player->sendCancel("Invalid guildcommand parameters.");
		}
		else
			player->sendCancel("You don't have rights to revoke an invite of someone in your guild.");
	}
	else if(text.substr(1, 7) == "promote" || text.substr(1, 6) == "demote" || text.substr(1, 14) == "passleadership" || text.substr(1, 4) == "kick")
	{
		if(player->getGuildLevel() == GUILDLEVEL_LEADER)
		{
			std::string param;
			uint32_t length = 0;
			if(text[2] == 'r')
				length = 9;
			else if(text[2] == 'e')
				length = 7;
			else if(text[2] == 'a')
				length = 16;
			else
				length = 6;

			if(text.length() < length)
			{
				player->sendCancel("Invalid guildcommand parameters.");
				return true;
			}

			param = text.substr(length);
			trimString(param);

			Player* paramPlayer = NULL;
			if(g_game.getPlayerByNameWildcard(param, paramPlayer) == RET_NOERROR)
			{
				if(paramPlayer->getGuildId())
				{
					if(IOGuild::getInstance()->guildExists(paramPlayer->getGuildId()))
					{
						if(player->getGuildId() == paramPlayer->getGuildId())
						{
							if(text[2] == 'r')
							{
								if(paramPlayer->getGuildLevel() == GUILDLEVEL_MEMBER)
								{
									if(paramPlayer->isPremium())
									{
										paramPlayer->setGuildLevel(GUILDLEVEL_VICE);
										sprintf(buffer, "%s has promoted %s to %s.", player->getName().c_str(), paramPlayer->getName().c_str(), paramPlayer->getRankName().c_str());
										channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
									}
									else
										player->sendCancel("A player with that name does not have a premium account.");
								}
								else
									player->sendCancel("You can only promote Members to Vice-Leaders.");
							}
							else if(text[2] == 'e')
							{
								if(paramPlayer->getGuildLevel() == GUILDLEVEL_VICE)
								{
									paramPlayer->setGuildLevel(GUILDLEVEL_MEMBER);
									sprintf(buffer, "%s has demoted %s to %s.", player->getName().c_str(), paramPlayer->getName().c_str(), paramPlayer->getRankName().c_str());
									channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
								}
								else
									player->sendCancel("You can only demote Vice-Leaders to Members.");
							}
							else if(text[2] == 'a')
							{
								if(paramPlayer->getGuildLevel() == GUILDLEVEL_VICE)
								{
									const uint32_t levelToFormGuild = g_config.getNumber(ConfigManager::LEVEL_TO_FORM_GUILD);
									if(paramPlayer->getLevel() >= levelToFormGuild)
									{
										paramPlayer->setGuildLevel(GUILDLEVEL_LEADER);
										player->setGuildLevel(GUILDLEVEL_VICE);

										IOGuild::getInstance()->updateOwnerId(paramPlayer->getGuildId(), paramPlayer->getGUID());
										sprintf(buffer, "%s has passed the guild leadership to %s.", player->getName().c_str(), paramPlayer->getName().c_str());
										channel->talk("", MSG_GAMEMASTER_CHANNEL, buffer);
									}
									else
									{
										sprintf(buffer, "The new guild leader has to be at least Level %d.", levelToFormGuild);
										player->sendCancel(buffer);
									}
								}
								else
									player->sendCancel("A player with that name is not a Vice-Leader.");
							}
							else
							{
								if(player->getGuildLevel() > paramPlayer->getGuildLevel())
								{
									if(!player->hasEnemy())
									{
										sprintf(buffer, "%s has been kicked from the guild by %s.", paramPlayer->getName().c_str(), player->getName().c_str());
										channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
										paramPlayer->leaveGuild();
									}
									else
										player->sendCancel("Your guild is currently at war, you cannot kick right now.");
								}
								else
									player->sendCancel("You may only kick players with a guild rank below your.");
							}
						}
						else
							player->sendCancel("You are not in the same guild as a player with that name.");
					}
					else
						player->sendCancel("Could not find the guild of a player with that name.");
				}
				else
					player->sendCancel("A player with that name is not in a guild.");
			}
			else if(IOLoginData::getInstance()->playerExists(param))
			{
				uint32_t guid;
				IOLoginData::getInstance()->getGuidByName(guid, param);
				if(IOGuild::getInstance()->hasGuild(guid))
				{
					if(player->getGuildId() == IOGuild::getInstance()->getGuildId(guid))
					{
						if(text[2] == 'r')
						{
							if(IOGuild::getInstance()->getGuildLevel(guid) == GUILDLEVEL_MEMBER)
							{
								if(IOLoginData::getInstance()->isPremium(guid))
								{
									IOGuild::getInstance()->setGuildLevel(guid, GUILDLEVEL_VICE);
									sprintf(buffer, "%s has promoted %s to %s.", player->getName().c_str(), param.c_str(), IOGuild::getInstance()->getRank(guid).c_str());
									channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
								}
								else
									player->sendCancel("A player with that name does not have a premium account.");
							}
							else
								player->sendCancel("You can only promote Members to Vice-Leaders.");
						}
						else if(text[2] == 'e')
						{
							if(IOGuild::getInstance()->getGuildLevel(guid) == GUILDLEVEL_VICE)
							{
								IOGuild::getInstance()->setGuildLevel(guid, GUILDLEVEL_MEMBER);
								sprintf(buffer, "%s has demoted %s to %s.", player->getName().c_str(), param.c_str(), IOGuild::getInstance()->getRank(guid).c_str());
								channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
							}
							else
								player->sendCancel("You can only demote Vice-Leaders to Members.");
						}
						else if(text[2] == 'a')
						{
							if(IOGuild::getInstance()->getGuildLevel(guid) == GUILDLEVEL_VICE)
							{
								const uint32_t levelToFormGuild = g_config.getNumber(ConfigManager::LEVEL_TO_FORM_GUILD);
								if(IOLoginData::getInstance()->getLevel(guid) >= levelToFormGuild)
								{
									IOGuild::getInstance()->setGuildLevel(guid, GUILDLEVEL_LEADER);
									player->setGuildLevel(GUILDLEVEL_VICE);

									sprintf(buffer, "%s has passed the guild leadership to %s.", player->getName().c_str(), param.c_str());
									channel->talk("", MSG_GAMEMASTER_CHANNEL, buffer);
								}
								else
								{
									sprintf(buffer, "The new guild leader has to be at least Level %d.", levelToFormGuild);
									player->sendCancel(buffer);
								}
							}
							else
								player->sendCancel("A player with that name is not a Vice-Leader.");
						}
						else
						{
							sprintf(buffer, "%s has been kicked from the guild by %s.", param.c_str(), player->getName().c_str());
							channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
							IOLoginData::getInstance()->resetGuildInformation(guid);
						}
					}
				}
				else
					player->sendCancel("A player with that name is not in a guild.");
			}
			else
				player->sendCancel("A player with that name does not exist.");
		}
		else
			player->sendCancel("You are not the leader of your guild.");
	}
	else if(text.substr(1, 4) == "nick" && text.length() > 5)
	{
		StringVec params = explodeString(text.substr(6), ",");
		if(params.size() >= 2)
		{
			std::string param1 = params[0], param2 = params[1];
			trimString(param1);
			trimString(param2);

			Player* paramPlayer = NULL;
			if(g_game.getPlayerByNameWildcard(param1, paramPlayer) == RET_NOERROR)
			{
				if(paramPlayer->getGuildId())
				{
					if(param2.length() > 2)
					{
						if(param2.length() < 21)
						{
							if(isValidName(param2, false))
							{
								if(IOGuild::getInstance()->guildExists(paramPlayer->getGuildId()))
								{
									if(player->getGuildId() == paramPlayer->getGuildId())
									{
										if(paramPlayer->getGuildLevel() < player->getGuildLevel() || (player == paramPlayer && player->getGuildLevel() > GUILDLEVEL_MEMBER))
										{
											paramPlayer->setGuildNick(param2);
											if(player != paramPlayer)
												sprintf(buffer, "%s has set the guildnick of %s to \"%s\".", player->getName().c_str(), paramPlayer->getName().c_str(), param2.c_str());
											else
												sprintf(buffer, "%s has set %s guildnick to \"%s\".", player->getName().c_str(), (player->getSex(false) ? "his" : "her"), param2.c_str());

											channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
										}
										else
											player->sendCancel("You may only change the guild nick of players that have a lower rank than you.");
									}
									else
										player->sendCancel("A player with that name is not in your guild.");
								}
								else
									player->sendCancel("A player with that name's guild could not be found.");
							}
							else
								player->sendCancel("That guildnick is not valid.");
						}
						else
							player->sendCancel("That guildnick is too long, please select a shorter one.");
					}
					else
						player->sendCancel("That guildnick is too short, please select a longer one.");
				}
				else
					player->sendCancel("A player with that name is not in a guild.");
			}
			else if(IOLoginData::getInstance()->playerExists(param1))
			{
				uint32_t guid;
				IOLoginData::getInstance()->getGuidByName(guid, (std::string&)param1);
				if(IOGuild::getInstance()->hasGuild(guid))
				{
					if(param2.length() > 2)
					{
						if(param2.length() < 21)
						{
							if(isValidName(param2, false))
							{
								if(IOGuild::getInstance()->guildExists(guid))
								{
									if(player->getGuildId() == IOGuild::getInstance()->getGuildId(guid))
									{
										if(IOGuild::getInstance()->getGuildLevel(guid) < player->getGuildLevel())
										{
											IOGuild::getInstance()->setGuildNick(guid, param2);
											sprintf(buffer, "%s has set the guildnick of %s to \"%s\".", player->getName().c_str(), param1.c_str(), param2.c_str());
											channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
										}
										else
											player->sendCancel("You may only change the guild nick of players that have a lower rank than you.");
									}
									else
										player->sendCancel("A player with that name is not in your guild.");
								}
								else
									player->sendCancel("A player with that name's guild could not be found.");
							}
							else
								player->sendCancel("That guildnick is not valid.");
						}
						else
							player->sendCancel("That guildnick is too long, please select a shorter one.");
					}
					else
						player->sendCancel("That guildnick is too short, please select a longer one.");
				}
				else
					player->sendCancel("A player with that name is not in any guild.");
			}
			else
				player->sendCancel("A player with that name does not exist.");
		}
		else
			player->sendCancel("Invalid guildcommand parameters.");
	}
	else if(text.substr(1, 11) == "setrankname" && text.length() > 12)
	{
		StringVec params = explodeString(text.substr(13), ",");
		if(params.size() >= 2)
		{
			std::string param1 = params[0], param2 = params[1];
			trimString(param1);
			trimString(param2);

			if(player->getGuildLevel() == GUILDLEVEL_LEADER)
			{
				if(param2.length() > 2)
				{
					if(param2.length() < 21)
					{
						if(isValidName(param2, false))
						{
							if(IOGuild::getInstance()->getRankIdByName(player->getGuildId(), param1))
							{
								if(!IOGuild::getInstance()->getRankIdByName(player->getGuildId(), param2))
								{
									IOGuild::getInstance()->changeRank(player->getGuildId(), param1, param2);
									sprintf(buffer, "%s has renamed the guildrank: \"%s\", to: \"%s\".", player->getName().c_str(), param1.c_str(), param2.c_str());
									channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
								}
								else
									player->sendCancel("There is already a rank in your guild with that name.");
							}
							else
								player->sendCancel("There is no such rankname in your guild.");
						}
						else
							player->sendCancel("The new guildrank contains invalid characters.");
					}
					else
						player->sendCancel("The new rankname is too long.");
				}
				else
					player->sendCancel("The new rankname is too short.");
			}
			else
				player->sendCancel("You are not the leader of your guild.");
		}
		else
			player->sendCancel("Invalid guildcommand parameters");
	}
	else if(text.substr(1, 7) == "setmotd")
	{
		if(player->getGuildLevel() == GUILDLEVEL_LEADER)
		{
			if(text.length() > 8)
			{
				std::string param = text.substr(9);
				trimString(param);
				if(param.length() > 2)
				{
					if(param.length() < 225)
					{
						IOGuild::getInstance()->setMotd(player->getGuildId(), param);
						sprintf(buffer, "%s has set the Message of the Day to: %s", player->getName().c_str(), param.c_str());
						channel->talk("", MSG_GAMEMASTER_CHANNEL, buffer);
					}
					else
						player->sendCancel("That motd is too long.");
				}
				else
					player->sendCancel("That motd is too short.");
			}
			else
				player->sendCancel("Invalid guildcommand parameters.");
		}
		else
			player->sendCancel("Only the leader of your guild can set the guild motd.");
	}
	else if(text.substr(1, 9) == "cleanmotd")
	{
		if(player->getGuildLevel() == GUILDLEVEL_LEADER)
		{
			IOGuild::getInstance()->setMotd(player->getGuildId(), "");
			sprintf(buffer, "%s has cleaned the Message of the Day.", player->getName().c_str());
			channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
		}
		else
			player->sendCancel("Only the leader of your guild can clean the guild motd.");
	}
	else if(text.substr(1, 8) == "commands")
	{
		sprintf(buffer, "Guild commands with parameters: disband, invite[name], leave, kick[name], revoke[name], demote[name], promote[name], passleadership[name], nick[name, nick], setrankname[oldName, newName], setmotd[text] and cleanmotd.");
		channel->talk("", MSG_CHANNEL_HIGHLIGHT, buffer);
	}
	else
		return false;

	return true;
}

std::string Chat::getChannelName(Player* player, uint16_t channelId)
{
	if(ChatChannel* channel = getChannel(player, channelId))
		return channel->getName();

	return "";
}

ChannelsList Chat::getChannelList(Player* player)
{
	ChannelsList list;
	if(!player || player->isRemoved())
		return list;

	ChatChannel* channel = NULL;
	if(player->getParty() && ((channel = getChannel(player, CHANNEL_PARTY)) || (channel = createChannel(player, CHANNEL_PARTY))))
		list.push_back(std::make_pair(channel->getId(), channel->getName()));

	if(player->getGuildId() && player->getGuildName().length() && ((channel = getChannel(
		player, CHANNEL_GUILD)) || (channel = createChannel(player, CHANNEL_GUILD))))
		list.push_back(std::make_pair(channel->getId(), channel->getName()));

	for(NormalChannelMap::iterator it = m_normalChannels.begin(); it != m_normalChannels.end(); ++it)
	{
		if((channel = getChannel(player, it->first)))
			list.push_back(std::make_pair(channel->getId(), channel->getName()));
	}

	bool hasPrivate = false;
	PrivateChatChannel* privateChannel = NULL;
	for(PrivateChannelMap::iterator pit = m_privateChannels.begin(); pit != m_privateChannels.end(); ++pit)
	{
		if(!(privateChannel = pit->second))
			continue;

		if(privateChannel->isInvited(player))
			list.push_back(std::make_pair(privateChannel->getId(), privateChannel->getName()));

		if(privateChannel->getOwner() == player->getGUID())
			hasPrivate = true;
	}

	if(!hasPrivate && player->isPremium())
		list.push_front(std::make_pair(dummyPrivate->getId(), dummyPrivate->getName()));

	return list;
}

ChatChannel* Chat::getChannel(Player* player, uint16_t channelId)
{
	#ifdef __DEBUG_CHAT__
	std::clog << "Chat::getChannel - getChannel id " << channelId << std::endl;
	#endif
	if(!player || player->isRemoved())
		return NULL;

	if(channelId == CHANNEL_GUILD)
	{
		GuildChannelMap::iterator git = m_guildChannels.find(player->getGuildId());
		if(git != m_guildChannels.end())
			return git->second;

		return NULL;
	}

	if(channelId == CHANNEL_PARTY)
	{
		if(player->getParty())
		{
			PartyChannelMap::iterator it = m_partyChannels.find(player->getParty());
			if(it != m_partyChannels.end())
				return it->second;
		}

		return NULL;
	}

	NormalChannelMap::iterator nit = m_normalChannels.find(channelId);
	if(nit != m_normalChannels.end())
	{
		#ifdef __DEBUG_CHAT__
		std::clog << "Chat::getChannel - found normal channel" << std::endl;
		#endif
		ChatChannel* tmpChannel = nit->second;
		if(!tmpChannel || !tmpChannel->hasFlag(CHANNELFLAG_ENABLED) || player->getAccess() < tmpChannel->getAccess()
			|| (!player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges) && !tmpChannel->checkVocation(
			player->getVocationId())))
		{
			#ifdef __DEBUG_CHAT__
			std::clog << "Chat::getChannel - cannot access normal channel" << std::endl;
			#endif
			return NULL;
		}

		if(channelId == CHANNEL_RVR && !player->hasFlag(PlayerFlag_CanAnswerRuleViolations))
			return NULL;

		#ifdef __DEBUG_CHAT__
		std::clog << "Chat::getChannel - endpoint return" << std::endl;
		#endif
		return tmpChannel;
	}

	PrivateChannelMap::iterator pit = m_privateChannels.find(channelId);
	if(pit != m_privateChannels.end() && pit->second->isInvited(player))
		return pit->second;

	return NULL;
}

ChatChannel* Chat::getChannelById(uint16_t channelId)
{
	NormalChannelMap::iterator it = m_normalChannels.find(channelId);
	if(it != m_normalChannels.end())
		return it->second;

	return NULL;
}

PrivateChatChannel* Chat::getPrivateChannel(Player* player)
{
	if(!player || player->isRemoved())
		return NULL;

	PrivateChatChannel* channel = NULL;
	for(PrivateChannelMap::iterator it = m_privateChannels.begin(); it != m_privateChannels.end(); ++it)
	{
		if((channel = it->second) && channel->getOwner() == player->getGUID())
			return channel;
	}

	return NULL;
}

ChannelList Chat::getPublicChannels() const
{
	ChannelList list;
	for(NormalChannelMap::const_iterator it = m_normalChannels.begin(); it != m_normalChannels.end(); ++it)
	{
		if(isPublicChannel(it->first))
			list.push_back(it->second);
	}

	return list;
}
