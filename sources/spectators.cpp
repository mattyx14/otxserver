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
#include "spectators.h"
#include "configmanager.h"

#include "player.h"
#include "chat.h"

#include "database.h"
#include "tools.h"

#include "game.h"
extern Game g_game;
extern ConfigManager g_config;
extern Chat g_chat;

bool Spectators::check(const std::string& _password)
{
	if(password.empty())
		return true;

	std::string t = _password;
	return trimString(t) == password;
}

void Spectators::sendLook(ProtocolGame* client, const Position& pos, uint16_t spriteId, int16_t stackpos)
{
	if (!owner)
		return;

	SpectatorList::iterator sit = spectators.find(client);
	if (sit == spectators.end())
		return;

	Thing* thing = g_game.internalGetThing(client->player, pos, stackpos, spriteId, STACKPOS_LOOK);
	if (!thing)
		return;

	Position thingPos = pos;
	if (pos.x == 0xFFFF)
		thingPos = thing->getPosition();

	Position playerPos = client->player->getPosition();
	int32_t lookDistance = -1;
	if (thing != client->player)
	{
		lookDistance = std::max(std::abs(playerPos.x - thingPos.x), std::abs(playerPos.y - thingPos.y));
		if (playerPos.z != thingPos.z)
			lookDistance += 15;
	}

	std::ostringstream ss;
	ss << "You see " << thing->getDescription(lookDistance);

	if (Item * item = thing->getItem())
	{
		if (client->player->hasCustomFlag(PlayerCustomFlag_CanSeeItemDetails))
		{
			ss << ".";
			const ItemType& it = Item::items[item->getID()];
			if (it.transformEquipTo)
				ss << std::endl << "TransformTo: [" << it.transformEquipTo << "] (onEquip).";
			else if (it.transformDeEquipTo)
				ss << std::endl << "TransformTo: [" << it.transformDeEquipTo << "] (onDeEquip).";

			if (it.transformUseTo)
				ss << std::endl << "TransformTo: [" << it.transformUseTo << "] (onUse).";

			if (it.decayTo != -1)
				ss << std::endl << "DecayTo: [" << it.decayTo << "].";
		}
	}

	client->sendTextMessage(MSG_INFO_DESCR, ss.str());
	return;
}

