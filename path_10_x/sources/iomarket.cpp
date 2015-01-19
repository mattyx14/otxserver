//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "otpch.h"

#include "iomarket.h"
#include "iologindata.h"
#include "configmanager.h"

extern ConfigManager g_config;

MarketOfferList IOMarket::getActiveOffers(MarketAction_t action, uint16_t itemId)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `player_id`, `amount`, `price`, `created`, `anonymous` FROM `market_offers` WHERE `sale` = "
		<< action << " AND `itemtype` = " << itemId << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return MarketOfferList();

	MarketOfferList offerList;
	do
	{
		MarketOffer offer;
		offer.amount = result->getDataInt("amount");
		offer.price = result->getDataInt("price");
		offer.timestamp = result->getDataInt("created") + g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION);
		offer.counter = result->getDataInt("id") & 0xFFFF;

		if(!result->getDataInt("anonymous"))
		{
			IOLoginData::getInstance()->getNameByGuid(result->getDataInt("player_id"), offer.playerName);
			if(offer.playerName.empty())
				offer.playerName = "Anonymous";
		}
		else
			offer.playerName = "Anonymous";

		offerList.push_back(offer);
	}
	while(result->next());
	result->free();
	return offerList;
}

MarketOfferList IOMarket::getOwnOffers(MarketAction_t action, uint32_t playerId)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `amount`, `price`, `created`, `anonymous`, `itemtype` FROM `market_offers` WHERE `player_id` = "
		<< playerId << " AND `sale` = " << action << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return MarketOfferList();

	MarketOfferList offerList;
	do
	{
		MarketOffer offer;
		offer.amount = result->getDataInt("amount");
		offer.price = result->getDataInt("price");
		offer.timestamp = result->getDataInt("created") + g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION);
		offer.counter = result->getDataInt("id") & 0xFFFF;
		offer.itemId = result->getDataInt("itemtype");

		offerList.push_back(offer);
	}
	while(result->next());
	result->free();
	return offerList;
}

HistoryMarketOfferList IOMarket::getOwnHistory(MarketAction_t action, uint32_t playerId)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `itemtype`, `amount`, `price`, `expires_at`, `state` FROM `market_history` WHERE `player_id` = "
		<< playerId << " AND `sale` = " << action << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return HistoryMarketOfferList();

	HistoryMarketOfferList offerList;
	do
	{
		HistoryMarketOffer offer;
		offer.itemId = result->getDataInt("itemtype");
		offer.amount = result->getDataInt("amount");
		offer.price = result->getDataInt("price");
		offer.timestamp = result->getDataInt("expires_at");

		MarketOfferState_t offerState = (MarketOfferState_t)result->getDataInt("state");
		if(offerState == OFFERSTATE_ACCEPTEDEX)
			offerState = OFFERSTATE_ACCEPTED;

		offer.state = offerState;
		offerList.push_back(offer);
	}
	while(result->next());
	result->free();
	return offerList;
}

ExpiredMarketOfferList IOMarket::getExpiredOffers(MarketAction_t action)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `amount`, `price`, `itemtype`, `player_id` FROM `market_offers` WHERE `sale` = " << action << " AND `created` <= "
		<< (time(NULL) - g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION)) << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	
	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return ExpiredMarketOfferList();

	ExpiredMarketOfferList offerList;
	do
	{
		ExpiredMarketOffer offer;
		offer.id = result->getDataInt("id");
		offer.amount = result->getDataInt("amount");
		offer.price = result->getDataInt("price");
		offer.itemId = result->getDataInt("itemtype");
		offer.playerId = result->getDataInt("player_id");

		offerList.push_back(offer);
	}
	while(result->next());
	result->free();
	return offerList;
}

int32_t IOMarket::getPlayerOfferCount(uint32_t playerId)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT COUNT(*) AS `count` FROM `market_offers` WHERE `player_id` = " << playerId << " AND `world_id` = " 
		<< g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return -1;

	int32_t tmp = result->getDataInt("count");
	result->free();
	return tmp;
}

MarketOfferEx IOMarket::getOfferById(uint32_t id)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `sale`, `itemtype`, `amount`, `created`, `price`, `player_id`, `anonymous` FROM `market_offers` WHERE `id` = " << id 
		<< " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return MarketOfferEx();

	MarketOfferEx offer;
	offer.type = (MarketAction_t)result->getDataInt("sale");
	offer.amount = result->getDataInt("amount");
	offer.counter = result->getDataInt("id") & 0xFFFF;
	offer.timestamp = result->getDataInt("created");
	offer.price = result->getDataInt("price");
	offer.itemId = result->getDataInt("itemtype");

	int32_t playerId = result->getDataInt("player_id");
	offer.playerId = playerId;
	if(!result->getDataInt("anonymous"))
	{
		IOLoginData::getInstance()->getNameByGuid(playerId, offer.playerName);
		if(offer.playerName.empty())
			offer.playerName = "Anonymous";
	}
	else
		offer.playerName = "Anonymous";

	result->free();
	return offer;
}

