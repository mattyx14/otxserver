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
#include <boost/function.hpp>
#include <iostream>
#include <iomanip>

#include "protocolgame.h"
#include "textlogger.h"

#include "waitlist.h"
#include "player.h"

#include "connection.h"
#include "networkmessage.h"
#include "outputmessage.h"

#include "iologindata.h"
#include "ioban.h"
#include "iomarket.h"

#include "items.h"
#include "tile.h"
#include "house.h"

#include "actions.h"
#include "creatureevent.h"
#include "quests.h"
#include "mounts.h"

#include "chat.h"
#include "configmanager.h"
#include "game.h"

#include "resources.h"

/*
Bytes not yet added:
	0x87 -> Position swapping (http://www.tibia.com/support/?subtopic=faq&question=swapping)
	0xF4 -> Flash client objects cache?
	0xF5 -> Flash client inventory?
*/

#if defined(WINDOWS) && !defined(_CONSOLE)
#include "gui.h"
#endif

extern Game g_game;
extern ConfigManager g_config;
extern Actions actions;
extern CreatureEvents* g_creatureEvents;
extern Chat g_chat;

template<class FunctionType>
void ProtocolGame::addGameTaskInternal(uint32_t delay, const FunctionType& func)
{
	if(delay > 0)
		Dispatcher::getInstance().addTask(createTask(delay, func));
	else
		Dispatcher::getInstance().addTask(createTask(func));
}

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolGame::protocolGameCount = 0;

#endif
void ProtocolGame::setPlayer(Player* p)
{
	player = p;
}

void ProtocolGame::releaseProtocol()
{
	if(player && player->client == this)
		player->client = NULL;

	Protocol::releaseProtocol();
}

void ProtocolGame::deleteProtocolTask()
{
	if(player)
	{
		g_game.freeThing(player);
		player = NULL;
	}

	Protocol::deleteProtocolTask();
}

bool ProtocolGame::login(const std::string& name, uint32_t id, const std::string&,
	OperatingSystem_t operatingSystem, uint16_t version, bool gamemaster)
{
	//dispatcher thread
	PlayerVector players = g_game.getPlayersByName(name);
	Player* _player = NULL;
	if(!players.empty())
		_player = players[random_range(0, (players.size() - 1))];

	bool accountManager = g_config.getBool(ConfigManager::ACCOUNT_MANAGER);
	if(!_player || g_config.getNumber(ConfigManager::ALLOW_CLONES) ||
		(accountManager && name == "Account Manager"))
	{
		player = new Player(name, this);
		player->addRef();

		player->setID();
		if(!IOLoginData::getInstance()->loadPlayer(player, name, true))
		{
			disconnectClient(0x14, "Your character could not be loaded.");
			return false;
		}

		Ban ban;
		ban.value = player->getGUID();
		ban.param = PLAYERBAN_BANISHMENT;

		ban.type = BAN_PLAYER;
		if(IOBan::getInstance()->getData(ban) && !player->hasFlag(PlayerFlag_CannotBeBanned))
		{
			bool deletion = ban.expires < 0;
			std::string name_ = "Automatic ";
			if(!ban.adminId)
				name_ += (deletion ? "deletion" : "banishment");
			else
				IOLoginData::getInstance()->getNameByGuid(ban.adminId, name_, true);

			std::stringstream stream;
			stream << "Your character has been " << (deletion ? "deleted" : "banished") << " at:\n" << formatDateEx(ban.added, "%d %b %Y").c_str() << " by: " << name_.c_str()
				   << ".\nThe comment given was:\n" << ban.comment.c_str() << ".\nYour " << (deletion ? "character won't be undeleted" : "banishment will be lifted at:\n")
				   << (deletion ? "" : formatDateEx(ban.expires).c_str()) << ".";

			disconnectClient(0x14, stream.str().c_str());
			return false;
		}

		if(IOBan::getInstance()->isPlayerBanished(player->getGUID(), PLAYERBAN_LOCK) && id != 1)
		{
			if(g_config.getBool(ConfigManager::NAMELOCK_MANAGER))
			{
				player->name = "Account Manager";
				player->accountManager = MANAGER_NAMELOCK;

				player->managerNumber = id;
				player->managerString2 = name;
			}
			else
			{
				disconnectClient(0x14, "Your character has been namelocked.");
				return false;
			}
		}
		else if(player->getName() == "Account Manager")
		{
			if(!g_config.getBool(ConfigManager::ACCOUNT_MANAGER))
			{
				disconnectClient(0x14, "Account Manager is disabled.");
				return false;
			}

			if(id != 1)
			{
				player->accountManager = MANAGER_ACCOUNT;
				player->managerNumber = id;
			}
			else
				player->accountManager = MANAGER_NEW;
		}

		if(gamemaster && !player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges))
		{
			disconnectClient(0x14, "You are not a gamemaster! Turn off the gamemaster mode in your IP changer.");
			return false;
		}

		if(!player->hasFlag(PlayerFlag_CanAlwaysLogin))
		{
			if(g_game.getGameState() == GAMESTATE_CLOSING)
			{
				disconnectClient(0x14, "Gameworld is just going down, please come back later.");
				return false;
			}

			if(g_game.getGameState() == GAMESTATE_CLOSED)
			{
				disconnectClient(0x14, "Gameworld is currently closed, please come back later.");
				return false;
			}
		}

		if(g_config.getBool(ConfigManager::ONE_PLAYER_ON_ACCOUNT) && !player->isAccountManager() &&
			!IOLoginData::getInstance()->hasCustomFlag(id, PlayerCustomFlag_CanLoginMultipleCharacters))
		{
			bool found = false;
			PlayerVector tmp = g_game.getPlayersByAccount(id);
			for(PlayerVector::iterator it = tmp.begin(); it != tmp.end(); ++it)
			{
				if((*it)->getName() != name)
					continue;

				found = true;
				break;
			}

			if(tmp.size() > 0 && !found)
			{
				disconnectClient(0x14, "You may only login with one character\nof your account at the same time.");
				return false;
			}
		}

		if(!WaitingList::getInstance()->login(player))
		{
			if(OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false))
			{
				TRACK_MESSAGE(output);
				std::stringstream ss;
				ss << "Too many players online.\n" << "You are ";

				int32_t slot = WaitingList::getInstance()->getSlot(player);
				if(slot)
				{
					ss << "at ";
					if(slot > 0)
						ss << slot;
					else
						ss << "unknown";

					ss << " place on the waiting list.";
				}
				else
					ss << "awaiting connection...";

				output->put<char>(0x16);
				output->putString(ss.str());
				output->put<char>(WaitingList::getTime(slot));
				OutputMessagePool::getInstance()->send(output);
			}

			disconnect();
			return false;
		}

		if(!IOLoginData::getInstance()->loadPlayer(player, name))
		{
			disconnectClient(0x14, "Your character could not be loaded.");
			return false;
		}

		player->setClientVersion(version);
		player->setOperatingSystem(operatingSystem);

		if(player->isUsingOtclient())
		{
			player->registerCreatureEvent("ExtendedOpcode");
		}

		player->lastIP = player->getIP();
		player->lastLoad = OTSYS_TIME();
		player->lastLogin = std::max(time(NULL), player->lastLogin + 1);

		sendPendingStateEntered();
		if(!g_game.placeCreature(player, player->getLoginPosition()) && !g_game.placeCreature(player, player->getMasterPosition(), false, true))
		{
			disconnectClient(0x14, "Temple position is wrong. Contact with the administration.");
			return false;
		}

		m_acceptPackets = true;
		return true;
	}

	if(gamemaster && !_player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges))
	{
		disconnectClient(0x14, "You are not a gamemaster! Turn off the gamemaster mode in your IP changer.");
		return false;
	}

	if(_player->hasClient())
	{
		if(m_eventConnect || !g_config.getBool(ConfigManager::REPLACE_KICK_ON_LOGIN))
		{
			// task has already been scheduled just bail out (should not be overriden)
			disconnectClient(0x14, "You are already logged in.");
			return false;
		}

		_player->client->disconnect();
		_player->isConnecting = true;
		_player->setClientVersion(version);

		addRef();
		m_eventConnect = Scheduler::getInstance().addEvent(createSchedulerTask(
			1000, boost::bind(&ProtocolGame::connect, this, _player->getID(), operatingSystem, version)));
		return true;
	}

	addRef();
	return connect(_player->getID(), operatingSystem, version);
}

bool ProtocolGame::logout(bool displayEffect, bool forceLogout)
{
	//dispatcher thread
	if(!player)
		return false;

	if(!player->isRemoved())
	{
		if(!forceLogout)
		{
			if(!IOLoginData::getInstance()->hasCustomFlag(player->getAccount(), PlayerCustomFlag_CanLogoutAnytime))
			{
				if(player->getTile()->hasFlag(TILESTATE_NOLOGOUT))
				{
					player->sendCancelMessage(RET_YOUCANNOTLOGOUTHERE);
					return false;
				}

				if(player->getZone() != ZONE_PROTECTION && player->hasCondition(CONDITION_INFIGHT))
				{
					player->sendCancelMessage(RET_YOUMAYNOTLOGOUTDURINGAFIGHT);
					return false;
				}

				if(!g_creatureEvents->playerLogout(player, false)) //let the script handle the error message
					return false;
			}
			else
				g_creatureEvents->playerLogout(player, true);
		}
		else if(!g_creatureEvents->playerLogout(player, true))
			return false;

		if(displayEffect && !player->isGhost())
			g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	}

	disconnect();
	if(player->isRemoved())
		return true;

	return g_game.removeCreature(player);
}

bool ProtocolGame::connect(uint32_t playerId, OperatingSystem_t operatingSystem, uint16_t version)
{
	unRef();
	m_eventConnect = 0;

	Player* _player = g_game.getPlayerByID(playerId);
	if(!_player || _player->isRemoved() || _player->hasClient())
	{
		disconnectClient(0x14, "You are already logged in.");
		return false;
	}

	player = _player;
	player->addRef();
	player->client = this;
	player->isConnecting = false;
	sendPendingStateEntered();

	player->sendCreatureAppear(player);
	player->setOperatingSystem(operatingSystem);
	player->setClientVersion(version);

	player->lastIP = player->getIP();
	player->lastLoad = OTSYS_TIME();

	g_chat.reOpenChannels(player);
	m_acceptPackets = true;
	return true;
}

void ProtocolGame::disconnect()
{
	if(Connection_ptr connection = getConnection())
		connection->close();
}

void ProtocolGame::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
	if(!output)
		return;

	TRACK_MESSAGE(output);
	output->put<char>(error);
	output->putString(message);

	OutputMessagePool::getInstance()->send(output);
	disconnect();
}

void ProtocolGame::onConnect()
{
	OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
	if(!output)
		return;

	TRACK_MESSAGE(output);
	enableChecksum();

	output->put<char>(0x1F);
	output->put<uint16_t>(random_range(0, 0xFFFF));
	output->put<uint16_t>(0x00);
	output->put<char>(random_range(0, 0xFF));

	OutputMessagePool::getInstance()->send(output);
}

