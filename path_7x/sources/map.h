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

#ifndef __MAP__
#define __MAP__
#include "tools.h"

#include "fileloader.h"
#include "position.h"

#include "waypoints.h"
#include "tile.h"

class Creature;
class Player;
class Game;
class Tile;
class Map;

struct FindPathParams;
struct AStarNode
{
	uint16_t x, y;
	AStarNode* parent;
	int32_t f, g, h;
};

using boost::shared_ptr;
#define MAP_MAX_LAYERS 16

#define MAX_NODES 512
#define GET_NODE_INDEX(a) (a - &nodes[0])

#define MAP_NORMALWALKCOST 10
#define MAP_DIAGONALWALKCOST 25

class AStarNodes
{
	public:
		AStarNodes();
		virtual ~AStarNodes() {}

		void openNode(AStarNode* node);
		void closeNode(AStarNode* node);

		uint32_t countOpenNodes();
		uint32_t countClosedNodes();

		AStarNode* getBestNode();
		AStarNode* createOpenNode();
		AStarNode* getNodeInList(uint16_t x, uint16_t y);

		bool isInList(uint16_t x, uint16_t y);
		int32_t getEstimatedDistance(uint16_t x, uint16_t y, uint16_t xGoal, uint16_t yGoal);

		int32_t getMapWalkCost(const Creature* creature, AStarNode* node,
			const Tile* neighbourTile, const Position& neighbourPos);
		static int32_t getTileWalkCost(const Creature* creature, const Tile* tile);

	private:
		AStarNode nodes[MAX_NODES];

		std::bitset<MAX_NODES> openNodes;
		uint32_t curNode;
};

template<class T> class lessPointer: public std::binary_function<T*, T*, bool>
{
	public:
		bool operator()(T*& t1, T*& t2) {return *t1 < *t2;}
};

#define FLOOR_BITS 3
#define FLOOR_SIZE (1 << FLOOR_BITS)
#define FLOOR_MASK (FLOOR_SIZE - 1)

struct Floor
{
	Floor();
	Tile* tiles[FLOOR_SIZE][FLOOR_SIZE];
};

class FrozenPathingConditionCall;
class QTreeLeafNode;

class QTreeNode
{
	public:
		QTreeNode();
		virtual ~QTreeNode();

		bool isLeaf() const {return m_isLeaf;}

		QTreeLeafNode* getLeaf(uint16_t x, uint16_t y);
		static QTreeLeafNode* getLeafStatic(QTreeNode* root, uint16_t x, uint16_t y);

		QTreeLeafNode* createLeaf(uint16_t x, uint16_t y, uint16_t level);

	protected:
		bool m_isLeaf;
		QTreeNode* m_child[4];

		friend class Map;
};

class QTreeLeafNode : public QTreeNode
{
	public:
		QTreeLeafNode();
		virtual ~QTreeLeafNode();

		Floor* createFloor(uint16_t z);
		Floor* getFloor(uint16_t z) {return m_array[z];}

		QTreeLeafNode* stepSouth() {return m_leafS;}
		QTreeLeafNode* stepEast() {return m_leafE;}

		void addCreature(Creature* c);
		void removeCreature(Creature* c);

	protected:
		static bool newLeaf;

		QTreeLeafNode* m_leafS;
		QTreeLeafNode* m_leafE;

		Floor* m_array[MAP_MAX_LAYERS];
		CreatureVector creatureList;

		friend class Map;
		friend class QTreeNode;
};

/**
* Map class.
* Holds all the actual map-data
*/

class Map
{
	public:
		Map();
		virtual ~Map() {}

		static const int32_t maxViewportX = 11; //min value: maxClientViewportX + 1
		static const int32_t maxViewportY = 11; //min value: maxClientViewportY + 1
		static const int32_t maxClientViewportX = 8;
		static const int32_t maxClientViewportY = 6;

		/**
		* Load a map.
		* \returns true if the map was loaded successfully
		*/
		bool loadMap(const std::string& identifier);

		/**
		* Save a map.
		* \param identifier file/database to save to
		* \returns true if the map was saved successfully
		*/
		bool saveMap();

		/**
		* \returns true if updating house auctions completed
		*/
		bool updateAuctions();

		/**
		* Get a single tile.
		* \returns A pointer to that tile.
		*/
		Tile* getTile(int32_t x, int32_t y, int32_t z);
		Tile* getTile(const Position& pos) {return getTile(pos.x, pos.y, pos.z);}

