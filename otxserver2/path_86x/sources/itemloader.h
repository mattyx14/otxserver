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

#ifndef __ITEMLOADER__
#define __ITEMLOADER__
#include "fileloader.h"

typedef uint8_t attribute_t;
typedef uint16_t datasize_t;
typedef uint32_t flags_t;

enum itemgroup_t
{
	ITEM_GROUP_NONE = 0,
	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_WEAPON, //deprecated
	ITEM_GROUP_AMMUNITION, //deprecated
	ITEM_GROUP_ARMOR, //deprecated
	ITEM_GROUP_CHARGES,
	ITEM_GROUP_TELEPORT, //deprecated
	ITEM_GROUP_MAGICFIELD, //deprecated
	ITEM_GROUP_WRITEABLE, //deprecated
	ITEM_GROUP_KEY, //deprecated
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_DOOR, //deprecated
	ITEM_GROUP_DEPRECATED,
	ITEM_GROUP_LAST
};

enum clientVersion_t
{
	CLIENT_VERSION_750 = 1,
	CLIENT_VERSION_755 = 2,
	CLIENT_VERSION_760 = 3,
	CLIENT_VERSION_770 = 3,
	CLIENT_VERSION_780 = 4,
	CLIENT_VERSION_790 = 5,
	CLIENT_VERSION_792 = 6,
	CLIENT_VERSION_800 = 7,
	CLIENT_VERSION_810 = 8,
	CLIENT_VERSION_811 = 9,
	CLIENT_VERSION_820 = 10,
	CLIENT_VERSION_830 = 11,
	CLIENT_VERSION_840 = 12,
	CLIENT_VERSION_841 = 13,
	CLIENT_VERSION_842 = 14,
	CLIENT_VERSION_850 = 15,
	CLIENT_VERSION_854_OLD = 16,
	CLIENT_VERSION_854 = 17,
	CLIENT_VERSION_855 = 18,
	CLIENT_VERSION_860_OLD = 19,
	CLIENT_VERSION_860 = 20
};

enum rootattrib_
{
	ROOT_ATTR_VERSION = 0x01
};

enum itemattrib_t
{
	ITEM_ATTR_FIRST = 0x10,
	ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
	ITEM_ATTR_CLIENTID,
	ITEM_ATTR_NAME,
	ITEM_ATTR_DESCR,
	ITEM_ATTR_SPEED,
	ITEM_ATTR_SLOT,
	ITEM_ATTR_MAXITEMS,
	ITEM_ATTR_WEIGHT,
	ITEM_ATTR_WEAPON,
	ITEM_ATTR_AMU,
	ITEM_ATTR_ARMOR,
	ITEM_ATTR_MAGLEVEL,
	ITEM_ATTR_MAGFIELDTYPE,
	ITEM_ATTR_WRITEABLE,
	ITEM_ATTR_ROTATETO,
	ITEM_ATTR_DECAY,
	ITEM_ATTR_SPRITEHASH,
	ITEM_ATTR_MINIMAPCOLOR,
	ITEM_ATTR_07,
	ITEM_ATTR_08,
	ITEM_ATTR_LIGHT,
	ITEM_ATTR_DECAY2,
	ITEM_ATTR_WEAPON2,
	ITEM_ATTR_AMU2,
	ITEM_ATTR_ARMOR2,
	ITEM_ATTR_WRITEABLE2,
	ITEM_ATTR_LIGHT2,
	ITEM_ATTR_TOPORDER,
	ITEM_ATTR_WRITEABLE3,
	ITEM_ATTR_WAREID,
	ITEM_ATTR_LAST
};

enum itemflags_t
{
	FLAG_BLOCK_SOLID = 1 << 0,
	FLAG_BLOCK_PROJECTILE = 1 << 1,
	FLAG_BLOCK_PATHFIND = 1 << 2,
	FLAG_HAS_HEIGHT = 1 << 3,
	FLAG_USABLE = 1 << 4,
	FLAG_PICKUPABLE = 1 << 5,
	FLAG_MOVABLE = 1 << 6,
	FLAG_STACKABLE = 1 << 7,
	FLAG_FLOORCHANGEDOWN = 1 << 8,
	FLAG_FLOORCHANGENORTH = 1 << 9,
	FLAG_FLOORCHANGEEAST = 1 << 10,
	FLAG_FLOORCHANGESOUTH = 1 << 11,
	FLAG_FLOORCHANGEWEST = 1 << 12,
	FLAG_ALWAYSONTOP = 1 << 13,
	FLAG_READABLE = 1 << 14,
	FLAG_ROTABLE = 1 << 15,
	FLAG_HANGABLE = 1 << 16,
	FLAG_VERTICAL = 1 << 17,
	FLAG_HORIZONTAL = 1 << 18,
	FLAG_CANNOTDECAY = 1 << 19,
	FLAG_ALLOWDISTREAD = 1 << 20,
	FLAG_UNUSED = 1 << 21,
	FLAG_CLIENTCHARGES = 1 << 22, //deprecated
	FLAG_LOOKTHROUGH = 1 << 23,
	FLAG_ANIMATION = 1 << 24,
	FLAG_WALKSTACK = 1 << 25
};

#pragma pack(1)
struct VERSIONINFO
{
	uint32_t dwMajorVersion;
	uint32_t dwMinorVersion;
	uint32_t dwBuildNumber;
	uint8_t CSDVersion[128];
};

struct lightBlock2
{
	uint16_t lightLevel;
	uint16_t lightColor;
};
#pragma pack()
#endif
