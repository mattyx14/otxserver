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

#ifndef __IOMAPSERIALIZE__
#define __IOMAPSERIALIZE__
#include "otsystem.h"

#include "database.h"
#include "map.h"

typedef std::map<int32_t, std::pair<Item*, int32_t> > ItemMap;
typedef std::list<std::pair<Container*, int32_t> > ContainerStackList;

class House;
class IOMapSerialize
{
	public:
		virtual ~IOMapSerialize() {}
		static IOMapSerialize* getInstance()
		{
			static IOMapSerialize instance;
			return &instance;
		}

		bool loadMap(Map* map);
		bool saveMap(Map* map);

		bool updateAuctions();

		bool loadHouses();
		bool updateHouses();
		bool saveHouses();

		bool saveHouse(Database* db, House* house);
		bool saveHouseItems(Database* db, House* house);

	protected:
		IOMapSerialize() {}

		// Relational storage uses a row for each item/tile
		bool loadMapRelational(Map* map);
		bool saveMapRelational(Map* map);
		bool saveHouseRelational(Database* db, House* house, uint32_t& tileId);

		// Binary storage uses a giant BLOB field for storing everything
		bool loadMapBinary(Map* map);
		bool saveMapBinary(Map* map);
		bool saveHouseBinary(Database* db, DBInsert& stmt, House* house);

		// Binary-tilebased storage uses a BLOB field for each tile in houses, so that corrupt blobs will only wipe tiles instead of entire houses
		bool loadMapBinaryTileBased(Map* map);
		bool saveMapBinaryTileBased(Map* map);
		bool saveHouseBinaryTileBased(Database* db, DBInsert& stmt, House* house);

		bool loadItems(DBResult* result, Cylinder* parent, bool depotTransfer);
		bool saveItems(Database* db, uint32_t& tileId, uint32_t houseId, const Tile* tile);

		bool loadItem(PropStream& propStream, Cylinder* parent, bool depotTransfer);
		bool loadContainer(PropStream& propStream, Container* container);

		bool saveTile(PropWriteStream& stream, const Tile* tile);
		bool saveItem(PropWriteStream& stream, const Item* item);
};
#endif
