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

#ifndef __CONST__
#define __CONST__
#include "definitions.h"

enum OperatingSystem_t
{
	CLIENTOS_LINUX = 1,
	CLIENTOS_WINDOWS = 2,

	CLIENTOS_OTCLIENT_LINUX = 10,
	CLIENTOS_OTCLIENT_WINDOWS = 11,
	CLIENTOS_OTCLIENT_MAC = 12
};

enum ReportType_t
{
	REPORT_NAME = 0,
	REPORT_STATEMENT = 1,
	REPORT_BOT = 2
};

enum MagicEffect_t
{
	MAGIC_EFFECT_DRAW_BLOOD = 0,
	MAGIC_EFFECT_LOSE_ENERGY = 1,
	MAGIC_EFFECT_POFF = 2,
	MAGIC_EFFECT_BLOCKHIT = 3,
	MAGIC_EFFECT_EXPLOSION_AREA = 4,
	MAGIC_EFFECT_EXPLOSION_DAMAGE = 5,
	MAGIC_EFFECT_FIRE_AREA = 6,
	MAGIC_EFFECT_YELLOW_RINGS = 7,
	MAGIC_EFFECT_POISON_RINGS = 8,
	MAGIC_EFFECT_HIT_AREA = 9,
	MAGIC_EFFECT_TELEPORT = 10,
	MAGIC_EFFECT_ENERGY_DAMAGE = 11,
	MAGIC_EFFECT_WRAPS_BLUE = 12,
	MAGIC_EFFECT_WRAPS_RED = 13,
	MAGIC_EFFECT_WRAPS_GREEN = 14,
	MAGIC_EFFECT_HITBY_FIRE = 15,
	MAGIC_EFFECT_POISON = 16,
	MAGIC_EFFECT_MORT_AREA = 17,
	MAGIC_EFFECT_SOUND_GREEN = 18,
	MAGIC_EFFECT_SOUND_RED = 19,
	MAGIC_EFFECT_POISON_AREA = 20,
	MAGIC_EFFECT_SOUND_YELLOW = 21,
	MAGIC_EFFECT_SOUND_PURPLE = 22,
	MAGIC_EFFECT_SOUND_BLUE = 23,
	MAGIC_EFFECT_SOUND_WHITE = 24,
	MAGIC_EFFECT_LAST = MAGIC_EFFECT_SOUND_WHITE,

	//for internal use, dont send to client
	MAGIC_EFFECT_NONE = 0xFF,
	MAGIC_EFFECT_UNKNOWN = 0xFFFF
};

enum ShootEffect_t
{
	SHOOT_EFFECT_SPEAR = 0,
	SHOOT_EFFECT_BOLT = 1,
	SHOOT_EFFECT_ARROW = 2,
	SHOOT_EFFECT_FIRE = 3,
	SHOOT_EFFECT_ENERGY = 4,
	SHOOT_EFFECT_POISONARROW = 5,
	SHOOT_EFFECT_BURSTARROW = 6,
	SHOOT_EFFECT_THROWINGSTAR = 7,
	SHOOT_EFFECT_THROWINGKNIFE = 8,
	SHOOT_EFFECT_SMALLSTONE = 9,
	SHOOT_EFFECT_DEATH = 10,
	SHOOT_EFFECT_LARGEROCK = 11,
	SHOOT_EFFECT_SNOWBALL = 12,
	SHOOT_EFFECT_POWERBOLT = 13,
	SHOOT_EFFECT_POISONFIELD = 14,
	SHOOT_EFFECT_LAST = SHOOT_EFFECT_POISONFIELD,

	//for internal use, dont send to client
	SHOOT_EFFECT_WEAPONTYPE = 0xFE, //254
	SHOOT_EFFECT_NONE = 0xFF,
	SHOOT_EFFECT_UNKNOWN = 0xFFFF
};

