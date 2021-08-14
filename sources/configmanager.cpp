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
#include <iostream>

#include "configmanager.h"
#include "house.h"
#include "tools.h"

ConfigManager::ConfigManager()
{
	L = NULL;
	m_loaded = false;
	m_startup = true;

	m_confNumber[ENCRYPTION] = ENCRYPTION_SHA1;
	m_confString[CONFIG_FILE] = getFilePath(FILE_TYPE_CONFIG, "config.lua");

	m_confNumber[LOGIN_PORT] = m_confNumber[ADMIN_PORT] = m_confNumber[MANAGER_PORT] = m_confNumber[STATUS_PORT] = 0;
	m_confString[DATA_DIRECTORY] = m_confString[LOGS_DIRECTORY] = m_confString[IP] = m_confString[RUNFILE] = m_confString[OUTPUT_LOG] = "";
	m_confBool[LOGIN_ONLY_LOGINSERVER] = m_confBool[START_CLOSED] = m_confBool[DAEMONIZE] = false;
	m_confBool[SCRIPT_SYSTEM] = true;
}

bool ConfigManager::load()
{
	if(L)
		lua_close(L);

	L = luaL_newstate();
	if(!L)
		return false;

	luaL_openlibs(L);
	if(luaL_dofile(L, m_confString[CONFIG_FILE].c_str()))
	{
		lua_close(L);
		L = NULL;
		return false;
	}

	if(!m_loaded) //info that must be loaded one time- unless we reset the modules involved
	{
		if(m_confString[DATA_DIRECTORY] == "")
			m_confString[DATA_DIRECTORY] = getGlobalString("dataDirectory", "data/");

		if(m_confString[LOGS_DIRECTORY] == "")
			m_confString[LOGS_DIRECTORY] = getGlobalString("logsDirectory", "logs/");

		if(m_confString[IP] == "")
			m_confString[IP] = getGlobalString("ip", "127.0.0.1");

		if(m_confNumber[LOGIN_PORT] == 0)
			m_confNumber[LOGIN_PORT] = getGlobalNumber("loginPort", 7171);

		if(m_confString[GAME_PORT] == "")
			m_confString[GAME_PORT] = getGlobalString("gamePort", "7172");

		if(m_confNumber[ADMIN_PORT] == 0)
			m_confNumber[ADMIN_PORT] = getGlobalNumber("adminPort", 7171);

		if(m_confNumber[MANAGER_PORT] == 0)
			m_confNumber[MANAGER_PORT] = getGlobalNumber("managerPort", 7171);

		if(m_confNumber[STATUS_PORT] == 0)
			m_confNumber[STATUS_PORT] = getGlobalNumber("statusPort", 7171);

		if(m_confString[RUNFILE] == "")
			m_confString[RUNFILE] = getGlobalString("runFile", "");

		if(m_confString[OUTPUT_LOG] == "")
			m_confString[OUTPUT_LOG] = getGlobalString("outputLog", "");

		m_confBool[TRUNCATE_LOG] = getGlobalBool("truncateLogOnStartup", true);
		#ifdef MULTI_SQL_DRIVERS
		m_confString[SQL_TYPE] = getGlobalString("sqlType", "sqlite");
		#endif
		m_confString[SQL_HOST] = getGlobalString("sqlHost", "localhost");
		m_confNumber[SQL_PORT] = getGlobalNumber("sqlPort", 3306);
		m_confString[SQL_DB] = getGlobalString("sqlDatabase", "theotxserver");
		m_confString[SQL_USER] = getGlobalString("sqlUser", "root");
		m_confString[SQL_PASS] = getGlobalString("sqlPass", "");
		m_confString[SQL_FILE] = getGlobalString("sqlFile", "otxserver.s3db");
		m_confNumber[SQL_KEEPALIVE] = getGlobalNumber("sqlKeepAlive", 0);
		m_confNumber[MYSQL_READ_TIMEOUT] = getGlobalNumber("mysqlReadTimeout", 10);
		m_confNumber[MYSQL_WRITE_TIMEOUT] = getGlobalNumber("mysqlWriteTimeout", 10);
		m_confBool[OPTIMIZE_DATABASE] = getGlobalBool("startupDatabaseOptimization", true);
		m_confString[MAP_NAME] = getGlobalString("mapName", "forgotten.otbm");
		m_confBool[GLOBALSAVE_ENABLED] = getGlobalBool("globalSaveEnabled", true);
		m_confNumber[GLOBALSAVE_H] = getGlobalNumber("globalSaveHour", 8);
		m_confNumber[GLOBALSAVE_M] = getGlobalNumber("globalSaveMinute", 0);
		m_confString[HOUSE_RENT_PERIOD] = getGlobalString("houseRentPeriod", "monthly");
		m_confNumber[WORLD_ID] = getGlobalNumber("worldId", 0);
		m_confBool[RANDOMIZE_TILES] = getGlobalBool("randomizeTiles", true);
		m_confBool[STORE_TRASH] = getGlobalBool("storeTrash", true);
		m_confString[DEFAULT_PRIORITY] = getGlobalString("defaultPriority", "high");
		m_confBool[BIND_ONLY_GLOBAL_ADDRESS] = getGlobalBool("bindOnlyGlobalAddress", false);
		m_confBool[GUILD_HALLS] = getGlobalBool("guildHalls", false);
		#ifndef __LOGIN_SERVER__
		m_confBool[LOGIN_ONLY_LOGINSERVER] = getGlobalBool("loginOnlyWithLoginServer", false);
		#endif
		m_confString[ENCRYPTION_TYPE] = getGlobalString("encryptionType", "sha1");
	}

	m_confString[MAP_AUTHOR] = getGlobalString("mapAuthor", "Unknown");
	m_confNumber[LOGIN_TRIES] = getGlobalNumber("loginTries", 3);
	m_confNumber[RETRY_TIMEOUT] = getGlobalNumber("retryTimeout", 30000);
	m_confNumber[LOGIN_TIMEOUT] = getGlobalNumber("loginTimeout", 5000);
	m_confNumber[MAX_MESSAGEBUFFER] = getGlobalNumber("maxMessageBuffer", 4);
	m_confNumber[MAX_PLAYERS] = getGlobalNumber("maxPlayers", 1000);
	m_confNumber[DEFAULT_DESPAWNRANGE] = getGlobalNumber("deSpawnRange", 2);
	m_confNumber[DEFAULT_DESPAWNRADIUS] = getGlobalNumber("deSpawnRadius", 50);
	m_confNumber[PZ_LOCKED] = getGlobalNumber("pzLocked", 60000);
	m_confNumber[HUNTING_DURATION] = getGlobalNumber("huntingDuration", 60000);
	m_confString[SERVER_NAME] = getGlobalString("serverName");
	m_confString[OWNER_NAME] = getGlobalString("ownerName");
	m_confString[OWNER_EMAIL] = getGlobalString("ownerEmail");
	m_confString[URL] = getGlobalString("url");
	m_confString[LOCATION] = getGlobalString("location");
	m_confString[MOTD] = getGlobalString("motd");
	m_confNumber[ALLOW_CLONES] = getGlobalNumber("allowClones", 0);
	m_confBool[EXPERIENCE_STAGES] = getGlobalBool("experienceStages", false);
	m_confDouble[RATE_EXPERIENCE] = getGlobalDouble("rateExperience", 1);
	m_confDouble[RATE_SKILL] = getGlobalDouble("rateSkill", 1);
	m_confDouble[RATE_SKILL_OFFLINE] = getGlobalDouble("rateSkillOffline", 0.5);
	m_confDouble[RATE_MAGIC] = getGlobalDouble("rateMagic", 1);
	m_confDouble[RATE_MAGIC_OFFLINE] = getGlobalDouble("rateMagicOffline", 0.5);
	m_confDouble[RATE_LOOT] = getGlobalDouble("rateLoot", 1);
	m_confNumber[RATE_SPAWN_MIN] = getGlobalNumber("rateSpawnMin", 1);
	m_confNumber[RATE_SPAWN_MAX] = getGlobalNumber("rateSpawnMax", 1);
	m_confNumber[PARTY_RADIUS_X] = getGlobalNumber("experienceShareRadiusX", 30);
	m_confNumber[PARTY_RADIUS_Y] = getGlobalNumber("experienceShareRadiusY", 30);
	m_confNumber[PARTY_RADIUS_Z] = getGlobalNumber("experienceShareRadiusZ", 1);
	m_confDouble[PARTY_DIFFERENCE] = getGlobalDouble("experienceShareLevelDifference", (double)2 / 3);
	m_confNumber[SPAWNPOS_X] = getGlobalNumber("newPlayerSpawnPosX", 100);
	m_confNumber[SPAWNPOS_Y] = getGlobalNumber("newPlayerSpawnPosY", 100);
	m_confNumber[SPAWNPOS_Z] = getGlobalNumber("newPlayerSpawnPosZ", 7);
	m_confNumber[SPAWNTOWN_ID] = getGlobalNumber("newPlayerTownId", 1);
	m_confString[WORLD_TYPE] = getGlobalString("worldType", "open");
	m_confBool[ACCOUNT_MANAGER] = getGlobalBool("accountManager", true);
	m_confBool[NAMELOCK_MANAGER] = getGlobalBool("namelockManager", false);
	m_confNumber[START_LEVEL] = getGlobalNumber("newPlayerLevel", 1);
	m_confNumber[START_MAGICLEVEL] = getGlobalNumber("newPlayerMagicLevel", 0);
	m_confBool[START_CHOOSEVOC] = getGlobalBool("newPlayerChooseVoc", false);
	m_confNumber[HOUSE_PRICE] = getGlobalNumber("housePriceEachSquare", 1000);
	m_confNumber[WHITE_SKULL_TIME] = getGlobalNumber("whiteSkullTime", 900000);
	m_confBool[ON_OR_OFF_CHARLIST] = getGlobalBool("displayOnOrOffAtCharlist", false);
	m_confBool[ALLOW_CHANGEOUTFIT] = getGlobalBool("allowChangeOutfit", true);
	m_confBool[ONE_PLAYER_ON_ACCOUNT] = getGlobalBool("onePlayerOnlinePerAccount", true);
	m_confBool[CANNOT_ATTACK_SAME_LOOKFEET] = getGlobalBool("noDamageToSameLookfeet", false);
	m_confBool[AIMBOT_HOTKEY_ENABLED] = getGlobalBool("hotkeyAimbotEnabled", true);
	m_confNumber[ACTIONS_DELAY_INTERVAL] = getGlobalNumber("timeBetweenActions", 200);
	m_confNumber[EX_ACTIONS_DELAY_INTERVAL] = getGlobalNumber("timeBetweenExActions", 1000);
	m_confNumber[CUSTOM_ACTIONS_DELAY_INTERVAL] = getGlobalNumber("timeBetweenCustomActions", 500);
	m_confNumber[CRITICAL_HIT_CHANCE] = getGlobalNumber("criticalHitChance", 5);
	m_confBool[REMOVE_WEAPON_AMMO] = getGlobalBool("removeWeaponAmmunition", true);
	m_confBool[REMOVE_WEAPON_CHARGES] = getGlobalBool("removeWeaponCharges", true);
	m_confBool[REMOVE_RUNE_CHARGES] = getGlobalBool("removeRuneCharges", true);
	m_confDouble[RATE_PVP_EXPERIENCE] = getGlobalDouble("rateExperienceFromPlayers", 0);
	m_confDouble[EFP_MIN_THRESHOLD] = getGlobalDouble("minLevelThresholdForKilledPlayer", 0.9f);
	m_confDouble[EFP_MAX_THRESHOLD] = getGlobalDouble("maxLevelThresholdForKilledPlayer", 1.1f);
	m_confBool[SHUTDOWN_AT_GLOBALSAVE] = getGlobalBool("shutdownAtGlobalSave", false);
	m_confBool[CLEAN_MAP_AT_GLOBALSAVE] = getGlobalBool("cleanMapAtGlobalSave", true);
	m_confBool[FREE_PREMIUM] = getGlobalBool("freePremium", false);
	m_confNumber[PROTECTION_LEVEL] = getGlobalNumber("protectionLevel", 1);
	m_confNumber[STATUSQUERY_TIMEOUT] = getGlobalNumber("statusTimeout", 300000);
	m_confBool[BROADCAST_BANISHMENTS] = getGlobalBool("broadcastBanishments", true);
	m_confBool[GENERATE_ACCOUNT_NUMBER] = getGlobalBool("generateAccountNumber", false);
	m_confBool[GENERATE_ACCOUNT_SALT] = getGlobalBool("generateAccountSalt", false);
	m_confBool[INGAME_GUILD_MANAGEMENT] = getGlobalBool("ingameGuildManagement", true);
	m_confBool[EXTERNAL_GUILD_WARS_MANAGEMENT] = getGlobalBool("externalGuildWarsManagement", false);
	m_confNumber[LEVEL_TO_FORM_GUILD] = getGlobalNumber("levelToFormGuild", 8);
	m_confNumber[MIN_GUILDNAME] = getGlobalNumber("guildNameMinLength", 4);
	m_confNumber[MAX_GUILDNAME] = getGlobalNumber("guildNameMaxLength", 20);
	m_confNumber[LEVEL_TO_BUY_HOUSE] = getGlobalNumber("levelToBuyHouse", 1);
	m_confNumber[HOUSES_PER_ACCOUNT] = getGlobalNumber("housesPerAccount", 0);
	m_confBool[HOUSE_BUY_AND_SELL] = getGlobalBool("buyableAndSellableHouses", true);
	m_confBool[REPLACE_KICK_ON_LOGIN] = getGlobalBool("replaceKickOnLogin", true);
	m_confBool[HOUSE_NEED_PREMIUM] = getGlobalBool("houseNeedPremium", true);
	m_confBool[HOUSE_RENTASPRICE] = getGlobalBool("houseRentAsPrice", false);
	m_confBool[HOUSE_PRICEASRENT] = getGlobalBool("housePriceAsRent", false);
	m_confString[HOUSE_STORAGE] = getGlobalString("houseDataStorage", "binary");
	m_confNumber[RED_SKULL_LENGTH] = getGlobalNumber("redSkullLength", 2592000);
	m_confNumber[MAX_VIOLATIONCOMMENT_SIZE] = getGlobalNumber("maxViolationCommentSize", 60);
	m_confNumber[BLACK_SKULL_LENGTH] = getGlobalNumber("blackSkullLength", 3888000);
	m_confNumber[NOTATIONS_TO_BAN] = getGlobalNumber("notationsToBan", 3);
	m_confNumber[WARNINGS_TO_FINALBAN] = getGlobalNumber("warningsToFinalBan", 4);
	m_confNumber[WARNINGS_TO_DELETION] = getGlobalNumber("warningsToDeletion", 5);
	m_confNumber[BAN_LENGTH] = getGlobalNumber("banLength", 604800);
	m_confNumber[KILLS_BAN_LENGTH] = getGlobalNumber("killsBanLength", 604800);
	m_confNumber[FINALBAN_LENGTH] = getGlobalNumber("finalBanLength", 2592000);
	m_confNumber[IPBAN_LENGTH] = getGlobalNumber("ipBanLength", 86400);
	m_confBool[BANK_SYSTEM] = getGlobalBool("bankSystem", true);
	m_confBool[PREMIUM_FOR_PROMOTION] = getGlobalBool("premiumForPromotion", true);
	m_confBool[INIT_PREMIUM_UPDATE] = getGlobalBool("updatePremiumStateAtStartup", true);
	m_confBool[SHOW_HEALTH_CHANGE] = getGlobalBool("showHealthChange", true);
	m_confBool[SHOW_MANA_CHANGE] = getGlobalBool("showManaChange", true);
	m_confBool[TELEPORT_SUMMONS] = getGlobalBool("teleportAllSummons", false);
	m_confBool[TELEPORT_PLAYER_SUMMONS] = getGlobalBool("teleportPlayerSummons", false);
	m_confBool[PVP_TILE_IGNORE_PROTECTION] = getGlobalBool("pvpTileIgnoreLevelAndVocationProtection", true);
	m_confBool[DISPLAY_CRITICAL_HIT] = getGlobalBool("displayCriticalHitNotify", false);
	m_confBool[CLEAN_PROTECTED_ZONES] = getGlobalBool("cleanProtectedZones", true);
	m_confBool[SPELL_NAME_INSTEAD_WORDS] = getGlobalBool("spellNameInsteadOfWords", false);
	m_confBool[EMOTE_SPELLS] = getGlobalBool("emoteSpells", false);
	m_confNumber[MAX_PLAYER_SUMMONS] = getGlobalNumber("maxPlayerSummons", 2);
	m_confBool[SAVE_GLOBAL_STORAGE] = getGlobalBool("saveGlobalStorage", true);
	m_confBool[SAVE_PLAYER_DATA] = getGlobalBool("savePlayerData", true);
	m_confBool[FORCE_CLOSE_SLOW_CONNECTION] = getGlobalBool("forceSlowConnectionsToDisconnect", false);
	m_confBool[BLESSINGS] = getGlobalBool("blessings", true);
	m_confBool[BLESSING_ONLY_PREMIUM] = getGlobalBool("blessingOnlyPremium", true);
	m_confBool[BED_REQUIRE_PREMIUM] = getGlobalBool("bedsRequirePremium", true);
	m_confNumber[FIELD_OWNERSHIP] = getGlobalNumber("fieldOwnershipDuration", 5000);
	m_confBool[ALLOW_CHANGECOLORS] = getGlobalBool("allowChangeColors", true);
	m_confBool[STOP_ATTACK_AT_EXIT] = getGlobalBool("stopAttackingAtExit", false);
	m_confNumber[EXTRA_PARTY_PERCENT] = getGlobalNumber("extraPartyExperiencePercent", 5);
	m_confNumber[EXTRA_PARTY_LIMIT] = getGlobalNumber("extraPartyExperienceLimit", 20);
	m_confBool[DISABLE_OUTFITS_PRIVILEGED] = getGlobalBool("disableOutfitsForPrivilegedPlayers", false);
	m_confBool[TRACER_BOX] = getGlobalBool("promptExceptionTracerErrorBox", true);
	m_confNumber[LOGIN_PROTECTION] = getGlobalNumber("loginProtectionPeriod", 10000);
	m_confBool[STORE_DIRECTION] = getGlobalBool("storePlayerDirection", false);
	m_confNumber[PLAYER_DEEPNESS] = getGlobalNumber("playerQueryDeepness", -1);
	m_confDouble[CRITICAL_HIT_MUL] = getGlobalDouble("criticalHitMultiplier", 1);
	m_confNumber[STAIRHOP_DELAY] = getGlobalNumber("stairhopDelay", 2 * 1000);
	m_confNumber[RATE_STAMINA_LOSS] = getGlobalNumber("rateStaminaLoss", 1);
	m_confDouble[RATE_STAMINA_GAIN] = getGlobalDouble("rateStaminaGain", 3);
	m_confDouble[RATE_STAMINA_THRESHOLD] = getGlobalDouble("rateStaminaThresholdGain", 12);
	m_confDouble[RATE_STAMINA_ABOVE] = getGlobalDouble("rateStaminaAboveNormal", 1.5f);
	m_confDouble[RATE_STAMINA_UNDER] = getGlobalDouble("rateStaminaUnderNormal", 0.5f);
	m_confNumber[STAMINA_LIMIT_TOP] = getGlobalNumber("staminaRatingLimitTop", 2460);
	m_confNumber[STAMINA_LIMIT_BOTTOM] = getGlobalNumber("staminaRatingLimitBottom", 840);
	m_confBool[DISPLAY_LOGGING] = getGlobalBool("displayPlayersLogging", true);
	m_confBool[STAMINA_BONUS_PREMIUM] = getGlobalBool("staminaThresholdOnlyPremium", true);
	m_confNumber[BLESS_REDUCTION_BASE] = getGlobalNumber("blessingReductionBase", 30);
	m_confNumber[BLESS_REDUCTION_DECREMENT] = getGlobalNumber("blessingReductionDecrement", 5);
	m_confBool[ALLOW_CHANGEADDONS] = getGlobalBool("allowChangeAddons", true);
	m_confNumber[BLESS_REDUCTION] = getGlobalNumber("eachBlessReduction", 8);
	m_confDouble[FORMULA_LEVEL] = getGlobalDouble("formulaLevel", 5.0);
	m_confDouble[FORMULA_MAGIC] = getGlobalDouble("formulaMagic", 1.0);
	m_confString[PREFIX_CHANNEL_LOGS] = getGlobalString("prefixChannelLogs", "");
	m_confBool[GHOST_INVISIBLE_EFFECT] = getGlobalBool("ghostModeInvisibleEffect", false);
	m_confString[CORES_USED] = getGlobalString("coresUsed", "-1");
	m_confNumber[NICE_LEVEL] = getGlobalNumber("niceLevel", 5);
	m_confNumber[EXPERIENCE_COLOR] = getGlobalNumber("gainExperienceColor", COLOR_WHITE);
	m_confBool[SHOW_HEALTH_CHANGE_MONSTER] = getGlobalBool("showHealthChangeForMonsters", true);
	m_confBool[SHOW_MANA_CHANGE_MONSTER] = getGlobalBool("showManaChangeForMonsters", true);
	m_confBool[CHECK_CORPSE_OWNER] = getGlobalBool("checkCorpseOwner", true);
	m_confBool[BUFFER_SPELL_FAILURE] = getGlobalBool("bufferMutedOnSpellFailure", false);
	m_confNumber[GUILD_PREMIUM_DAYS] = getGlobalNumber("premiumDaysToFormGuild", 0);
	m_confNumber[PUSH_CREATURE_DELAY] = getGlobalNumber("pushCreatureDelay", 2000);
	m_confNumber[DEATH_CONTAINER] = getGlobalNumber("deathContainerId", 1987);
	m_confBool[PREMIUM_SKIP_WAIT] = getGlobalBool("premiumPlayerSkipWaitList", false);
	m_confNumber[MAXIMUM_DOOR_LEVEL] = getGlobalNumber("maximumDoorLevel", 500);
	m_confBool[DEATH_LIST] = getGlobalBool("deathListEnabled", true);
	m_confNumber[DEATH_ASSISTS] = getGlobalNumber("deathAssistCount", 1);
	m_confNumber[FRAG_LIMIT] = getGlobalNumber("fragsLimit", 86400);
	m_confNumber[FRAG_SECOND_LIMIT] = getGlobalNumber("fragsSecondLimit", 604800);
	m_confNumber[FRAG_THIRD_LIMIT] = getGlobalNumber("fragsThirdLimit", 2592000);
	m_confNumber[RED_LIMIT] = getGlobalNumber("fragsToRedSkull", 3);
	m_confNumber[RED_SECOND_LIMIT] = getGlobalNumber("fragsSecondToRedSkull", 5);
	m_confNumber[RED_THIRD_LIMIT] = getGlobalNumber("fragsThirdToRedSkull", 10);
	m_confNumber[BLACK_LIMIT] = getGlobalNumber("fragsToBlackSkull", m_confNumber[RED_LIMIT]);
	m_confNumber[BLACK_SECOND_LIMIT] = getGlobalNumber("fragsSecondToBlackSkull", m_confNumber[RED_SECOND_LIMIT]);
	m_confNumber[BLACK_THIRD_LIMIT] = getGlobalNumber("fragsThirdToBlackSkull", m_confNumber[RED_THIRD_LIMIT]);
	m_confNumber[BAN_LIMIT] = getGlobalNumber("fragsToBanishment", m_confNumber[RED_LIMIT]);
	m_confNumber[BAN_SECOND_LIMIT] = getGlobalNumber("fragsSecondToBanishment", m_confNumber[RED_SECOND_LIMIT]);
	m_confNumber[BAN_THIRD_LIMIT] = getGlobalNumber("fragsThirdToBanishment", m_confNumber[RED_THIRD_LIMIT]);
	m_confNumber[BLACK_SKULL_DEATH_HEALTH] = getGlobalNumber("blackSkulledDeathHealth", 40);
	m_confNumber[BLACK_SKULL_DEATH_MANA] = getGlobalNumber("blackSkulledDeathMana", 0);
	m_confNumber[DEATHLIST_REQUIRED_TIME] = getGlobalNumber("deathListRequiredTime", 60000);
	m_confNumber[EXPERIENCE_SHARE_ACTIVITY] = getGlobalNumber("experienceShareActivity", 120000);
	m_confBool[GHOST_SPELL_EFFECTS] = getGlobalBool("ghostModeSpellEffects", true);
	m_confBool[PVPZONE_ADDMANASPENT] = getGlobalBool("addManaSpentInPvPZone", true);
	m_confBool[PVPZONE_RECOVERMANA] = getGlobalBool("recoverManaAfterDeathInPvPZone", false);
	m_confNumber[TILE_LIMIT] = getGlobalNumber("tileLimit", 0);
	m_confNumber[PROTECTION_TILE_LIMIT] = getGlobalNumber("protectionTileLimit", 0);
	m_confNumber[HOUSE_TILE_LIMIT] = getGlobalNumber("houseTileLimit", 0);
	m_confNumber[TRADE_LIMIT] = getGlobalNumber("tradeLimit", 100);
	m_confString[MAILBOX_DISABLED_TOWNS] = getGlobalString("mailboxDisabledTowns", "");
	m_confNumber[SQUARE_COLOR] = getGlobalNumber("squareColor", 0);
	m_confBool[USE_BLACK_SKULL] = getGlobalBool("useBlackSkull", true);
	m_confBool[USE_FRAG_HANDLER] = getGlobalBool("useFragHandler", true);
	m_confNumber[LOOT_MESSAGE] = getGlobalNumber("monsterLootMessage", 3);
	m_confNumber[LOOT_MESSAGE_TYPE] = getGlobalNumber("monsterLootMessageType", 19);
	m_confNumber[NAME_REPORT_TYPE] = getGlobalNumber("violationNameReportActionType", 2);
	m_confNumber[HOUSE_CLEAN_OLD] = getGlobalNumber("houseCleanOld", 0);
	m_confBool[VIPLIST_PER_PLAYER] = getGlobalBool("separateVipListPerCharacter", false);
	m_confDouble[RATE_MONSTER_HEALTH] = getGlobalDouble("rateMonsterHealth", 1);
	m_confDouble[RATE_MONSTER_MANA] = getGlobalDouble("rateMonsterMana", 1);
	m_confDouble[RATE_MONSTER_ATTACK] = getGlobalDouble("rateMonsterAttack", 1);
	m_confDouble[RATE_MONSTER_DEFENSE] = getGlobalDouble("rateMonsterDefense", 1);
	m_confBool[MANAGER_LOCALHOST_ONLY] = getGlobalBool("managerLocalhostOnly", true);
	m_confNumber[MANAGER_CONNECTIONS_LIMIT] = getGlobalNumber("managerConnectionsLimit", 1);
	m_confString[MANAGER_PASSWORD] = getGlobalString("managerPassword", "");
	m_confBool[MANAGER_LOGS] = getGlobalBool("managerLogs", false);
	m_confBool[ADMIN_LOGS] = getGlobalBool("adminLogs", false);
	m_confString[ADMIN_PASSWORD] = getGlobalString("adminPassword", "");
	m_confNumber[ADMIN_CONNECTIONS_LIMIT] = getGlobalNumber("adminConnectionsLimit", 1);
	m_confBool[ADMIN_LOCALHOST_ONLY] = getGlobalBool("adminLocalhostOnly", true);
	m_confBool[ADMIN_REQUIRE_LOGIN] = getGlobalBool("adminRequireLogin", true);
	m_confString[ADMIN_ENCRYPTION] = getGlobalString("adminEncryption", "");
	m_confString[ADMIN_ENCRYPTION_DATA] = getGlobalString("adminEncryptionData", "");
	m_confBool[ADDONS_PREMIUM] = getGlobalBool("addonsOnlyPremium", true);
	m_confBool[UNIFIED_SPELLS] = getGlobalBool("unifiedSpells", true);
	m_confBool[OPTIONAL_WAR_ATTACK_ALLY] = getGlobalBool("optionalWarAttackableAlly", false);
	m_confNumber[VIPLIST_DEFAULT_LIMIT] = getGlobalNumber("vipListDefaultLimit", 20);
	m_confNumber[VIPLIST_DEFAULT_PREMIUM_LIMIT]	= getGlobalNumber("vipListDefaultPremiumLimit", 100);
	m_confNumber[STAMINA_DESTROY_LOOT] = getGlobalNumber("staminaLootLimit", 840);
	m_confNumber[FIST_BASE_ATTACK] = getGlobalNumber("fistBaseAttack", 7);
	m_confBool[MONSTER_SPAWN_WALKBACK] = getGlobalBool("monsterSpawnWalkback", true);
	m_confNumber[PVP_BLESSING_THRESHOLD] = getGlobalNumber("pvpBlessingThreshold", 40);
	m_confNumber[FAIRFIGHT_TIMERANGE] = getGlobalNumber("fairFightTimeRange", 60);
	m_confNumber[DEFAULT_DEPOT_SIZE_PREMIUM] = getGlobalNumber("defaultDepotSizePremium", 2000);
	m_confNumber[DEFAULT_DEPOT_SIZE] = getGlobalNumber("defaultDepotSize", 2000);
	m_confBool[USE_CAPACITY] = getGlobalBool("useCapacity", true);
	m_confBool[DAEMONIZE] = getGlobalBool("daemonize", false);
	m_confBool[SKIP_ITEMS_VERSION] = getGlobalBool("skipItemsVersionCheck", false);
	m_confBool[SILENT_LUA] = getGlobalBool("disableLuaErrors", false);
	m_confNumber[MAIL_ATTEMPTS] = getGlobalNumber("mailMaxAttempts", 20);
	m_confNumber[MAIL_BLOCK] = getGlobalNumber("mailBlockPeriod", 3600000);
	m_confNumber[MAIL_ATTEMPTS_FADE] = getGlobalNumber("mailAttemptsFadeTime", 600000);
	m_confBool[ROOK_SYSTEM] = getGlobalBool("useRookSystem", false);
	m_confNumber[ROOK_TOWN] = getGlobalNumber("rookTownId", 1);
	m_confNumber[ROOK_LEVELTO] = getGlobalNumber("rookLevelToGetRooked", 5);
	m_confNumber[ROOK_TOLEVEL] = getGlobalNumber("rookLevelToLeaveRook", 8);
	m_confBool[HOUSE_SKIP_INIT_RENT] = getGlobalBool("houseSkipInitialRent", true);
	m_confBool[HOUSE_PROTECTION] = getGlobalBool("houseProtection", true);
	m_confBool[FAIRFIGHT_REDUCTION] = getGlobalBool("useFairfightReduction", true);
	m_confNumber[MYSQL_RECONNECTION_ATTEMPTS] = getGlobalNumber("mysqlReconnectionAttempts", 3);
	m_confBool[ALLOW_BLOCK_SPAWN] = getGlobalBool("allowBlockSpawn", true);
	m_confNumber[FOLLOW_EXHAUST] = getGlobalNumber("playerFollowExhaust", 2000);
	m_confBool[MULTIPLE_NAME] = getGlobalBool("multipleNames", false);
	m_confNumber[MAX_PACKETS_PER_SECOND] = getGlobalNumber("packetsPerSecond", 50);
	m_confBool[SAVE_STATEMENT] = getGlobalBool("logPlayersStatements", true);
	m_confNumber[GUI_PREMIUM_DAYS] = getGlobalNumber("premiumDaysToAddByGui", 30);
	m_confBool[MANUAL_ADVANCED_CONFIG] = getGlobalBool("manualVersionConfig", false);
	m_confNumber[VERSION_MIN] = getGlobalNumber("versionMin", CLIENT_VERSION_MIN);
	m_confNumber[VERSION_MAX] = getGlobalNumber("versionMax", CLIENT_VERSION_MAX);
	m_confString[VERSION_MSG] = getGlobalString("versionMsg", "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");
	m_confBool[USE_RUNE_REQUIREMENTS] = getGlobalBool("useRunesRequirements", true);
	m_confNumber[HIGHSCORES_TOP] = getGlobalNumber("highscoreDisplayPlayers", 10);
	m_confNumber[HIGHSCORES_UPDATETIME] = getGlobalNumber("updateHighscoresAfterMinutes", 60);
	m_confNumber[LOGIN_PROTECTION_TIME] = getGlobalNumber("loginProtectionTime", 10);
	m_confBool[CLASSIC_EQUIPMENT_SLOTS] = getGlobalBool("classicEquipmentSlots", false);
	m_confBool[NO_ATTACKHEALING_SIMULTANEUS] = getGlobalBool("noAttackHealingSimultaneus", false);
	m_confBool[OPTIONAL_PROTECTION] = getGlobalBool("optionalProtection", true);
	m_confBool[MONSTER_ATTACK_MONSTER] = getGlobalBool("monsterAttacksOnlyDamagePlayers", true);
	m_confBool[ALLOW_CORPSE_BLOCK] = getGlobalBool("allowCorpseBlock", false);
	m_confBool[ALLOW_INDEPENDENT_PUSH] = getGlobalBool("allowIndependentCreaturePush", true);
	m_confBool[PZLOCK_ON_ATTACK_SKULLED_PLAYERS] = getGlobalBool("pzlockOnAttackSkulledPlayers", false);
	m_loaded = true;
	return true;
}

