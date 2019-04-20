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

#ifndef __TILE__
#define __TILE__
#include <boost/shared_ptr.hpp>
#include <unordered_set>

#include "cylinder.h"
#include "item.h"

class Teleport;
class TrashHolder;
class Mailbox;
class MagicField;
class BedItem;

class Player;
class Creature;
class House;
class HouseTile;
class QTreeLeafNode;

typedef std::unordered_set<Creature*> SpectatorVec;
typedef std::vector<Creature*> CreatureVector;
typedef std::map<Position, SpectatorVec> SpectatorCache;

enum tileflags_t
{
	TILESTATE_NONE = 0,
	TILESTATE_PROTECTIONZONE = 1 << 0,
	TILESTATE_TRASHED = 1 << 1,
	TILESTATE_OPTIONALZONE = 1 << 2,
	TILESTATE_NOLOGOUT = 1 << 3,
	TILESTATE_HARDCOREZONE = 1 << 4,
	TILESTATE_REFRESH = 1 << 5,

	//internal usage
	TILESTATE_HOUSE = 1 << 6,
	TILESTATE_FLOORCHANGE = 1 << 7,
	TILESTATE_FLOORCHANGE_DOWN = 1 << 8,
	TILESTATE_FLOORCHANGE_NORTH = 1 << 9,
	TILESTATE_FLOORCHANGE_SOUTH = 1 << 10,
	TILESTATE_FLOORCHANGE_EAST = 1 << 11,
	TILESTATE_FLOORCHANGE_WEST = 1 << 12,
	TILESTATE_FLOORCHANGE_NORTH_EX = 1 << 13,
	TILESTATE_FLOORCHANGE_SOUTH_EX = 1 << 14,
	TILESTATE_FLOORCHANGE_EAST_EX = 1 << 15,
	TILESTATE_FLOORCHANGE_WEST_EX = 1 << 16,
	TILESTATE_TELEPORT = 1 << 17,
	TILESTATE_MAGICFIELD = 1 << 18,
	TILESTATE_MAILBOX = 1 << 19,
	TILESTATE_TRASHHOLDER = 1 << 20,
	TILESTATE_BED = 1 << 21,
	TILESTATE_DEPOT = 1 << 22,
	TILESTATE_BLOCKSOLID = 1 << 23,
	TILESTATE_BLOCKPATH = 1 << 24,
	TILESTATE_IMMOVABLEBLOCKSOLID = 1 << 25,
	TILESTATE_IMMOVABLEBLOCKPATH = 1 << 26,
	TILESTATE_IMMOVABLENOFIELDBLOCKPATH = 1 << 27,
	TILESTATE_NOFIELDBLOCKPATH = 1 << 28,
	TILESTATE_DYNAMIC_TILE = 1 << 29
};

enum ZoneType_t
{
	ZONE_PROTECTION,
	ZONE_OPTIONAL,
	ZONE_HARDCORE,
	ZONE_NOLOGOUT,
	ZONE_OPEN
};

class TileItemVector
{
	public:
		TileItemVector(): downItemCount(0) {}
		virtual ~TileItemVector() {}

		ItemVector::iterator begin() {return items.begin();}
		ItemVector::const_iterator begin() const {return items.begin();}
		ItemVector::reverse_iterator rbegin() {return items.rbegin();}
		ItemVector::const_reverse_iterator rbegin() const {return items.rbegin();}

		ItemVector::iterator end() {return items.end();}
		ItemVector::const_iterator end() const {return items.end();}
		ItemVector::reverse_iterator rend() {return items.rend();}
		ItemVector::const_reverse_iterator rend() const {return items.rend();}

		size_t size() {return items.size();}
		size_t size() const {return items.size();}
		bool empty() {return items.empty();}
		bool empty() const {return items.empty();}

		void push_back(Item* item) {items.push_back(item);}
		void push_front(Item* item) {items.insert(items.begin(), item);}
		ItemVector::iterator insert(ItemVector::iterator _where, Item* item) {return items.insert(_where, item);}
		ItemVector::iterator erase(ItemVector::iterator _pos) {return items.erase(_pos);}