void ProtocolGame::onRecvFirstMessage(NetworkMessage& msg)
{
	if(
#if defined(WINDOWS) && !defined(_CONSOLE)
		!GUI::getInstance()->m_connections ||
#endif
		g_game.getGameState() == GAMESTATE_SHUTDOWN)
	{
		disconnect();
		return;
	}

	OperatingSystem_t operatingSystem = (OperatingSystem_t)msg.get<uint16_t>();
	uint16_t version = msg.get<uint16_t>();

	if(version >= 971)
		msg.skip(5);

	if(!RSA_decrypt(msg))
	{
		disconnect();
		return;
	}

	uint32_t key[4] = {msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>()};
	enableXTEAEncryption();

	setXTEAKey(key);
	if(operatingSystem >= CLIENTOS_OTCLIENT_LINUX)
		sendExtendedOpcode(0x00, std::string());

	bool gamemaster = (msg.get<char>() != (char)0);
	std::string name = msg.getString(), character = msg.getString(), password = msg.getString();

	msg.skip(6);
	if(!g_config.getBool(ConfigManager::MANUAL_ADVANCED_CONFIG))
	{
		if(version < g_config.getNumber(ConfigManager::VERSION_MIN) || version > g_config.getNumber(ConfigManager::VERSION_MAX))
		{
			disconnectClient(0x14, g_config.getString(ConfigManager::VERSION_MSG).c_str());
			return;
		}
	}
	else
	{
		if(version < CLIENT_VERSION_MIN || version > CLIENT_VERSION_MAX)
		{
			disconnectClient(0x14, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");
			return;
		}
	}

	if(name.empty())
	{
		if(!g_config.getBool(ConfigManager::ACCOUNT_MANAGER))
		{
			disconnectClient(0x14, "Invalid account name.");
			return;
		}

		name = "1";
		password = "1";
	}

	if(g_game.getGameState() < GAMESTATE_NORMAL)
	{
		disconnectClient(0x14, "Gameworld is just starting up, please wait.");
		return;
	}

	if(g_game.getGameState() == GAMESTATE_MAINTAIN)
	{
		disconnectClient(0x14, "Gameworld is under maintenance, please re-connect in a while.");
		return;
	}

	if(ConnectionManager::getInstance()->isDisabled(getIP(), protocolId))
	{
		disconnectClient(0x14, "Too many connections attempts from your IP address, please try again later.");
		return;
	}

	if(IOBan::getInstance()->isIpBanished(getIP()))
	{
		disconnectClient(0x14, "Your IP is banished!");
		return;
	}

	uint32_t id = 1;
	if(!IOLoginData::getInstance()->getAccountId(name, id))
	{
		ConnectionManager::getInstance()->addAttempt(getIP(), protocolId, false);
		disconnectClient(0x14, "Invalid account name.");
		return;
	}

	std::string hash, salt;
	if(!IOLoginData::getInstance()->getPassword(id, hash, salt, character) || !encryptTest(salt + password, hash))
	{
		ConnectionManager::getInstance()->addAttempt(getIP(), protocolId, false);
		disconnectClient(0x14, "Invalid password.");
		return;
	}

	Ban ban;
	ban.value = id;

	ban.type = BAN_ACCOUNT;
	if(IOBan::getInstance()->getData(ban) && !IOLoginData::getInstance()->hasFlag(id, PlayerFlag_CannotBeBanned))
	{
		bool deletion = ban.expires < 0;
		std::string name_ = "Automatic ";
		if(!ban.adminId)
			name_ += (deletion ? "deletion" : "banishment");
		else
			IOLoginData::getInstance()->getNameByGuid(ban.adminId, name_, true);

		std::stringstream stream;
		stream << "Your account has been " << (deletion ? "deleted" : "banished") << " at:\n" << formatDateEx(ban.added, "%d %b %Y").c_str() << " by: " << name_.c_str()
			   << ".\nThe comment given was:\n" << ban.comment.c_str() << ".\nYour " << (deletion ? "account won't be undeleted" : "banishment will be lifted at:\n")
			   << (deletion ? "" : formatDateEx(ban.expires).c_str()) << ".";

		disconnectClient(0x14, stream.str().c_str());
		return;
	}

	ConnectionManager::getInstance()->addAttempt(getIP(), protocolId, true);
	Dispatcher::getInstance().addTask(createTask(boost::bind(
		&ProtocolGame::login, this, character, id, password, operatingSystem, version, gamemaster)));
}

void ProtocolGame::parsePacket(NetworkMessage &msg)
{
	if(!m_acceptPackets || g_game.getGameState() == GAMESTATE_SHUTDOWN || !msg.size())
		return;

	uint8_t recvbyte = msg.get<char>();
	if(!player)
	{
		if(recvbyte == 0x0F)
			disconnect();

		return;
	}

	//a dead player can not performs actions
	if(player->isRemoved() || player->getHealth() <= 0)
	{
		if(recvbyte == 0x0F)
		{
			disconnect();
			return;
		}

		if(recvbyte != 0x14)
			return;
	}

	if(player->isAccountManager())
	{
		switch(recvbyte)
		{
			case 0x0F: break;
			case 0x14: parseLogout(msg); break;
			case 0x96: parseSay(msg); break;
			case 0x1D: parseReceivePingBack(msg); break;
			case 0x1E: parseReceivePing(msg); break;
			case 0xC9: parseUpdateTile(msg); break;
			case 0xE8: parseDebugAssert(msg); break;
			case 0xA1: parseCancelTarget(msg); break;

			default:
				parseCancelWalk(msg);
			break;
		}
	}
	else
	{
		switch(recvbyte)
		{
			case 0x0F: break;
			case 0x14: parseLogout(msg); break;
			case 0x1D: parseReceivePingBack(msg); break;
			case 0x1E: parseReceivePing(msg); break;
			case 0x32: parseExtendedOpcode(msg); break;
			case 0x64: parseAutoWalk(msg); break;
			case 0x65:
			case 0x66:
			case 0x67:
			case 0x68: parseMove(msg, (Direction)(recvbyte - 0x65)); break;
			case 0x69: addGameTask(&Game::playerStopAutoWalk, player->getID()); break;
			case 0x6A: parseMove(msg, NORTHEAST); break;
			case 0x6B: parseMove(msg, SOUTHEAST); break;
			case 0x6C: parseMove(msg, SOUTHWEST); break;
			case 0x6D: parseMove(msg, NORTHWEST); break;
			case 0x6F:
			case 0x70:
			case 0x71:
			case 0x72: parseTurn(msg, (Direction)(recvbyte - 0x6F)); break;
			case 0x78: parseThrow(msg); break;
			case 0x79: parseLookInShop(msg); break;
			case 0x7A: parsePlayerPurchase(msg); break;
			case 0x7B: parsePlayerSale(msg); break;
			case 0x7C: parseCloseShop(msg); break;
			case 0x7D: parseRequestTrade(msg); break;
			case 0x7E: parseLookInTrade(msg); break;
			case 0x7F: parseAcceptTrade(msg); break;
			case 0x80: parseCloseTrade(); break;
			case 0x82: parseUseItem(msg); break;
			case 0x83: parseUseItemEx(msg); break;
			case 0x84: parseBattleWindow(msg); break;
			case 0x85: parseRotateItem(msg); break;
			case 0x87: parseCloseContainer(msg); break;
			case 0x88: parseUpArrowContainer(msg); break;
			case 0x89: parseTextWindow(msg); break;
			case 0x8A: parseHouseWindow(msg); break;
			case 0x8C: parseLookAt(msg); break;
			case 0x8D: parseLookInBattleList(msg); break;
			case 0x96: parseSay(msg); break;
			case 0x97: parseGetChannels(msg); break;
			case 0x98: parseOpenChannel(msg); break;
			case 0x99: parseCloseChannel(msg); break;
			case 0x9A: parseOpenPrivate(msg); break;
			case 0x9E: parseCloseNpc(msg); break;
			case 0xA0: parseFightModes(msg); break;
			case 0xA1: parseAttack(msg); break;
			case 0xA2: parseFollow(msg); break;
			case 0xA3: parseInviteToParty(msg); break;
			case 0xA4: parseJoinParty(msg); break;
			case 0xA5: parseRevokePartyInvite(msg); break;
			case 0xA6: parsePassPartyLeadership(msg); break;
			case 0xA7: parseLeaveParty(msg); break;
			case 0xA8: parseSharePartyExperience(msg); break;
			case 0xAA: parseCreatePrivateChannel(msg); break;
			case 0xAB: parseChannelInvite(msg); break;
			case 0xAC: parseChannelExclude(msg); break;
			case 0xBE: parseCancelMove(msg); break;
			case 0xC9: parseUpdateTile(msg); break;
			case 0xCA: parseUpdateContainer(msg); break;
			case 0xD2:
				if((!player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges) || !g_config.getBool(
					ConfigManager::DISABLE_OUTFITS_PRIVILEGED)) && (g_config.getBool(ConfigManager::ALLOW_CHANGEOUTFIT)
					|| g_config.getBool(ConfigManager::ALLOW_CHANGECOLORS) || g_config.getBool(ConfigManager::ALLOW_CHANGEADDONS)))
					parseRequestOutfit(msg);
				break;
			case 0xD3:
				if((!player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges) || !g_config.getBool(ConfigManager::DISABLE_OUTFITS_PRIVILEGED))
					&& (g_config.getBool(ConfigManager::ALLOW_CHANGECOLORS) || g_config.getBool(ConfigManager::ALLOW_CHANGEOUTFIT)))
					parseSetOutfit(msg);
				break;
			case 0xD4:
				if(g_config.getBool(ConfigManager::ALLOW_MOUNTS))
					parseMountStatus(msg);
				break;
			case 0xDC: parseAddVip(msg); break;
			case 0xDD: parseRemoveVip(msg); break;
			case 0xE6: parseBugReport(msg); break;
			case 0xE7: parseThankYou(msg); break;
			case 0xE8: parseDebugAssert(msg); break;
			case 0xF0: parseQuests(msg); break;
			case 0xF1: parseQuestInfo(msg); break;
			case 0xF2: parseViolationReport(msg); break;
			case 0xF4: parseMarketLeave(); break;
			case 0xF5: parseMarketBrowse(msg); break;
			case 0xF6: parseMarketCreateOffer(msg); break;
			case 0xF7: parseMarketCancelOffer(msg); break;
			case 0xF8: parseMarketAcceptOffer(msg); break;
			case 0xF9: parseModalDialogAnswer(msg); break;
			case 0xDE: parseEditVip(msg); break;

			default:
			{
				std::stringstream s;
				s << "Sent unknown byte: 0x" << std::hex << (int16_t)recvbyte << std::dec;
				Logger::getInstance()->eFile("bots/" + player->getName() + ".log", s.str(), true);
				break;
			}
		}
	}
}

void ProtocolGame::GetTileDescription(const Tile* tile, NetworkMessage_ptr msg)
{
	msg->put<uint16_t>(0x00);
	int32_t count = 0;
	if(tile->ground)
	{
		msg->putItem(tile->ground);
		++count;
	}

	const TileItemVector* items = tile->getItemList();
	const CreatureVector* creatures = tile->getCreatures();

	ItemVector::const_iterator it;
	if(items)
	{
		for(it = items->getBeginTopItem(); (it != items->getEndTopItem() && count < 10); ++it, ++count)
			msg->putItem(*it);
	}

	if(creatures)
	{
		for(CreatureVector::const_reverse_iterator cit = creatures->rbegin(); (cit != creatures->rend() && count < 10); ++cit)
		{
			if(!player->canSeeCreature(*cit))
				continue;

			bool known;
			uint32_t removedKnown;
			checkCreatureAsKnown((*cit)->getID(), known, removedKnown);

			AddCreature(msg, (*cit), known, removedKnown);
			++count;
		}
	}

	if(items)
	{
		for(it = items->getBeginDownItem(); (it != items->getEndDownItem() && count < 10); ++it, ++count)
			msg->putItem(*it);
	}
}

void ProtocolGame::GetMapDescription(int32_t x, int32_t y, int32_t z,
	int32_t width, int32_t height, NetworkMessage_ptr msg)
{
	int32_t skip = -1, startz, endz, zstep = 0;
	if(z > 7)
	{
		startz = z - 2;
		endz = std::min((int32_t)MAP_MAX_LAYERS - 1, z + 2);
		zstep = 1;
	}
	else
	{
		startz = 7;
		endz = 0;
		zstep = -1;
	}

	for(int32_t nz = startz; nz != endz + zstep; nz += zstep)
		GetFloorDescription(msg, x, y, nz, width, height, z - nz, skip);

	if(skip >= 0)
	{
		msg->put<char>(skip);
		msg->put<char>(0xFF);
		//cc += skip;
	}
}

void ProtocolGame::GetFloorDescription(NetworkMessage_ptr msg, int32_t x, int32_t y, int32_t z,
		int32_t width, int32_t height, int32_t offset, int32_t& skip)
{
	Tile* tile = NULL;
	for(int32_t nx = 0; nx < width; ++nx)
	{
		for(int32_t ny = 0; ny < height; ++ny)
		{
			if((tile = g_game.getTile(Position(x + nx + offset, y + ny + offset, z))))
			{
				if(skip >= 0)
				{
					msg->put<char>(skip);
					msg->put<char>(0xFF);
				}

				skip = 0;
				GetTileDescription(tile, msg);
			}
			else if(++skip == 0xFF)
			{
				msg->put<char>(0xFF);
				msg->put<char>(0xFF);
				skip = -1;
			}
		}
	}
}