		/**
		* Set a single tile.
		* \param a tile to set for the position
		*/
		void setTile(uint16_t _x, uint16_t _y, uint16_t _z, Tile* newTile);
		void setTile(const Position& pos, Tile* newTile) {setTile(pos.x, pos.y, pos.z, newTile);}

		/**
		* Place a creature on the map
		* \param pos The position to place the creature
		* \param creature Creature to place on the map
		* \param extendedPos If true, the creature will in first-hand be placed 2 tiles away
		* \param forceLogin If true, placing the creature will not fail becase of obstacles (creatures/chests)
		*/
		bool placeCreature(const Position& centerPos, Creature* creature, bool extendedPos = false, bool forceLogin = false);

		/**
		* Remove a creature from the map.
		* \param c Creature pointer to the creature to remove
		*/
		bool removeCreature(Creature* c);

		/**
		* Checks if you can throw an object to that position
		*	\param fromPos from Source point
		*	\param toPos Destination point
		*	\param rangex maximum allowed range horizontially
		*	\param rangey maximum allowed range vertically
		*	\param checkLineOfSight checks if there is any blocking objects in the way
		*	\returns The result if you can throw there or not
		*/
		bool canThrowObjectTo(const Position& fromPos, const Position& toPos, bool checkLineOfSight = true,
			int32_t rangex = Map::maxClientViewportX, int32_t rangey = Map::maxClientViewportY);

		/**
		* Checks if path is clear from fromPos to toPos
		* Notice: This only checks a straight line if the path is clear, for path finding use getPathTo.
		*	\param fromPos from Source point
		*	\param toPos Destination point
		*	\param floorCheck if true then view is not clear if fromPos.z is not the same as toPos.z
		*	\returns The result if there is no obstacles
		*/
		bool isSightClear(const Position& fromPos, const Position& toPos, bool floorCheck) const;
		bool checkSightLine(const Position& fromPos, const Position& toPos) const;

		/**
		* Get the path to a specific position on the map.
		* \param creature The creature that wants a path
		* \param destPos The position we want a path calculated to
		* \param listDir contains a list of directions to the destination
		* \param maxDist Maximum distance from our current position to search, default: -1 (no limit)
		* \returns returns true if a path was found
		*/
		bool getPathTo(const Creature* creature, const Position& destPos,
			std::list<Direction>& listDir, int32_t maxDist = -1);
		bool getPathMatching(const Creature* creature, std::list<Direction>& dirList,
			const FrozenPathingConditionCall& pathCondition, const FindPathParams& fpp);

		QTreeLeafNode* getLeaf(uint16_t x, uint16_t y) {return root.getLeaf(x, y);}
		const Tile* canWalkTo(const Creature* creature, const Position& pos);
		Waypoints waypoints;

	protected:
		QTreeNode root;

		uint32_t mapWidth, mapHeight;
		std::string spawnfile, housefile;
		StringVec descriptions;

		SpectatorCache spectatorCache;
		void clearSpectatorCache() {spectatorCache.clear();}

		// Actually scans the map for spectators
		void getSpectatorsInternal(SpectatorVec& list, const Position& centerPos, bool checkforduplicate,
			int32_t minRangeX, int32_t maxRangeX, int32_t minRangeY, int32_t maxRangeY,
			int32_t minRangeZ, int32_t maxRangeZ);
		// Use this when a custom spectator vector is needed, this support many
		// more parameters than the heavily cached version below.
		void getSpectators(SpectatorVec& list, const Position& centerPos, bool checkforduplicate = false, bool multifloor = false,
			int32_t minRangeX = 0, int32_t maxRangeX = 0, int32_t minRangeY = 0, int32_t maxRangeY = 0);
		// The returned SpectatorVec is a temporary and should not be kept around
		// Take special heed in that the vector will be destroyed if any function
		// that calls clearSpectatorCache is called.
		const SpectatorVec& getSpectators(const Position& centerPos);

		friend class Game;
		friend class IOMap;
};

inline void QTreeLeafNode::addCreature(Creature* c)
{
	creatureList.push_back(c);
}

inline void QTreeLeafNode::removeCreature(Creature* c)
{
	CreatureVector::iterator it = std::find(creatureList.begin(), creatureList.end(), c);
	assert(it != creatureList.end());
	creatureList.erase(it);
}
#endif