enum MessageClasses
{
	MSG_NONE = 0,
	MSG_SPEAK_SAY = 1,
	MSG_SPEAK_WHISPER = 2,
	MSG_SPEAK_YELL = 3,
	MSG_PRIVATE = 4,
	MSG_CHANNEL = 5,
	MSG_RVR_CHANNEL = 6,
	MSG_RVR_ANSWER = 7,
	MSG_RVR_CONTINUE = 8,
	MSG_GAMEMASTER_BROADCAST = 9,
	MSG_GAMEMASTER_CHANNEL = 10,
	MSG_GAMEMASTER_PRIVATE = 11,
	MSG_CHANNEL_HIGHLIGHT = 12,
	MSG_CHANNEL_MANAGEMENT = 13,
	MSG_GAMEMASTER_ANONYMOUS = 14,
	MSG_SPEAK_MONSTER_SAY = 16,
	MSG_SPEAK_MONSTER_YELL = 17,
	MSG_SPEAK_FIRST = MSG_SPEAK_SAY,
	MSG_SPEAK_LAST = MSG_GAMEMASTER_ANONYMOUS,
	MSG_SPEAK_MONSTER_FIRST = MSG_SPEAK_MONSTER_SAY,
	MSG_SPEAK_MONSTER_LAST = MSG_SPEAK_MONSTER_YELL,

	MSG_EVENT_ORANGE = 16, /*Orange message in the console*/
	MSG_STATUS_CONSOLE_ORANGE = 17, /*Orange message in the console*/
	MSG_STATUS_WARNING = 18, /*Red message in game window and in the console*/
	MSG_EVENT_ADVANCE = 19, /*White message in game window and in the console*/
	MSG_EVENT_DEFAULT = 20, /*White message at the bottom of the game window and in the console*/
	MSG_STATUS_DEFAULT = 21, /*White message at the bottom of the game window and in the console*/
	MSG_INFO_DESCR = 22, /*Green message in game window and in the console*/
	MSG_STATUS_SMALL = 23, /*White message at the bottom of the game window"*/
	MSG_STATUS_CONSOLE_BLUE = 24, /*Blue message in the console*/
	MSG_STATUS_CONSOLE_RED = 25, /*Red message in the console*/

	//for internal use
	MSG_EVENT_GUILD = MSG_INFO_DESCR, /*Green message in game window and in the console*/
	MSG_PARTY_MANAGEMENT = MSG_INFO_DESCR, /*Green message in game window and in the console*/
	MSG_PARTY = MSG_INFO_DESCR /*Green message in game window and in the console*/
};

enum FluidColors_t
{
	FLUID_EMPTY = 0,
	FLUID_BLUE = 1,
	FLUID_RED = 2,
	FLUID_BROWN = 3,
	FLUID_GREEN = 4,
	FLUID_YELLOW = 5,
	FLUID_WHITE = 6,
	FLUID_PURPLE = 7
};

enum FluidTypes_t
{
	FLUID_NONE = FLUID_EMPTY,
	FLUID_WATER = FLUID_BLUE,
	FLUID_BLOOD = FLUID_RED,
	FLUID_BEER = FLUID_BROWN,
	FLUID_SLIME = FLUID_GREEN,
	FLUID_LEMONADE = FLUID_YELLOW,
	FLUID_MILK = FLUID_WHITE,
	FLUID_MANA = FLUID_PURPLE,

	FLUID_LIFE = FLUID_RED + 8,
	FLUID_OIL = FLUID_BROWN + 8,
	FLUID_URINE = FLUID_YELLOW + 8,
	FLUID_COCONUTMILK = FLUID_WHITE + 8,
	FLUID_WINE = FLUID_PURPLE + 8,

	FLUID_MUD = FLUID_BROWN + 16,
	FLUID_FRUITJUICE = FLUID_YELLOW + 16,

	FLUID_LAVA = FLUID_RED + 24,
	FLUID_RUM = FLUID_BROWN + 24,
	FLUID_SWAMP = FLUID_GREEN + 24,