void ProtocolGame::checkCreatureAsKnown(uint32_t id, bool& known, uint32_t& removedKnown)
{
	// loop through the known creature list and check if the given creature is in
	for(std::list<uint32_t>::iterator it = knownCreatureList.begin(); it != knownCreatureList.end(); ++it)
	{
		if((*it) != id)
			continue;

		// know... make the creature even more known...
		knownCreatureList.erase(it);
		knownCreatureList.push_back(id);

		known = true;
		return;
	}

	// ok, he is unknown...
	known = false;
	// ... but not in future
	knownCreatureList.push_back(id);
	// too many known creatures?
	if(knownCreatureList.size() > 1300)
	{
		// lets try to remove one from the end of the list
		Creature* c = NULL;
		for(int16_t n = 0; n < 1300; ++n)
		{
			removedKnown = knownCreatureList.front();
			if(!(c = g_game.getCreatureByID(removedKnown)) || !canSee(c))
				break;

			// this creature we can't remove, still in sight, so back to the end
			knownCreatureList.pop_front();
			knownCreatureList.push_back(removedKnown);
		}

		// hopefully we found someone to remove :S, we got only 250 tries
		// if not... lets kick some players with debug errors :)
		knownCreatureList.pop_front();
	}
	else // we can cache without problems :)
		removedKnown = 0;
}

bool ProtocolGame::canSee(const Creature* c) const
{
	return !c->isRemoved() && player->canSeeCreature(c) && canSee(c->getPosition());
}

bool ProtocolGame::canSee(const Position& pos) const
{
	return canSee(pos.x, pos.y, pos.z);
}

bool ProtocolGame::canSee(uint16_t x, uint16_t y, uint16_t z) const
{
	const Position& myPos = player->getPosition();
	if(myPos.z <= 7)
	{
		//we are on ground level or above (7 -> 0), view is from 7 -> 0
		if(z > 7)
			return false;
	}
	else if(myPos.z >= 8 && std::abs(myPos.z - z) > 2) //we are underground (8 -> 15), view is +/- 2 from the floor we stand on
		return false;

	//negative offset means that the action taken place is on a lower floor than ourself
	int32_t offsetz = myPos.z - z;
	return ((x >= myPos.x - 8 + offsetz) && (x <= myPos.x + 9 + offsetz) &&
		(y >= myPos.y - 6 + offsetz) && (y <= myPos.y + 7 + offsetz));
}

//********************** Parse methods *******************************//
void ProtocolGame::parseLogout(NetworkMessage&)
{
	Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolGame::logout, this, true, false)));
}

void ProtocolGame::parseCancelWalk(NetworkMessage&)
{
	Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolGame::sendCancelWalk, this)));
}

void ProtocolGame::parseCancelTarget(NetworkMessage&)
{
	Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolGame::sendCancelTarget, this)));
}

void ProtocolGame::parseCreatePrivateChannel(NetworkMessage&)
{
	addGameTask(&Game::playerCreatePrivateChannel, player->getID());
}

void ProtocolGame::parseChannelInvite(NetworkMessage& msg)
{
	const std::string name = msg.getString();
	addGameTask(&Game::playerChannelInvite, player->getID(), name);
}

void ProtocolGame::parseChannelExclude(NetworkMessage& msg)
{
	const std::string name = msg.getString();
	addGameTask(&Game::playerChannelExclude, player->getID(), name);
}

void ProtocolGame::parseGetChannels(NetworkMessage&)
{
	addGameTask(&Game::playerRequestChannels, player->getID());
}

void ProtocolGame::parseOpenChannel(NetworkMessage& msg)
{
	uint16_t channelId = msg.get<uint16_t>();
	addGameTask(&Game::playerOpenChannel, player->getID(), channelId);
}

void ProtocolGame::parseCloseChannel(NetworkMessage& msg)
{
	uint16_t channelId = msg.get<uint16_t>();
	addGameTask(&Game::playerCloseChannel, player->getID(), channelId);
}

void ProtocolGame::parseOpenPrivate(NetworkMessage& msg)
{
	const std::string receiver = msg.getString();
	addGameTask(&Game::playerOpenPrivateChannel, player->getID(), receiver);
}

void ProtocolGame::parseCloseNpc(NetworkMessage&)
{
	addGameTask(&Game::playerCloseNpcChannel, player->getID());
}

void ProtocolGame::parseCancelMove(NetworkMessage&)
{
	addGameTask(&Game::playerCancelAttackAndFollow, player->getID());
}

void ProtocolGame::parseReceivePing(NetworkMessage&)
{
	addGameTask(&Game::playerReceivePing, player->getID());
}

void ProtocolGame::parseReceivePingBack(NetworkMessage&)
{
	addGameTask(&Game::playerReceivePingBack, player->getID());
}

void ProtocolGame::parseAutoWalk(NetworkMessage& msg)
{
	uint8_t dirCount = msg.get<char>();
	if(dirCount > 128) //client limit
	{
		for(uint8_t i = 0; i < dirCount; ++i)
			msg.get<char>();

		std::stringstream s;
		s << "Attempt to auto walk for " << (uint16_t)dirCount << " steps - client is limited to 128 steps.";
		Logger::getInstance()->eFile("bots/" + player->getName() + ".log", s.str(), true);
		return;
	}

	std::list<Direction> path;
	for(uint8_t i = 0; i < dirCount; ++i)
	{
		Direction dir = SOUTH;
		switch(msg.get<char>())
		{
			case 1:
				dir = EAST;
				break;
			case 2:
				dir = NORTHEAST;
				break;
			case 3:
				dir = NORTH;
				break;
			case 4:
				dir = NORTHWEST;
				break;
			case 5:
				dir = WEST;
				break;
			case 6:
				dir = SOUTHWEST;
				break;
			case 7:
				break;
			case 8:
				dir = SOUTHEAST;
				break;
			default:
				continue;
		}

		path.push_back(dir);
	}

	addGameTask(&Game::playerAutoWalk, player->getID(), path);
}

void ProtocolGame::parseMove(NetworkMessage&, Direction dir)
{
	addGameTask(&Game::playerMove, player->getID(), dir);
}

void ProtocolGame::parseTurn(NetworkMessage&, Direction dir)
{
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerTurn, player->getID(), dir);
}

void ProtocolGame::parseRequestOutfit(NetworkMessage&)
{
	addGameTask(&Game::playerRequestOutfit, player->getID());
}

void ProtocolGame::parseSetOutfit(NetworkMessage& msg)
{
	Outfit_t newOutfit = player->defaultOutfit;
	if(g_config.getBool(ConfigManager::ALLOW_CHANGEOUTFIT))
		newOutfit.lookType = msg.get<uint16_t>();
	else
		msg.skip(2);

	if(g_config.getBool(ConfigManager::ALLOW_CHANGECOLORS))
	{
		newOutfit.lookHead = msg.get<char>();
		newOutfit.lookBody = msg.get<char>();
		newOutfit.lookLegs = msg.get<char>();
		newOutfit.lookFeet = msg.get<char>();
	}
	else
		msg.skip(4);

	if(g_config.getBool(ConfigManager::ALLOW_CHANGEADDONS))
		newOutfit.lookAddons = msg.get<char>();
	else
		msg.skip(1);

	if(g_config.getBool(ConfigManager::ALLOW_MOUNTS))
		newOutfit.lookMount = msg.get<uint16_t>();
	else
		msg.skip(2);

	addGameTask(&Game::playerChangeOutfit, player->getID(), newOutfit);
}

void ProtocolGame::parseMountStatus(NetworkMessage& msg)
{
	bool status = msg.get<char>() != (char)0;
	addGameTask(&Game::playerChangeMountStatus, player->getID(), status);
}

void ProtocolGame::parseUseItem(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t stackpos = msg.get<char>();
	uint8_t index = msg.get<char>();
	bool isHotkey = (pos.x == 0xFFFF && !pos.y && !pos.z);
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseItem, player->getID(), pos, stackpos, index, spriteId, isHotkey);
}

void ProtocolGame::parseUseItemEx(NetworkMessage& msg)
{
	Position fromPos = msg.getPosition();
	uint16_t fromSpriteId = msg.get<uint16_t>();
	int16_t fromStackpos = msg.get<char>();
	Position toPos = msg.getPosition();
	uint16_t toSpriteId = msg.get<uint16_t>();
	int16_t toStackpos = msg.get<char>();
	bool isHotkey = (fromPos.x == 0xFFFF && !fromPos.y && !fromPos.z);
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseItemEx, player->getID(),
		fromPos, fromStackpos, fromSpriteId, toPos, toStackpos, toSpriteId, isHotkey);
}

void ProtocolGame::parseBattleWindow(NetworkMessage& msg)
{
	Position fromPos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t fromStackpos = msg.get<char>();
	uint32_t creatureId = msg.get<uint32_t>();
	bool isHotkey = (fromPos.x == 0xFFFF && !fromPos.y && !fromPos.z);
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseBattleWindow, player->getID(), fromPos, fromStackpos, creatureId, spriteId, isHotkey);
}

void ProtocolGame::parseCloseContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.get<char>();
	addGameTask(&Game::playerCloseContainer, player->getID(), cid);
}

void ProtocolGame::parseUpArrowContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.get<char>();
	addGameTask(&Game::playerMoveUpContainer, player->getID(), cid);
}

void ProtocolGame::parseUpdateTile(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	addGameTask(&Game::playerUpdateTile, player->getID(), pos);
}

void ProtocolGame::parseUpdateContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.get<char>();
	addGameTask(&Game::playerUpdateContainer, player->getID(), cid);
}

void ProtocolGame::parseThrow(NetworkMessage& msg)
{
	Position fromPos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t fromStackpos = msg.get<char>();
	Position toPos = msg.getPosition();
	uint8_t count = msg.get<char>();
	if(toPos != fromPos)
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerMoveThing,
			player->getID(), fromPos, spriteId, fromStackpos, toPos, count);
}

void ProtocolGame::parseLookAt(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t stackpos = msg.get<char>();
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookAt, player->getID(), pos, spriteId, stackpos);
}

void ProtocolGame::parseLookInBattleList(NetworkMessage& msg)
{
	uint32_t creatureId = msg.get<uint32_t>();
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookInBattleList, player->getID(), creatureId);
}

void ProtocolGame::parseSay(NetworkMessage& msg)
{
	std::string receiver;
	uint16_t channelId = 0;

	MessageClasses type = (MessageClasses)msg.get<char>();
	switch(type)
	{
		case MSG_PRIVATE_TO:
		case MSG_GAMEMASTER_PRIVATE_TO:
			receiver = msg.getString();
			break;

		case MSG_CHANNEL:
		case MSG_CHANNEL_HIGHLIGHT:
		case MSG_GAMEMASTER_CHANNEL:
			channelId = msg.get<uint16_t>();
			break;

		default:
			break;
	}

	const std::string text = msg.getString();
	if(text.length() > 255) //client limit
	{
		std::stringstream s;
		s << "Attempt to send message with size " << text.length() << " - client is limited to 255 characters.";
		Logger::getInstance()->eFile("bots/" + player->getName() + ".log", s.str(), true);
		return;
	}

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSay, player->getID(), channelId, type, receiver, text);
}

void ProtocolGame::parseFightModes(NetworkMessage& msg)
{
	uint8_t rawFightMode = msg.get<char>(); //1 - offensive, 2 - balanced, 3 - defensive
	uint8_t rawChaseMode = msg.get<char>(); // 0 - stand while fightning, 1 - chase opponent
	uint8_t rawSecureMode = msg.get<char>(); // 0 - can't attack unmarked, 1 - can attack unmarked
	// uint8_t rawPvpMode = msg.get<char>(); // pvp mode introduced in 10.0

	chaseMode_t chaseMode;
	if(rawChaseMode == 1)
		chaseMode = CHASEMODE_FOLLOW;
	else
		chaseMode = CHASEMODE_STANDSTILL;

	fightMode_t fightMode;
	if(rawFightMode == 1)
		fightMode = FIGHTMODE_ATTACK;
	else if(rawFightMode == 2)
		fightMode = FIGHTMODE_BALANCED;
	else
		fightMode = FIGHTMODE_DEFENSE;

	secureMode_t secureMode;
	if(rawSecureMode == 1)
		secureMode = SECUREMODE_ON;
	else
		secureMode = SECUREMODE_OFF;

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSetFightModes, player->getID(), fightMode, chaseMode, secureMode);
}

void ProtocolGame::parseAttack(NetworkMessage& msg)
{
	uint32_t creatureId = msg.get<uint32_t>();
	// msg.get<uint32_t>(); creatureId (same as above)
	addGameTask(&Game::playerSetAttackedCreature, player->getID(), creatureId);
}

