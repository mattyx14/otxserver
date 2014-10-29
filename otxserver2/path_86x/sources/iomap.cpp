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
#include "iomap.h"

#include "map.h"
#include "town.h"
#include "tile.h"
#include "item.h"
#include "container.h"
#include "depot.h"

#include "teleport.h"
#include "beds.h"

#include "fileloader.h"
#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;

typedef uint8_t attribute_t;
typedef uint32_t flags_t;

/*
	OTBM_ROOTV2
	|
	|--- OTBM_MAP_DATA
	|	|
	|	|--- OTBM_TILE_AREA
	|	|	|--- OTBM_TILE
	|	|	|--- OTBM_TILE_SQUARE (not implemented)
	|	|	|--- OTBM_TILE_REF (not implemented)
	|	|	|--- OTBM_HOUSETILE
	|	|
	|	|--- OTBM_SPAWNS (not implemented)
	|	|	|--- OTBM_SPAWN_AREA (not implemented)
	|	|	|--- OTBM_MONSTER (not implemented)
	|	|
	|	|--- OTBM_TOWNS
	|	|	|--- OTBM_TOWN
	|	|
	|	|--- OTBM_WAYPOINTS
	|		|--- OTBM_WAYPOINT
	|
	|--- OTBM_ITEM_DEF (not implemented)
*/

Tile* IOMap::createTile(Item*& ground, Item* item, uint16_t px, uint16_t py, uint16_t pz)
{
	Tile* tile = NULL;
	if(ground)
	{
		if((item && item->isBlocking(NULL)) || ground->isBlocking(NULL)) //tile is blocking with possibly some decoration, should be static
			tile = new StaticTile(px, py, pz);
		else //tile is not blocking with possibly multiple items, use dynamic
			tile = new DynamicTile(px, py, pz);

		tile->__internalAddThing(ground);
		if(ground->getDecaying() != DECAYING_TRUE)
		{
			ground->__startDecaying();
			ground->setLoadedFromMap(true);
		}

		ground = NULL;
	}
	else //no ground on this tile, so it will always block
		tile = new StaticTile(px, py, pz);

	return tile;
}