void Spectators::handle(ProtocolGame* client, const std::string& text, uint16_t channelId)
{
	if(!owner)
		return;

	SpectatorList::iterator sit = spectators.find(client);
	if(sit == spectators.end())
		return;

	PrivateChatChannel* channel = g_chat.getPrivateChannel(owner->getPlayer());
	if(text[0] == '/')
	{
		StringVec t = explodeString(text.substr(1, text.length()), " ", true, 1);
		toLowerCaseString(t[0]);
		if(t[0] == "show")
		{
			std::ostringstream s;
			s << spectators.size() << " spectators. ";
			for(SpectatorList::const_iterator it = spectators.begin(); it != spectators.end(); ++it)
			{
				if (!it->first->spy)
				{
					if(it != spectators.begin())
						s << " ,";

					s << it->second.first;
				}
			}

			s << ".";
			client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, s.str(), NULL, 0);
		}
		else if(t[0] == "name")
		{
			if(t.size() > 1)
			{
				if(t[1].length() > 2)
				{
					if(t[1].length() < 26)
					{
						t[1] += " [G]";
						bool found = false;
						for(SpectatorList::iterator iit = spectators.begin(); iit != spectators.end(); ++iit)
						{
							if(asLowerCaseString(iit->second.first) != asLowerCaseString(t[1]))
								continue;

							found = true;
							break;
						}

						if(!found)
						{
							client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Your name has been set to " + t[1] + ".", NULL, 0);
							if(!auth && channel)
								sendChannelMessage("", sit->second.first + " is now known as " + t[1] + ".", MSG_GAMEMASTER_CHANNEL, channel->getId());

							StringVec::iterator mit = std::find(mutes.begin(), mutes.end(), asLowerCaseString(sit->second.first));
							if(mit != mutes.end())
								(*mit) = asLowerCaseString(t[1]);

							sit->second.first = t[1];
							sit->second.second = false;
						}
						else
							client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Specified name is already taken.", NULL, 0);
					}
					else
						client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Specified name is too long.", NULL, 0);
				}
				else
					client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Specified name is too short.", NULL, 0);
			}
			else
				client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Not enough param(s) given.", NULL, 0);
		}
		else if(t[0] == "auth")
		{
			if((time(NULL) - client->lastCastMsg) < 5) 
			{
				client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "You need to wait a bit before trying to authenticate again.", NULL, 0);
				return;
			}
			client->lastCastMsg = time(NULL);
			if(t.size() > 1)
			{
				StringVec _t = explodeString(t[1], " ", true, 1);
				if(_t.size() > 1)
				{
					Database* db = Database::getInstance();
					std::ostringstream query;

					query << "SELECT `id`, `salt`, `password` FROM `accounts` WHERE `name` " << db->getStringComparer() << db->escapeString(_t[0]) << " LIMIT 1";
					if(DBResult* result = db->storeQuery(query.str()))
					{
						std::string password = result->getDataString("salt") + _t[1],
							hash = result->getDataString("password");
						uint32_t id = result->getDataInt("id");

						result->free();
						if(encryptTest(password, hash))
						{
							query.str("");
							query << "SELECT `name` FROM `players` WHERE `account_id` = " << id << " ORDER BY `level` DESC LIMIT 1";
							if((result = db->storeQuery(query.str())))
							{
								std::string nickname = result->getDataString("name");
								result->free();

								client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "You have authenticated as " + nickname + ".", NULL, 0);
								if(channel)
									sendChannelMessage("", sit->second.first + " authenticated as " + nickname + ".", MSG_GAMEMASTER_CHANNEL, channel->getId());

								StringVec::iterator mit = std::find(mutes.begin(), mutes.end(), asLowerCaseString(sit->second.first));
								if(mit != mutes.end())
									(*mit) = asLowerCaseString(nickname);

								sit->second.first = nickname;
								sit->second.second = true;
							}
							else
								client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Your account has no characters yet.", NULL, 0);
						}
						else
							client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Invalid password.", NULL, 0);
					}
					else
						client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Invalid account name.", NULL, 0);
				}
				else
					client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Not enough param(s) given.", NULL, 0);
			}
			else
				client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Not enough param(s) given.", NULL, 0);
		}
		else
			client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Command not found.", NULL, 0);

		return;
	}

	if(!auth || sit->second.second)
	{
		StringVec::const_iterator mit = std::find(mutes.begin(), mutes.end(), asLowerCaseString(sit->second.first));
		if(mit == mutes.end())
		{
			if (channel && channel->getId() == channelId)
			{
				// prevents exhaust < 2s
				uint16_t exhaust = g_config.getNumber(ConfigManager::EXHAUST_SPECTATOR_SAY);
				if(exhaust <= 2)
					exhaust = 2;

				if ((time(NULL) - client->lastCastMsg) < exhaust)
				{
					uint16_t timeToSpeak = (client->lastCastMsg - time(NULL) + exhaust);
					client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, ("You are exhausted, wait " + std::to_string(timeToSpeak) + " second" + (timeToSpeak > 1 ? "s" : "") + " to talk again!"), NULL, 0);
					return;
				}
				client->lastCastMsg = time(NULL);
				std::string _text = asLowerCaseString(text);    
				StringVec prohibitedWords;
				prohibitedWords = explodeString(g_config.getString(ConfigManager::ADVERTISING_BLOCK), ";");
				bool fakeChat = false;

				std::string concatenatedText = g_game.removeNonAlphabetic(_text);
				// if advertising is empty, don't need check nothing
				if (!prohibitedWords.empty() && !_text.empty())
				{
					for (const auto& prohibitedWord : prohibitedWords)
					{
						if (!prohibitedWord.empty() && concatenatedText.find(prohibitedWord) != std::string::npos)
						{
							fakeChat = true;
							break;
						}
					}
				}
				channel->talk(sit->second.first, MSG_CHANNEL_HIGHLIGHT, text, fakeChat, client->getIP());
			}
		}
		else
			client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "You are muted.", NULL, 0);
	}
	else
		client->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "This chat is protected, you have to authenticate first.", NULL, 0);
}

void Spectators::chat(uint16_t channelId)
{
	if(!owner)
		return;

	for (auto& spectator : spectators)
	{
		spectator.first->sendClosePrivate(channelId);
		// idk why cast channelId is 100
		if(channelId == 100)
			spectator.first->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Chat has been disabled.", NULL, 0);
	}
}

void Spectators::kick(StringVec list)
{
	for (const auto& name : list)
	{
		for (auto it = spectators.begin(); it != spectators.end(); ++it)
		{
			if (!it->first->spy && asLowerCaseString(it->second.first) == name)
				it->first->disconnect();
		}
	}
}

void Spectators::ban(StringVec _bans)
{
	for (auto bit = bans.begin(); bit != bans.end();)
	{
		auto it = std::find(_bans.begin(), _bans.end(), bit->first);
		if (it == _bans.end())
			bit = bans.erase(bit);
		else
			++bit;
	}

	for (const auto& ban : _bans)
	{
		for (const auto& spectator : spectators)
		{
			if (asLowerCaseString(spectator.second.first) == ban)
			{
				bans[ban] = spectator.first->getIP();
				spectator.first->disconnect();
			}
		}
	}
}

