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
#include <iomanip>

#include <boost/config.hpp>
#include <boost/bind.hpp>

#include "iomap.h"
#include "map.h"
#include "tile.h"

#include "creature.h"
#include "player.h"
#include "combat.h"

#include "iomapserialize.h"
#include "items.h"

#include "game.h"

extern Game g_game;

Map::Map()
{
	mapWidth = 0;
	mapHeight = 0;
}

bool Map::loadMap(const std::string& identifier)
{
	int64_t start = OTSYS_TIME();
	IOMap* loader = new IOMap();
	if(!loader->loadMap(this, identifier))
	{
		std::clog << "> FATAL: OTBM Loader - " << loader->getLastErrorString() << std::endl;
		return false;
	}

	std::clog << std::endl << ">>> Loading time: " << (OTSYS_TIME() - start) / (1000.) << " seconds." << std::endl;
	start = OTSYS_TIME();
	if(!loader->loadSpawns(this))
		std::clog << ">>> WARNING: Could not load spawn data." << std::endl;

	if(!loader->loadHouses(this))
		std::clog << ">>> WARNING: Could not load house data." << std::endl;

	delete loader;
	std::clog << ">>> Parsing time: " << (OTSYS_TIME() - start) / (1000.) << " seconds." << std::endl;
	start = OTSYS_TIME();

	IOMapSerialize::getInstance()->updateHouses();
	IOMapSerialize::getInstance()->updateAuctions();
	std::clog << ">>> Synchronization time: " << (OTSYS_TIME() - start) / (1000.) << " seconds." << std::endl;

	start = OTSYS_TIME();
	IOMapSerialize::getInstance()->loadHouses();
	IOMapSerialize::getInstance()->loadMap(this);

	std::clog << ">>> Unserialization time: " << (OTSYS_TIME() - start) / (1000.) << " seconds." << std::endl;
	return true;
}

bool Map::saveMap()
{
	IOMapSerialize* IOLoader = IOMapSerialize::getInstance();
	bool saved = false;
	for(uint32_t tries = 0; tries < 3; ++tries)
	{
		if(!IOLoader->saveHouses())
			continue;

		saved = true;
		break;
	}

	if(!saved)
		return false;

	saved = false;
	for(uint32_t tries = 0; tries < 3; ++tries)
	{
		if(!IOLoader->saveMap(this))
			continue;

		saved = true;
		break;
	}

	return saved;
}

bool Map::updateAuctions()
{
	return IOMapSerialize::getInstance()->updateAuctions();
}

Tile* Map::getTile(int32_t x, int32_t y, int32_t z) const
{
	if(x < 0 || x > 0xFFFF || y < 0 || y > 0xFFFF || z < 0 || z >= MAP_MAX_LAYERS)
		return NULL;

	const QTreeLeafNode* leaf = QTreeNode::getLeafStatic<const QTreeLeafNode*, const QTreeNode*>(&root, x, y);
	if(!leaf)
		return NULL;

	const Floor* floor = leaf->getFloor(z);
	if(!floor)
		return NULL;

	return floor->tiles[x & FLOOR_MASK][y & FLOOR_MASK];
}

void Map::setTile(uint16_t x, uint16_t y, uint16_t z, Tile* newTile)
{
	if(z >= MAP_MAX_LAYERS)
	{
		std::clog << "[Error - Map::setTile]: Attempt to set tile on invalid Z coordinate - " << z << "!" << std::endl;
		return;
	}

	QTreeLeafNode::newLeaf = false;
	QTreeLeafNode* leaf = root.createLeaf(x, y, 15);
	if(QTreeLeafNode::newLeaf)
	{
		//update north
		QTreeLeafNode* northLeaf = root.getLeaf(x, y - FLOOR_SIZE);
		if(northLeaf)
			northLeaf->m_leafS = leaf;

		//update west leaf
		QTreeLeafNode* westLeaf = root.getLeaf(x - FLOOR_SIZE, y);
		if(westLeaf)
			westLeaf->m_leafE = leaf;

		//update south
		QTreeLeafNode* southLeaf = root.getLeaf(x, y + FLOOR_SIZE);
		if(southLeaf)
			leaf->m_leafS = southLeaf;

		//update east
		QTreeLeafNode* eastLeaf = root.getLeaf(x + FLOOR_SIZE, y);
		if(eastLeaf)
			leaf->m_leafE = eastLeaf;
	}

	uint32_t offsetX = x & FLOOR_MASK, offsetY = y & FLOOR_MASK;
	Floor* floor = leaf->createFloor(z);
	if(!floor->tiles[offsetX][offsetY])
	{
		floor->tiles[offsetX][offsetY] = newTile;
		newTile->qt_node = leaf;
	}
	else
		std::clog << "[Error - Map::setTile] Tile already exists - pos " << offsetX << "/" << offsetY << "/" << z << std::endl;

	if(newTile->hasFlag(TILESTATE_REFRESH))
	{
		RefreshBlock_t rb;
		if(TileItemVector* tileItems = newTile->getItemList())
		{
			for(ItemVector::iterator it = tileItems->getBeginDownItem(); it != tileItems->getEndDownItem(); ++it)
				rb.list.push_back((*it)->clone());
		}

		rb.lastRefresh = OTSYS_TIME();
		g_game.addRefreshTile(newTile, rb);
	}
}