bool IOMap::loadMap(Map* map, const std::string& identifier)
{
	FileLoader f;
	if(!f.openFile(identifier.c_str(), "OTBM", false, true))
	{
		std::stringstream ss;
		ss << "Could not open the file " << identifier << ".";
		setLastErrorString(ss.str());
		return false;
	}

	uint32_t type = 0;
	NODE root = f.getChildNode((NODE)NULL, type);

	PropStream propStream;
	if(!f.getProps(root, propStream))
	{
		setLastErrorString("Could not read root property.");
		return false;
	}

	OTBM_root_header* rootHeader;
	if(!propStream.getStruct(rootHeader))
	{
		setLastErrorString("Could not read header.");
		return false;
	}

	uint32_t headerVersion = rootHeader->version;
	if(headerVersion <= 0)
	{
		//In otbm version 1 the count variable after splashes/fluidcontainers and stackables
		//are saved as attributes instead, this solves alot of problems with items
		//that is changed (stackable/charges/fluidcontainer/splash) during an update.
		setLastErrorString("This map needs to be upgraded by using the latest map editor version to be able to load correctly.");
		return false;
	}

	if(headerVersion > 3)
	{
		setLastErrorString("Unknown OTBM version detected.");
		return false;
	}

	uint32_t headerMajorItems = rootHeader->majorVersionItems;
	if(headerMajorItems < 3)
	{
		setLastErrorString("This map needs to be upgraded by using the latest map editor version to be able to load correctly.");
		return false;
	}

	if(headerMajorItems > (uint32_t)Items::dwMajorVersion)
	{
		setLastErrorString("The map was saved with a different items.otb version, an upgraded items.otb is required.");
		return false;
	}

	uint32_t headerMinorItems = rootHeader->minorVersionItems;
	if(headerMinorItems < CLIENT_VERSION_810)
	{
		setLastErrorString("This map needs an updated items.otb.");
		return false;
	}

	if(headerMinorItems > (uint32_t)Items::dwMinorVersion)
		setLastErrorString("This map needs an updated items.otb.");

	std::clog << ">>> Map size: " << rootHeader->width << "x" << rootHeader->height << "." << std::endl;
	map->mapWidth = rootHeader->width;
	map->mapHeight = rootHeader->height;

	NODE nodeMap = f.getChildNode(root, type);
	if(type != OTBM_MAP_DATA)
	{
		setLastErrorString("Could not read data node.");
		return false;
	}

	if(!f.getProps(nodeMap, propStream))
	{
		setLastErrorString("Could not read map data attributes.");
		return false;
	}

	std::string tmp;
	uint8_t attribute;
	while(propStream.getByte(attribute))
	{
		switch(attribute)
		{
			case OTBM_ATTR_DESCRIPTION:
			{
				if(!propStream.getString(tmp))
				{
					setLastErrorString("Invalid description tag.");
					return false;
				}

				map->descriptions.push_back(tmp);
				break;
			}
			case OTBM_ATTR_EXT_SPAWN_FILE:
			{
				if(!propStream.getString(tmp))
				{
					setLastErrorString("Invalid spawnfile tag.");
					return false;
				}

				map->spawnfile = identifier.substr(0, identifier.rfind('/') + 1);
				map->spawnfile += tmp;
				break;
			}
			case OTBM_ATTR_EXT_HOUSE_FILE:
			{
				if(!propStream.getString(tmp))
				{
					setLastErrorString("Invalid housefile tag.");
					return false;
				}

				map->housefile = identifier.substr(0, identifier.rfind('/') + 1);
				map->housefile += tmp;
				break;
			}
			default:
			{
				setLastErrorString("Unknown header node.");
				return false;
			}
		}
	}

	std::clog << ">>> Map descriptions:";
	for(StringVec::iterator it = map->descriptions.begin(); it != map->descriptions.end(); ++it)
		std::clog << " - \"" << (*it) << "\"";

#ifdef __GROUND_CACHE__
	typedef std::map<uint16_t, std::pair<Item*, int32_t> > CacheMap;
	CacheMap groundCache;

#endif
	NODE nodeMapData = f.getChildNode(nodeMap, type);
	while(nodeMapData != NO_NODE)
	{
		if(f.getError() != ERROR_NONE)
		{
			setLastErrorString("Invalid map node.");
			return false;
		}

		if(type == OTBM_TILE_AREA)
		{
			if(!f.getProps(nodeMapData, propStream))
			{
				setLastErrorString("Invalid map node.");
				return false;
			}

			OTBM_Destination_coords* areaCoord;
			if(!propStream.getStruct(areaCoord))
			{
				setLastErrorString("Invalid map node.");
				return false;
			}

			int32_t baseX = areaCoord->_x, baseY = areaCoord->_y, baseZ = areaCoord->_z;
			NODE nodeTile = f.getChildNode(nodeMapData, type);
			while(nodeTile != NO_NODE)
			{
				if(f.getError() != ERROR_NONE)
				{
					setLastErrorString("Could not read node data.");
					return false;
				}

				if(type == OTBM_TILE || type == OTBM_HOUSETILE)
				{
					if(!f.getProps(nodeTile, propStream))
					{
						setLastErrorString("Could not read node data.");
						return false;
					}

					OTBM_Tile_coords* tileCoord;
					if(!propStream.getStruct(tileCoord))
					{
						setLastErrorString("Could not read tile position.");
						return false;
					}

					Tile* tile = NULL;
					Item* ground = NULL;
					uint32_t flags = 0;

					uint16_t px = baseX + tileCoord->_x, py = baseY + tileCoord->_y, pz = baseZ;
					House* house = NULL;
					if(type == OTBM_HOUSETILE)
					{
						uint32_t houseId;
						if(!propStream.getLong(houseId))
						{
							std::stringstream ss;
							ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Could not read house id.";

							setLastErrorString(ss.str());
							return false;
						}

						house = Houses::getInstance()->getHouse(houseId, true);
						if(!house)
						{
							std::stringstream ss;
							ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Could not create house id: " << houseId;

							setLastErrorString(ss.str());
							return false;
						}

						tile = new HouseTile(px, py, pz, house);
						house->addTile(static_cast<HouseTile*>(tile));
					}

					//read tile attributes
					uint8_t attribute = 0;
					while(propStream.getByte(attribute))
					{
						switch(attribute)
						{
							case OTBM_ATTR_TILE_FLAGS:
							{
								uint32_t _flags;
								if(!propStream.getLong(_flags))
								{
									std::stringstream ss;
									ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Failed to read tile flags.";

									setLastErrorString(ss.str());
									return false;
								}

								if((_flags & TILESTATE_PROTECTIONZONE) == TILESTATE_PROTECTIONZONE)
									flags |= TILESTATE_PROTECTIONZONE;
								else if((_flags & TILESTATE_OPTIONALZONE) == TILESTATE_OPTIONALZONE)
									flags |= TILESTATE_OPTIONALZONE;
								else if((_flags & TILESTATE_HARDCOREZONE) == TILESTATE_HARDCOREZONE)
									flags |= TILESTATE_HARDCOREZONE;

								if((_flags & TILESTATE_NOLOGOUT) == TILESTATE_NOLOGOUT)
									flags |= TILESTATE_NOLOGOUT;

								if((_flags & TILESTATE_REFRESH) == TILESTATE_REFRESH)
								{
									if(house)
										std::clog << "[x:" << px << ", y:" << py << ", z:" << pz << "] House tile flagged as refreshing!";

									flags |= TILESTATE_REFRESH;
								}

								break;
							}

							case OTBM_ATTR_ITEM:
							{
								Item* item = Item::CreateItem(propStream);
								if(!item)
								{
									std::stringstream ss;
									ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Failed to create item.";

									setLastErrorString(ss.str());
									return false;
								}

								if(item->getItemCount() <= 0)
									item->setItemCount(1);

								if(house && item->isMovable())
								{
									std::clog << "[Warning - IOMap::loadMap] Movable item in house: " << house->getId()
										<< ", item type: " << item->getID() << ", at position " << px << "/" << py << "/"
										<< pz << std::endl;

									delete item;
									item = NULL;
								}
								else if(tile)
								{
									tile->__internalAddThing(item);
									if(item->getDecaying() != DECAYING_TRUE)
									{
										item->__startDecaying();
										item->setLoadedFromMap(true);
									}
								}
								else if(item->isGroundTile())
								{
									if(ground)
									{
#ifdef __GROUND_CACHE__
										CacheMap::iterator it = groundCache.find(ground->getID());
										bool erase = it == groundCache.end();
										if(!erase)
										{
											it->second.second--;
											erase = it->second.second < 1;
											if(erase)
												groundCache.erase(it);
										}

										if(erase)
#endif
											delete ground;
									}

#ifdef __GROUND_CACHE__
									const ItemType& tit = Item::items[item->getID()];
									if(!(tit.magicEffect != MAGIC_EFFECT_NONE || !tit.walkStack || tit.transformUseTo != 0 || tit.cache ||
										item->floorChange() || item->canDecay() || item->getActionId() > 0 || item->getUniqueId() > 0))
									{
										CacheMap::iterator it = groundCache.find(item->getID());
										if(it != groundCache.end())
										{
											delete item;
											item = it->second.first;
											it->second.second++;
										}
										else
											groundCache[item->getID()] = std::make_pair(item, 1);
									}

#endif
									ground = item;
								}
								else
								{
									tile = createTile(ground, item, px, py, pz);
									tile->__internalAddThing(item);
									if(item->getDecaying() != DECAYING_TRUE)
									{
										item->__startDecaying();
										item->setLoadedFromMap(true);
									}
								}

								break;
							}

							default:
							{
								std::stringstream ss;
								ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Unknown tile attribute.";

								setLastErrorString(ss.str());
								return false;
							}
						}
					}

					NODE nodeItem = f.getChildNode(nodeTile, type);
					while(nodeItem)
					{
						if(type == OTBM_ITEM)
						{
							PropStream propStream;
							f.getProps(nodeItem, propStream);

							Item* item = Item::CreateItem(propStream);
							if(!item)
							{
								std::stringstream ss;
								ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Failed to create item.";

								setLastErrorString(ss.str());
								return false;
							}

							if(item->unserializeItemNode(f, nodeItem, propStream))
							{
								if(item->getItemCount() <= 0)
									item->setItemCount(1);

								if(house && item->isMovable())
								{
									std::clog << "[Warning - IOMap::loadMap] Movable item in house: "
										<< house->getId() << ", item type: " << item->getID()
										<< ", pos " << px << "/" << py << "/" << pz << std::endl;

									delete item;
									item = NULL;
								}
								else if(tile)
								{
									tile->__internalAddThing(item);
									if(item->getDecaying() != DECAYING_TRUE)
									{
										item->__startDecaying();
										item->setLoadedFromMap(true);
									}
								}
								else if(item->isGroundTile())
								{
									if(ground)
									{
#ifdef __GROUND_CACHE__
										CacheMap::iterator it = groundCache.find(ground->getID());
										bool erase = it == groundCache.end();
										if(!erase)
										{
											it->second.second--;
											erase = it->second.second < 1;
											if(erase)
												groundCache.erase(it);
										}

										if(erase)
#endif
											delete ground;
									}

#ifdef __GROUND_CACHE__
									const ItemType& tit = Item::items[item->getID()];
									if(!(tit.magicEffect != MAGIC_EFFECT_NONE || !tit.walkStack || tit.transformUseTo != 0 || tit.cache ||
										item->floorChange() || item->canDecay() || item->getActionId() > 0 || item->getUniqueId() > 0))
									{
										CacheMap::iterator it = groundCache.find(item->getID());
										if(it != groundCache.end())
										{
											delete item;
											item = it->second.first;
											it->second.second++;
										}
										else
											groundCache[item->getID()] = std::make_pair(item, 1);
									}

#endif
									ground = item;
								}
								else
								{
									tile = createTile(ground, item, px, py, pz);
									tile->__internalAddThing(item);
									if(item->getDecaying() != DECAYING_TRUE)
									{
										item->__startDecaying();
										item->setLoadedFromMap(true);
									}
								}
							}
							else
							{
								std::stringstream ss;
								ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Failed to load item " << item->getID() << ".";
								setLastErrorString(ss.str());

								delete item;
								item = NULL;
								return false;
							}
						}
						else
						{
							std::stringstream ss;
							ss << "[x:" << px << ", y:" << py << ", z:" << pz << "] Unknown node type.";
							setLastErrorString(ss.str());
						}

						nodeItem = f.getNextNode(nodeItem, type);
					}

					if(!tile)
						tile = createTile(ground, NULL, px, py, pz);

					tile->setFlag((tileflags_t)flags);
					map->setTile(px, py, pz, tile);
				}
				else
				{
					setLastErrorString("Unknown tile node.");
					return false;
				}

				nodeTile = f.getNextNode(nodeTile, type);
			}
		}
		else if(type == OTBM_TOWNS)
		{
			NODE nodeTown = f.getChildNode(nodeMapData, type);
			while(nodeTown != NO_NODE)
			{
				if(type == OTBM_TOWN)
				{
					if(!f.getProps(nodeTown, propStream))
					{
						setLastErrorString("Could not read town data.");
						return false;
					}

					uint32_t townId = 0;
					if(!propStream.getLong(townId))
					{
						setLastErrorString("Could not read town id.");
						return false;
					}

					Town* town = Towns::getInstance()->getTown(townId);
					if(!town)
					{
						town = new Town(townId);
						Towns::getInstance()->addTown(townId, town);
					}

					std::string townName;
					if(!propStream.getString(townName))
					{
						setLastErrorString("Could not read town name.");
						return false;
					}

					town->setName(townName);
					OTBM_Destination_coords *townCoords;
					if(!propStream.getStruct(townCoords))
					{
						setLastErrorString("Could not read town coordinates.");
						return false;
					}

					town->setPosition(Position(townCoords->_x, townCoords->_y, townCoords->_z));
				}
				else
				{
					setLastErrorString("Unknown town node.");
					return false;
				}

				nodeTown = f.getNextNode(nodeTown, type);
			}
		}
		else if(type == OTBM_WAYPOINTS && headerVersion > 1)
		{
			NODE nodeWaypoint = f.getChildNode(nodeMapData, type);
			while(nodeWaypoint != NO_NODE)
			{
				if(type == OTBM_WAYPOINT)
				{
					if(!f.getProps(nodeWaypoint, propStream))
					{
						setLastErrorString("Could not read waypoint data.");
						return false;
					}

					std::string name;
					if(!propStream.getString(name))
					{
						setLastErrorString("Could not read waypoint name.");
						return false;
					}

					OTBM_Destination_coords* waypoint_coords;
					if(!propStream.getStruct(waypoint_coords))
					{
						setLastErrorString("Could not read waypoint coordinates.");
						return false;
					}

					map->waypoints.addWaypoint(WaypointPtr(new Waypoint(name,
						Position(waypoint_coords->_x, waypoint_coords->_y, waypoint_coords->_z))));
				}
				else
				{
					setLastErrorString("Unknown waypoint node.");
					return false;
				}

				nodeWaypoint = f.getNextNode(nodeWaypoint, type);
			}
		}
		else
		{
			setLastErrorString("Unknown map node.");
			return false;
		}

		nodeMapData = f.getNextNode(nodeMapData, type);
	}

#ifdef __GROUND_CACHE__
	for(CacheMap::iterator it = groundCache.begin(); it != groundCache.end(); ++it)
	{
		//it->second.first->setParent(NULL);
		g_game.grounds[it->second.first] = it->second.second;
	}

	groundCache.clear();
#endif
	return true;
}

bool IOMap::loadSpawns(Map* map)
{
	if(map->spawnfile.empty())
		map->spawnfile =  g_config.getString(ConfigManager::MAP_NAME) + "-spawn.xml";

	return Spawns::getInstance()->loadFromXml(map->spawnfile);
}

bool IOMap::loadHouses(Map* map)
{
	if(map->housefile.empty())
		map->housefile = g_config.getString(ConfigManager::MAP_NAME) + "-house.xml";

	return Houses::getInstance()->loadFromXml(map->housefile);
}