		Item* at(size_t _pos) {return items.at(_pos);}
		Item* at(size_t _pos) const {return items.at(_pos);}
		Item* back() {return items.back();}
		const Item* back() const {return items.back();}

		ItemVector::iterator getBeginDownItem() {return items.begin();}
		ItemVector::const_iterator getBeginDownItem() const {return items.begin();}
		ItemVector::iterator getEndDownItem() {return items.begin() + downItemCount;}
		ItemVector::const_iterator getEndDownItem() const {return items.begin() + downItemCount;}

		ItemVector::iterator getBeginTopItem() {return items.begin() + downItemCount;}
		ItemVector::const_iterator getBeginTopItem() const {return items.begin() + downItemCount;}
		ItemVector::iterator getEndTopItem() {return items.end();}
		ItemVector::const_iterator getEndTopItem() const {return items.end();}

		uint32_t getTopItemCount() const {return items.size() - downItemCount;}
		uint32_t getDownItemCount() const {return downItemCount;}
		Item* getTopTopItem();
		Item* getTopDownItem();

		void addDownItem() {++downItemCount;}
		void removeDownItem() {--downItemCount;}

	private:
		friend class Tile;

		ItemVector items;
		uint16_t downItemCount;
};

class Tile : public Cylinder
{
	public:
		static Tile& nullTile;
		Tile(uint16_t x, uint16_t y, uint16_t z);
		virtual ~Tile();

		TileItemVector* getItemList();
		const TileItemVector* getItemList() const;
		TileItemVector* makeItemList();

		CreatureVector* getCreatures();
		const CreatureVector* getCreatures() const;
		CreatureVector* makeCreatures();

		virtual HouseTile* getHouseTile() {return NULL;}
		virtual const HouseTile* getHouseTile() const {return NULL;}
		virtual House* getHouse() {return NULL;}
		virtual const House* getHouse() const {return NULL;}

		MagicField* getFieldItem() const;
		Teleport* getTeleportItem() const;
		TrashHolder* getTrashHolder() const;
		Mailbox* getMailbox() const;
		BedItem* getBedItem() const;

		Item* getTopTopItem();
		Item* getTopDownItem();
		Item* getItemByTopOrder(uint32_t topOrder);

		Creature* getTopCreature();
		Creature* getBottomCreature();
		Thing* getTopVisibleThing(const Creature* creature);
		Creature* getTopVisibleCreature(const Creature* creature);
		const Creature* getTopVisibleCreature(const Creature* creature) const;
		Creature* getBottomVisibleCreature(const Creature* creature);
		const Creature* getBottomVisibleCreature(const Creature* creature) const;

		uint32_t getThingCount() const {return thingCount;}
		void updateThingCount(int32_t amount) {thingCount += amount;}

		uint32_t getCreatureCount() const;
		uint32_t getItemCount() const;

		uint32_t getTopItemCount() const;
		uint32_t getDownItemCount() const;

		bool hasProperty(enum ITEMPROPERTY prop) const;
		bool hasProperty(Item* exclude, enum ITEMPROPERTY prop) const;

		bool hasFlag(tileflags_t flag) const {return ((m_flags & (uint32_t)flag) == (uint32_t)flag);}
		void setFlag(tileflags_t flag) {m_flags |= (uint32_t)flag;}
		void resetFlag(tileflags_t flag) {m_flags &= ~(uint32_t)flag;}

