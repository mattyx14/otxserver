function doPlayerSendToChannel(cid, target, type, text, channel, time)
	return doCreatureChannelSay(cid, target, text, type, channel)
end

function getItemWeaponType(uid)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	return getItemInfo(thing.itemid).weaponType
end

function getItemRWInfo(uid)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	local item, flags = getItemInfo(thing.itemid), 0
	if(item.readable) then
		flags = 1
	end

	if(item.writable) then
		flags = flags + 2
	end

	return flags
end

function getItemLevelDoor(itemid)
	local item = getItemInfo(itemid)
	return item and item.levelDoor or false
end

function isContainer(uid)
	local thing = getThing(uid)
	return thing.uid > 0 and thing.items ~= nil
end

function isItemStackable(itemid)
	local item = getItemInfo(itemid)
	return item and item.stackable or false
end

function isItemRune(itemid)
	local item = getItemInfo(itemid)
	return item and item.type == ITEM_TYPE_RUNE or false
end

function isItemDoor(itemid)
	local item = getItemInfo(itemid)
	return item and item.type == ITEM_TYPE_DOOR or false
end

function isItemContainer(itemid)
	local item = getItemInfo(itemid)
	return item and item.group == ITEM_GROUP_CONTAINER or false
end

function isItemFluidContainer(itemid)
	local item = getItemInfo(itemid)
	return item and item.group == ITEM_GROUP_FLUID or false
end

function isItemMovable(itemid)
	local item = getItemInfo(itemid)
	return item and item.movable or false
end

function isCorpse(uid)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	local item = getItemInfo(thing.itemid)
	return item and item.corpseType ~= 0 or false
end

function getContainerCapById(itemid)
	local item = getItemInfo(itemid)
	if(not item or item.group ~= 2) then
		return false
	end

	return item.maxItems
end

function getMonsterAttackSpells(name)
	local monster = getMonsterInfo(name)
	return monster and monster.attacks or false
end

function getMonsterHealingSpells(name)
	local monster = getMonsterInfo(name)
	return monster and monster.defenses or false
end

function getMonsterLootList(name)
	local monster = getMonsterInfo(name)
	return monster and monster.loot or false
end

function getMonsterSummonList(name)
	local monster = getMonsterInfo(name)
	return monster and monster.summons or false
end

function doItemSetActionId(uid, aid)
	return doItemSetAttribute(uid, "aid", aid)
end

function getFluidSourceType(itemid)
	local item = getItemInfo(itemid)
	return item and item.fluidSource or false
end

function getDepotId(uid)
	return getItemAttribute(uid, "depotid") or false
end

function getItemDescriptions(uid)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	local item = getItemInfo(thing.itemid)
	return {
		name = getItemAttribute(uid, "name") or item.name,
		plural = getItemAttribute(uid, "pluralname") or item.plural,
		article = getItemAttribute(uid, "article") or item.article,
		special = getItemAttribute(uid, "description") or "",
		text = getItemAttribute(uid, "text") or "",
		writer = getItemAttribute(uid, "writer") or "",
		date = getItemAttribute(uid, "date") or 0
	}
end

function doRemoveThing(uid)
	if(isCreature(uid)) then
		return doRemoveCreature(uid)
	end

	return doRemoveItem(uid)
end

function setAttackFormula(combat, type, minl, maxl, minm, maxm, min, max)
	local min, max = min or 0, max or 0
	return setCombatFormula(combat, type, -1, 0, -1, 0, minl, maxl, minm, maxm, -min, -max)
end

function setHealingFormula(combat, type, minl, maxl, minm, maxm, min, max)
	local min, max = min or 0, max or 0
	return setCombatFormula(combat, type, 1, 0, 1, 0, minl, maxl, minm, maxm, min, max)
end

function doChangeTypeItem(uid, subtype)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	local subtype = subtype or 1
	return doTransformItem(thing.uid, thing.itemid, subtype)
end

function doPlayerResetIdleTime(cid)
	return doPlayerSetIdleTime(cid, 0)
end

function doPlayerSetExperienceRate(cid, value)
	return doPlayerSetRate(cid, SKILL__LEVEL, value)
end

function doPlayerSetMagicRate(cid, value)
	return doPlayerSetRate(cid, SKILL__MAGLEVEL, value)
end

function isSummon(cid)
	return getCreatureMaster(cid) and getCreatureMaster(cid) ~= cid
end

function getPartyLeader(cid)
	local party = getPartyMembers(cid)
	if(type(party) ~= 'table') then
		return 0
	end

	return party[1]
end

function isInParty(cid)
	return type(getPartyMembers(cid)) == 'table'
end

function doShutdown()
	return doSetGameState(GAMESTATE_SHUTDOWN)
end

function getPlayerGroupName(cid)
	return getGroupInfo(getPlayerGroupId(cid)).name
end

function getPlayerVocationName(cid)
	return getVocationInfo(getPlayerVocation(cid)).name
end

function getPromotedVocation(vid)
	return getVocationInfo(vid).promotedVocation
end

function doPlayerRemovePremiumDays(cid, days)
	return doPlayerAddPremiumDays(cid, -days)
end

function getPlayerMasterPos(cid)
	return getTownTemplePosition(getPlayerTown(cid))
