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

#ifndef __IOMAP__
#define __IOMAP__
#include "status.h"

#include "map.h"
#include "house.h"

#include "spawn.h"
#include "item.h"

enum OTBM_AttrTypes_t
{
	OTBM_ATTR_DESCRIPTION = 1,
	OTBM_ATTR_EXT_FILE = 2,
	OTBM_ATTR_TILE_FLAGS = 3,
	OTBM_ATTR_ACTION_ID = 4,
	OTBM_ATTR_UNIQUE_ID = 5,
	OTBM_ATTR_TEXT = 6,
	OTBM_ATTR_DESC = 7,
	OTBM_ATTR_TELE_DEST = 8,
	OTBM_ATTR_ITEM = 9,
	OTBM_ATTR_DEPOT_ID = 10,
	OTBM_ATTR_EXT_SPAWN_FILE = 11,
	OTBM_ATTR_RUNE_CHARGES = 12,
	OTBM_ATTR_EXT_HOUSE_FILE = 13,
	OTBM_ATTR_HOUSEDOORID = 14,
	OTBM_ATTR_COUNT = 15,
	OTBM_ATTR_DURATION = 16,
	OTBM_ATTR_DECAYING_STATE = 17,
	OTBM_ATTR_WRITTENDATE = 18,
	OTBM_ATTR_WRITTENBY = 19,
	OTBM_ATTR_SLEEPERGUID = 20,
	OTBM_ATTR_SLEEPSTART = 21,
	OTBM_ATTR_CHARGES = 22,
	OTBM_ATTR_CONTAINER_ITEMS = 23,
	OTBM_ATTR_ATTRIBUTE_MAP = 128
};

enum OTBM_NodeTypes_t
{
	OTBM_ROOTV2 = 1,
	OTBM_MAP_DATA = 2,
	OTBM_ITEM_DEF = 3,
	OTBM_TILE_AREA = 4,
	OTBM_TILE = 5,
	OTBM_ITEM = 6,
	OTBM_TILE_SQUARE = 7,
	OTBM_TILE_REF = 8,
	OTBM_SPAWNS = 9,
	OTBM_SPAWN_AREA = 10,
	OTBM_MONSTER = 11,
	OTBM_TOWNS = 12,
	OTBM_TOWN = 13,
	OTBM_HOUSETILE = 14,
	OTBM_WAYPOINTS = 15,
	OTBM_WAYPOINT = 16
};

#pragma pack(1)
struct OTBM_root_header
{
	uint32_t version;
	uint16_t width, height;
	uint32_t majorVersionItems, minorVersionItems;
};

struct OTBM_Destination_coords
{
	uint16_t _x, _y;
	uint8_t _z;
};

struct OTBM_Tile_coords
{
	uint8_t _x, _y;
};

struct OTBM_HouseTile_coords
{
	uint8_t _x, _y;
	uint32_t _houseid;
};
#pragma pack()

class IOMap
{
	public:
		IOMap() {}
		virtual ~IOMap() {}

		static Tile* createTile(Item*& ground, Item* item, uint16_t px, uint16_t py, uint16_t pz);
		bool loadMap(Map* map, const std::string& identifier);

		/* Load the spawns
		 * \param map pointer to the Map class
		 * \returns Returns true if the spawns were loaded successfully
		 */
		bool loadSpawns(Map* map);

		/* Load the houses (not house tile-data)
		 * \param map pointer to the Map class
		 * \returns Returns true if the houses were loaded successfully
		 */
		bool loadHouses(Map* map);

		const std::string& getLastErrorString() const {return errorString;}
		void setLastErrorString(const std::string& _errorString) {errorString = _errorString;}

	protected:
		std::string errorString;
};
#endif