bool Map::placeCreature(const Position& centerPos, Creature* creature, bool extendedPos /*= false*/, bool forced /*= false*/)
{
	bool foundTile = false, placeInPz = false;
	Tile* tile = getTile(centerPos);
	if(tile && !extendedPos)
	{
		placeInPz = tile->hasFlag(TILESTATE_PROTECTIONZONE);
		uint32_t flags = FLAG_IGNOREBLOCKITEM;
		if(creature->isAccountManager())
			flags |= FLAG_IGNOREBLOCKCREATURE;

		ReturnValue ret = tile->__queryAdd(0, creature, 1, flags);
		if(forced || ret == RET_NOERROR || ret == RET_PLAYERISNOTINVITED)
			foundTile = true;
	}

	size_t shufflePos = 0;
	PairVector relList;
	if(extendedPos)
	{
		shufflePos = 8;
		relList.push_back(PositionPair(-2, 0));
		relList.push_back(PositionPair(0, -2));
		relList.push_back(PositionPair(0, 2));
		relList.push_back(PositionPair(2, 0));
		std::random_shuffle(relList.begin(), relList.end());
	}

	relList.push_back(PositionPair(-1, -1));
	relList.push_back(PositionPair(-1, 0));
	relList.push_back(PositionPair(-1, 1));
	relList.push_back(PositionPair(0, -1));
	relList.push_back(PositionPair(0, 1));
	relList.push_back(PositionPair(1, -1));
	relList.push_back(PositionPair(1, 0));
	relList.push_back(PositionPair(1, 1));
	std::random_shuffle(relList.begin() + shufflePos, relList.end());

	uint32_t radius = 1;
	Position tryPos;
	for(uint32_t n = 1; n <= radius && !foundTile; ++n)
	{
		for(PairVector::iterator it = relList.begin(); it != relList.end() && !foundTile; ++it)
		{
			int32_t dx = it->first * n, dy = it->second * n;
			tryPos = centerPos;

			tryPos.x = tryPos.x + dx;
			tryPos.y = tryPos.y + dy;
			if(!(tile = getTile(tryPos)) || (placeInPz && !tile->hasFlag(TILESTATE_PROTECTIONZONE)))
				continue;

			if(tile->__queryAdd(0, creature, 1, 0) == RET_NOERROR)
			{
				if(!extendedPos)
				{
					foundTile = true;
					break;
				}

				if(isSightClear(centerPos, tryPos, false))
				{
					foundTile = true;
					break;
				}
			}
		}
	}

	if(!foundTile)
		return false;

	int32_t index = 0;
	uint32_t flags = 0;

	Item* toItem = NULL;
	if(Cylinder* toCylinder = tile->__queryDestination(index, creature, &toItem, flags))
	{
		toCylinder->__internalAddThing(creature);
		if(Tile* toTile = toCylinder->getTile())
			toTile->qt_node->addCreature(creature);
	}

	return true;
}

bool Map::removeCreature(Creature* creature)
{
	Tile* tile = creature->getTile();
	if(!tile)
		return false;

	tile->qt_node->removeCreature(creature);
	tile->__removeThing(creature, 0);
	return true;
}