bool ConfigManager::reload()
{
	if(!m_loaded)
		return false;

	uint32_t tmp = m_confNumber[HOUSE_PRICE];
	if(!load())
		return false;

	if((uint32_t)m_confNumber[HOUSE_PRICE] == tmp)
		return true;

	for(HouseMap::iterator it = Houses::getInstance()->getHouseBegin();
		it != Houses::getInstance()->getHouseEnd(); ++it)
	{
		uint32_t price = it->second->getTilesCount() * m_confNumber[HOUSE_PRICE];
		if(m_confBool[HOUSE_RENTASPRICE])
		{
			uint32_t rent = it->second->getRent();
			if(!m_confBool[HOUSE_PRICEASRENT] && it->second->getPrice() != rent)
				price = rent;
		}

		it->second->setPrice(price);
		if(m_confBool[HOUSE_PRICEASRENT])
			it->second->setRent(price);

		if(!it->second->getOwner())
			it->second->updateDoorDescription();
	}

	return true;
}

const std::string& ConfigManager::getString(uint32_t _what) const
{
	if((m_loaded && _what < LAST_STRING_CONFIG) || _what <= CONFIG_FILE)
		return m_confString[_what];

	if(!m_startup)
		std::clog << "[Warning - ConfigManager::getString] " << _what << std::endl;

	return m_confString[DUMMY_STR];
}

