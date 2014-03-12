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

#ifndef __HOUSETILE__
#define __HOUSETILE__
#include "tile.h"

class House;
class HouseTile : public DynamicTile
{
	public:
		HouseTile(int32_t x, int32_t y, int32_t z, House* _house);
		virtual ~HouseTile() {}

		virtual HouseTile* getHouseTile() {return this;}
		virtual const HouseTile* getHouseTile() const {return this;}
		virtual House* getHouse() {return house;}
		virtual const House* getHouse() const {return house;}

		//cylinder implementations
		virtual ReturnValue __queryAdd(int32_t index, const Thing* thing, uint32_t count,
			uint32_t flags, Creature* actor = NULL) const;
		virtual Cylinder* __queryDestination(int32_t& index, const Thing* thing, Item** destItem,
			uint32_t& flags);
		virtual ReturnValue __queryRemove(const Thing* thing, uint32_t count, uint32_t flags, Creature* actor = NULL) const;

		virtual void __addThing(Creature* actor, int32_t index, Thing* thing);
		virtual void __internalAddThing(uint32_t index, Thing* thing);

	private:
		void updateHouse(Item* item);
		House* house;
};
#endif