void Map::getSpectatorsInternal(SpectatorVec& list, const Position& centerPos, int32_t minRangeX, int32_t maxRangeX, int32_t minRangeY, int32_t maxRangeY, int32_t minRangeZ, int32_t maxRangeZ, bool onlyPlayers) const
{
	/*
	int32_t minoffset = centerPos.z - maxRangeZ, maxoffset = centerPos.z - minRangeZ,
		x1 = std::min((int32_t)0xFFFF, std::max((int32_t)0, (centerPos.x + minRangeX + minoffset))),
		y1 = std::min((int32_t)0xFFFF, std::max((int32_t)0, (centerPos.y + minRangeY + minoffset))),
		x2 = std::min((int32_t)0xFFFF, std::max((int32_t)0, (centerPos.x + maxRangeX + maxoffset))),
		y2 = std::min((int32_t)0xFFFF, std::max((int32_t)0, (centerPos.y + maxRangeY + maxoffset))),
		startx1 = x1 - (x1 % FLOOR_SIZE), starty1 = y1 - (y1 % FLOOR_SIZE),
		endx2 = x2 - (x2 % FLOOR_SIZE), endy2 = y2 - (y2 % FLOOR_SIZE);

	QTreeLeafNode* startLeaf = getLeaf(startx1, starty1);
	QTreeLeafNode* leafS = startLeaf;

	QTreeLeafNode* leafE;
	for(int32_t ny = starty1; ny <= endy2; ny += FLOOR_SIZE)
	{
		leafE = leafS;
		for(int32_t nx = startx1; nx <= endx2; nx += FLOOR_SIZE)
		{
			if(leafE)
			{
				CreatureVector& nodeList = (onlyPlayers ? leafE->playerList : leafE->creatureList);
				CreatureVector::const_iterator it = nodeList.begin();
				if(it != nodeList.end())
				{
					do
					{
						const Position& pos = (*it)->getPosition();
						if(pos.z < minRangeZ || pos.z > maxRangeZ)
							continue;

						int32_t offsetZ = centerPos.z - pos.z;
						if(pos.y < (centerPos.y + minRangeY + offsetZ) || pos.y > (centerPos.y + maxRangeY + offsetZ))
							continue;

						if(pos.x < (centerPos.x + minRangeX + offsetZ) || pos.x > (centerPos.x + maxRangeX + offsetZ))
							continue;

						if(!checkForDuplicate || std::find(list.begin(), list.end(), *it) == list.end())
							list.push_back(*it);
					}
					while(++it != nodeList.end());
				}

				leafE = leafE->stepEast();
			}
			else
				leafE = getLeaf(nx + FLOOR_SIZE, ny);
		}

		if(leafS)
			leafS = leafS->stepSouth();
		else
			leafS = getLeaf(startx1, ny + FLOOR_SIZE);
	}
	*/
	int_fast16_t min_y = centerPos.y + minRangeY;
	int_fast16_t min_x = centerPos.x + minRangeX;
	int_fast16_t max_y = centerPos.y + maxRangeY;
	int_fast16_t max_x = centerPos.x + maxRangeX;

	int32_t minoffset = static_cast<int32_t>(centerPos.z) - maxRangeZ;
	uint16_t x1 = std::min<uint32_t>(0xFFFF, std::max<int32_t>(0, (min_x + minoffset)));
	uint16_t y1 = std::min<uint32_t>(0xFFFF, std::max<int32_t>(0, (min_y + minoffset)));

	int32_t maxoffset = static_cast<int32_t>(centerPos.z) - minRangeZ;
	uint16_t x2 = std::min<uint32_t>(0xFFFF, std::max<int32_t>(0, (max_x + maxoffset)));
	uint16_t y2 = std::min<uint32_t>(0xFFFF, std::max<int32_t>(0, (max_y + maxoffset)));

	int32_t startx1 = x1 - (x1 % FLOOR_SIZE);
	int32_t starty1 = y1 - (y1 % FLOOR_SIZE);
	int32_t endx2 = x2 - (x2 % FLOOR_SIZE);
	int32_t endy2 = y2 - (y2 % FLOOR_SIZE);

	const QTreeLeafNode* startLeaf = QTreeNode::getLeafStatic<const QTreeLeafNode*, const QTreeNode*>(&root, startx1, starty1);
	const QTreeLeafNode* leafS = startLeaf;
	const QTreeLeafNode* leafE;

	for (int_fast32_t ny = starty1; ny <= endy2; ny += FLOOR_SIZE) {
		leafE = leafS;
		for (int_fast32_t nx = startx1; nx <= endx2; nx += FLOOR_SIZE) {
			if (leafE) {
				const CreatureVector& node_list = (onlyPlayers ? leafE->playerList : leafE->creatureList);
				CreatureVector::const_iterator node_iter = node_list.begin();
				CreatureVector::const_iterator node_end = node_list.end();
				if (node_iter != node_end) {
					do {
						Creature* creature = *node_iter;

						const Position& cpos = creature->getPosition();
						if (cpos.z < minRangeZ || cpos.z > maxRangeZ) {
							continue;
						}

						int_fast16_t offsetZ = Position::getOffsetZ(centerPos, cpos);
						if (cpos.y < (min_y + offsetZ) || cpos.y > (max_y + offsetZ)) {
							continue;
						}

						if (cpos.x < (min_x + offsetZ) || cpos.x > (max_x + offsetZ)) {
							continue;
						}

						list.insert(creature);
					} while (++node_iter != node_end);
				}
				leafE = leafE->m_leafE;
			} else {
				leafE = QTreeNode::getLeafStatic<const QTreeLeafNode*, const QTreeNode*>(&root, nx + FLOOR_SIZE, ny);
			}
		}

		if (leafS) {
			leafS = leafS->m_leafS;
		} else {
			leafS = QTreeNode::getLeafStatic<const QTreeLeafNode*, const QTreeNode*>(&root, startx1, ny + FLOOR_SIZE);
		}
	}
}