void ProtocolGame::parseFollow(NetworkMessage& msg)
{
	uint32_t creatureId = msg.get<uint32_t>();
	// msg.get<uint32_t>(); creatureId (same as above)
	addGameTask(&Game::playerFollowCreature, player->getID(), creatureId);
}

void ProtocolGame::parseTextWindow(NetworkMessage& msg)
{
	uint32_t windowTextId = msg.get<uint32_t>();
	const std::string newText = msg.getString();
	addGameTask(&Game::playerWriteItem, player->getID(), windowTextId, newText);
}

void ProtocolGame::parseHouseWindow(NetworkMessage &msg)
{
	uint8_t doorId = msg.get<char>();
	uint32_t id = msg.get<uint32_t>();
	const std::string text = msg.getString();
	addGameTask(&Game::playerUpdateHouseWindow, player->getID(), doorId, id, text);
}

void ProtocolGame::parseLookInShop(NetworkMessage &msg)
{
	uint16_t id = msg.get<uint16_t>();
	uint8_t count = msg.get<char>();
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookInShop, player->getID(), id, count);
}

void ProtocolGame::parsePlayerPurchase(NetworkMessage &msg)
{
	uint16_t id = msg.get<uint16_t>();
	uint8_t count = msg.get<char>();
	uint8_t amount = msg.get<char>();
	bool ignoreCap = (msg.get<char>() != (char)0);
	bool inBackpacks = (msg.get<char>() != (char)0);
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerPurchaseItem, player->getID(), id, count, amount, ignoreCap, inBackpacks);
}

void ProtocolGame::parsePlayerSale(NetworkMessage &msg)
{
	uint16_t id = msg.get<uint16_t>();
	uint8_t count = msg.get<char>();
	uint8_t amount = msg.get<char>();
	bool ignoreEquipped = (msg.get<char>() != (char)0);
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSellItem, player->getID(), id, count, amount, ignoreEquipped);
}

void ProtocolGame::parseCloseShop(NetworkMessage&)
{
	addGameTask(&Game::playerCloseShop, player->getID());
}

void ProtocolGame::parseRequestTrade(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t stackpos = msg.get<char>();
	uint32_t playerId = msg.get<uint32_t>();
	addGameTask(&Game::playerRequestTrade, player->getID(), pos, stackpos, playerId, spriteId);
}

void ProtocolGame::parseAcceptTrade(NetworkMessage&)
{
	addGameTask(&Game::playerAcceptTrade, player->getID());
}

void ProtocolGame::parseLookInTrade(NetworkMessage& msg)
{
	bool counter = (msg.get<char>() != (char)0);
	int32_t index = msg.get<char>();
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookInTrade, player->getID(), counter, index);
}

void ProtocolGame::parseCloseTrade()
{
	addGameTask(&Game::playerCloseTrade, player->getID());
}

void ProtocolGame::parseAddVip(NetworkMessage& msg)
{
	const std::string name = msg.getString();
	if(name.size() > 30)
		return;

	addGameTask(&Game::playerRequestAddVip, player->getID(), name);
}

void ProtocolGame::parseRemoveVip(NetworkMessage& msg)
{
	uint32_t guid = msg.get<uint32_t>();
	addGameTask(&Game::playerRequestRemoveVip, player->getID(), guid);
}

void ProtocolGame::parseEditVip(NetworkMessage& msg)
{
	uint32_t guid = msg.get<uint32_t>();
	std::string description = msg.getString();
	uint32_t icon = msg.get<uint32_t>();
	bool notify = msg.get<bool>();

	if(description.size() > 128)
		return;

	if(icon > VIP_ICON_LAST)
		return;

	addGameTask(&Game::playerRequestEditVip, player->getID(), guid, description, icon, notify);
}

void ProtocolGame::parseRotateItem(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.get<uint16_t>();
	int16_t stackpos = msg.get<char>();
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerRotateItem, player->getID(), pos, stackpos, spriteId);
}

void ProtocolGame::parseDebugAssert(NetworkMessage& msg)
{
	if(m_debugAssertSent)
		return;

	std::stringstream s;
	s << "----- " << formatDate() << " - " << player->getName() << " (" << convertIPAddress(getIP())
		<< ") -----" << std::endl
		<< msg.getString() << std::endl
		<< msg.getString() << std::endl
		<< msg.getString() << std::endl
		<< msg.getString() << std::endl
		<< std::endl;

	m_debugAssertSent = true;
	Logger::getInstance()->iFile(LOGFILE_ASSERTIONS, s.str(), false);
}

void ProtocolGame::parseBugReport(NetworkMessage& msg)
{
	std::string comment = msg.getString();
	addGameTask(&Game::playerReportBug, player->getID(), comment);
}

void ProtocolGame::parseThankYou(NetworkMessage& msg)
{
	uint32_t statementId = msg.get<uint32_t>();
	addGameTask(&Game::playerThankYou, player->getID(), statementId);
}

void ProtocolGame::parseInviteToParty(NetworkMessage& msg)
{
	uint32_t targetId = msg.get<uint32_t>();
	addGameTask(&Game::playerInviteToParty, player->getID(), targetId);
}

void ProtocolGame::parseJoinParty(NetworkMessage& msg)
{
	uint32_t targetId = msg.get<uint32_t>();
	addGameTask(&Game::playerJoinParty, player->getID(), targetId);
}

void ProtocolGame::parseRevokePartyInvite(NetworkMessage& msg)
{
	uint32_t targetId = msg.get<uint32_t>();
	addGameTask(&Game::playerRevokePartyInvitation, player->getID(), targetId);
}

void ProtocolGame::parsePassPartyLeadership(NetworkMessage& msg)
{
	uint32_t targetId = msg.get<uint32_t>();
	addGameTask(&Game::playerPassPartyLeadership, player->getID(), targetId);
}

void ProtocolGame::parseLeaveParty(NetworkMessage&)
{
	addGameTask(&Game::playerLeaveParty, player->getID(), false);
}

void ProtocolGame::parseSharePartyExperience(NetworkMessage& msg)
{
	bool activate = (msg.get<char>() != (char)0);
	addGameTask(&Game::playerSharePartyExperience, player->getID(), activate);
}

void ProtocolGame::parseQuests(NetworkMessage&)
{
	addGameTask(&Game::playerQuests, player->getID());
}

void ProtocolGame::parseQuestInfo(NetworkMessage& msg)
{
	uint16_t questId = msg.get<uint16_t>();
	addGameTask(&Game::playerQuestInfo, player->getID(), questId);
}

void ProtocolGame::parseViolationReport(NetworkMessage& msg)
{
	ReportType_t type = (ReportType_t)msg.get<char>();
	uint8_t reason = msg.get<char>();

	std::string name = msg.getString(), comment = msg.getString(), translation = "";
	if(type != REPORT_BOT)
		translation = msg.getString();

	uint32_t statementId = 0;
	if(type == REPORT_STATEMENT)
		statementId = msg.get<uint32_t>();

	addGameTask(&Game::playerReportViolation, player->getID(), type, reason, name, comment, translation, statementId);
}

void ProtocolGame::parseMarketLeave()
{
	addGameTask(&Game::playerLeaveMarket, player->getID());
}

void ProtocolGame::parseMarketBrowse(NetworkMessage& msg)
{
	uint16_t browseId = msg.get<uint16_t>();
	if(browseId == MARKETREQUEST_OWN_OFFERS)
		addGameTask(&Game::playerBrowseMarketOwnOffers, player->getID());
	else if(browseId == MARKETREQUEST_OWN_HISTORY)
		addGameTask(&Game::playerBrowseMarketOwnHistory, player->getID());
	else
		addGameTask(&Game::playerBrowseMarket, player->getID(), browseId);
}

void ProtocolGame::parseMarketCreateOffer(NetworkMessage& msg)
{
	uint8_t type = msg.get<char>();
	uint16_t spriteId = msg.get<uint16_t>();
	uint16_t amount = msg.get<uint16_t>();
	uint32_t price = msg.get<uint32_t>();
	bool anonymous = (msg.get<char>() != 0);
	addGameTask(&Game::playerCreateMarketOffer, player->getID(), type, spriteId, amount, price, anonymous);
}

void ProtocolGame::parseMarketCancelOffer(NetworkMessage& msg)
{
	uint32_t timestamp = msg.get<uint32_t>();
	uint16_t counter = msg.get<uint16_t>();
	addGameTask(&Game::playerCancelMarketOffer, player->getID(), timestamp, counter);
}

void ProtocolGame::parseMarketAcceptOffer(NetworkMessage& msg)
{
	uint32_t timestamp = msg.get<uint32_t>();
	uint16_t counter = msg.get<uint16_t>();
	uint16_t amount = msg.get<uint16_t>();
	addGameTask(&Game::playerAcceptMarketOffer, player->getID(), timestamp, counter, amount);
}

//********************** Send methods *******************************//
void ProtocolGame::sendOpenPrivateChannel(const std::string& receiver)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xAD);
	msg->putString(receiver);
}

void ProtocolGame::sendChannelEvent(uint16_t channelId, const std::string& playerName, ChannelEvent_t channelEvent)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF3);
	msg->put<uint16_t>(channelId);
	msg->putString(playerName);
	msg->put<char>(channelEvent);
}

void ProtocolGame::sendCreatureOutfit(const Creature* creature, const Outfit_t& outfit)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x8E);
	msg->put<uint32_t>(creature->getID());
	AddCreatureOutfit(msg, creature, outfit);
}

void ProtocolGame::sendCreatureLight(const Creature* creature)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddCreatureLight(msg, creature);
}

void ProtocolGame::sendWorldLight(const LightInfo& lightInfo)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddWorldLight(msg, lightInfo);
}

void ProtocolGame::sendCreatureWalkthrough(const Creature* creature, bool walkthrough)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x92);
	msg->put<uint32_t>(creature->getID());
	msg->put<char>(!walkthrough);
}

void ProtocolGame::sendCreatureShield(const Creature* creature)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x91);
	msg->put<uint32_t>(creature->getID());
	msg->put<char>(player->getPartyShield(creature));
}

void ProtocolGame::sendCreatureType(uint32_t creatureId, uint8_t creatureType)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x95);
	msg->put<uint32_t>(creatureId);
	msg->put<char>(creatureType);
}

void ProtocolGame::sendCreatureSkull(const Creature* creature)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x90);
	msg->put<uint32_t>(creature->getID());
	msg->put<char>(player->getSkullType(creature));
}

void ProtocolGame::sendCreatureSquare(const Creature* creature, uint8_t color)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x86);
	msg->put<uint32_t>(creature->getID());
	msg->put<char>(color);
}

void ProtocolGame::sendTutorial(uint8_t tutorialId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xDC);
	msg->put<char>(tutorialId);
}

void ProtocolGame::sendAddMarker(const Position& pos, MapMarks_t markType, const std::string& desc)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xDD);
	msg->putPosition(pos);
	msg->put<char>(markType);
	msg->putString(desc);
}

void ProtocolGame::sendReLoginWindow(uint8_t pvpPercent)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x28);
	msg->put<char>(pvpPercent);
}

void ProtocolGame::sendStats()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddPlayerStats(msg);
}

void ProtocolGame::sendTextMessage(MessageClasses mClass, const std::string& message)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddTextMessage(msg, mClass, message);
}

void ProtocolGame::sendClosePrivate(uint16_t channelId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	if(channelId == CHANNEL_GUILD || channelId == CHANNEL_PARTY)
		g_chat.removeUserFromChannel(player, channelId);

	msg->put<char>(0xB3);
	msg->put<uint16_t>(channelId);
}

void ProtocolGame::sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xB2);
	msg->put<uint16_t>(channelId);
	msg->putString(channelName);

	msg->put<uint16_t>(0x01);
	msg->putString(player->getName());
	msg->put<uint16_t>(0x00);
}

void ProtocolGame::sendChannelsDialog(const ChannelsList& channels)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xAB);

	msg->put<char>(channels.size());
	for(ChannelsList::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		msg->put<uint16_t>(it->first);
		msg->putString(it->second);
	}
}