void Spectators::addSpectator(ProtocolGame* client, std::string name, bool spy)
{
	if(++id == 65536)
		id = 1;

	std::ostringstream s;
	if (name.empty())
		s << "Spectator " << id << "";
	else
	{
		s << name << " (Telescope)";
		for (const auto& it : spectators)
		{
			if (it.second.first.compare(name) == 0)
				s << " " << id;
		}
	}
 
	spectators[client] = std::make_pair(s.str(), false);
	if (!spy)
	{
		sendTextMessage(MSG_EVENT_ORANGE, s.str() + " has entered the cast.");

		Database* db = Database::getInstance();
		std::ostringstream query;

		query << "SELECT `castDescription` FROM `players` WHERE `id` = " << owner->getPlayer()->getGUID() << ";";
		if(DBResult* result = db->storeQuery(query.str()))
		{
			std::string comment = result->getDataString("castDescription");
			result->free();

			if(comment != "")
				client->sendCreatureSay(owner->getPlayer(), MSG_STATUS_WARNING, comment, NULL, 0);
		}
	}
}

void Spectators::removeSpectator(ProtocolGame* client, bool spy)
{
	SpectatorList::iterator it = spectators.find(client);
	if(it == spectators.end())
		return;

	mutes.erase(std::remove(mutes.begin(), mutes.end(), it->second.first), mutes.end());

	if (!spy) 
		sendTextMessage(MSG_STATUS_CONSOLE_RED, it->second.first + " has left the cast.");

	spectators.erase(it);
}

void Spectators::sendChannelMessage(std::string author, std::string text, MessageClasses type, uint16_t channel, bool fakeChat, uint32_t ip)
{
	if(!owner)
		return;
	
	if(!fakeChat)
		owner->sendChannelMessage(author, text, type, channel);
	PrivateChatChannel* tmp = g_chat.getPrivateChannel(owner->getPlayer());
	if(!tmp || tmp->getId() != channel)
		return;

	for (auto& spectator : spectators)
	{
		if(fakeChat && spectator.first->getIP() != ip)
			continue;

		spectator.first->sendChannelMessage(author, text, type, channel);
	}
}

void Spectators::sendCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos, uint32_t statementId, bool fakeChat)
{
	if (!owner)
		return;

	owner->sendCreatureSay(creature, type, text, pos, statementId);
	if (type == MSG_PRIVATE || type == MSG_GAMEMASTER_PRIVATE || type == MSG_NPC_TO) // care for privacy!
		return;
	
	if (owner->getPlayer())
	{
		for (auto& spectator : spectators)
		{
			if(fakeChat && spectator.first->getIP() != owner->getIP())
				continue;

			spectator.first->sendCreatureSay(creature, type, text, pos, statementId);
		}
	}
}

void Spectators::sendCreatureChannelSay(const Creature* creature, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId, bool fakeChat)
{
	if(!owner)
		return;

	owner->sendCreatureChannelSay(creature, type, text, channelId, statementId);
	PrivateChatChannel* tmp = g_chat.getPrivateChannel(owner->getPlayer());
	if(!tmp || tmp->getId() != channelId)
		return;

	if (owner->getPlayer())
	{
		for (const auto& spectator : spectators)
		{
			if(fakeChat && spectator.first->getIP() != owner->getIP())
				continue;

			spectator.first->sendCreatureChannelSay(creature, type, text, channelId, statementId);
		}
	}
}

void Spectators::sendClosePrivate(uint16_t channelId)
{
	if(!owner)
		return;

	owner->sendClosePrivate(channelId);
	PrivateChatChannel* tmp = g_chat.getPrivateChannel(owner->getPlayer());
	if(!tmp || tmp->getId() != channelId)
		return;

	for (const auto& spectator : spectators)
	{
		spectator.first->sendClosePrivate(channelId);
		spectator.first->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Chat has been disabled.", NULL, 0);
	}
}

void Spectators::sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName)
{
	if(!owner)
		return;

	owner->sendCreatePrivateChannel(channelId, channelName);
	PrivateChatChannel* tmp = g_chat.getPrivateChannel(owner->getPlayer());
	if(!tmp || tmp->getId() != channelId)
		return;

	for (const auto& spectator : spectators)
	{
		spectator.first->sendCreatePrivateChannel(channelId, channelName);
		spectator.first->sendCreatureSay(owner->getPlayer(), MSG_PRIVATE, "Chat has been enabled.", NULL, 0);
	}
}
