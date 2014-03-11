//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "depotlocker.h"
#include "tools.h"

DepotLocker::DepotLocker(uint16_t _type) :
	Container(_type)
{
	maxSize = 2;
}

DepotLocker::~DepotLocker()
{
	//
}

Attr_ReadValue DepotLocker::readAttr(AttrTypes_t attr, PropStream& propStream)
{
	if(attr != ATTR_DEPOT_ID)
		return Item::readAttr(attr, propStream);

	uint16_t depotId;
	if(!propStream.getShort(depotId))
		return ATTR_READ_ERROR;

	setAttribute("depotid", depotId);
	return ATTR_READ_CONTINUE;
}

ReturnValue DepotLocker::__queryAdd(int32_t index, const Thing* thing, uint32_t count,
	uint32_t flags, Creature* actor/* = NULL*/) const
{
	return Container::__queryAdd(index, thing, count, flags, actor);
}

void DepotLocker::postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent, int32_t index, CylinderLink_t /*link = LINK_OWNER*/)
{
	if(getParent())
		getParent()->postAddNotification(actor, thing, oldParent, index, LINK_PARENT);
}

void DepotLocker::postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent, int32_t index, bool isCompleteRemoval, CylinderLink_t /*link = LINK_OWNER*/)
{
	if(getParent())
		getParent()->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_PARENT);
}

void DepotLocker::removeInbox(Inbox* inbox)
{
	ItemList::iterator cit = std::find(itemlist.begin(), itemlist.end(), inbox);
	if(cit == itemlist.end())
		return;

	itemlist.erase(cit);
}
