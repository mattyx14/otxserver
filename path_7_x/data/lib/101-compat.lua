--[[
 * File containing deprecated functions and constants used by alot of scripts and other engines
]]--

TRUE = true
FALSE = false
LUA_ERROR = false
LUA_NO_ERROR = true
LUA_NULL = nil

TALKTYPE_SPELL = TALKTYPE_SAY
TALKTYPE_PRIVATE_PN = TALKTYPE_NPC_TO
TALKTYPE_PRIVATE_NP = TALKTYPE_NPC_FROM
TALKTYPE_PRIVATE_FROM = TALKTYPE_PRIVATE
TALKTYPE_PRIVATE_TO = TALKTYPE_PRIVATE
TALKTYPE_CHANNEL_Y = TALKTYPE_CHANNEL
TALKTYPE_CHANNEL_W = TALKTYPE_CHANNEL_MANAGEMENT
TALKTYPE_CHANNEL_RA = TALKTYPE_CHANNEL_MANAGEMENT
TALKTYPE_CHANNEL_R2 = TALKTYPE_CHANNEL_MANAGEMENT
TALKTYPE_BROADCAST = TALKTYPE_GAMEMASTER_BROADCAST
TALKTYPE_CHANNEL_RN = TALKTYPE_GAMEMASTER_CHANNEL
TALKTYPE_PRIVATE_RED = TALKTYPE_GAMEMASTER_PRIVATE
TALKTYPE_GAMEMASTER_PRIVATE_FROM = TALKTYPE_GAMEMASTER_PRIVATE
TALKTYPE_GAMEMASTER_PRIVATE_TO = TALKTYPE_GAMEMASTER_PRIVATE
TALKTYPE_CHANNEL_O = TALKTYPE_CHANNEL_ORANGE
TALKTYPE_CHANNEL_ORANGE = TALKTYPE_CHANNEL_HIGHLIGHT
TALKTYPE_MONSTER = TALKTYPE_MONSTER_SAY
TALKTYPE_ORANGE_1 = TALKTYPE_MONSTER_SAY
TALKTYPE_ORANGE_2 = TALKTYPE_MONSTER_YELL

TEXTCOLOR_BLACK = COLOR_BLACK
TEXTCOLOR_BLUE = COLOR_BLUE
TEXTCOLOR_GREEN = COLOR_GREEN
TEXTCOLOR_LIGHTGREEN = COLOR_LIGHTGREEN
TEXTCOLOR_DARKBROWN = COLOR_DARKBROWN
TEXTCOLOR_LIGHTBLUE = COLOR_LIGHTBLUE
TEXTCOLOR_DARKRED = COLOR_DARKRED
TEXTCOLOR_DARKPURPLE = COLOR_DARKPURPLE
TEXTCOLOR_BROWN = COLOR_BROWN
TEXTCOLOR_GREY = COLOR_GREY
TEXTCOLOR_TEAL = COLOR_TEAL
TEXTCOLOR_DARKPINK = COLOR_DARKPINK
TEXTCOLOR_PURPLE = COLOR_PURPLE
TEXTCOLOR_DARKORANGE = COLOR_DARKORANGE
TEXTCOLOR_RED = COLOR_RED
TEXTCOLOR_PINK = COLOR_PINK
TEXTCOLOR_ORANGE = COLOR_ORANGE
TEXTCOLOR_DARKYELLOW = COLOR_DARKYELLOW
TEXTCOLOR_YELLOW = COLOR_YELLOW
TEXTCOLOR_WHITE = COLOR_WHITE
TEXTCOLOR_NONE = COLOR_NONE