bool ConfigManager::getBool(uint32_t _what) const
{
	if(m_loaded && _what < LAST_BOOL_CONFIG)
		return m_confBool[_what];

	if(!m_startup)
		std::clog << "[Warning - ConfigManager::getBool] " << _what << std::endl;

	return false;
}

int64_t ConfigManager::getNumber(uint32_t _what) const
{
	if(m_loaded && _what < LAST_NUMBER_CONFIG)
		return m_confNumber[_what];

	if(!m_startup)
		std::clog << "[Warning - ConfigManager::getNumber] " << _what << std::endl;

	return 0;
}

double ConfigManager::getDouble(uint32_t _what) const
{
	if(m_loaded && _what < LAST_DOUBLE_CONFIG)
		return m_confDouble[_what];

	if(!m_startup)
		std::clog << "[Warning - ConfigManager::getDouble] " << _what << std::endl;

	return 0;
}

bool ConfigManager::setString(uint32_t _what, const std::string& _value)
{
	if(_what < LAST_STRING_CONFIG)
	{
		m_confString[_what] = _value;
		return true;
	}

	std::clog << "[Warning - ConfigManager::setString] " << _what << std::endl;
	return false;
}

bool ConfigManager::setNumber(uint32_t _what, int64_t _value)
{
	if(_what < LAST_NUMBER_CONFIG)
	{
		m_confNumber[_what] = _value;
		return true;
	}

	std::clog << "[Warning - ConfigManager::setNumber] " << _what << std::endl;
	return false;
}

bool ConfigManager::setBool(uint32_t _what, bool _value)
{
	if(_what < LAST_BOOL_CONFIG)
	{
		m_confBool[_what] = _value;
		return true;
	}

	std::clog << "[Warning - ConfigManager::setBool] " << _what << std::endl;
	return false;
}