//void Map::getSpectators(SpectatorVec& list, const Position& centerPos, bool checkforduplicate /*= false*/, bool multifloor /*= false*/, bool onlyPlayers/* = false*/,
//	int32_t minRangeX /*= 0*/, int32_t maxRangeX /*= 0*/, int32_t minRangeY /*= 0*/, int32_t maxRangeY /*= 0*/)
void Map::getSpectators(SpectatorVec& list, const Position& centerPos, bool multifloor /*= false*/, bool onlyPlayers /*= false*/, int32_t minRangeX /*= 0*/, int32_t maxRangeX /*= 0*/, int32_t minRangeY /*= 0*/, int32_t maxRangeY /*= 0*/)
{
	if (centerPos.z >= MAP_MAX_LAYERS) {
		return;
	}

	bool foundCache = false;
	bool cacheResult = false;

	minRangeX = (minRangeX == 0 ? -maxViewportX : -minRangeX);
	maxRangeX = (maxRangeX == 0 ? maxViewportX : maxRangeX);
	minRangeY = (minRangeY == 0 ? -maxViewportY : -minRangeY);
	maxRangeY = (maxRangeY == 0 ? maxViewportY : maxRangeY);

	if (minRangeX == -maxViewportX && maxRangeX == maxViewportX && minRangeY == -maxViewportY && maxRangeY == maxViewportY && multifloor) {
		if (onlyPlayers) {
			auto it = playersSpectatorCache.find(centerPos);
			if (it != playersSpectatorCache.end()) {
				if (!list.empty()) {
					const SpectatorVec& cachedList = it->second;
					list.insert(cachedList.begin(), cachedList.end());
				} else {
					list = it->second;
				}

				foundCache = true;
			}
		}

		if (!foundCache) {
			auto it = spectatorCache.find(centerPos);
			if (it != spectatorCache.end()) {
				if (!onlyPlayers) {
					if (!list.empty()) {
						const SpectatorVec& cachedList = it->second;
						list.insert(cachedList.begin(), cachedList.end());
					} else {
						list = it->second;
					}
				} else {
					const SpectatorVec& cachedList = it->second;
					for (Creature* spectator : cachedList) {
						if (spectator->getPlayer()) {
							list.insert(spectator);
						}
					}
				}

				foundCache = true;
			} else {
				cacheResult = true;
			}
		}
	}

	if (!foundCache) {
		int32_t minRangeZ;
		int32_t maxRangeZ;

		if (multifloor) {
			if (centerPos.z > 7) {
				//underground

				//8->15
				minRangeZ = std::max<int32_t>(static_cast<int_fast16_t>(centerPos.z) - 2, 0);
				maxRangeZ = std::min<int32_t>(centerPos.z + 2, MAP_MAX_LAYERS - 1);
			} else if (centerPos.z == 6) {
				minRangeZ = 0;
				maxRangeZ = 8;
			} else if (centerPos.z == 7) {
				minRangeZ = 0;
				maxRangeZ = 9;
			} else {
				minRangeZ = 0;
				maxRangeZ = 7;
			}
		} else {
			minRangeZ = centerPos.z;
			maxRangeZ = centerPos.z;
		}

		getSpectatorsInternal(list, centerPos, minRangeX, maxRangeX, minRangeY, maxRangeY, minRangeZ, maxRangeZ, onlyPlayers);

		if (cacheResult) {
			if (onlyPlayers) {
				playersSpectatorCache[centerPos] = list;
			} else {
				spectatorCache[centerPos] = list;
			}
		}
	}
}