	FLUID_TEA = FLUID_BROWN + 32,
	FLUID_MEAD = FLUID_BROWN + 40
};

const uint8_t reverseFluidMap[] =
{
	FLUID_EMPTY,
	FLUID_WATER,
	FLUID_MANA,
	FLUID_BEER,
	FLUID_EMPTY,
	FLUID_BLOOD,
	FLUID_SLIME,
	FLUID_EMPTY,
	FLUID_LEMONADE,
	FLUID_MILK
};

const uint8_t clientToServerFluidMap[] =
{
	FLUID_EMPTY,
	FLUID_WATER,
	FLUID_MANA,
	FLUID_BEER,
	FLUID_MUD,
	FLUID_BLOOD,
	FLUID_SLIME,
	FLUID_RUM,
	FLUID_LEMONADE,
	FLUID_MILK,
	FLUID_WINE,
	FLUID_LIFE,
	FLUID_URINE,
	FLUID_OIL,
	FLUID_FRUITJUICE,
	FLUID_COCONUTMILK,
	FLUID_TEA,
	FLUID_MEAD
};

enum ClientFluidTypes_t
{
	CLIENTFLUID_EMPTY = 0,
	CLIENTFLUID_BLUE = 1,
	CLIENTFLUID_PURPLE = 2,
	CLIENTFLUID_BROWN_1 = 3,
	CLIENTFLUID_BROWN_2 = 4,
	CLIENTFLUID_RED = 5,
	CLIENTFLUID_GREEN = 6,
	CLIENTFLUID_BROWN = 7
};

const uint8_t fluidMap[] =
{
	CLIENTFLUID_EMPTY,
	CLIENTFLUID_BLUE,
	CLIENTFLUID_PURPLE,
	CLIENTFLUID_BROWN_1,
	CLIENTFLUID_BROWN_2,
	CLIENTFLUID_RED,
	CLIENTFLUID_GREEN,
	CLIENTFLUID_BROWN 
};

enum Color_t
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 5,
	COLOR_GREEN = 18,
	COLOR_LIGHTBLUE = 35,
	COLOR_LIGHTGREEN = 66,
	COLOR_DARKBROWN = 78,
	COLOR_MAYABLUE = 95,
	COLOR_DARKRED = 108,
	COLOR_DARKPURPLE = 112,
	COLOR_BROWN = 120,
	COLOR_GREY = 129,
	COLOR_TEAL = 143,
	COLOR_DARKPINK = 152,
	COLOR_PURPLE = 154,
	COLOR_DARKORANGE = 156,
	COLOR_RED = 180,
	COLOR_PINK = 190,
	COLOR_ORANGE = 192,
	COLOR_DARKYELLOW = 205,
	COLOR_YELLOW = 210,
	COLOR_WHITE = 215,
	COLOR_NONE = 255,
	COLOR_UNKNOWN = 256
};

enum Icons_t
{
	ICON_NONE = 0,
	ICON_POISON = 1 << 0,
	ICON_BURN = 1 << 1,
	ICON_ENERGY = 1 << 2,
	ICON_DRUNK = 1 << 3,
	ICON_MANASHIELD = 1 << 4,
	ICON_PARALYZE = 1 << 5,
	ICON_HASTE = 1 << 6,
	ICON_SWORDS = 1 << 7
};

enum skills_t
{
	SKILL_NONE = -1,
	SKILL_FIRST = 0,
	SKILL_FIST = SKILL_FIRST,
	SKILL_CLUB,
	SKILL_SWORD,
	SKILL_AXE,
	SKILL_DIST,
	SKILL_SHIELD,
	SKILL_FISH,
	SKILL__MAGLEVEL,
	SKILL__LEVEL,
	SKILL__EXPERIENCE,
	SKILL_LAST = SKILL_FISH,
	SKILL__LAST = SKILL__EXPERIENCE
};

