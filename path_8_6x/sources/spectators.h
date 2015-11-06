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

#ifndef __SPECTATORS__
#define __SPECTATORS__

#include "otsystem.h"
#include "enums.h"

#include "protocolgame.h"

class Creature;
class Player;
class House;
class Container;
class Tile;
class Quest;

typedef std::map<ProtocolGame*, std::pair<std::string, bool> > SpectatorList;
typedef std::map<std::string, uint32_t> DataList;

class Spectators
{
	public:
		Spectators(ProtocolGame* client): owner(client)
		{
			id = 0;
			broadcast = false;
			auth = false;
		}
		virtual ~Spectators() {}

		void clear(bool full)
		{
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->disconnect();

			spectators.clear();
			mutes.clear();

			id = 0;
			if(!full)
				return;

			bans.clear();
			password = "";
			broadcast = auth = false;
		}

		bool check(const std::string& _password);
		void handle(ProtocolGame* client, const std::string& text, uint16_t channelId);
		void chat(uint16_t channelId);

		StringVec list()
		{
			StringVec t;
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				t.push_back(it->second.first);

			return t;
		}

		void kick(StringVec list);
		StringVec muteList() {return mutes;}
		void mute(StringVec _mutes) {mutes = _mutes;}
		DataList banList() {return bans;}
		void ban(StringVec _bans);

		bool banned(uint32_t ip) const
		{
			for(DataList::const_iterator it = bans.begin(); it != bans.end(); ++it)
			{
				if(it->second == ip)
					return true;
			}

			return false;
		}

		ProtocolGame* getOwner() const {return owner;}
		void setOwner(ProtocolGame* client) {owner = client;}

		std::string getPassword() const {return password;}
		void setPassword(const std::string& value) {password = value;}

		bool isBroadcasting() const {return broadcast;}
		void setBroadcast(bool value) {broadcast = value;}

		bool isAuth() const {return auth;}
		void setAuth(bool value) {auth = value;}

		void addSpectator(ProtocolGame* client);
		void removeSpectator(ProtocolGame* client);

		// inherited
		uint32_t getIP() const
		{
			if(!owner)
				return 0;

			return owner->getIP();
		}

		bool logout(bool displayEffect, bool forceLogout)
		{
			if(!owner)
				return true;

			return owner->logout(displayEffect, forceLogout);
		}

		void disconnect()
		{
			if(owner)
				owner->disconnect();
		}

	private:
		friend class Player;

		SpectatorList spectators;
		StringVec mutes;
		DataList bans;

		ProtocolGame* owner;
		uint32_t id;
		std::string password;
		bool broadcast, auth;

		// inherited
		bool canSee(const Position& pos) const
		{
			if(!owner)
				return false;

			return owner->canSee(pos);
		}

