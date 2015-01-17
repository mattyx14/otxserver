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

#ifndef __TRASHHOLDER__
#define __TRASHHOLDER__

#include "tile.h"
#include "const.h"

class TrashHolder : public Item, public Cylinder
{
	public:
		TrashHolder(uint16_t type, MagicEffect_t _effect = MAGIC_EFFECT_NONE): Item(type), effect(_effect) {}
		virtual ~TrashHolder() {}

		virtual TrashHolder* getTrashHolder() {return this;}
		virtual const TrashHolder* getTrashHolder() const {return this;}

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

		virtual ReturnValue __queryAdd(int32_t, const Thing*, uint32_t,
			uint32_t, Creature* = NULL) const {return RET_NOERROR;}
		virtual ReturnValue __queryMaxCount(int32_t, const Thing*, uint32_t,
			uint32_t&, uint32_t) const {return RET_NOERROR;}
		virtual ReturnValue __queryRemove(const Thing*, uint32_t,
			uint32_t, Creature* = NULL) const {return RET_NOTPOSSIBLE;}
		virtual Cylinder* __queryDestination(int32_t&, const Thing*, Item**,
			uint32_t&) {return this;}

		virtual void __addThing(Creature* actor, Thing* thing) {return __addThing(actor, 0, thing);}
		virtual void __addThing(Creature* actor, int32_t index, Thing* thing);

		virtual void __updateThing(Thing*, uint16_t, uint32_t) {}
		virtual void __replaceThing(uint32_t, Thing*) {}

		virtual void __removeThing(Thing*, uint32_t) {}

		virtual void postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
			int32_t index, CylinderLink_t)
		{
			if(getParent())
				getParent()->postAddNotification(actor, thing, oldParent, index, LINK_PARENT);
		}
		virtual void postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
			int32_t index, bool isCompleteRemoval, CylinderLink_t)
		{
			if(getParent())
				getParent()->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_PARENT);
		}

		MagicEffect_t getEffect() const {return effect;}

	private:
		MagicEffect_t effect;
};
#endif