bool Map::canThrowObjectTo(const Position& fromPos, const Position& toPos, bool checkLineOfSight /*= true*/,
	int32_t rangex /*= Map::maxClientViewportX*/, int32_t rangey /*= Map::maxClientViewportY*/)
{
	//z checks
	//underground 8->15
	//ground level and above 7->0
	if((fromPos.z >= 8 && toPos.z < 8) || (toPos.z >= 8 && fromPos.z < 8))
		return false;

	int32_t deltaz = std::abs(fromPos.z - toPos.z);
	if(deltaz > 2)
		return false;

	int32_t deltax = std::abs(fromPos.x - toPos.x);
	int32_t deltay = std::abs(fromPos.y - toPos.y);

	//distance checks
	if(deltax - deltaz > rangex || deltay - deltaz > rangey)
		return false;

	if(!checkLineOfSight)
		return true;

	return isSightClear(fromPos, toPos, false);
}

bool Map::checkSightLine(const Position& fromPos, const Position& toPos) const
{
	if(fromPos == toPos)
		return true;

	Position start(fromPos.z > toPos.z ? toPos : fromPos);
	Position destination(fromPos.z > toPos.z ? fromPos : toPos);

	const int8_t mx = start.x < destination.x ? 1 : start.x == destination.x ? 0 : -1;
	const int8_t my = start.y < destination.y ? 1 : start.y == destination.y ? 0 : -1;

	int32_t A = destination.y - start.y;
	int32_t B = start.x - destination.x;
	int32_t C = -(A*destination.x + B*destination.y);

	while(!Position::areInRange<0,0,15>(start, destination))
	{
		int32_t move_hor = std::abs(A * (start.x + mx) + B * (start.y) + C);
		int32_t move_ver = std::abs(A * (start.x) + B * (start.y + my) + C);
		int32_t move_cross = std::abs(A * (start.x + mx) + B * (start.y + my) + C);

		if(start.y != destination.y && (start.x == destination.x || move_hor > move_ver || move_hor > move_cross))
			start.y += my;

		if(start.x != destination.x && (start.y == destination.y || move_ver > move_hor || move_ver > move_cross))
			start.x += mx;

		const Tile* tile = const_cast<Map*>(this)->getTile(start.x, start.y, start.z);
		if(tile && tile->hasProperty(BLOCKPROJECTILE))
			return false;
	}

	// now we need to perform a jump between floors to see if everything is clear (literally)
	while(start.z != destination.z)
	{
		const Tile* tile = const_cast<Map*>(this)->getTile(start.x, start.y, start.z);
		if(tile && tile->getThingCount() > 0)
			return false;

		start.z++;
	}
	return true;
}

bool Map::isSightClear(const Position& fromPos, const Position& toPos, bool floorCheck) const
{
	if(floorCheck && fromPos.z != toPos.z)
		return false;

	// Cast two converging rays and see if either yields a result.
	return checkSightLine(fromPos, toPos) || checkSightLine(toPos, fromPos);
}

