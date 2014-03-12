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
#include "thing.h"

#include "cylinder.h"
#include "tile.h"

#include "item.h"
#include "creature.h"
#include "player.h"

Cylinder* Thing::getTopParent()
{
	//tile
	Cylinder* aux = getParent();
	if(!aux)
		return dynamic_cast<Cylinder*>(this);

	Cylinder* prev = dynamic_cast<Cylinder*>(this);
	while(aux->getParent())
	{
		prev = aux;
		aux = aux->getParent();
	}

	if(dynamic_cast<Cylinder*>(prev))
		return prev;

	return aux;
}

const Cylinder* Thing::getTopParent() const
{
	//tile
	const Cylinder* aux = getParent();
	if(!aux)
		return dynamic_cast<const Cylinder*>(this);

	const Cylinder* prev = dynamic_cast<const Cylinder*>(this);
	while(aux->getParent())
	{
		prev = aux;
		aux = aux->getParent();
	}

	if(dynamic_cast<const Cylinder*>(prev))
		return prev;

	return aux;
}

Tile* Thing::getTile()
{
	Cylinder* cylinder = getTopParent();
#ifdef __DEBUG_MOVESYS__
	if(!cylinder)
	{
		std::clog << "[Failure - Thing::getTile] NULL tile" << std::endl;
		return &(Tile::nullTile);
	}
#endif

	if(cylinder->getParent())
		cylinder = cylinder->getParent();

	return dynamic_cast<Tile*>(cylinder);
}

const Tile* Thing::getTile() const
{
	const Cylinder* cylinder = getTopParent();
#ifdef __DEBUG_MOVESYS__
	if(!cylinder)
	{
		std::clog << "[Failure - Thing::getTile] NULL tile" << std::endl;
		return &(Tile::nullTile);
	}
#endif

	if(cylinder->getParent())
		cylinder = cylinder->getParent();

	return dynamic_cast<const Tile*>(cylinder);
}

Position Thing::getPosition() const
{
	if(const Tile* tile = getTile())
		return tile->getPosition();

#ifdef __DEBUG_MOVESYS__
	std::clog << "[Failure - Thing::getTile] NULL tile" << std::endl;
#endif
	return Tile::nullTile.getPosition();
}

bool Thing::isRemoved() const
{
	const Cylinder* aux = getParent();
	if(!aux)
		return true;

	return aux->isRemoved();
}