CONDITION_PARAM_STAT_MAXHITPOINTS = CONDITION_PARAM_STAT_MAXHEALTH
CONDITION_PARAM_STAT_MAXMANAPOINTS = CONDITION_PARAM_STAT_MAXMANA
CONDITION_PARAM_STAT_SOULPOINTS = CONDITION_PARAM_STAT_SOUL
CONDITION_PARAM_STAT_MAGICPOINTS = CONDITION_PARAM_STAT_MAGICLEVEL
CONDITION_PARAM_STAT_MAXHITPOINTSPERCENT = CONDITION_PARAM_STAT_MAXHEALTHPERCENT
CONDITION_PARAM_STAT_MAXMANAPOINTSPERCENT = CONDITION_PARAM_STAT_MAXMANAPERCENT
CONDITION_PARAM_STAT_SOULPOINTSPERCENT = CONDITION_PARAM_STAT_SOULPERCENT
CONDITION_PARAM_STAT_MAGICPOINTSPERCENT = CONDITION_PARAM_STAT_MAGICLEVELPERCENT

CONDITION_PHYSICAL = CONDITION_BLEEDING

STACKPOS_FIRST_ITEM_ABOVE_GROUNDTILE = 1
STACKPOS_SECOND_ITEM_ABOVE_GROUNDTILE = 2
STACKPOS_THIRD_ITEM_ABOVE_GROUNDTILE = 3
STACKPOS_FOURTH_ITEM_ABOVE_GROUNDTILE = 4
STACKPOS_FIFTH_ITEM_ABOVE_GROUNDTILE = 5

WORLD_TYPE_NO_PVP = WORLDTYPE_OPTIONAL
WORLD_TYPE_PVP = WORLDTYPE_OPEN
WORLD_TYPE_PVP_ENFORCED = WORLDTYPE_HARDCORE

WORLDTYPE_NO_PVP = WORLDTYPE_OPTIONAL
WORLDTYPE_PVP = WORLDTYPE_OPEN
WORLDTYPE_PVP_ENFORCED = WORLDTYPE_HARDCORE

GUILDLEVEL_MEMBER = GUILD_MEMBER
GUILDLEVEL_VICE = GUILD_VICE
GUILDLEVEL_LEADER = GUILD_LEADER

DATABASE_ENGINE_NONE = DATABASE_NONE
DATABASE_ENGINE_MYSQL = DATABASE_MYSQL
DATABASE_ENGINE_SQLITE = DATABASE_SQLITE
DATABASE_ENGINE_POSTGRESQL = DATABASE_POSTGRESQL
DATABASE_ENGINE_ODBC = DATABASE_ODBC

CHANNEL_STAFF = 4
CHANNEL_COUNSELOR = 2
CHANNEL_GAMECHAT = 5
CHANNEL_TRADE = 6
CHANNEL_TRADEROOK = 7
CHANNEL_RLCHAT = 8

BANTYPE_IP_BANISHMENT = 1
BANTYPE_NAMELOCK = 2
BANTYPE_BANISHMENT = 3
BANTYPE_NOTATION = 4
BANTYPE_DELETION = 3

CONST_PROP_MOVEABLE = CONST_PROP_MOVABLE
CONST_PROP_BLOCKINGANDNOTMOVEABLE = CONST_PROP_BLOCKINGANDNOTMOVABLE

CONDITION_EARTH = CONDITION_POISON

STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE = STACKPOS_TOP_MOVABLE_ITEM_OR_CREATURE

RETURNVALUE_NOTMOVEABLE = RETURNVALUE_NOTMOVABLE

SKILLS = SKILL_NAMES