enum WeaponType_t
{
	WEAPON_NONE = 0,
	WEAPON_SWORD,
	WEAPON_CLUB,
	WEAPON_AXE,
	WEAPON_DIST,
	WEAPON_SHIELD,
	WEAPON_FIST,
	WEAPON_WAND,
	WEAPON_AMMO
};

enum Ammo_t
{
	AMMO_NONE = 0,
	AMMO_BOLT,
	AMMO_ARROW,
	AMMO_SPEAR,
	AMMO_THROWINGSTAR,
	AMMO_THROWINGKNIFE,
	AMMO_STONE,
	AMMO_SNOWBALL
};

enum AmmoAction_t
{
	AMMOACTION_NONE = 0,
	AMMOACTION_REMOVECOUNT,
	AMMOACTION_REMOVECHARGE,
	AMMOACTION_MOVE,
	AMMOACTION_MOVEBACK
};

enum WieldInfo_t
{
	WIELDINFO_LEVEL = 1,
	WIELDINFO_MAGLV = 2,
	WIELDINFO_VOCREQ = 4,
	WIELDINFO_PREMIUM = 8
};

enum Skulls_t
{
	SKULL_NONE = 0,
	SKULL_YELLOW,
	SKULL_GREEN,
	SKULL_WHITE,
	SKULL_RED,
	SKULL_LAST = SKULL_RED
};

enum PartyShields_t
{
	SHIELD_NONE = 0,
	SHIELD_WHITEYELLOW,
	SHIELD_WHITEBLUE,
	SHIELD_BLUE,
	SHIELD_YELLOW,
	SHIELD_LAST = SHIELD_YELLOW
};

enum SpellGroup_t
{
	SPELLGROUP_NONE = 0,
	SPELLGROUP_ATTACK = 1,
	SPELLGROUP_HEALING = 2,
	SPELLGROUP_SUPPORT = 3,
	SPELLGROUP_SPECIAL = 4
};

enum item_t
{
	ITEM_FIREFIELD = 1487,
	ITEM_FIREFIELD_SAFE = 1500,
	ITEM_POISONFIELD = 1490,
	ITEM_POISONFIELD_SAFE = 1503,
	ITEM_ENERGYFIELD = 1491,
	ITEM_ENERGYFIELD_SAFE = 1504,
	ITEM_MAGICWALL = 1497,
	ITEM_MAGICWALL_SAFE = ITEM_MAGICWALL,
	ITEM_WILDGROWTH = 1499,
	ITEM_WILDGROWTH_SAFE = ITEM_WILDGROWTH,
	ITEM_DEPOT = 2594,
	ITEM_LOCKER = 2589,
	ITEM_MALE_CORPSE = 3058,
	ITEM_FEMALE_CORPSE = 3065,
	ITEM_FULLSPLASH = 2016,
	ITEM_SMALLSPLASH = 2019,
	ITEM_PARCEL = 2595,
	ITEM_PARCEL_STAMPED = 2596,
	ITEM_LETTER = 2597,
	ITEM_LETTER_STAMPED = 2598,
	ITEM_LABEL = 2599,
	ITEM_STEALTH_RING = 2202,
	ITEM_HOUSE_TRANSFER = 1968
};