end

function getHouseOwner(houseId)
	return getHouseInfo(houseId).owner
end

function getHouseName(houseId)
	return getHouseInfo(houseId).name
end

function getHouseEntry(houseId)
	return getHouseInfo(houseId).entry
end

function getHouseRent(houseId)
	return getHouseInfo(houseId).rent
end

function getHousePrice(houseId)
	return getHouseInfo(houseId).price
end

function getHouseTown(houseId)
	return getHouseInfo(houseId).town
end

function getHouseDoorsCount(houseId)
	return table.maxn(getHouseInfo(houseId).doors)
end

function getHouseBedsCount(houseId)
	return table.maxn(getHouseInfo(houseId).beds)
end

function getHouseTilesCount(houseId)
	return table.maxn(getHouseInfo(houseId).tiles)
end

function getItemNameById(itemid)
	return getItemDescriptionsById(itemid).name
end

function getItemPluralNameById(itemid)
	return getItemDescriptionsById(itemid).plural
end

function getItemArticleById(itemid)
	return getItemDescriptionsById(itemid).article
end

function getItemName(uid)
	return getItemDescriptions(uid).name
end

function getItemPluralName(uid)
	return getItemDescriptions(uid).plural
end

function getItemArticle(uid)
	return getItemDescriptions(uid).article
end

function getItemText(uid)
	return getItemDescriptions(uid).text
end

function getItemSpecialDescription(uid)
	return getItemDescriptions(uid).special
end

function doSetItemSpecialDescription(uid, str)
	return doItemSetAttribute(uid, "description", str)
end

function getItemWriter(uid)
	return getItemDescriptions(uid).writer
end

function getItemDate(uid)
	return getItemDescriptions(uid).date
end

function getTilePzInfo(pos)
	return getTileInfo(pos).protection
end

function getTileZoneInfo(pos)
	local tmp = getTileInfo(pos)
	if(tmp.pvp) then
		return 2
	end

	if(tmp.nopvp) then
		return 1
	end

	return 0
end

function playerExists(name, multiworld)
	return getPlayerGUIDByName(name, multiworld or false) ~= nil
end

function doPlayerWithdrawAllMoney(cid)
	return doPlayerWithdrawMoney(cid, getPlayerBalance(cid))
end

function doPlayerDepositAllMoney(cid)
	return doPlayerDepositMoney(cid, getPlayerMoney(cid))
end

function doPlayerTransferAllMoneyTo(cid, target)
	return doPlayerTransferMoneyTo(cid, target, getPlayerBalance(cid))
end

function isNumeric(str)
	return tonumber(str) ~= nil
end

function doPlayerBuyItem(cid, itemid, count, cost, charges)
	return doPlayerRemoveMoney(cid, cost) and doPlayerGiveItem(cid, itemid, count, charges)
end

function doPlayerBuyItemContainer(cid, containerid, itemid, count, cost, charges)
	return doPlayerRemoveMoney(cid, cost) and doPlayerGiveItemContainer(cid, containerid, itemid, count, charges)
end

function isPlayerUsingOtclient(cid)
	return getPlayerOperatingSystem(cid) >= CLIENTOS_OTCLIENT_LINUX
end

function getPlayerPassword(cid)
local AccInfo = db.getResult("SELECT `password` FROM `accounts` WHERE `id` = " .. getPlayerAccountId(cid) .. " LIMIT 1")
	local AccPass = AccInfo:getDataString("password")
	return AccPass
end

function doPlayerAddPremiumPoints(cid, points)
	return db.Query("UPDATE `accounts` SET `premium_points` = `premium_points` + " .. points .. " WHERE `id` = " .. getPlayerAccountId(cid) .. ";")
end

function doRemoveHouse(cid)
	local pid = getPlayerGUID(cid)
		cleanHouse(getHouseByPlayerGUID(pid))
		setHouseOwner(getHouseByPlayerGUID(pid), NO_OWNER_PHRASE,true)
	return true
end

function warnPlayer(cid, msg)
	doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
	return doPlayerSendCancel(cid, msg)
end

function createCombat(typea, effect, distEffect, area, mins, maxs)
	local combat = createCombatObject()
		setCombatParam(combat, COMBAT_PARAM_TYPE, typea)
		setCombatParam(combat, COMBAT_PARAM_EFFECT, effect)
		if(distEffect)then
			setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, distEffect)
		end
		if(type(mins) == "string" and type(maxs) == "string")then
			function getSpellDamage(cid, skill, att, attackStrength)
				local lvl, mlvl, minss, maxss = getPlayerLevel(cid), getPlayerMagLevel(cid), "", ""
				minss = "return " .. mins
				minss = minss:gsub("lvl", lvl)
				minss = minss:gsub("mlv", mlvl)
				maxss = "return " .. maxs
				maxss = maxss:gsub("lvl", lvl)
				maxss = maxss:gsub("mlv", mlvl)
				local min = -math.ceil(loadstring(minss)())
				local max = -math.ceil(loadstring(maxss)())

				return min, max
			end
		setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")
	end

		if(type(area) == "table")then
			local areaa = createCombatArea(area)
			setCombatArea(combat, areaa)
		end
	return combat
end