const Tile* Map::canWalkTo(const Creature* creature, const Position& pos)
{
	switch(creature->getWalkCache(pos))
	{
		case 0:
			return NULL;
		case 1:
			return getTile(pos);
		default:
			break;
	}

	//used for none-cached tiles
	Tile* tile = getTile(pos);
	if(creature->getTile() != tile && (!tile || tile->__queryAdd(0, creature, 1,
		FLAG_PATHFINDING | FLAG_IGNOREFIELDDAMAGE) != RET_NOERROR))
		return NULL;

	return tile;
}

bool Map::getPathTo(const Creature* creature, const Position& destPos,
	std::list<Direction>& listDir, int32_t maxSearchDist /*= -1*/)
{
	FindPathParams fpp;
	fpp.maxSearchDist = maxSearchDist;
	getPathMatching(creature, listDir, FrozenPathingConditionCall(destPos), fpp);
	return !listDir.empty();
}

bool Map::getPathMatching(const Creature* creature, std::list<Direction>& dirList,
	const FrozenPathingConditionCall& pathCondition, const FindPathParams& fpp)
{
	dirList.clear();

	Position pos = creature->getPosition();
	Position endPos;

	AStarNodes nodes(pos.x, pos.y);

	int32_t bestMatch = 0;

	static int_fast32_t dirNeighbors[8][5][2] = {
		{{-1, 0}, {0, 1}, {1, 0}, {1, 1}, {-1, 1}},
		{{-1, 0}, {0, 1}, {0, -1}, {-1, -1}, {-1, 1}},
		{{-1, 0}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}},
		{{0, 1}, {1, 0}, {0, -1}, {1, -1}, {1, 1}},
		{{1, 0}, {0, -1}, {-1, -1}, {1, -1}, {1, 1}},
		{{-1, 0}, {0, -1}, {-1, -1}, {1, -1}, {-1, 1}},
		{{0, 1}, {1, 0}, {1, -1}, {1, 1}, {-1, 1}},
		{{-1, 0}, {0, 1}, {-1, -1}, {1, 1}, {-1, 1}}
	};
	static int_fast32_t allNeighbors[8][2] = {
		{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}, {1, 1}, {-1, 1}
	};

	const Position startPos = pos;

	AStarNode* found = NULL;
	while (fpp.maxSearchDist != -1 || nodes.getClosedNodes() < fpp.maxClosedNodes) {
		AStarNode* n = nodes.getBestNode();
		if (!n) {
			if (found) {
				break;
			}
			return false;
		}

		const int_fast32_t x = n->x;
		const int_fast32_t y = n->y;
		pos.x = x;
		pos.y = y;
		if (pathCondition(startPos, pos, fpp, bestMatch)) {
			found = n;
			endPos = pos;
			if (bestMatch == 0) {
				break;
			}
		}

		uint_fast32_t dirCount;
		int_fast32_t* neighbors;
		if (n->parent) {
			const int_fast32_t offset_x = n->parent->x - x;
			const int_fast32_t offset_y = n->parent->y - y;
			if (offset_y == 0) {
				if (offset_x == -1) {
					neighbors = *dirNeighbors[WEST];
				} else {
					neighbors = *dirNeighbors[EAST];
				}
			} else if (!fpp.allowDiagonal || offset_x == 0) {
				if (offset_y == -1) {
					neighbors = *dirNeighbors[NORTH];
				} else {
					neighbors = *dirNeighbors[SOUTH];
				}
			} else if (offset_y == -1) {
				if (offset_x == -1) {
					neighbors = *dirNeighbors[NORTHWEST];
				} else {
					neighbors = *dirNeighbors[NORTHEAST];
				}
			} else if (offset_x == -1) {
				neighbors = *dirNeighbors[SOUTHWEST];
			} else {
				neighbors = *dirNeighbors[SOUTHEAST];
			}
			dirCount = fpp.allowDiagonal ? 5 : 3;
		} else {
			dirCount = 8;
			neighbors = *allNeighbors;
		}

		const int_fast32_t f = n->f;
		for (uint_fast32_t i = 0; i < dirCount; ++i) {
			pos.x = x + *neighbors++;
			pos.y = y + *neighbors++;

			if(fpp.maxSearchDist != -1 && (std::abs(startPos.x - pos.x) > fpp.maxSearchDist ||
				std::abs(startPos.y - pos.y) > fpp.maxSearchDist))
				continue;

			if(fpp.keepDistance && !pathCondition.isInRange(startPos, pos, fpp)) {
				continue;
			}


			const Tile* tile;
			AStarNode* neighborNode = nodes.getNodeByPosition(pos.x, pos.y);
			if (neighborNode) {
				tile = getTile(pos.x, pos.y, pos.z);
			} else {
				tile = canWalkTo(creature, pos);
				if (!tile) {
					continue;
				}
			}

			//The cost (g) for this neighbor
			const int_fast32_t cost = AStarNodes::getMapWalkCost(n, pos);
			const int_fast32_t extraCost = AStarNodes::getTileWalkCost(creature, tile);
			const int_fast32_t newf = f + cost + extraCost;

			if (neighborNode) {
				if (neighborNode->f <= newf) {
					//The node on the closed/open list is cheaper than this one
					continue;
				}

				neighborNode->f = newf;
				neighborNode->parent = n;
				nodes.openNode(neighborNode);
			} else {
				//Does not exist in the open/closed list, create a new node
				neighborNode = nodes.createOpenNode(n, pos.x, pos.y, newf);
				if (!neighborNode) {
					if (found) {
						break;
					}
					return false;
				}
			}
		}

		nodes.closeNode(n);
	}

	if (!found) {
		return false;
	}

	int_fast32_t prevx = endPos.x;
	int_fast32_t prevy = endPos.y;

	found = found->parent;
	while (found) {
		pos.x = found->x;
		pos.y = found->y;

		int_fast32_t dx = pos.x - prevx;
		int_fast32_t dy = pos.y - prevy;

		prevx = pos.x;
		prevy = pos.y;

		if (dx == 1 && dy == 1) {
			dirList.push_front(NORTHWEST);
		} else if (dx == -1 && dy == 1) {
			dirList.push_front(NORTHEAST);
		} else if (dx == 1 && dy == -1) {
			dirList.push_front(SOUTHWEST);
		} else if (dx == -1 && dy == -1) {
			dirList.push_front(SOUTHEAST);
		} else if (dx == 1) {
			dirList.push_front(WEST);
		} else if (dx == -1) {
			dirList.push_front(EAST);
		} else if (dy == 1) {
			dirList.push_front(NORTH);
		} else if (dy == -1) {
			dirList.push_front(SOUTH);
		}

		found = found->parent;
	}
	return true;
}