void ProtocolGame::sendChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xAC);

	msg->put<uint16_t>(channelId);
	msg->putString(channelName);
	if(channelId == CHANNEL_PARTY || channelId == CHANNEL_GUILD || channelId == CHANNEL_PRIVATE)
	{
		if(ChatChannel* channel = g_chat.getChannelById(channelId))
		{
			const UsersMap& users = channel->getUsers();
			msg->put<uint16_t>(users.size());
			for(UsersMap::const_iterator itt = users.begin(); itt != users.end(); ++itt)
				msg->putString(itt->second->getName());

			if(PrivateChatChannel* privateChannel = dynamic_cast<PrivateChatChannel*>(channel))
			{
				const InviteList& invitedUsers = privateChannel->getInvitedUsers();
				msg->put<uint16_t>(invitedUsers.size());
				for(InviteList::const_iterator it = invitedUsers.begin(); it != invitedUsers.end(); ++it)
				{
					if(Player* _player = g_game.getPlayerByID(*it))
						msg->putString(_player->getName());
				}
			}
			else
				msg->put<uint16_t>(0x00);

				return;
		}
	}

	msg->put<uint16_t>(0x00);
	msg->put<uint16_t>(0x00);
}

void ProtocolGame::sendIcons(int32_t icons)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xA2);
	msg->put<uint16_t>(icons);
}

void ProtocolGame::sendContainer(uint32_t cid, const Container* container, bool hasParent)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x6E);
	msg->put<char>(cid);

	if(container->getID() == ITEM_BROWSEFIELD)
	{
		msg->putItem(1987, 1);
		msg->putString("Browse Field");
	}
	else
	{
		msg->putItem(container);
		msg->putString(container->getName());
	}

	msg->put<char>(container->capacity());

	msg->put<char>(hasParent ? 0x01 : 0x00);

	uint32_t maxItemsToSend;

	msg->put<char>(0x01); // Drag and drop
	msg->put<char>(0x00); // Pagination
	msg->put<uint16_t>(container->size());
	msg->put<uint16_t>(0x00);

	maxItemsToSend = container->capacity();

	msg->put<char>(std::min<uint32_t>(maxItemsToSend, container->size()));
	uint32_t i = 0;
	for(ItemList::const_iterator cit = container->getItems(), end = container->getEnd(); i < maxItemsToSend && cit != end; ++cit, ++i)
		msg->putItem(*cit);
}

void ProtocolGame::sendShop(Npc* npc, const ShopInfoList& shop)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x7A);
	msg->putString(npc->getName());
	msg->put<uint16_t>(std::min(shop.size(), (size_t)65535));

	ShopInfoList::const_iterator it = shop.begin();
	for(uint16_t i = 0; it != shop.end() && i < 65535; ++it, ++i)
		AddShopItem(msg, (*it));
}

void ProtocolGame::sendCloseShop()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x7C);
}

void ProtocolGame::sendGoods(const ShopInfoList& shop)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x7B);
	msg->put<uint64_t>((uint64_t)g_game.getMoney(player));

	std::map<uint32_t, uint32_t> goodsMap;
	if(shop.size() >= 5)
	{
		for(ShopInfoList::const_iterator sit = shop.begin(); sit != shop.end(); ++sit)
		{
			if(sit->sellPrice < 0)
				continue;

			int8_t subType = -1;
			if(sit->subType)
			{
				const ItemType& it = Item::items[sit->itemId];
				if(it.hasSubType() && !it.stackable)
					subType = sit->subType;
			}

			uint32_t count = player->__getItemTypeCount(sit->itemId, subType);
			if(count > 0)
				goodsMap[sit->itemId] = count;
		}
	}
	else
	{
		std::map<uint32_t, uint32_t> tmpMap;
		player->__getAllItemTypeCount(tmpMap);
		for(ShopInfoList::const_iterator sit = shop.begin(); sit != shop.end(); ++sit)
		{
			if(sit->sellPrice < 0)
				continue;

			int8_t subType = -1;
			const ItemType& it = Item::items[sit->itemId];
			if(sit->subType && it.hasSubType() && !it.stackable)
				subType = sit->subType;

			if(subType != -1)
			{
				uint32_t count = subType;
				if(!it.isFluidContainer() && !it.isSplash())
					count = player->__getItemTypeCount(sit->itemId, subType);

				if(count > 0)
					goodsMap[sit->itemId] = count;
				else
					goodsMap[sit->itemId] = 0;
			}
			else
				goodsMap[sit->itemId] = tmpMap[sit->itemId];
		}
	}

	msg->put<char>(std::min(goodsMap.size(), (size_t)255));
	std::map<uint32_t, uint32_t>::const_iterator it = goodsMap.begin();
	for(uint32_t i = 0; it != goodsMap.end() && i < 255; ++it, ++i)
	{
		msg->putItemId(it->first);
		msg->put<char>(std::min(it->second, (uint32_t)255));
	}
}

void ProtocolGame::sendMarketEnter(uint32_t depotId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF6);
	msg->put<uint64_t>(player->balance);
	msg->put<char>(std::min((int32_t)0xFF, IOMarket::getInstance()->getPlayerOfferCount(player->getGUID())));

	DepotChest* depotChest = player->getDepotChest(depotId, false);
	if(!depotChest)
	{
		msg->put<uint16_t>(0x00);
		return;
	}

	std::map<uint16_t, uint32_t> depotItems;
	std::list<Container*> containerList;

	containerList.push_back(depotChest);
	containerList.push_back(player->getInbox());
	do
	{
		Container* container = containerList.front();
		containerList.pop_front();
		for(ItemList::const_iterator it = container->getItems(), end = container->getEnd(); it != end; ++it)
		{
			Item* item = *it;

			Container* c = item->getContainer();
			if(c && !c->empty())
			{
				containerList.push_back(c);
				continue;
			}

			const ItemType& itemType = Item::items[item->getID()];
			if(itemType.wareId == 0)
				continue;

			if(!itemType.isRune() && item->getCharges() != itemType.charges)
				continue;

			if(item->getDuration() != (int32_t)itemType.decayTime)
				continue;

			depotItems[item->getID()] += Item::countByType(item, -1);
		}
	}
	while(!containerList.empty());
	player->setMarketDepotId(depotId);
	msg->put<uint16_t>(std::min((size_t)0xFFFF, depotItems.size()));

	uint16_t i = 0;
	for(std::map<uint16_t, uint32_t>::const_iterator it = depotItems.begin(), end = depotItems.end(); it != end && i < 65535; ++it, ++i)
	{
		msg->putItemId(it->first);
		msg->put<uint16_t>(std::min((uint32_t)0xFFFF, it->second));
	}
}

void ProtocolGame::sendMarketLeave()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF7);
}

void ProtocolGame::sendMarketBrowseItem(uint16_t itemId, const MarketOfferList& buyOffers, const MarketOfferList& sellOffers)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF9);
	msg->putItemId(itemId);

	msg->put<uint32_t>(buyOffers.size());
	for(MarketOfferList::const_iterator it = buyOffers.begin(), end = buyOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(it->counter);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
		msg->putString(it->playerName);
	}

	msg->put<uint32_t>(sellOffers.size());
	for(MarketOfferList::const_iterator it = sellOffers.begin(), end = sellOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(it->counter);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
		msg->putString(it->playerName);
	}
}

void ProtocolGame::sendMarketAcceptOffer(MarketOfferEx offer)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF9);
	msg->putItemId(offer.itemId);

	if(offer.type == MARKETACTION_BUY)
	{
		msg->put<uint32_t>(0x01);
		msg->put<uint32_t>(offer.timestamp);
		msg->put<uint16_t>(offer.counter);
		msg->put<uint16_t>(offer.amount);
		msg->put<uint32_t>(offer.price);
		msg->putString(offer.playerName);
		msg->put<uint32_t>(0x00);
	}
	else
	{
		msg->put<uint32_t>(0x00);
		msg->put<uint32_t>(0x01);
		msg->put<uint32_t>(offer.timestamp);
		msg->put<uint16_t>(offer.counter);
		msg->put<uint16_t>(offer.amount);
		msg->put<uint32_t>(offer.price);
		msg->putString(offer.playerName);
	}
}

void ProtocolGame::sendMarketBrowseOwnOffers(const MarketOfferList& buyOffers, const MarketOfferList& sellOffers)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF9);
	msg->put<uint16_t>(MARKETREQUEST_OWN_OFFERS);

	msg->put<uint32_t>(buyOffers.size());
	for(MarketOfferList::const_iterator it = buyOffers.begin(), end = buyOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(it->counter);
		msg->putItemId(it->itemId);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
	}

	msg->put<uint32_t>(sellOffers.size());
	for(MarketOfferList::const_iterator it = sellOffers.begin(), end = sellOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(it->counter);
		msg->putItemId(it->itemId);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
	}
}

void ProtocolGame::sendMarketCancelOffer(MarketOfferEx offer)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF9);
	msg->put<uint16_t>(MARKETREQUEST_OWN_OFFERS);

	if(offer.type == MARKETACTION_BUY)
	{
		msg->put<uint32_t>(0x01);
		msg->put<uint32_t>(offer.timestamp);
		msg->put<uint16_t>(offer.counter);
		msg->putItemId(offer.itemId);
		msg->put<uint16_t>(offer.amount);
		msg->put<uint32_t>(offer.price);
		msg->put<uint32_t>(0x00);
	}
	else
	{
		msg->put<uint32_t>(0x00);
		msg->put<uint32_t>(0x01);
		msg->put<uint32_t>(offer.timestamp);
		msg->put<uint16_t>(offer.counter);
		msg->putItemId(offer.itemId);
		msg->put<uint16_t>(offer.amount);
		msg->put<uint32_t>(offer.price);
	}
}

void ProtocolGame::sendMarketBrowseOwnHistory(const HistoryMarketOfferList& buyOffers, const HistoryMarketOfferList& sellOffers)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF9);
	msg->put<uint16_t>(MARKETREQUEST_OWN_HISTORY);

	std::map<uint32_t, uint16_t> counterMap;
	msg->put<uint32_t>(buyOffers.size());
	for(HistoryMarketOfferList::const_iterator it = buyOffers.begin(), end = buyOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(counterMap[it->timestamp]++);
		msg->putItemId(it->itemId);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
		msg->put<char>(it->state);
	}

	counterMap.clear();
	msg->put<uint32_t>(sellOffers.size());
	for(HistoryMarketOfferList::const_iterator it = sellOffers.begin(), end = sellOffers.end(); it != end; ++it)
	{
		msg->put<uint32_t>(it->timestamp);
		msg->put<uint16_t>(counterMap[it->timestamp]++);
		msg->putItemId(it->itemId);
		msg->put<uint16_t>(it->amount);
		msg->put<uint32_t>(it->price);
		msg->put<char>(it->state);
	}
}