		bool positionChange() const {return hasFlag(TILESTATE_TELEPORT);}
		bool floorChange(FloorChange_t change = CHANGE_NONE) const
		{
			switch(change)
			{
				case CHANGE_DOWN:
					return hasFlag(TILESTATE_FLOORCHANGE_DOWN);
				case CHANGE_NORTH:
					return hasFlag(TILESTATE_FLOORCHANGE_NORTH);
				case CHANGE_SOUTH:
					return hasFlag(TILESTATE_FLOORCHANGE_SOUTH);
				case CHANGE_EAST:
					return hasFlag(TILESTATE_FLOORCHANGE_EAST);
				case CHANGE_WEST:
					return hasFlag(TILESTATE_FLOORCHANGE_WEST);
				case CHANGE_NORTH_EX:
					return hasFlag(TILESTATE_FLOORCHANGE_NORTH_EX);
				case CHANGE_SOUTH_EX:
					return hasFlag(TILESTATE_FLOORCHANGE_SOUTH_EX);
				case CHANGE_EAST_EX:
					return hasFlag(TILESTATE_FLOORCHANGE_EAST_EX);
				case CHANGE_WEST_EX:
					return hasFlag(TILESTATE_FLOORCHANGE_WEST_EX);
				case CHANGE_NONE:
					return hasFlag(TILESTATE_FLOORCHANGE);
				default:
					break;
			}

			return false;
		}

		ZoneType_t getZone() const
		{
			if(hasFlag(TILESTATE_PROTECTIONZONE))
				return ZONE_PROTECTION;

			if(hasFlag(TILESTATE_OPTIONALZONE))
				return ZONE_OPTIONAL;

			if(hasFlag(TILESTATE_HARDCOREZONE))
				return ZONE_HARDCORE;

			return ZONE_OPEN;
		}

		bool hasHeight(uint32_t n) const;
		bool isFull() const;

		void moveCreature(Creature* actor, Creature* creature, Cylinder* toCylinder, bool forceTeleport = false);
		int32_t getClientIndexOfThing(const Player* player, const Thing* thing) const;

		//cylinder implementations
		virtual Cylinder* getParent() {return NULL;}
		virtual const Cylinder* getParent() const {return NULL;}
		virtual bool isRemoved() const {return false;}
		virtual Position getPosition() const {return pos;}
		virtual Tile* getTile() {return this;}
		virtual const Tile* getTile() const {return this;}
		virtual Item* getItem() {return NULL;}
		virtual const Item* getItem() const {return NULL;}
		virtual Creature* getCreature() {return NULL;}
		virtual const Creature* getCreature() const {return NULL;}

		virtual ReturnValue __queryAdd(int32_t index, const Thing* thing, uint32_t count,
			uint32_t flags, Creature* actor = NULL) const;
		virtual ReturnValue __queryMaxCount(int32_t index, const Thing* thing, uint32_t count,
			uint32_t& maxQueryCount, uint32_t flags) const;
		virtual ReturnValue __queryRemove(const Thing* thing, uint32_t count, uint32_t flags, Creature* actor = NULL) const;
		virtual Cylinder* __queryDestination(int32_t& index, const Thing* thing, Item** destItem,
			uint32_t& flags);

		virtual void __addThing(Creature* actor, Thing* thing) {__addThing(actor, 0, thing);}
		virtual void __addThing(Creature* actor, int32_t index, Thing* thing);

		virtual void __updateThing(Thing* thing, uint16_t itemId, uint32_t count);
		virtual void __replaceThing(uint32_t index, Thing* thing);

		virtual void __removeThing(Thing* thing, uint32_t count);

		virtual int32_t __getIndexOfThing(const Thing* thing) const;
		virtual int32_t __getFirstIndex() const {return 0;}
		virtual int32_t __getLastIndex() const {return thingCount;}

		virtual Thing* __getThing(uint32_t index) const;
		virtual uint32_t __getItemTypeCount(uint16_t itemId, int32_t subType = -1) const;

		virtual void postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
			int32_t index, CylinderLink_t link = LINK_OWNER);
		virtual void postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
			int32_t index, bool isCompleteRemoval, CylinderLink_t link = LINK_OWNER);

		virtual void __internalAddThing(Thing* thing) {__internalAddThing(0, thing);}
		virtual void __internalAddThing(uint32_t index, Thing* thing);

		void onUpdateTile();
		void updateTileFlags(Item* item, bool remove);

	private:
		void onAddTileItem(Item* item);
		void onUpdateTileItem(Item* oldItem, const ItemType& oldType, Item* newItem, const ItemType& newType);
		void onRemoveTileItem(const SpectatorVec& list, std::vector<int32_t>& oldStackPosVector, Item* item);

	protected:
		bool isDynamic() const {return (m_flags & TILESTATE_DYNAMIC_TILE) != 0;}

	public:
		QTreeLeafNode* qt_node;
		Item* ground;

	protected:
		Position pos;
		uint32_t m_flags, thingCount;
};