doSetCreatureDropLoot = doCreatureSetDropLoot
doPlayerSay = doCreatureSay
doPlayerAddMana = doCreatureAddMana
playerLearnInstantSpell = doPlayerLearnInstantSpell
doPlayerRemOutfit = doPlayerRemoveOutfit
pay = doPlayerRemoveMoney
broadcastMessage = doBroadcastMessage
getPlayerName = getCreatureName
getCreaturePosition = getThingPosition
getPlayerPosition = getCreaturePosition
getCreaturePos = getCreaturePosition
creatureGetPosition = getCreaturePosition
getPlayerMana = getCreatureMana
getPlayerMaxMana = getCreatureMaxMana
hasCondition = hasCreatureCondition
getCreatureCondition = hasCreatureCondition
isMoveable = isMovable
isItemMoveable = isItemMovable
saveData = saveServer
savePlayers = saveServer
getPlayerSkill = getPlayerSkillLevel
getPlayerSkullType = getCreatureSkullType
getCreatureSkull = getCreatureSkullType
getAccountNumberByName = getAccountIdByName
getIPByName = getIpByName
getPlayersByIP = getPlayersByIp
getThingFromPos = getThingFromPosition
getThingfromPos = getThingFromPos
getHouseFromPos = getHouseFromPosition
getPlayersByAccountNumber = getPlayersByAccountId
getIPByPlayerName = getIpByName
getPlayersByIPNumber = getPlayersByIp
getAccountNumberByPlayerName = getAccountIdByName
convertIntToIP = doConvertIntegerToIp
convertIPToInt = doConvertIpToInteger
queryTileAddThing = doTileQueryAdd
getTileHouseInfo = getHouseFromPos
executeRaid = doExecuteRaid
saveServer = doSaveServer
cleanHouse = doCleanHouse
cleanMap = doCleanMap
shutdown = doShutdown
mayNotMove = doCreatureSetNoMove
getTileItemsByType = getTileItemByType
doPlayerSetNoMove = doCreatureSetNoMove
getPlayerNoMove = getCreatureNoMove
getConfigInfo = getConfigValue
doPlayerAddExp = doPlayerAddExperience
isInArea = isInRange
doPlayerSetSkillRate = doPlayerSetRate
getCreatureLookDir = getCreatureLookDirection
getPlayerLookDir = getCreatureLookDirection
getPlayerLookDirection = getCreatureLookDirection
doCreatureSetLookDir = doCreatureSetLookDirection
getPlayerLookPos = getCreatureLookPosition
setPlayerStamina = doPlayerSetStamina
setPlayerPromotionLevel = doPlayerSetPromotionLevel
setPlayerGroupId = doPlayerSetGroupId
setPlayerPartner = doPlayerSetPartner
doPlayerSetStorageValue = doCreatureSetStorage
setPlayerStorageValue = doPlayerSetStorageValue
getPlayerStorageValue = getCreatureStorage
getGlobalStorageValue = getStorage
setGlobalStorageValue = doSetStorage
getPlayerMount = canPlayerRideMount
setPlayerBalance = doPlayerSetBalance
doAddMapMark = doPlayerAddMapMark
doSendTutorial = doPlayerSendTutorial
getWaypointsList = getWaypointList
getPlayerLastLoginSaved = getPlayerLastLogin
getThingPos = getThingPosition
doAreaCombatHealth = doCombatAreaHealth
doAreaCombatMana = doCombatAreaMana
doAreaCombatCondition = doCombatAreaCondition
doAreaCombatDispel = doCombatAreaDispel
getItemDescriptionsById = getItemInfo
hasProperty = hasItemProperty
hasClient = hasPlayerClient
print = std.cout
getPosByDir = getPositionByDirection
isNumber = isNumeric
doSetItemActionId = doItemSetActionId
getOnlinePlayers = getPlayersOnlineEx
addDialog = doPlayerAddDialog
doSendPlayerExtendedOpcode = doPlayerSendExtendedOpcode