uint32_t IOMarket::getOfferIdByCounter(uint32_t timestamp, uint16_t counter)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id` FROM `market_offers` WHERE `created` = " << (timestamp - g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION))
		<< " AND (`id` & 65535) = " << counter << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << " LIMIT 1;";
	
	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return 0;

	uint32_t tmp = result->getDataInt("id");
	result->free();
	return tmp;
}

void IOMarket::createOffer(uint32_t playerId, MarketAction_t action, uint32_t itemId, uint16_t amount, uint32_t price, bool anonymous)
{
	DBQuery query;
	query << "INSERT INTO `market_offers` (`player_id`, `world_id`, `sale`, `itemtype`, `amount`, `price`, `created`, `anonymous`) VALUES ("
		<< playerId << ", " << g_config.getNumber(ConfigManager::WORLD_ID) << ", " << action << ", " << itemId << ", " << amount << ", " << price 
		<< ", " << time(NULL) << ", " << anonymous << ");";
	Database::getInstance()->query(query.str());
}

void IOMarket::acceptOffer(uint32_t offerId, uint16_t amount)
{
	DBQuery query;
	query << "UPDATE `market_offers` SET `amount` = `amount` - " << amount << " WHERE `id` = " << offerId << " AND `world_id` = " 
		<< g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	Database::getInstance()->query(query.str());
}

void IOMarket::appendHistory(uint32_t playerId, MarketAction_t type, uint16_t itemId, uint16_t amount, uint32_t price, time_t timestamp, MarketOfferState_t state)
{
	DBQuery query;
	query << "INSERT INTO `market_history` (`player_id`, `world_id`, `sale`, `itemtype`, `amount`, `price`, `expires_at`, `inserted`, `state`) VALUES "
		<< "(" << playerId << ", " << g_config.getNumber(ConfigManager::WORLD_ID) << ", " << type << ", " << itemId << ", " << amount << ", " << price << ", " << timestamp << ", " << time(NULL) << ", " << state << ");";
	Database::getInstance()->query(query.str());
}

void IOMarket::moveOfferToHistory(uint32_t offerId, MarketOfferState_t state)
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `player_id`, `sale`, `itemtype`, `amount`, `price`, `created` FROM `market_offers` WHERE `id` = " << offerId << " AND `world_id` = " 
		<< g_config.getNumber(ConfigManager::WORLD_ID) << ";";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return;

	query.str("");
	query << "DELETE FROM `market_offers` WHERE `id` = " << offerId << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	if(!db->query(query.str()))
	{
		result->free();
		return;
	}

	appendHistory(result->getDataInt("player_id"), (MarketAction_t)result->getDataInt("sale"), result->getDataInt("itemtype"), result->getDataInt("amount"),
		result->getDataInt("price"), result->getDataInt("created") + g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION), state);
	result->free();
}

void IOMarket::clearOldHistory()
{
	DBQuery query;
	query << "DELETE FROM `market_history` WHERE `inserted` <= " << (time(NULL) - g_config.getNumber(ConfigManager::MARKET_OFFER_DURATION)) << " AND `world_id` = " 
		<< g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	Database::getInstance()->query(query.str());
}

void IOMarket::updateStatistics()
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `sale`, `itemtype`, COUNT(`price`) AS `num`, MIN(`price`) AS `min`, MAX(`price`) AS `max`, SUM(`price`) AS `sum` FROM `market_history` WHERE `state` = "
		<< OFFERSTATE_ACCEPTED << " AND `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << " GROUP BY `itemtype`, `sale`;";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return;

	do
	{
		MarketStatistics* statistics;
		if(result->getDataInt("sale") == MARKETACTION_BUY)
			statistics = &purchaseStatistics[result->getDataInt("itemtype")];
		else
			statistics = &saleStatistics[result->getDataInt("itemtype")];

		statistics->numTransactions = result->getDataInt("num");
		statistics->lowestPrice = result->getDataInt("min");
		statistics->totalPrice = result->getDataLong("sum");
		statistics->highestPrice = result->getDataInt("max");
	}
	while(result->next());
	result->free();
}

MarketStatistics* IOMarket::getPurchaseStatistics(uint16_t itemId)
{
	std::map<uint16_t, MarketStatistics>::iterator it = purchaseStatistics.find(itemId);
	if(it == purchaseStatistics.end())
		return NULL;

	return &it->second;
}

MarketStatistics* IOMarket::getSaleStatistics(uint16_t itemId)
{
	std::map<uint16_t, MarketStatistics>::iterator it = saleStatistics.find(itemId);
	if(it == saleStatistics.end())
		return NULL;

	return &it->second;
}