		void sendChannelMessage(std::string author, std::string text, MessageClasses type, uint16_t channel);
		void sendClosePrivate(uint16_t channelId);
		void sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName);
		void sendChannelsDialog(const ChannelsList& channels)
		{
			if(!owner)
				return;

			owner->sendChannelsDialog(channels);
		}
		void sendChannel(uint16_t channelId, const std::string& channelName)
		{
			if(!owner)
				return;

			owner->sendChannel(channelId, channelName);
		}
		void sendOpenPrivateChannel(const std::string& receiver)
		{
			if(!owner)
				return;

			owner->sendOpenPrivateChannel(receiver);
		}
		void sendIcons(int32_t icons)
		{
			if(!owner)
				return;

			owner->sendIcons(icons);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendIcons(icons);
		}
		void sendFYIBox(const std::string& message)
		{
			if(!owner)
				return;

			owner->sendFYIBox(message);
		}

		void sendDistanceShoot(const Position& from, const Position& to, uint8_t type)
		{
			if(!owner)
				return;

			owner->sendDistanceShoot(from, to, type);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendDistanceShoot(from, to, type);
		}
		void sendMagicEffect(const Position& pos, uint8_t type)
		{
			if(!owner)
				return;

			owner->sendMagicEffect(pos, type);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendMagicEffect(pos, type);
		}
		void sendAnimatedText(const Position& pos, uint8_t color, const std::string& text)
		{
			if(!owner)
				return;

			owner->sendAnimatedText(pos, color, text);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendAnimatedText(pos, color, text);
		}
		void sendCreatureHealth(const Creature* creature)
		{
			if(!owner)
				return;

			owner->sendCreatureHealth(creature);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureHealth(creature);
		}
		void sendSkills()
		{
			if(!owner)
				return;

			owner->sendSkills();
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendSkills();
		}
		void sendPing()
		{
			if(!owner)
				return;

			owner->sendPing();
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendPing();
		}
		void sendCreatureTurn(const Creature* creature, int16_t stackpos)
		{
			if(!owner)
				return;

			owner->sendCreatureTurn(creature, stackpos);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureTurn(creature, stackpos);
		}
		void sendCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos, uint32_t statementId)
		{
			if(!owner)
				return;

			owner->sendCreatureSay(creature, type, text, pos, statementId);
			if(type == MSG_PRIVATE || type == MSG_GAMEMASTER_PRIVATE || type == MSG_NPC_TO) // care for privacy!
				return;

			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureSay(creature, type, text, pos, statementId);
		}
		void sendCreatureChannelSay(const Creature* creature, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId); // extended

		void sendCancel(const std::string& message)
		{
			if(!owner)
				return;

			owner->sendCancel(message);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCancel(message);
		}
		void sendCancelWalk()
		{
			if(!owner)
				return;

			owner->sendCancelWalk();
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCancelWalk();
		}
		void sendChangeSpeed(const Creature* creature, uint32_t speed)
		{
			if(!owner)
				return;

			owner->sendChangeSpeed(creature, speed);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendChangeSpeed(creature, speed);
		}
		void sendCancelTarget()
		{
			if(!owner)
				return;

			owner->sendCancelTarget();
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCancelTarget();
		}
		void sendCreatureOutfit(const Creature* creature, const Outfit_t& outfit)
		{
			if(!owner)
				return;

			owner->sendCreatureOutfit(creature, outfit);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureOutfit(creature, outfit);
		}
		void sendStats()
		{
			if(!owner)
				return;

			owner->sendStats();
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendStats();
		}
		void sendTextMessage(MessageClasses mclass, const std::string& message)
		{
			if(!owner)
				return;

			owner->sendTextMessage(mclass, message);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendTextMessage(mclass, message);
		}
		void sendStatsMessage(MessageClasses mclass, const std::string& message,
			Position pos, MessageDetails* details = NULL)
		{
			if(!owner)
				return;

			owner->sendStatsMessage(mclass, message, pos, details);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendStatsMessage(mclass, message, pos, details);
		}
		void sendReLoginWindow()
		{
			if(!owner)
				return;

			owner->sendReLoginWindow();
			clear(true); // just smoothly disconnect
		}

		void sendTutorial(uint8_t tutorialId)
		{
			if(!owner)
				return;

			owner->sendTutorial(tutorialId);
		}
		void sendAddMarker(const Position& pos, MapMarks_t markType, const std::string& desc)
		{
			if(!owner)
				return;

			owner->sendAddMarker(pos, markType, desc);
		}
		void sendExtendedOpcode(uint8_t opcode, const std::string& buffer)
		{
			if(!owner)
				return;

			owner->sendExtendedOpcode(opcode, buffer);
		}

		void sendRuleViolationsChannel(uint16_t channelId)
		{
			if(!owner)
				return;

			owner->sendRuleViolationsChannel(channelId);
		}
		void sendRemoveReport(const std::string& name)
		{
			if(!owner)
				return;

			owner->sendRemoveReport(name);
		}
		void sendLockRuleViolation()
		{
			if(!owner)
				return;

			owner->sendLockRuleViolation();
		}
		void sendRuleViolationCancel(const std::string& name)
		{
			if(!owner)
				return;

			owner->sendRuleViolationCancel(name);
		}

		void sendCreatureSkull(const Creature* creature)
		{
			if(!owner)
				return;

			owner->sendCreatureSkull(creature);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureSkull(creature);
		}
		void sendCreatureShield(const Creature* creature)
		{
			if(!owner)
				return;

			owner->sendCreatureShield(creature);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureShield(creature);
		}
		void sendCreatureEmblem(const Creature* creature)
		{
			if(!owner)
				return;

			owner->sendCreatureEmblem(creature);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureEmblem(creature);
		}
		void sendCreatureWalkthrough(const Creature* creature, bool walkthrough)
		{
			if(!owner)
				return;

			owner->sendCreatureWalkthrough(creature, walkthrough);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureWalkthrough(creature, walkthrough);
		}

		void sendShop(Npc* npc, const ShopInfoList& shop)
		{
			if(!owner)
				return;

			owner->sendShop(npc, shop);
		}
		void sendCloseShop()
		{
			if(!owner)
				return;

			owner->sendCloseShop();
		}
		void sendGoods(const ShopInfoList& shop)
		{
			if(!owner)
				return;

			owner->sendGoods(shop);
		}
		void sendTradeItemRequest(const Player* player, const Item* item, bool ack)
		{
			if(!owner)
				return;

			owner->sendTradeItemRequest(player, item, ack);
		}
		void sendCloseTrade()
		{
			if(!owner)
				return;

			owner->sendCloseTrade();
		}

		void sendTextWindow(uint32_t windowTextId, Item* item, uint16_t maxLen, bool canWrite)
		{
			if(!owner)
				return;

			owner->sendTextWindow(windowTextId, item, maxLen, canWrite);
		}
		void sendHouseWindow(uint32_t windowTextId, House* house, uint32_t listId, const std::string& text)
		{
			if(!owner)
				return;

			owner->sendHouseWindow(windowTextId, house, listId, text);
		}

		void sendOutfitWindow()
		{
			if(!owner)
				return;

			owner->sendOutfitWindow();
		}
		void sendQuests()
		{
			if(!owner)
				return;

			owner->sendQuests();
		}
		void sendQuestInfo(Quest* quest)
		{
			if(!owner)
				return;

			owner->sendQuestInfo(quest);
		}

		void sendVIPLogIn(uint32_t guid)
		{
			if(owner)
				owner->sendVIPLogIn(guid);
		}
		void sendVIPLogOut(uint32_t guid)
		{
			if(owner)
				owner->sendVIPLogOut(guid);
		}
		void sendVIP(uint32_t guid, const std::string& name, bool isOnline)
		{
			if(owner)
				owner->sendVIP(guid, name, isOnline);
		}

		void sendCreatureLight(const Creature* creature)
		{
			if(!owner)
				return;

			owner->sendCreatureLight(creature);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureLight(creature);
		}
		void sendWorldLight(const LightInfo& lightInfo)
		{
			if(!owner)
				return;

			owner->sendWorldLight(lightInfo);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendWorldLight(lightInfo);
		}

		void sendCreatureSquare(const Creature* creature, uint8_t color)
		{
			if(!owner)
				return;

			owner->sendCreatureSquare(creature, color);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCreatureSquare(creature, color);
		}

		void sendAddTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item* item)
		{
			if(!owner)
				return;

			owner->sendAddTileItem(tile, pos, stackpos, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendAddTileItem(tile, pos, stackpos, item);
		}
		void sendUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item* item)
		{
			if(!owner)
				return;

			owner->sendUpdateTileItem(tile, pos, stackpos, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendUpdateTileItem(tile, pos, stackpos, item);
		}
		void sendRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos)
		{
			if(!owner)
				return;

			owner->sendRemoveTileItem(tile, pos, stackpos);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendRemoveTileItem(tile, pos, stackpos);
		}
		void sendUpdateTile(const Tile* tile, const Position& pos)
		{
			if(!owner)
				return;

			owner->sendUpdateTile(tile, pos);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendUpdateTile(tile, pos);
		}

		void sendAddCreature(const Creature* creature, const Position& pos, uint32_t stackpos)
		{
			if(!owner)
				return;

			owner->sendAddCreature(creature, pos, stackpos);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendAddCreature(creature, pos, stackpos);
		}
		void sendRemoveCreature(const Creature* creature, const Position& pos, uint32_t stackpos)
		{
			if(!owner)
				return;

			owner->sendRemoveCreature(creature, pos, stackpos);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendRemoveCreature(creature, pos, stackpos);
		}
		void sendMoveCreature(const Creature* creature, const Tile* newTile, const Position& newPos, uint32_t newStackPos,
			const Tile* oldTile, const Position& oldPos, uint32_t oldStackpos, bool teleport)
		{
			if(!owner)
				return;

			owner->sendMoveCreature(creature, newTile, newPos, newStackPos, oldTile, oldPos, oldStackpos, teleport);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendMoveCreature(creature, newTile, newPos, newStackPos, oldTile, oldPos, oldStackpos, teleport);
		}

		void sendAddContainerItem(uint8_t cid, const Item* item)
		{
			if(!owner)
				return;

			owner->sendAddContainerItem(cid, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendAddContainerItem(cid, item);
		}
		void sendUpdateContainerItem(uint8_t cid, uint8_t slot, const Item* item)
		{
			if(!owner)
				return;

			owner->sendUpdateContainerItem(cid, slot, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendUpdateContainerItem(cid, slot, item);
		}
		void sendRemoveContainerItem(uint8_t cid, uint8_t slot)
		{
			if(!owner)
				return;

			owner->sendRemoveContainerItem(cid, slot);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendRemoveContainerItem(cid, slot);
		}

		void sendContainer(uint32_t cid, const Container* container, bool hasParent)
		{
			if(!owner)
				return;

			owner->sendContainer(cid, container, hasParent);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendContainer(cid, container, hasParent);
		}
		void sendCloseContainer(uint32_t cid)
		{
			if(!owner)
				return;

			owner->sendCloseContainer(cid);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendCloseContainer(cid);
		}

		void sendAddInventoryItem(slots_t slot, const Item* item)
		{
			if(!owner)
				return;

			owner->sendAddInventoryItem(slot, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendAddInventoryItem(slot, item);
		}
		void sendUpdateInventoryItem(slots_t slot, const Item* item)
		{
			if(!owner)
				return;

			owner->sendUpdateInventoryItem(slot, item);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendUpdateInventoryItem(slot, item);
		}
		void sendRemoveInventoryItem(slots_t slot)
		{
			if(!owner)
				return;

			owner->sendRemoveInventoryItem(slot);
			for(SpectatorList::iterator it = spectators.begin(); it != spectators.end(); ++it)
				it->first->sendRemoveInventoryItem(slot);
		}
};
#endif