PlayerFlag_CannotUseCombat = 0
PlayerFlag_CannotAttackPlayer = 1
PlayerFlag_CannotAttackMonster = 2
PlayerFlag_CannotBeAttacked = 3
PlayerFlag_CanConvinceAll = 4
PlayerFlag_CanSummonAll = 5
PlayerFlag_CanIllusionAll = 6
PlayerFlag_CanSenseInvisibility = 7
PlayerFlag_IgnoredByMonsters = 8
PlayerFlag_NotGainInFight = 9
PlayerFlag_HasInfiniteMana = 10
PlayerFlag_HasInfiniteSoul = 11
PlayerFlag_HasNoExhaustion = 12
PlayerFlag_CannotUseSpells = 13
PlayerFlag_CannotPickupItem = 14
PlayerFlag_CanAlwaysLogin = 15
PlayerFlag_CanBroadcast = 16
PlayerFlag_CanEditHouses = 17
PlayerFlag_CannotBeBanned = 18
PlayerFlag_CannotBePushed = 19
PlayerFlag_HasInfiniteCapacity = 20
PlayerFlag_CanPushAllCreatures = 21
PlayerFlag_CanTalkRedPrivate = 22
PlayerFlag_CanTalkRedChannel = 23
PlayerFlag_TalkOrangeHelpChannel = 24
PlayerFlag_NotGainExperience = 25
PlayerFlag_NotGainMana = 26
PlayerFlag_NotGainHealth = 27
PlayerFlag_NotGainSkill = 28
PlayerFlag_SetMaxSpeed = 29
PlayerFlag_SpecialVIP = 30
PlayerFlag_NotGenerateLoot = 31
PlayerFlag_CanTalkRedChannelAnonymous = 32
PlayerFlag_IgnoreProtectionZone = 33
PlayerFlag_IgnoreSpellCheck = 34
PlayerFlag_IgnoreWeaponCheck = 35
PlayerFlag_CannotBeMuted = 36
PlayerFlag_IsAlwaysPremium = 37
PlayerFlag_ShowGroupNameInsteadOfVocation = 40
PlayerFlag_HasInfiniteStamina = 41
PlayerFlag_CannotMoveItems = 42
PlayerFlag_CannotMoveCreatures = 43
PlayerFlag_CanReportBugs = 44
PlayerFlag_CannotBeSeen = 46
PlayerFlag_HideHealth = 47
PlayerFlag_CanPassThroughAllCreatures = 48

PlayerCustomFlag_AllowIdle = 0
PlayerCustomFlag_CanSeePosition	= 1
PlayerCustomFlag_CanSeeItemDetails = 2
PlayerCustomFlag_CanSeeCreatureDetails = 3
PlayerCustomFlag_NotSearchable = 4
PlayerCustomFlag_GamemasterPrivileges = 5
PlayerCustomFlag_CanThrowAnywhere = 6
PlayerCustomFlag_CanPushAllItems = 7
PlayerCustomFlag_CanMoveAnywhere = 8
PlayerCustomFlag_CanMoveFromFar = 9
PlayerCustomFlag_CanUseFar = 10
PlayerCustomFlag_CanLoginMultipleCharacters = 11
PlayerCustomFlag_CanLogoutAnytime = 12
PlayerCustomFlag_HideLevel = 13
PlayerCustomFlag_IsProtected = 14
PlayerCustomFlag_IsImmune = 15
PlayerCustomFlag_NotGainSkull = 16
PlayerCustomFlag_NotGainUnjustified = 17
PlayerCustomFlag_IgnorePacification = 18
PlayerCustomFlag_IgnoreLoginDelay = 19
PlayerCustomFlag_CanStairhop = 20
PlayerCustomFlag_CanTurnhop = 21
PlayerCustomFlag_IgnoreHouseRent = 22
PlayerCustomFlag_CanWearAllAddons = 23
PlayerCustomFlag_IsWalkable = 24
PlayerCustomFlag_HasFullLight = 26

Creature = {}
Player = {}
Monster = {}
Npc = {}
Game = {}
Condition = {}
Combat = {}

function createClass(class, inheritance)
	setmetatable(class, {
		__index = inheritance and inheritance or nil,
		__call =
		function(self, cid)
		if not cid then
			self = {__index = class}
			setmetatable(self, self)
			return self
		end
		local cid = type(cid) == "number" and cid or type(cid) == "string" and getPlayerByNameWildcard(cid)
		if cid then
			if not self[cid] then
				self[cid] = {__index = class}
				setmetatable(self[cid], self[cid])
				self[cid].cid = cid
				return self[cid]
			else
				return self[cid]
			end
		else
			return nil
		end
	end
	})
end

createClass(Creature)
createClass(Player, Creature)
createClass(Monster, Creature)
createClass(Npc, Creature)
createClass(Game)
createClass(Condition)
createClass(Combat)