enum PlayerFlags
{
	PlayerFlag_CannotUseCombat = 0,
	PlayerFlag_CannotAttackPlayer,
	PlayerFlag_CannotAttackMonster,
	PlayerFlag_CannotBeAttacked,
	PlayerFlag_CanConvinceAll,
	PlayerFlag_CanSummonAll,
	PlayerFlag_CanIllusionAll,
	PlayerFlag_CanSenseInvisibility,
	PlayerFlag_IgnoredByMonsters,
	PlayerFlag_NotGainInFight,
	PlayerFlag_HasInfiniteMana,
	PlayerFlag_HasInfiniteSoul,
	PlayerFlag_HasNoExhaustion,
	PlayerFlag_CannotUseSpells,
	PlayerFlag_CannotPickupItem,
	PlayerFlag_CanAlwaysLogin,
	PlayerFlag_CanBroadcast,
	PlayerFlag_CanEditHouses,
	PlayerFlag_CannotBeBanned,
	PlayerFlag_CannotBePushed,
	PlayerFlag_HasInfiniteCapacity,
	PlayerFlag_CanPushAllCreatures,
	PlayerFlag_CanTalkRedPrivate,
	PlayerFlag_CanTalkRedChannel,
	PlayerFlag_TalkOrangeHelpChannel,
	PlayerFlag_NotGainExperience,
	PlayerFlag_NotGainMana,
	PlayerFlag_NotGainHealth,
	PlayerFlag_NotGainSkill,
	PlayerFlag_SetMaxSpeed,
	PlayerFlag_SpecialVIP,
	PlayerFlag_NotGenerateLoot,
	PlayerFlag_CanTalkRedChannelAnonymous,
	PlayerFlag_IgnoreProtectionZone,
	PlayerFlag_IgnoreSpellCheck,
	PlayerFlag_IgnoreEquipCheck,
	PlayerFlag_CannotBeMuted,
	PlayerFlag_IsAlwaysPremium,
	PlayerFlag_CanAnswerRuleViolations,
	PlayerFlag_39,
	PlayerFlag_ShowGroupNameInsteadOfVocation,
	PlayerFlag_HasInfiniteStamina,
	PlayerFlag_CannotMoveItems,
	PlayerFlag_CannotMoveCreatures,
	PlayerFlag_CanReportBugs,
	PlayerFlag_45,
	PlayerFlag_CannotBeSeen,
	PlayerFlag_HideHealth,
	PlayerFlag_CanPassThroughAllCreatures,
	PlayerFlag_LastFlag
};

enum PlayerCustomFlags
{
	PlayerCustomFlag_AllowIdle = 0,
	PlayerCustomFlag_CanSeePosition,
	PlayerCustomFlag_CanSeeItemDetails,
	PlayerCustomFlag_CanSeeCreatureDetails,
	PlayerCustomFlag_NotSearchable,
	PlayerCustomFlag_GamemasterPrivileges,
	PlayerCustomFlag_CanThrowAnywhere,
	PlayerCustomFlag_CanPushAllItems,
	PlayerCustomFlag_CanMoveAnywhere,
	PlayerCustomFlag_CanMoveFromFar,
	PlayerCustomFlag_CanUseFar,
	PlayerCustomFlag_CanLoginMultipleCharacters,
	PlayerCustomFlag_CanLogoutAnytime,
	PlayerCustomFlag_HideLevel,
	PlayerCustomFlag_IsProtected,
	PlayerCustomFlag_IsImmune,
	PlayerCustomFlag_NotGainSkull,
	PlayerCustomFlag_NotGainUnjustified,
	PlayerCustomFlag_IgnorePacification,
	PlayerCustomFlag_IgnoreLoginDelay,
	PlayerCustomFlag_CanStairhop,
	PlayerCustomFlag_CanTurnhop,
	PlayerCustomFlag_IgnoreHouseRent,
	PlayerCustomFlag_CanWearAllAddons,
	PlayerCustomFlag_IsWalkable,
	PlayerCustomFlag_25,
	PlayerCustomFlag_HasFullLight,
	PlayerCustomFlag_LastFlag
};

#define NETWORK_CRYPTOHEADER_SIZE 8
#define NETWORK_RETRY_TIMEOUT 5000
#define NETWORK_DEFAULT_SIZE 4096
#define NETWORK_HEADER_SIZE 2
#define NETWORK_MAX_SIZE 24590

#define IPBAN_FLAG 128
#define LOCALHOST 2130706433
#define SWIMMING_OUTFIT 267
#define GRATIS_PREMIUM 65535

#endif