void ProtocolGame::sendMarketDetail(uint16_t itemId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF8);
	msg->putItemId(itemId);

	const ItemType& it = Item::items[itemId];
	std::stringstream ss;
	if(it.armor != 0)
	{
		ss << it.armor;
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	if(it.attack != 0)
	{
		ss.str("");
		ss << it.attack;
		if(it.abilities && it.abilities->elementType != COMBAT_NONE && it.decayTo > 0)
			ss << " physical +" << it.abilities->elementDamage << " " << getCombatName(it.abilities->elementType);

		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	if(it.isContainer())
	{
		ss.str("");
		ss << it.maxItems;
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	if(it.defense != 0)
	{
		ss.str("");
		ss << it.defense;
		if(it.extraDefense != 0)
			ss << " " << std::showpos << it.extraDefense << std::noshowpos;

		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	if(!it.description.empty())
	{
		std::string descr = it.description;
		if(descr[descr.length() - 1] == '.')
			descr.erase(descr.length() - 1);

		msg->putString(descr);
	}
	else
		msg->put<uint16_t>(0x00);

	if(it.decayTime != 0)
	{
		ss.str("");
		ss << it.decayTime << " seconds";
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	ss.str("");
	if(it.hasAbilities())
	{
		bool separator = false;
		for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
		{
			if(!it.abilities->absorb[i])
				continue;

			if(separator)
				ss << ", ";
			else
				separator = true;

			ss << getCombatName((CombatType_t)i) << " " << std::showpos << it.abilities->absorb[i] << std::noshowpos << "%";
		}
	}

	msg->putString(ss.str());
	if(it.minReqLevel != 0)
	{
		ss.str("");
		ss << it.minReqLevel;
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	if(it.minReqMagicLevel != 0)
	{
		ss.str("");
		ss << it.minReqMagicLevel;
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	msg->putString(it.vocationString);
	msg->putString(it.runeSpellName);

	ss.str("");
	if(it.hasAbilities())
	{
		bool separator = false;
		for(uint16_t i = SKILL_FIRST; i <= SKILL_LAST; i++)
		{
			if(!it.abilities->skills[i])
				continue;

			if(separator)
				ss << ", ";
			else
				separator = true;

			ss << getSkillName(i) << " " << std::showpos << it.abilities->skills[i] << std::noshowpos;
		}

		if(it.abilities->stats[STAT_MAGICLEVEL] != 0)
		{
			if(separator)
				ss << ", ";
			else
				separator = true;

			ss << "magic level " << std::showpos << it.abilities->stats[STAT_MAGICLEVEL] << std::noshowpos;
		}

		if(it.abilities->speed != 0)
		{
			if(separator)
				ss << ", ";

			ss << "speed" << " " << std::showpos << (int32_t)(it.abilities->speed / 2) << std::noshowpos;
		}
	}

	msg->putString(ss.str());
	if(it.charges != 0)
	{
		ss.str("");
		ss << it.charges;
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	std::string weaponName = getWeaponName(it.weaponType);
	if(it.slotPosition & SLOTP_TWO_HAND)
	{
		if(!weaponName.empty())
			weaponName += ", two-handed";
		else
			weaponName = "two-handed";
	}

	msg->putString(weaponName);
	if(it.weight > 0)
	{
		ss.str("");
		ss << std::fixed << std::setprecision(2) << it.weight << " oz";
		msg->putString(ss.str());
	}
	else
		msg->put<uint16_t>(0x00);

	MarketStatistics* statistics = IOMarket::getInstance()->getPurchaseStatistics(itemId);
	if(statistics)
	{
		msg->put<char>(0x01);
		msg->put<uint32_t>(statistics->numTransactions);
		msg->put<uint32_t>(std::min((uint64_t)0xFFFFFFFF, statistics->totalPrice));
		msg->put<uint32_t>(statistics->highestPrice);
		msg->put<uint32_t>(statistics->lowestPrice);
	}
	else
		msg->put<char>(0x00);

	if((statistics = IOMarket::getInstance()->getSaleStatistics(itemId)))
	{
		msg->put<char>(0x01);
		msg->put<uint32_t>(statistics->numTransactions);
		msg->put<uint32_t>(std::min((uint64_t)0xFFFFFFFF, statistics->totalPrice));
		msg->put<uint32_t>(statistics->highestPrice);
		msg->put<uint32_t>(statistics->lowestPrice);
	}
	else
		msg->put<char>(0x00);
}

void ProtocolGame::sendTradeItemRequest(const Player* _player, const Item* item, bool ack)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	if(ack)
		msg->put<char>(0x7D);
	else
		msg->put<char>(0x7E);

	msg->putString(_player->getName());
	if(const Container* container = item->getContainer())
	{
		msg->put<char>(container->getItemHoldingCount() + 1);
		msg->putItem(item);
		for(ContainerIterator it = container->begin(); it != container->end(); ++it)
			msg->putItem(*it);
	}
	else
	{
		msg->put<char>(1);
		msg->putItem(item);
	}
}

void ProtocolGame::sendCloseTrade()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x7F);
}

void ProtocolGame::sendCloseContainer(uint32_t cid)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x6F);
	msg->put<char>(cid);
}

void ProtocolGame::sendCreatureTurn(const Creature* creature, int16_t stackpos)
{
	if(stackpos >= 10 || !canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x6B);
	msg->putPosition(creature->getPosition());
	msg->put<char>(stackpos);
	msg->put<uint16_t>(0x63);
	msg->put<uint32_t>(creature->getID());
	msg->put<char>(creature->getDirection());
	msg->put<char>(player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges) ? 0x00 : 0x01);
}

void ProtocolGame::sendCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos, uint32_t statementId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddCreatureSpeak(msg, creature, type, text, 0, pos, statementId);
}

void ProtocolGame::sendCreatureChannelSay(const Creature* creature, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddCreatureSpeak(msg, creature, type, text, channelId, NULL, statementId);
}

void ProtocolGame::sendStatsMessage(MessageClasses type, const std::string& message,
	Position pos, MessageDetails* details/* = NULL*/)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddTextMessage(msg, type, message, &pos, details);
}

void ProtocolGame::sendBasicData()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x9F);

	msg->put<char>(player->isPremium());
	msg->put<char>(player->getVocation()->getClientId());

	msg->put<uint16_t>(0x00); // known spells
	//each spell: msg->put<char>(spellId);
}

void ProtocolGame::sendCancel(const std::string& message)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddTextMessage(msg, MSG_STATUS_SMALL, message);
}

void ProtocolGame::sendCancelTarget()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xA3);
	msg->put<uint32_t>(0);
}

void ProtocolGame::sendChangeSpeed(const Creature* creature, uint32_t speed)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x8F);
	msg->put<uint32_t>(creature->getID());
	msg->put<uint16_t>(speed / 2);
}

void ProtocolGame::sendCancelWalk()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xB5);
	msg->put<char>(player->getDirection());
}

void ProtocolGame::sendSkills()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddPlayerSkills(msg);
}

void ProtocolGame::sendPing()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x1D);
}

void ProtocolGame::sendPingBack()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x1E);
}

void ProtocolGame::sendDistanceShoot(const Position& from, const Position& to, uint8_t type)
{
	if(type > SHOOT_EFFECT_LAST || (!canSee(from) && !canSee(to)))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddDistanceShoot(msg, from, to, type);
}

void ProtocolGame::sendMagicEffect(const Position& pos, uint8_t type)
{
	if(type > MAGIC_EFFECT_LAST || !canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddMagicEffect(msg, pos, type);
}

void ProtocolGame::sendCreatureHealth(const Creature* creature)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddCreatureHealth(msg, creature);
}

void ProtocolGame::sendFYIBox(const std::string& message)
{
	if(message.empty() || message.length() > 1018) //Prevent client debug when message is empty or length is > 1018 (not confirmed)
	{
		std::clog << "[Warning - ProtocolGame::sendFYIBox] Trying to send an empty or too huge message." << std::endl;
		return;
	}

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x15);
	msg->putString(message);
}

//tile
void ProtocolGame::sendAddTileItem(const Tile*, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(!canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddTileItem(msg, pos, stackpos, item);
}

void ProtocolGame::sendUpdateTileItem(const Tile*, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(!canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	UpdateTileItem(msg, pos, stackpos, item);
}

void ProtocolGame::sendRemoveTileItem(const Tile*, const Position& pos, uint32_t stackpos)
{
	if(!canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	RemoveTileItem(msg, pos, stackpos);
}

void ProtocolGame::sendUpdateTile(const Tile* tile, const Position& pos)
{
	if(!canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x69);
	msg->putPosition(pos);
	if(tile)
	{
		GetTileDescription(tile, msg);
		msg->put<char>(0x00);
		msg->put<char>(0xFF);
	}
	else
	{
		msg->put<char>(0x01);
		msg->put<char>(0xFF);
	}
}

void ProtocolGame::sendPendingStateEntered()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x0A);
}

void ProtocolGame::sendEnterWorld()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x0F);
}

void ProtocolGame::sendAddCreature(const Creature* creature, const Position& pos, uint32_t stackpos)
{
	if(!canSee(creature))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	if(creature != player)
	{
		AddTileCreature(msg, pos, stackpos, creature);
		return;
	}

	msg->put<char>(0x17);
	msg->put<uint32_t>(player->getID());
	msg->put<uint16_t>(0x32);

	msg->putDouble(Creature::speedA, 3);
	msg->putDouble(Creature::speedB, 3);
	msg->putDouble(Creature::speedC, 3);

	msg->put<char>(player->hasFlag(PlayerFlag_CanReportBugs));

	sendEnterWorld();

	AddMapDescription(msg, pos);
	for(int32_t i = SLOT_FIRST; i < SLOT_LAST; ++i)
		AddInventoryItem(msg, (slots_t)i, player->getInventoryItem((slots_t)i));

	AddPlayerStats(msg);
	AddPlayerSkills(msg);

	LightInfo lightInfo;
	g_game.getWorldLightInfo(lightInfo);

	AddWorldLight(msg, lightInfo);
	AddCreatureLight(msg, creature);

	player->sendIcons();
	for(VIPMap::iterator it = player->VIPList.begin(); it != player->VIPList.end(); ++it)
	{
		std::string vipName;
		if(IOLoginData::getInstance()->getNameByGuid((*it).first, vipName))
		{
			VipStatus_t vipStatus;
			if(Player* tmpPlayer = g_game.getPlayerByName(vipName))
				vipStatus = player->canSeeCreature(tmpPlayer) ? VIPSTATUS_ONLINE : VIPSTATUS_OFFLINE;
			else
				vipStatus = VIPSTATUS_OFFLINE;

			sendVIP((*it).first, vipName, (*it).second.description, (*it).second.icon, (*it).second.notify, vipStatus);
		}
	}
}

void ProtocolGame::sendRemoveCreature(const Creature*, const Position& pos, uint32_t stackpos)
{
	if(!canSee(pos))
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	RemoveTileItem(msg, pos, stackpos);
}

void ProtocolGame::sendMoveCreature(const Creature* creature, const Tile*, const Position& newPos,
	uint32_t newStackpos, const Tile*, const Position& oldPos, uint32_t oldStackpos, bool teleport)
{
	if(creature == player)
	{
		NetworkMessage_ptr msg = getOutputBuffer();
		if(!msg)
			return;

		TRACK_MESSAGE(msg);
		if(teleport || oldStackpos >= 10)
		{
			RemoveTileItem(msg, oldPos, oldStackpos);
			AddMapDescription(msg, newPos);
		}
		else
		{
			if(oldPos.z != 7 || newPos.z < 8)
			{
				msg->put<char>(0x6D);
				msg->putPosition(oldPos);
				msg->put<char>(oldStackpos);
				msg->putPosition(newPos);
			}
			else
				RemoveTileItem(msg, oldPos, oldStackpos);

			if(newPos.z > oldPos.z)
				MoveDownCreature(msg, creature, newPos, oldPos, oldStackpos);
			else if(newPos.z < oldPos.z)
				MoveUpCreature(msg, creature, newPos, oldPos, oldStackpos);

			if(oldPos.y > newPos.y) // north, for old x
			{
				msg->put<char>(0x65);
				GetMapDescription(oldPos.x - 8, newPos.y - 6, newPos.z, 18, 1, msg);
			}
			else if(oldPos.y < newPos.y) // south, for old x
			{
				msg->put<char>(0x67);
				GetMapDescription(oldPos.x - 8, newPos.y + 7, newPos.z, 18, 1, msg);
			}

			if(oldPos.x < newPos.x) // east, [with new y]
			{
				msg->put<char>(0x66);
				GetMapDescription(newPos.x + 9, newPos.y - 6, newPos.z, 1, 14, msg);
			}
			else if(oldPos.x > newPos.x) // west, [with new y]
			{
				msg->put<char>(0x68);
				GetMapDescription(newPos.x - 8, newPos.y - 6, newPos.z, 1, 14, msg);
			}
		}
	}
	else if(canSee(oldPos) && canSee(newPos))
	{
		if(!player->canSeeCreature(creature))
			return;

		NetworkMessage_ptr msg = getOutputBuffer();
		if(!msg)
			return;

		TRACK_MESSAGE(msg);
		if(!teleport && (oldPos.z != 7 || newPos.z < 8) && oldStackpos < 10)
		{
			msg->put<char>(0x6D);
			msg->putPosition(oldPos);
			msg->put<char>(oldStackpos);
			msg->putPosition(newPos);
		}
		else
		{
			RemoveTileItem(msg, oldPos, oldStackpos);
			AddTileCreature(msg, newPos, newStackpos, creature);
		}
	}
	else if(canSee(oldPos))
	{
		if(!player->canSeeCreature(creature))
			return;

		NetworkMessage_ptr msg = getOutputBuffer();
		if(!msg)
			return;

			TRACK_MESSAGE(msg);
			RemoveTileItem(msg, oldPos, oldStackpos);
	}
	else if(canSee(newPos) && player->canSeeCreature(creature))
	{
		NetworkMessage_ptr msg = getOutputBuffer();
		if(!msg)
			return;

			TRACK_MESSAGE(msg);
			AddTileCreature(msg, newPos, newStackpos, creature);
	}
}

//inventory
void ProtocolGame::sendAddInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddInventoryItem(msg, slot, item);
}

void ProtocolGame::sendUpdateInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	UpdateInventoryItem(msg, slot, item);
}

void ProtocolGame::sendRemoveInventoryItem(slots_t slot)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	RemoveInventoryItem(msg, slot);
}

//containers
void ProtocolGame::sendAddContainerItem(uint8_t cid, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	AddContainerItem(msg, cid, item);
}