-- access metatable functions from creature
function Creature:getPlayer() return Player(self.cid) end
function Creature:getMonster() return Monster(self.cid) end
function Creature:getNpc() return Npc(self.cid) end

-- Creature
-- Creature get functions
function Creature:getId() return self.cid end
function Creature:getName() return getCreatureName(self.cid) end
function Creature:getHealth() return getCreatureHealth(self.cid) end
function Creature:getMaxHealth() return getCreatureMaxHealth(self.cid) end
function Creature:getMana() return getCreatureMana(self.cid) end
function Creature:getMaxMana() return getCreatureMaxMana(self.cid) end
function Creature:getStorageValue(id) return getCreatureStorageValue(self.cid, id) end
function Creature:getHiddenHealth() return getCreatureHideHealth(self.cid) end
function Creature:getSpeakType() return getCreatureSpeakType(self.cid) end
function Creature:getLookDirection() return getCreatureLookDirection(self.cid) end

-- Creature set functions
function Creature:setMaxHealth(health) return setCreatureMaxHealth(self.cid, health) end
function Creature:setMaxMana(mana) return setCreatureMaxMana(self.cid, mana) end
function Creature:setStorageValue(storage, value) return setCreatureStorageValue(self.cid, storage, value) end
function Creature:setHiddenHealth(hide) doCreatureSetHideHealth(self.cid, hide) end
function Creature:setSpeakType(type) doCreatureSetSpeakType(self.cid, type) end

-- Creature misc. functions
function Creature:teleportTo(newpos, pushmove) doTeleportThing(self.cid, newpos, pushmove or false) end
function Creature:addHealth(health, hitEffect, hitColor, force) doCreatureAddHealth(self.cid, health, hitEffect, hitColor, force) end
function Creature:addMana(mana) doCreatureAddMana(self.cid, mana) end
function Creature:say(text, type, ghost, cid, pos) doCreatureSay(self.cid, text, type or SPEAK_SAY, ghost or false, cid or 0, pos) end
function Creature:addCondition(condition) doAddCondition(self.cid, condition) end
function Creature:removeCondition(onlyPersistent, type, subId) if onlyPersistent then doRemoveConditions(self.cid, onlyPersistent) else doRemoveCondition(self.cid, type, subId) end end
function Creature:moveCreature(direction) doMoveCreature(self.cid, direction) end
function Creature:isCreature() return isCreature(self.cid) end
function Creature:isPlayer() return isPlayer(self.cid) end
function Creature:isMonster() return isMonster(self.cid) end
function Creature:isNpc() return isNpc(self.cid) end
function Creature:remove(forceLogout) doRemoveCreature(self.cid, forceLogout or true) end

-- Player
-- Player get functions
function Player:getLevel() return getPlayerLevel(self.cid) end
function Player:getExperience() return getPlayerExperience(self.cid) end
function Player:getMagicLevel(ignoreBuffs) return getPlayerMagLevel(self.cid, ignoreBuffs) end
function Player:getManaSpent() return getPlayerSpentMana(self.cid) end
function Player:getFood() return getPlayerFood(self.cid) end
function Player:getAccess() return getPlayerAccess(self.cid) end
function Player:getGhostAccess() return getPlayerGhostAccess(self.cid) end
function Player:getSkillLevel(skillid) return getPlayerSkillLevel(self.cid, skillid) end
function Player:getSkillTries(skillid) return getPlayerSkillTries(self.cid, skillid) end
function Player:getTown() return getPlayerTown(self.cid) end
function Player:getVocation() return getPlayerVocation(self.cid) end
function Player:getBaseVocation() local vocation = self:getVocation(); return vocation > 4 and vocation - 4 or vocation end
function Player:getIp() return getPlayerIp(self.cid) end
function Player:getRequiredMana(magicLevel) return getPlayerRequiredMana(self.cid, magicLevel) end
function Player:getRequiredSkillTries(skillId, skillLevel) return getPlayerRequiredSkillTries(self.cid, skillId, skillLevel) end
function Player:getItemCount(itemid, subType) return getPlayerItemCount(self.cid, itemid, subType or -1) end
function Player:getMoney() return getPlayerMoney(self.cid) end
function Player:getSoul() return getPlayerSoul(self.cid) end
function Player:getCap() return getPlayerFreeCap(self.cid) end
function Player:getLight() return getPlayerLight(self.cid) end
function Player:getSlotItem(slot) return getPlayerSlotItem(self.cid, slot) end