// Used for walkable tiles, where there is high likeliness of
// items being added/removed
class DynamicTile : public Tile
{
	// By allocating the vectors in-house, we avoid some memory fragmentation
	TileItemVector items;
	CreatureVector creatures;
	public:
		DynamicTile(uint16_t x, uint16_t y, uint16_t z);
		virtual ~DynamicTile();

		TileItemVector* getItemList() {return &items;}
		const TileItemVector* getItemList() const {return &items;}
		TileItemVector* makeItemList() {return &items;}

		CreatureVector* getCreatures() {return &creatures;}
		const CreatureVector* getCreatures() const {return &creatures;}
		CreatureVector* makeCreatures() {return &creatures;}
};

// For blocking tiles, where we very rarely actually have items
class StaticTile : public Tile
{
	// We very rarely even need the vectors, so don't keep them in memory
	TileItemVector* items;
	CreatureVector*	creatures;
	public:
		StaticTile(uint16_t x, uint16_t y, uint16_t z);
		virtual ~StaticTile();

		TileItemVector* getItemList() {return items;}
		const TileItemVector* getItemList() const {return items;}
		TileItemVector* makeItemList() {return (items) ? (items) : (items = new TileItemVector);}

		CreatureVector* getCreatures() {return creatures;}
		const CreatureVector* getCreatures() const {return creatures;}
		CreatureVector* makeCreatures() {return (creatures) ? (creatures) : (creatures = new CreatureVector);}
};

inline Tile::Tile(uint16_t x, uint16_t y, uint16_t z): qt_node(NULL),
	ground(NULL), pos(x, y, z), m_flags(0), thingCount(0) {}

inline Tile::~Tile() {}

inline CreatureVector* Tile::getCreatures()
{
	if(isDynamic())
		return static_cast<DynamicTile*>(this)->DynamicTile::getCreatures();

	return static_cast<StaticTile*>(this)->StaticTile::getCreatures();
}

inline const CreatureVector* Tile::getCreatures() const
{
	if(isDynamic())
		return static_cast<const DynamicTile*>(this)->DynamicTile::getCreatures();

	return static_cast<const StaticTile*>(this)->StaticTile::getCreatures();
}

inline CreatureVector* Tile::makeCreatures()
{
	if(isDynamic())
		return static_cast<DynamicTile*>(this)->DynamicTile::makeCreatures();

	return static_cast<StaticTile*>(this)->StaticTile::makeCreatures();
}

inline TileItemVector* Tile::getItemList()
{
	if(isDynamic())
		return static_cast<DynamicTile*>(this)->DynamicTile::getItemList();

	return static_cast<StaticTile*>(this)->StaticTile::getItemList();
}

inline const TileItemVector* Tile::getItemList() const
{
	if(isDynamic())
		return static_cast<const DynamicTile*>(this)->DynamicTile::getItemList();

	return static_cast<const StaticTile*>(this)->StaticTile::getItemList();
}

inline TileItemVector* Tile::makeItemList()
{
	if(isDynamic())
		return static_cast<DynamicTile*>(this)->DynamicTile::makeItemList();

	return static_cast<StaticTile*>(this)->StaticTile::makeItemList();
}

inline StaticTile::StaticTile(uint16_t x, uint16_t y, uint16_t z):
	Tile(x, y, z), items(NULL), creatures(NULL) {}

inline StaticTile::~StaticTile() {}

inline DynamicTile::DynamicTile(uint16_t x, uint16_t y, uint16_t z):
	Tile(x, y, z)
{
	m_flags |= TILESTATE_DYNAMIC_TILE;
}

inline DynamicTile::~DynamicTile() {}
#endif