void ProtocolGame::sendUpdateContainerItem(uint8_t cid, uint8_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	UpdateContainerItem(msg, cid, slot, item);
}

void ProtocolGame::sendRemoveContainerItem(uint8_t cid, uint8_t slot, const Item* lastItem)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	RemoveContainerItem(msg, cid, slot, lastItem);
}

void ProtocolGame::sendTextWindow(uint32_t windowTextId, Item* item, uint16_t maxLen, bool canWrite)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x96);
	msg->put<uint32_t>(windowTextId);
	msg->putItem(item);
	if(canWrite)
	{
		msg->put<uint16_t>(maxLen);
		msg->putString(item->getText());
	}
	else
	{
		msg->put<uint16_t>(item->getText().size());
		msg->putString(item->getText());
	}

	const std::string& writer = item->getWriter();
	if(writer.size())
		msg->putString(writer);
	else
		msg->putString("");

	time_t writtenDate = item->getDate();
	if(writtenDate > 0)
		msg->putString(formatDate(writtenDate));
	else
		msg->putString("");
}

void ProtocolGame::sendHouseWindow(uint32_t windowTextId, House*,
	uint32_t, const std::string& text)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x97);
	msg->put<char>(0x00);
	msg->put<uint32_t>(windowTextId);
	msg->putString(text);
}

void ProtocolGame::sendOutfitWindow()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xC8);
	AddCreatureOutfit(msg, player, player->getDefaultOutfit(), true);

	std::list<Outfit> outfitList;
	for(OutfitMap::iterator it = player->outfits.begin(); it != player->outfits.end(); ++it)
	{
		if(player->canWearOutfit(it->first, it->second.addons))
			outfitList.push_back(it->second);
	}

	if(outfitList.size())
	{
		msg->put<char>(outfitList.size());
		for(std::list<Outfit>::iterator it = outfitList.begin(); it != outfitList.end(); ++it)
		{
			msg->put<uint16_t>(it->lookType);
			msg->putString(it->name);
			if(player->hasCustomFlag(PlayerCustomFlag_CanWearAllAddons))
				msg->put<char>(0x03);
			else if(!g_config.getBool(ConfigManager::ADDONS_PREMIUM) || player->isPremium())
				msg->put<char>(it->addons);
			else
				msg->put<char>(0x00);
		}
	}
	else
	{
		msg->put<char>(1);
		msg->put<uint16_t>(player->getDefaultOutfit().lookType);
		msg->putString("Your outfit");
		msg->put<char>(player->getDefaultOutfit().lookAddons);
	}

	if(g_config.getBool(ConfigManager::ALLOW_MOUNTS))
	{
		std::list<Mount*> mountList;
		for(MountList::const_iterator it = Mounts::getInstance()->getFirstMount();
			it != Mounts::getInstance()->getLastMount(); ++it)
		{
			if((*it)->isTamed(player))
				mountList.push_back((*it));
		}

		if(mountList.size())
		{
			msg->put<char>(mountList.size());
			for(std::list<Mount*>::iterator it = mountList.begin(); it != mountList.end(); ++it)
			{
				msg->put<uint16_t>((*it)->getClientId());
				msg->putString((*it)->getName());
			}
		}
		else
			msg->put<char>(0);
	}
	else
		msg->put<char>(0);

	player->hasRequestedOutfit(true);
}

void ProtocolGame::sendQuests()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF0);

	msg->put<uint16_t>(Quests::getInstance()->getQuestCount(player));
	for(QuestList::const_iterator it = Quests::getInstance()->getFirstQuest(); it != Quests::getInstance()->getLastQuest(); ++it)
	{
		if(!(*it)->isStarted(player))
			continue;

		msg->put<uint16_t>((*it)->getId());
		msg->putString((*it)->getName());
		msg->put<char>((*it)->isCompleted(player));
	}
}

void ProtocolGame::sendQuestInfo(Quest* quest)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xF1);
	msg->put<uint16_t>(quest->getId());

	msg->put<char>(quest->getMissionCount(player));
	for(MissionList::const_iterator it = quest->getFirstMission(); it != quest->getLastMission(); ++it)
	{
		if(!(*it)->isStarted(player))
			continue;

		msg->putString((*it)->getName(player));
		msg->putString((*it)->getDescription(player));
	}
}

void ProtocolGame::sendUpdatedVIPStatus(uint32_t guid, VipStatus_t newStatus)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xD3);
	msg->put<uint32_t>(guid);
	msg->put<char>(newStatus);
}

void ProtocolGame::sendVIP(uint32_t guid, const std::string& name, const std::string& desc, uint32_t& icon, bool notify, VipStatus_t status)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xD2);
	msg->put<uint32_t>(guid);
	msg->putString(name);
	msg->putString(desc);//desc
	msg->put<uint32_t>(icon);//icon
	msg->put<bool>(notify);//notify
	msg->put<char>(status);//online or offline
}

void ProtocolGame::sendSpellCooldown(Spells_t icon, uint32_t cooldown)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xA4);
	msg->put<char>(icon);
	msg->put<uint32_t>(cooldown);
}

void ProtocolGame::sendSpellGroupCooldown(SpellGroup_t groupId, uint32_t cooldown)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xA5);
	msg->put<char>(groupId);
	msg->put<uint32_t>(cooldown);
}

void ProtocolGame::reloadCreature(const Creature* creature)
{
	if(!canSee(creature))
		return;

	// we are cheating the client in here!
	uint32_t stackpos = creature->getTile()->getClientIndexOfThing(player, creature);
	if(stackpos >= 10)
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	if(std::find(knownCreatureList.begin(), knownCreatureList.end(),
		creature->getID()) != knownCreatureList.end())
	{
		RemoveTileItem(msg, creature->getPosition(), stackpos);
		msg->put<char>(0x6A);

		msg->putPosition(creature->getPosition());
		msg->put<char>(stackpos);
		AddCreature(msg, creature, false, creature->getID());
	}
	else
		AddTileCreature(msg, creature->getPosition(), stackpos, creature);
}

void ProtocolGame::AddMapDescription(NetworkMessage_ptr msg, const Position& pos)
{
	msg->put<char>(0x64);
	msg->putPosition(player->getPosition());
	GetMapDescription(pos.x - 8, pos.y - 6, pos.z, 18, 14, msg);
}

void ProtocolGame::AddTextMessage(NetworkMessage_ptr msg, MessageClasses mClass, const std::string& message,
	Position* pos/* = NULL*/, MessageDetails* details/* = NULL*/)
{
	msg->put<char>(0xB4);
	msg->put<char>(mClass);
	switch(mClass)
	{
		case MSG_DAMAGE_DEALT:
		case MSG_DAMAGE_RECEIVED:
		case MSG_DAMAGE_OTHERS:
		{
			if(pos)
				msg->putPosition(*pos);
			else
				msg->putPosition(player->getPosition());

			if(!details)
			{
				msg->put<uint32_t>(0x00);
				msg->put<char>(0x00);
				msg->put<uint32_t>(0x00);
				msg->put<char>(0x00);
				break;
			}

			msg->put<uint32_t>(details->value);
			msg->put<char>(details->color);
			if(details->sub)
			{
				msg->put<uint32_t>(details->sub->value);
				msg->put<char>(details->sub->color);
			}
			else
			{
				msg->put<uint32_t>(0x00);
				msg->put<char>(0x00);
			}

			break;
		}

		case MSG_EXPERIENCE:
		case MSG_EXPERIENCE_OTHERS:
		case MSG_HEALED:
		case MSG_HEALED_OTHERS:
		{
			if(pos)
				msg->putPosition(*pos);
			else
				msg->putPosition(player->getPosition());

			if(details)
			{
				msg->put<uint32_t>(details->value);
				msg->put<char>(details->color);
			}
			else
			{
				msg->put<uint32_t>(0x00);
				msg->put<char>(0x00);
			}

			break;
		}

		default:
			break;
	}

	msg->putString(message);
}

void ProtocolGame::AddMagicEffect(NetworkMessage_ptr msg, const Position& pos, uint8_t type)
{
	msg->put<char>(0x83);
	msg->putPosition(pos);
	msg->put<char>(type + 1);
}

void ProtocolGame::AddDistanceShoot(NetworkMessage_ptr msg, const Position& from, const Position& to,
	uint8_t type)
{
	msg->put<char>(0x85);
	msg->putPosition(from);
	msg->putPosition(to);
	msg->put<char>(type + 1);
}

void ProtocolGame::AddCreature(NetworkMessage_ptr msg, const Creature* creature, bool known, uint32_t remove)
{
	/*
		TODO: fix the charges bellow.
		1 - otherPlayer fix me OTX Server use skulls to creatures too.
		2 - Creature skull, creature shield and guild emblemm need to stay as "creature" instead of "otherPlayer".
		3 - After moving skulls shield and emblems only to players use otherPlayer to fix charges an avoid crashs.
	*/
	CreatureType_t creatureType = creature->getType();
	const Player* otherPlayer = creature->getPlayer();

	if(!known)
	{
		msg->put<uint16_t>(0x61);
		msg->put<uint32_t>(remove);
		msg->put<uint32_t>(creature->getID());
		msg->put<char>(creatureType);
		msg->putString(creature->getHideName() ? "" : creature->getName());
	}
	else
	{
		msg->put<uint16_t>(0x62);
		msg->put<uint32_t>(creature->getID());
	}

	if(!creature->getHideHealth())
		msg->put<char>((uint8_t)std::ceil(creature->getHealth() * 100. / std::max(creature->getMaxHealth(), 1)));
	else
		msg->put<char>(0x00);

	msg->put<char>((uint8_t)creature->getDirection());
	AddCreatureOutfit(msg, creature, creature->getCurrentOutfit());

	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);

	msg->put<char>(lightInfo.level);
	msg->put<char>(lightInfo.color);

	msg->put<uint16_t>(creature->getStepSpeed() / 2);

	msg->put<char>(player->getSkullType(creature));
	msg->put<char>(player->getPartyShield(creature));

	if(!known)
		msg->put<char>(player->getGuildEmblem(creature));

	if(creatureType == CREATURETYPE_MONSTER)
	{
		const Creature* master = creature->getMaster();
		if(master)
		{
			const Player* masterPlayer = master->getPlayer();
			if(masterPlayer)
			{
				if(masterPlayer == player)
					creatureType = CREATURETYPE_SUMMON_OWN;
				else
					creatureType = CREATURETYPE_SUMMON_OTHERS;
			}
		}
	}

	msg->put<char>(creatureType); // Type (for summons)
	msg->put<char>(0xFF); // MARK_UNMARKED

	if(otherPlayer)
		msg->put<uint16_t>(0x00); // Helpers
	else
		msg->put<uint16_t>(0x00);

	msg->put<char>(!player->canWalkthrough(creature));
}

void ProtocolGame::AddPlayerStats(NetworkMessage_ptr msg)
{
	msg->put<char>(0xA0);

	msg->put<uint16_t>(player->getHealth());
	msg->put<uint16_t>(player->getPlayerInfo(PLAYERINFO_MAXHEALTH));

	msg->put<uint32_t>(uint32_t(player->getFreeCapacity() * 100));
	msg->put<uint32_t>(uint32_t(player->getCapacity() * 100));

	msg->put<uint64_t>(player->getExperience());

	msg->put<uint16_t>(player->getPlayerInfo(PLAYERINFO_LEVEL));
	msg->put<char>(player->getPlayerInfo(PLAYERINFO_LEVELPERCENT));

	msg->put<uint16_t>(player->getPlayerInfo(PLAYERINFO_MANA));
	msg->put<uint16_t>(player->getPlayerInfo(PLAYERINFO_MAXMANA));

	msg->put<char>(player->getMagicLevel());
	msg->put<char>(player->getBaseMagicLevel());
	msg->put<char>(player->getPlayerInfo(PLAYERINFO_MAGICLEVELPERCENT));

	msg->put<char>(player->getPlayerInfo(PLAYERINFO_SOUL));

	msg->put<uint16_t>(player->getStaminaMinutes());

	msg->put<uint16_t>(player->getSpeed() / 2);

	Condition* condition = player->getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT);
	msg->put<uint16_t>(condition ? condition->getTicks() / 1000 : 0x00);

	msg->put<uint16_t>(player->getOfflineTrainingTime() / 60 / 1000);
}