-- Player set functions
function Player:setMaxCap(cap) doPlayerSetMaxCapacity(self.cid, cap) end
function Player:setTown(townid) doPlayerSetTown(self.cid, townid) end
function Player:setVocation(vocation) doPlayerSetVocation(self.cid, vocation) end


-- Player send functions
function Player:sendCancelMsg(text) doPlayerSendCancel(self.cid, text) end
function Player:sendDefaultCancelMsg(ReturnValue) doPlayerSendDefaultCancel(self.cid, ReturnValue) end
function Player:sendTextMessage(MessageClasses, message) doPlayerSendTextMessage(self.cid, MessageClasses, message) end
function Player:sendChannelMessage(author, message, SpeakClasses, channel) doPlayerSendChannelMessage(self.cid, author, message, SpeakClasses, channel) end
function Player:sendToChannel(targetId, SpeakClasses, message, channel, time) doPlayerSendToChannel(self.cid, targetId, SpeakClasses, message, channel, time) end
function Player:addMoney(money) doPlayerAddMoney(self.cid, money) end
function Player:removeMoney(money) doPlayerRemoveMoney(self.cid, money) end
function Player:transferMoney(target, money) doPlayerTransferMoneyTo(self.cid, target, money) end
function Player:showTextDialog(itemid, text) doShowTextDialog(self.cid, itemid, text) end
function Player:addSkillTries(skillid, n, useMultiplier) doPlayerAddSkillTry(self.cid, skillid, n, useMultiplier or false) end

-- Player misc. functions
function Player:feed(food) doPlayerFeed(self.cid, food) end
function Player:addSpentMana(amount, useMultiplier) doPlayerAddSpentMana(self.cid, amount, useMultiplier) end
function Player:addSoul(soul) doPlayerAddSoul(self.cid, soul) end
function Player:addItem(itemid, count, canDropOnMap, subtype) return doPlayerAddItem(self.cid, itemid, count, canDropOnMap, subtype) end
function Player:addItemEx(uid, canDropOnMap) return doPlayerAddItemEx(self.cid, uid, canDropOnMap or false) end
function Player:addExperience(amount) doPlayerAddExperience(self.cid, amount) end
function Player:savePlayer(shallow) doPlayerSave(self.cid, shallow or false) end
function Player:isPzLocked() return isPlayerPzLocked(self.cid) end

-- Game
function Game.getStorageValue(key) return getGlobalStorageValue(key) end
function Game.setStorageValue(key, value) setGlobalStorageValue(key, value) end
function Game.getChannelUsers(channelId) return getChannelUsers(channelId) end
function Game.setWorldType(type) return setWorldType(type) end
function Game.getWorldType() return getWorldType() end
function Game.getSpectators(centerPos, rangex, rangey, multifloor) return getSpecators(centerPos, rangex, rangey, multifloor or false) end
function Game.getPlayers() return getPlayersOnline() end
function Game.getExperienceStage(level) return getExperienceStage(level) end
function Game.getGameState() return getGameState() end
function Game.setGameState(state) return doSetGameState(state) end
function Game.startRaid(raid) return doExecuteRaid(raid) end
function Game.createItem(itemId, count, position) return doCreateItem(itemId, count, position) end
function Game.createMonster(name, pos, extend, force, displayError) return doCreateMonster(name, pos, extend or false, force or false, displayError or true) end
function Game.createNpc(name, pos, displayError) return doCreateNpc(name, pos, displayError or true) end

-- Condition
--

-- Combat
--
