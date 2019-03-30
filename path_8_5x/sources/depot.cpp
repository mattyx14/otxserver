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
#include "depot.h"
#include "tools.h"

Depot::Depot(uint16_t type):
	Container(type), depotLimit(1000)
{}

Attr_ReadValue Depot::readAttr(AttrTypes_t attr, PropStream& propStream)
{
	if(attr != ATTR_DEPOT_ID)
		return Item::readAttr(attr, propStream);

	uint16_t depotId;
	if(!propStream.getShort(depotId))
		return ATTR_READ_ERROR;

	setAttribute("depotid", depotId);
	return ATTR_READ_CONTINUE;
}

ReturnValue Depot::__queryAdd(int32_t index, const Thing* thing, uint32_t count,
	uint32_t flags, Creature* actor/* = NULL*/) const
{
	const Item* item = thing->getItem();
	if(!item)
		return RET_NOTPOSSIBLE;

	int32_t addCount = 0;
	if((item->isStackable() && item->getItemCount() != count))
		addCount = 1;

	if(item->getTopParent() != this)
	{
		if(const Container* container = item->getContainer())
			addCount = container->getItemHoldingCount() + 1;
		else
			addCount = 1;
	}

	if(getItemHoldingCount() + addCount > depotLimit)
		return RET_DEPOTISFULL;

	return Container::__queryAdd(index, thing, count, flags, actor);
}

ReturnValue Depot::__queryMaxCount(int32_t index, const Thing* thing, uint32_t count,
	uint32_t& maxQueryCount, uint32_t flags) const
{
	return Container::__queryMaxCount(index, thing, count, maxQueryCount, flags);
}

void Depot::postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
	int32_t index, CylinderLink_t /*link = LINK_OWNER*/)
{
	if(getParent())
		getParent()->postAddNotification(actor, thing, oldParent, index, LINK_PARENT);
}

void Depot::postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
	int32_t index, bool isCompleteRemoval, CylinderLink_t /*link = LINK_OWNER*/)
{
	if(getParent())
		getParent()->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_PARENT);
}