void ProtocolGame::AddPlayerSkills(NetworkMessage_ptr msg)
{
	msg->put<char>(0xA1);
	for(uint8_t i = 0; i <= SKILL_LAST; ++i)
	{
		msg->put<uint16_t>(player->getSkill((skills_t)i, SKILL_LEVEL));
		msg->put<uint16_t>(player->getBaseSkill((skills_t)i));
		msg->put<char>(player->getSkill((skills_t)i, SKILL_PERCENT));
	}
}

void ProtocolGame::AddCreatureSpeak(NetworkMessage_ptr msg, const Creature* creature, MessageClasses type,
	std::string text, uint16_t channelId, Position* pos, uint32_t statementId)
{
	msg->put<char>(0xAA);
	if(creature)
	{
		msg->put<uint32_t>(statementId);
		msg->putString(!creature->getHideName() ? creature->getName() : "");
		if(creature->getSpeakType() != MSG_NONE)
			type = creature->getSpeakType();

		const Player* speaker = creature->getPlayer();
		if(speaker && !speaker->isAccountManager() && !speaker->hasCustomFlag(PlayerCustomFlag_HideLevel))
			msg->put<uint16_t>(speaker->getPlayerInfo(PLAYERINFO_LEVEL));
		else
			msg->put<uint16_t>(0x00);
	}
	else
	{
		msg->put<uint32_t>(0x00);
		msg->putString("");
		msg->put<uint16_t>(0x00);
	}

	msg->put<char>(type);
	switch(type)
	{
		case MSG_SPEAK_SAY:
		case MSG_SPEAK_WHISPER:
		case MSG_SPEAK_YELL:
		case MSG_SPEAK_MONSTER_SAY:
		case MSG_SPEAK_MONSTER_YELL:
		case MSG_SPEAK_SPELL:
		case MSG_NPC_FROM:
		{
			if(pos)
				msg->putPosition(*pos);
			else if(creature)
				msg->putPosition(creature->getPosition());
			else
				msg->putPosition(Position(0,0,7));

			break;
		}

		case MSG_CHANNEL:
		case MSG_CHANNEL_HIGHLIGHT:
		case MSG_GAMEMASTER_CHANNEL:
			msg->put<uint16_t>(channelId);
			break;

		default:
			break;
	}

	msg->putString(text);
}

void ProtocolGame::AddCreatureHealth(NetworkMessage_ptr msg,const Creature* creature)
{
	msg->put<char>(0x8C);
	msg->put<uint32_t>(creature->getID());
	if(!creature->getHideHealth())
		msg->put<char>((uint8_t)std::ceil(creature->getHealth() * 100. / std::max(creature->getMaxHealth(), (int32_t)1)));
	else
		msg->put<char>(0x00);
}

void ProtocolGame::AddCreatureOutfit(NetworkMessage_ptr msg, const Creature* creature, const Outfit_t& outfit, bool outfitWindow/* = false*/)
{
	if(outfitWindow || (!creature->isInvisible() && (!creature->isGhost()
		|| !g_config.getBool(ConfigManager::GHOST_INVISIBLE_EFFECT))))
	{
		msg->put<uint16_t>(outfit.lookType);
		if(outfit.lookType)
		{
			msg->put<char>(outfit.lookHead);
			msg->put<char>(outfit.lookBody);
			msg->put<char>(outfit.lookLegs);
			msg->put<char>(outfit.lookFeet);
			msg->put<char>(outfit.lookAddons);
		}
		else if(outfit.lookTypeEx)
			msg->putItemId(outfit.lookTypeEx);
		else
			msg->put<uint16_t>(outfit.lookTypeEx);

		const Player* _player = creature->getPlayer();
		if(!_player || _player->isMounted())
			msg->put<uint16_t>(outfit.lookMount);
		else
			msg->put<uint16_t>(0x00);
	}
	else
	{
		msg->put<uint32_t>(0x00);
		msg->put<uint16_t>(0x00);
	}
}

void ProtocolGame::AddWorldLight(NetworkMessage_ptr msg, const LightInfo& lightInfo)
{
	msg->put<char>(0x82);
	msg->put<char>(lightInfo.level);
	msg->put<char>(lightInfo.color);
}

void ProtocolGame::AddCreatureLight(NetworkMessage_ptr msg, const Creature* creature)
{
	msg->put<char>(0x8D);
	msg->put<uint32_t>(creature->getID());

	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);

	msg->put<char>(lightInfo.level);
	msg->put<char>(lightInfo.color);
}

//tile
void ProtocolGame::AddTileItem(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(stackpos >= 10)
		return;

	msg->put<char>(0x6A);
	msg->putPosition(pos);
	msg->put<char>(stackpos);
	msg->putItem(item);
}

void ProtocolGame::AddTileCreature(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos, const Creature* creature)
{
	if(stackpos >= 10)
		return;

	msg->put<char>(0x6A);
	msg->putPosition(pos);
	msg->put<char>(stackpos);

	bool known;
	uint32_t removedKnown;
	checkCreatureAsKnown(creature->getID(), known, removedKnown);
	AddCreature(msg, creature, known, removedKnown);
}

void ProtocolGame::UpdateTileItem(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(stackpos >= 10)
		return;

	msg->put<char>(0x6B);
	msg->putPosition(pos);
	msg->put<char>(stackpos);
	msg->putItem(item);
}

void ProtocolGame::RemoveTileItem(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos)
{
	if(stackpos >= 10)
		return;

	msg->put<char>(0x6C);
	msg->putPosition(pos);
	msg->put<char>(stackpos);
}

void ProtocolGame::MoveUpCreature(NetworkMessage_ptr msg, const Creature* creature,
	const Position& newPos, const Position& oldPos, uint32_t)
{
	if(creature != player)
		return;

	msg->put<char>(0xBE); //floor change up
	if(newPos.z == 7) //going to surface
	{
		int32_t skip = -1;
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 5, 18, 14, 3, skip); //(floor 7 and 6 already set)
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 4, 18, 14, 4, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 3, 18, 14, 5, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 2, 18, 14, 6, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 1, 18, 14, 7, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 0, 18, 14, 8, skip);
		if(skip >= 0)
		{
			msg->put<char>(skip);
			msg->put<char>(0xFF);
		}
	}
	else if(newPos.z > 7) //underground, going one floor up (still underground)
	{
		int32_t skip = -1;
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, oldPos.z - 3, 18, 14, 3, skip);
		if(skip >= 0)
		{
			msg->put<char>(skip);
			msg->put<char>(0xFF);
		}
	}

	//moving up a floor up makes us out of sync
	//west
	msg->put<char>(0x68);
	GetMapDescription(oldPos.x - 8, oldPos.y + 1 - 6, newPos.z, 1, 14, msg);

	//north
	msg->put<char>(0x65);
	GetMapDescription(oldPos.x - 8, oldPos.y - 6, newPos.z, 18, 1, msg);
}

void ProtocolGame::MoveDownCreature(NetworkMessage_ptr msg, const Creature* creature,
	const Position& newPos, const Position& oldPos, uint32_t)
{
	if(creature != player)
		return;

	msg->put<char>(0xBF); //floor change down
	if(newPos.z == 8) //going from surface to underground
	{
		int32_t skip = -1;
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, newPos.z, 18, 14, -1, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, newPos.z + 1, 18, 14, -2, skip);
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, newPos.z + 2, 18, 14, -3, skip);
		if(skip >= 0)
		{
			msg->put<char>(skip);
			msg->put<char>(0xFF);
		}
	}
	else if(newPos.z > oldPos.z && newPos.z > 8 && newPos.z < 14) //going further down
	{
		int32_t skip = -1;
		GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, newPos.z + 2, 18, 14, -3, skip);
		if(skip >= 0)
		{
			msg->put<char>(skip);
			msg->put<char>(0xFF);
		}
	}

	//moving down a floor makes us out of sync
	//east
	msg->put<char>(0x66);
	GetMapDescription(oldPos.x + 9, oldPos.y - 1 - 6, newPos.z, 1, 14, msg);

	//south
	msg->put<char>(0x67);
	GetMapDescription(oldPos.x - 8, oldPos.y + 7, newPos.z, 18, 1, msg);
}

//inventory
void ProtocolGame::AddInventoryItem(NetworkMessage_ptr msg, slots_t slot, const Item* item)
{
	if(item)
	{
		msg->put<char>(0x78);
		msg->put<char>(slot);
		msg->putItem(item);
	}
	else
		RemoveInventoryItem(msg, slot);
}

void ProtocolGame::RemoveInventoryItem(NetworkMessage_ptr msg, slots_t slot)
{
	msg->put<char>(0x79);
	msg->put<char>(slot);
}

void ProtocolGame::UpdateInventoryItem(NetworkMessage_ptr msg, slots_t slot, const Item* item)
{
	AddInventoryItem(msg, slot, item);
}

//containers
void ProtocolGame::AddContainerItem(NetworkMessage_ptr msg, uint8_t cid, const Item* item)
{
	msg->put<char>(0x70);
	msg->put<char>(cid);
	msg->put<uint16_t>(0x00); // slot
	msg->putItem(item);
}

void ProtocolGame::UpdateContainerItem(NetworkMessage_ptr msg, uint8_t cid, uint8_t slot, const Item* item)
{
	msg->put<char>(0x71);
	msg->put<char>(cid);
	msg->put<uint16_t>(slot);
	msg->putItem(item);
}

void ProtocolGame::RemoveContainerItem(NetworkMessage_ptr msg, uint8_t cid, uint8_t slot, const Item* lastItem)
{
	msg->put<char>(0x72);
	msg->put<char>(cid);
	msg->put<uint16_t>(slot);
	if(lastItem)
		msg->putItem(lastItem);
	else
		msg->put<uint16_t>(0x00);
}

void ProtocolGame::sendChannelMessage(std::string author, std::string text, MessageClasses type, uint16_t channel)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xAA);
	msg->put<uint32_t>(0x00);
	msg->putString(author);
	msg->put<uint16_t>(0x00);
	msg->put<char>(type);
	msg->put<uint16_t>(channel);
	msg->putString(text);
}

void ProtocolGame::AddShopItem(NetworkMessage_ptr msg, const ShopInfo& item)
{
	const ItemType& it = Item::items[item.itemId];
	msg->put<uint16_t>(it.clientId);
	if(it.isSplash() || it.isFluidContainer())
		msg->put<char>(serverFluidToClient(item.subType));
	else if(it.stackable || it.charges)
		msg->put<char>(item.subType);
	else
		msg->put<char>(0x00);

	msg->putString(item.itemName);
	msg->put<uint32_t>(uint32_t(it.weight * 100));
	msg->put<uint32_t>(item.buyPrice);
	msg->put<uint32_t>(item.sellPrice);
}

void ProtocolGame::sendModalDialog(ModalDialog& dialog)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0xFA);
	msg->put<uint32_t>(dialog.id);
	msg->putString(dialog.title);
	msg->putString(dialog.message);

	msg->put<uint8_t>(dialog.buttons.size());
	for(std::vector<ModalChoice>::iterator it = dialog.buttons.begin(); it != dialog.buttons.end(); it++)
	{
		msg->putString(it->value);
		msg->put<uint8_t>(it->id);
	}
	
	msg->put<uint8_t>(dialog.choices.size());
	for(std::vector<ModalChoice>::iterator it = dialog.choices.begin(); it != dialog.choices.end(); it++)
	{
		msg->putString(it->value);
		msg->put<uint8_t>(it->id);
	}

	msg->put<uint8_t>(dialog.buttonEnter);
	msg->put<uint8_t>(dialog.buttonEscape);
	msg->put<bool>(dialog.popup);
}

void ProtocolGame::parseModalDialogAnswer(NetworkMessage& msg)
{
	uint32_t dialog = msg.get<uint32_t>();
	uint8_t button = msg.get<uint8_t>();
	uint8_t choice = msg.get<uint8_t>();
	addGameTask(&Game::playerAnswerModalDialog, player->getID(), dialog, button, choice);
}

void ProtocolGame::parseExtendedOpcode(NetworkMessage& msg)
{
	uint8_t opcode = msg.get<char>();
	std::string buffer = msg.getString();

	// process additional opcodes via lua script event
	addGameTask(&Game::parsePlayerExtendedOpcode, player->getID(), opcode, buffer);
}

void ProtocolGame::sendExtendedOpcode(uint8_t opcode, const std::string& buffer)
{
	// extended opcodes can only be send to players using otclient, cipsoft's tibia can't understand them
	if(player && !player->isUsingOtclient())
		return;

	NetworkMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->put<char>(0x32);
	msg->put<char>(opcode);
	msg->putString(buffer);
}
