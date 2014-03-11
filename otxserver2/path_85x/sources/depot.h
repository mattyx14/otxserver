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

#ifndef __DEPOT__
#define __DEPOT__
#include "container.h"

class Depot : public Container
{
	public:
		Depot(uint16_t type);
		virtual ~Depot() {}

		virtual Depot* getDepot() {return this;}
		virtual const Depot* getDepot() const {return this;}

		//serialization
		virtual Attr_ReadValue readAttr(AttrTypes_t attr, PropStream& propStream);

		uint32_t getDepotId() const;

		void setMaxDepotLimit(uint32_t count) {depotLimit = count;}

		//cylinder implementations
		virtual Cylinder* getParent() {return Item::getParent();}
		virtual const Cylinder* getParent() const {return Item::getParent();}
		virtual bool isRemoved() const {return Item::isRemoved();}
		virtual Position getPosition() const {return Item::getPosition();}
		virtual Tile* getTile() {return Item::getTile();}
		virtual const Tile* getTile() const {return Item::getTile();}
		virtual Item* getItem() {return this;}
		virtual const Item* getItem() const {return this;}
		virtual Creature* getCreature() {return NULL;}
		virtual const Creature* getCreature() const {return NULL;}

		virtual ReturnValue __queryAdd(int32_t index, const Thing* thing, uint32_t count,
			uint32_t flags, Creature* actor = NULL) const;

		virtual ReturnValue __queryMaxCount(int32_t index, const Thing* thing, uint32_t count,
			uint32_t& maxQueryCount, uint32_t flags) const;

		virtual void postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
			int32_t index, CylinderLink_t link = LINK_OWNER);
		virtual void postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
			int32_t index, bool isCompleteRemoval, CylinderLink_t link = LINK_OWNER);

		//overrides
		virtual bool canRemove() const {return false;}

	private:
		uint32_t depotLimit;
};

inline uint32_t Depot::getDepotId() const
{
	bool ok;
	int32_t v = getIntegerAttribute("depotid", ok);
	if(ok)
		return (uint32_t)v;

	return 0;
}
#endif