//*********** AStarNodes *************

AStarNodes::AStarNodes(uint32_t x, uint32_t y)
	: openNodes()
{
	curNode = 1;
	closedNodes = 0;
	openNodes[0] = true;

	AStarNode& startNode = nodes[0];
	startNode.parent = nullptr;
	startNode.x = x;
	startNode.y = y;
	startNode.f = 0;
	nodeTable[(x << 16) | y] = &startNode;
}

AStarNode* AStarNodes::createOpenNode(AStarNode* parent, uint32_t x, uint32_t y, int_fast32_t f)
{
	if (curNode >= MAX_NODES) {
		return nullptr;
	}

	size_t retNode = curNode++;
	openNodes[retNode] = true;

	AStarNode* node = &nodes[retNode];
	nodeTable[(x << 16) | y] = node;
	node->parent = parent;
	node->x = x;
	node->y = y;
	node->f = f;
	return node;
}

AStarNode* AStarNodes::getBestNode()
{
	if (curNode == 0) {
		return nullptr;
	}

	int32_t best_node_f = std::numeric_limits<int32_t>::max();
	int32_t best_node = -1;
	for (size_t i = 0; i < curNode; i++) {
		if (openNodes[i] && nodes[i].f < best_node_f) {
			best_node_f = nodes[i].f;
			best_node = i;
		}
	}

	if (best_node >= 0) {
		return &nodes[best_node];
	}
	return nullptr;
}

void AStarNodes::closeNode(AStarNode* node)
{
	size_t pos = GET_NODE_INDEX(node);
	if (pos >= MAX_NODES) {
		std::cout << "AStarNodes. trying to close node out of range" << std::endl;
		return;
	}

	openNodes[pos] = false;
	++closedNodes;
}

void AStarNodes::openNode(AStarNode* node)
{
	size_t pos = GET_NODE_INDEX(node);
	if (pos >= MAX_NODES) {
		std::cout << "AStarNodes. trying to open node out of range" << std::endl;
		return;
	}

	if (!openNodes[pos]) {
		openNodes[pos] = true;
		--closedNodes;
	}
}

int_fast32_t AStarNodes::getClosedNodes() const
{
	return closedNodes;
}

AStarNode* AStarNodes::getNodeByPosition(uint32_t x, uint32_t y)
{
	auto it = nodeTable.find((x << 16) | y);
	if (it == nodeTable.end()) {
		return nullptr;
	}
	return it->second;
}

int_fast32_t AStarNodes::getMapWalkCost(AStarNode* node, const Position& neighborPos)
{
	if (std::abs(node->x - neighborPos.x) == std::abs(node->y - neighborPos.y)) {
		//diagonal movement extra cost
		return MAP_DIAGONALWALKCOST;
	}
	return MAP_NORMALWALKCOST;
}

int_fast32_t AStarNodes::getTileWalkCost(const Creature* creature, const Tile* tile)
{
	int_fast32_t cost = 0;
	if(tile->getTopVisibleCreature(creature)) //destroy creature cost
		cost += MAP_NORMALWALKCOST * 3;

	if(const MagicField* field = tile->getFieldItem())
	{
		if(!creature->isImmune(field->getCombatType()))
			cost += MAP_NORMALWALKCOST * 3;
	}

	return cost;
}

//*********** Floor constructor **************

Floor::Floor()
{
	for(int32_t i = 0; i < FLOOR_SIZE; ++i)
	{
		for(int32_t j = 0; j < FLOOR_SIZE; ++j)
			tiles[i][j] = 0;
	}
}

//**************** QTreeNode **********************
QTreeNode::QTreeNode()
{
	m_isLeaf = false;
	for(int32_t i = 0; i < 4; ++i)
		m_child[i] = NULL;
}

QTreeNode::~QTreeNode()
{
	for(int32_t i = 0; i < 4; ++i)
		delete m_child[i];
}

QTreeLeafNode* QTreeNode::getLeaf(uint16_t x, uint16_t y)
{
	if(isLeaf())
		return static_cast<QTreeLeafNode*>(this);

	uint32_t index = ((x & 0x8000) >> 15) | ((y & 0x8000) >> 14);
	if(m_child[index])
		return m_child[index]->getLeaf(x << 1, y << 1);

	return NULL;
}

QTreeLeafNode* QTreeNode::createLeaf(uint16_t x, uint16_t y, uint16_t level)
{
	if(!isLeaf())
	{
		uint32_t index = ((x & 0x8000) >> 15) | ((y & 0x8000) >> 14);
		if(!m_child[index])
		{
			if(level != FLOOR_BITS)
				m_child[index] = new QTreeNode();
			else
			{
				m_child[index] = new QTreeLeafNode();
				QTreeLeafNode::newLeaf = true;
			}
		}

		return m_child[index]->createLeaf(x << 1, y << 1, level - 1);
	}

	return static_cast<QTreeLeafNode*>(this);
}

void QTreeLeafNode::addCreature(Creature* c)
{
	creatureList.push_back(c);
	if (c->getPlayer()) {
		playerList.push_back(c);
	}
}

void QTreeLeafNode::removeCreature(Creature* c)
{
	CreatureVector::iterator it = std::find(creatureList.begin(), creatureList.end(), c);
	assert(it != creatureList.end());
	creatureList.erase(it);

	if (c->getPlayer()) {
		it = std::find(playerList.begin(), playerList.end(), c);
		assert(it != playerList.end());
		playerList.erase(it);
	}
}

//************ LeafNode  ************************
bool QTreeLeafNode::newLeaf = false;
QTreeLeafNode::QTreeLeafNode()
{
	for(int32_t i = 0; i < MAP_MAX_LAYERS; ++i)
		m_array[i] = NULL;

	m_isLeaf = true;
	m_leafS = NULL;
	m_leafE = NULL;
}

QTreeLeafNode::~QTreeLeafNode()
{
	for(int32_t i = 0; i < MAP_MAX_LAYERS; ++i)
		delete m_array[i];
}

Floor* QTreeLeafNode::createFloor(uint16_t z)
{
	if(!m_array[z])
		m_array[z] = new Floor();

	return m_array[z];
}
