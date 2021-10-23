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

#include "container.h"
#include "game.h"

#include "iomap.h"
#include "player.h"

extern Game g_game;

Container::Container(uint16_t type) : Item(type)
{
	maxSize = items[type].maxItems;
	serializationCount = 0;
	totalWeight = 0.0;
}

Container::~Container()
{
	for(ItemList::iterator cit = itemlist.begin(); cit != itemlist.end(); ++cit)
	{
		(*cit)->setParent(NULL);
		(*cit)->unRef();
	}

	itemlist.clear();
}

Item* Container::clone() const
{
	Container* _item = static_cast<Container*>(Item::clone());
	for(ItemList::const_iterator it = itemlist.begin(); it != itemlist.end(); ++it)
		_item->addItem((*it)->clone());

	_item->totalWeight = totalWeight;
	return _item;
}

Container* Container::getParentContainer()
{
	if(Cylinder* cylinder = getParent())
	{
		if(Item* item = cylinder->getItem())
			return item->getContainer();
	}

	return NULL;
}

void Container::addItem(Item* item)
{
	itemlist.push_back(item);
	item->setParent(this);
}

Attr_ReadValue Container::readAttr(AttrTypes_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case ATTR_CONTAINER_ITEMS:
		{
			uint32_t count;
			if(!propStream.getLong(count))
				return ATTR_READ_ERROR;

			serializationCount = count;
			return ATTR_READ_END;
		}

		default:
			break;
	}

	return Item::readAttr(attr, propStream);
}

bool Container::unserializeItemNode(FileLoader& f, NODE node, PropStream& propStream)
{
	if(!Item::unserializeItemNode(f, node, propStream))
		return false;

	uint32_t type;
	for(NODE nodeItem = f.getChildNode(node, type); nodeItem; nodeItem = f.getNextNode(nodeItem, type))
	{
		//load container items
		if(type != OTBM_ITEM)
			return false;

		PropStream itemPropStream;
		f.getProps(nodeItem, itemPropStream);

		Item* item = Item::CreateItem(itemPropStream);
		if(!item)
			return false;

		if(!item->unserializeItemNode(f, nodeItem, itemPropStream))
			return false;

		addItem(item);
		updateItemWeight(item->getWeight());
	}

	return true;
}

void Container::updateItemWeight(double diff)
{
	totalWeight += diff;
	if(Container* parent = getParentContainer())
		parent->updateItemWeight(diff);
}

double Container::getWeight() const
{
	return Item::getWeight() + totalWeight;
}

std::string Container::getContentDescription() const
{
	std::ostringstream s;
	return getContentDescription(s).str();
}

std::ostringstream& Container::getContentDescription(std::ostringstream& s) const
{
	bool begin = true;
	Container* evil = const_cast<Container*>(this);
	for(ContainerIterator it = evil->begin(); it != evil->end(); ++it)
	{
		Container* tmp = (*it)->getContainer();
		if(tmp && !tmp->empty())
			continue;

		if(!begin)
			s << ", ";
		else
			begin = false;

		s << (*it)->getNameDescription();
	}

	if(begin)
		s << "nothing";

	return s;
}

Item* Container::getItem(uint32_t index)
{
	size_t n = 0;
	for(ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit)
	{
		if(n == index)
			return *cit;
		else
			++n;
	}

	return NULL;
}

uint32_t Container::getItemHoldingCount() const
{
	uint32_t counter = 0;
	for(ContainerIterator it = begin(); it != end(); ++it)
		++counter;

	return counter;
}

bool Container::isHoldingItem(const Item* item) const
{
	for(ContainerIterator it = begin(); it != end(); ++it)
	{
		if((*it) == item)
			return true;
	}

	return false;
}

void Container::onAddContainerItem(Item* item)
{
	const Position& cylinderMapPos = getPosition();
	SpectatorVec list;

	SpectatorVec::iterator it;
	g_game.getSpectators(list, cylinderMapPos, false, true, 2, 2, 2, 2);

	//send to client
	for (Creature* spectator : list) {
		spectator->getPlayer()->sendAddContainerItem(this, item);
	}

	//event methods
	for (Creature* spectator : list) {
		spectator->getPlayer()->onAddContainerItem(this, item);
	}
}

void Container::onUpdateContainerItem(uint32_t index, Item* oldItem, const ItemType& oldType,
	Item* newItem, const ItemType& newType)
{
	const Position& cylinderMapPos = getPosition();
	SpectatorVec list;

	SpectatorVec::iterator it;
	g_game.getSpectators(list, cylinderMapPos, false, true, 2, 2, 2, 2);

	//send to client
	for (Creature* spectator : list) {
		spectator->getPlayer()->sendUpdateContainerItem(this, index, oldItem, newItem);
	}

	//event methods
	for (Creature* spectator : list) {
		spectator->getPlayer()->onUpdateContainerItem(this, index, oldItem, oldType, newItem, newType);
	}
}

void Container::onRemoveContainerItem(uint32_t index, Item* item)
{
	const Position& cylinderMapPos = getPosition();
	SpectatorVec list;

	SpectatorVec::iterator it;
	g_game.getSpectators(list, cylinderMapPos, false, true, 2, 2, 2, 2);

	//send change to client
	for (Creature* spectator : list) {
		spectator->getPlayer()->sendRemoveContainerItem(this, index, item);
	}

	//event methods
	for (Creature* spectator : list) {
		spectator->getPlayer()->onRemoveContainerItem(this, index, item);
	}
}

ReturnValue Container::__queryAdd(int32_t index, const Thing* thing, uint32_t count,
	uint32_t flags, Creature* actor/* = NULL*/) const
{
	if((flags & FLAG_CHILDISOWNER) == FLAG_CHILDISOWNER)
	{
		//a child container is querying, since we are the top container (not carried by a player)
		//just return with no error.
		return RET_NOERROR;
	}

	const Item* item = thing->getItem();
	if(!item)
		return RET_NOTPOSSIBLE;

	if(!item->isPickupable())
		return RET_CANNOTPICKUP;

	if(item == this)
		return RET_THISISIMPOSSIBLE;

	if(const Container* container = item->getContainer())
	{
		for(const Cylinder* cylinder = getParent(); cylinder; cylinder = cylinder->getParent())
		{
			if(cylinder == container)
				return RET_THISISIMPOSSIBLE;
		}
	}

	if((flags & FLAG_NOLIMIT) != FLAG_NOLIMIT && (index == INDEX_WHEREEVER && size() >= capacity()))
		return RET_CONTAINERNOTENOUGHROOM;

	const Cylinder* topParent = getTopParent();
	if(topParent != this)
		return topParent->__queryAdd(INDEX_WHEREEVER, item, count, flags | FLAG_CHILDISOWNER, actor);

	return RET_NOERROR;
}

ReturnValue Container::__queryMaxCount(int32_t index, const Thing* thing, uint32_t count,
	uint32_t& maxQueryCount, uint32_t flags) const
{
	const Item* item = thing->getItem();
	if(!item)
	{
		maxQueryCount = 0;
		return RET_NOTPOSSIBLE;
	}

	if(((flags & FLAG_NOLIMIT) == FLAG_NOLIMIT))
	{
		maxQueryCount = std::max((uint32_t)1, count);
		return RET_NOERROR;
	}

	int32_t freeSlots = std::max((int32_t)(capacity() - size()), (int32_t)0);
	if(item->isStackable())
	{
		uint32_t n = 0;
		if(index == INDEX_WHEREEVER)
		{
			//Iterate through every item and check how much free stackable slots there is.
			uint32_t slotIndex = 0;
			for(ItemList::const_iterator cit = itemlist.begin(); cit != itemlist.end(); ++cit, ++slotIndex)
			{
				if((*cit) != item && (*cit)->getID() == item->getID() && (*cit)->getItemCount() < 100)
				{
					uint32_t remainder = (100 - (*cit)->getItemCount());
					if(__queryAdd(slotIndex, item, remainder, flags) == RET_NOERROR)
						n += remainder;
				}
			}
		}
		else
		{
			const Thing* destThing = __getThing(index-1);
			const Item* destItem = NULL;
			if(destThing)
				destItem = destThing->getItem();

			if(destItem && destItem->getID() == item->getID() && destItem->getItemCount() < 100)
			{
				uint32_t remainder = 100 - destItem->getItemCount();
				if(__queryAdd(index, item, remainder, flags) == RET_NOERROR)
					n = remainder;
			}
		}

		maxQueryCount = freeSlots * 100 + n;
		if(maxQueryCount < count)
			return RET_CONTAINERNOTENOUGHROOM;
	}
	else
	{
		maxQueryCount = freeSlots;
		if(!maxQueryCount)
			return RET_CONTAINERNOTENOUGHROOM;
	}

	return RET_NOERROR;
}

ReturnValue Container::__queryRemove(const Thing* thing, uint32_t count, uint32_t flags, Creature*) const
{
	int32_t index = __getIndexOfThing(thing);
	if(index == -1)
		return RET_NOTPOSSIBLE;

	const Item* item = thing->getItem();
	if(!item)
		return RET_NOTPOSSIBLE;

	if(!count || (item->isStackable() && count > item->getItemCount() && item->getItemCount() != 0))
		return RET_NOTPOSSIBLE;

	if(!item->isMovable() && !hasBitSet(FLAG_IGNORENOTMOVABLE, flags))
		return RET_NOTMOVABLE;

	return RET_NOERROR;
}

Cylinder* Container::__queryDestination(int32_t& index, const Thing* thing, Item** destItem,
	uint32_t& flags)
{
	if(index == 254 /*move up*/)
	{
		index = INDEX_WHEREEVER;
		*destItem = NULL;

		Container* parentContainer = dynamic_cast<Container*>(getParent());
		if(parentContainer)
			return parentContainer;

		return this;
	}

	if(index == 255 /*add wherever*/)
	{
		index = INDEX_WHEREEVER;
		*destItem = NULL;
	}
	else if(index >= (int32_t)capacity())
	{
		/*
		if you have a container, maximize it to show all 20 slots
		then you open a bag that is inside the container you will have a bag with 8 slots
		and a "grey" area where the other 12 slots where from the container
		if you drop the item on that grey area the client calculates the slot position
		as if the bag has 20 slots
		*/

		index = INDEX_WHEREEVER;
		*destItem = NULL;
	}

	const Item* item = thing->getItem();
	if(!item)
		return this;

	if(index != INDEX_WHEREEVER)
	{
		Thing* destThing = __getThing(index);
		if(destThing)
			*destItem = destThing->getItem();

		if(Cylinder* subCylinder = dynamic_cast<Cylinder*>(*destItem))
		{
			index = INDEX_WHEREEVER;
			*destItem = NULL;
			return subCylinder;
		}
	}

	bool autoStack = !hasBitSet(FLAG_IGNOREAUTOSTACK, flags);
	if(autoStack && item->isStackable() && item->getParent() != this)
	{
		//try find a suitable item to stack with
		uint32_t n = itemlist.size();
		for(ItemList::reverse_iterator cit = itemlist.rbegin(); cit != itemlist.rend(); ++cit, --n)
		{
			if((*cit) != item && (*cit)->getID() == item->getID() && (*cit)->getItemCount() < 100)
			{
				*destItem = (*cit);
				index = n;
				return this;
			}
		}
	}
	return this;
}

void Container::__addThing(Creature* actor, Thing* thing)
{
	return __addThing(actor, 0, thing);
}

void Container::__addThing(Creature*, int32_t index, Thing* thing)
{
	if(index >= (int32_t)capacity())
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__addThing], index:" << index << ", index >= capacity()" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	Item* item = thing->getItem();
	if(!item)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__addThing] item == NULL" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

#ifdef __DEBUG_MOVESYS__
	if(index != INDEX_WHEREEVER && size() >= capacity())
	{
		std::clog << "Failure: [Container::__addThing] size() >= capacity()" << std::endl;
		return /*RET_CONTAINERNOTENOUGHROOM*/;
	}
#endif

	item->setParent(this);
	itemlist.push_front(item);

	totalWeight += item->getWeight();
	if(Container* parentContainer = getParentContainer())
		parentContainer->updateItemWeight(item->getWeight());

	//send change to client
	if(getParent() && getParent() != VirtualCylinder::virtualCylinder)
		onAddContainerItem(item);
}

void Container::__updateThing(Thing* thing, uint16_t itemId, uint32_t count)
{
	int32_t index = __getIndexOfThing(thing);
	if(index == -1)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__updateThing] index == -1" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	Item* item = thing->getItem();
	if(!item)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__updateThing] item == NULL" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	const ItemType& oldType = Item::items[item->getID()];
	const ItemType& newType = Item::items[itemId];

	const double oldWeight = item->getWeight();
	item->setID(itemId);
	item->setSubType(count);

	const double diffWeight = -oldWeight + item->getWeight();
	totalWeight += diffWeight;
	if(Container* parentContainer = getParentContainer())
		parentContainer->updateItemWeight(diffWeight);

	//send change to client
	if(getParent())
		onUpdateContainerItem(index, item, oldType, item, newType);
}

void Container::__replaceThing(uint32_t index, Thing* thing)
{
	Item* item = thing->getItem();
	if(!item)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__replaceThing] item == NULL" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	uint32_t count = 0;
	ItemList::iterator cit = itemlist.end();
	for(cit = itemlist.begin(); cit != itemlist.end(); ++cit)
	{
		if(count == index)
			break;

		++count;
	}

	if(cit == itemlist.end())
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__updateThing] item not found" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	totalWeight -= (*cit)->getWeight();
	totalWeight += item->getWeight();
	if(Container* parentContainer = getParentContainer())
		parentContainer->updateItemWeight(-(*cit)->getWeight() + item->getWeight());

	itemlist.insert(cit, item);
	item->setParent(this);
	//send change to client
	if(getParent())
	{
		const ItemType& oldType = Item::items[(*cit)->getID()];
		const ItemType& newType = Item::items[item->getID()];
		onUpdateContainerItem(index, *cit, oldType, item, newType);
	}

	(*cit)->setParent(NULL);
	itemlist.erase(cit);
}

void Container::__removeThing(Thing* thing, uint32_t count)
{
	Item* item = thing->getItem();
	if(!item)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__removeThing] item == NULL" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	int32_t index = __getIndexOfThing(thing);
	if(index == -1)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__removeThing] index == -1" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	ItemList::iterator cit = std::find(itemlist.begin(), itemlist.end(), thing);
	if(cit == itemlist.end())
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__removeThing] item not found" << std::endl;
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	if(item->isStackable() && count != item->getItemCount())
	{
		const double oldWeight = -item->getWeight();
		item->setItemCount(std::max(0, (int32_t)(item->getItemCount() - count)));

		const double diffWeight = oldWeight + item->getWeight();
		totalWeight += diffWeight;
		//send change to client
		if(getParent())
		{
			if(Container* parentContainer = getParentContainer())
				parentContainer->updateItemWeight(diffWeight);

			const ItemType& it = Item::items[item->getID()];
			onUpdateContainerItem(index, item, it, item, it);
		}
	}
	else
	{
		//send change to client
		if(getParent())
		{
			if(Container* parentContainer = getParentContainer())
				parentContainer->updateItemWeight(-item->getWeight());

			onRemoveContainerItem(index, item);
		}

		totalWeight -= item->getWeight();
		item->setParent(NULL);
		itemlist.erase(cit);
	}
}

Thing* Container::__getThing(uint32_t index) const
{
	if(index > size())
		return NULL;

	uint32_t count = 0;
	for(ItemList::const_iterator cit = itemlist.begin(); cit != itemlist.end(); ++cit)
	{
		if(count == index)
			return *cit;
		else
			++count;
	}

	return NULL;
}

int32_t Container::__getIndexOfThing(const Thing* thing) const
{
	uint32_t index = 0;
	for(ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit)
	{
		if(*cit == thing)
			return index;
		else
			++index;
	}

	return -1;
}

int32_t Container::__getFirstIndex() const
{
	return 0;
}

int32_t Container::__getLastIndex() const
{
	return size();
}

uint32_t Container::__getItemTypeCount(uint16_t itemId, int32_t subType /*= -1*/) const
{
	uint32_t count = 0;
	for(ItemList::const_iterator it = itemlist.begin(); it != itemlist.end(); ++it)
	{
		if((*it) && (*it)->getID() == itemId && (subType == -1 || subType == (*it)->getSubType()))
			count += (*it)->getItemCount();
	}

	return count;
}

std::map<uint32_t, uint32_t>& Container::__getAllItemTypeCount(std::map<uint32_t,
	uint32_t>& countMap) const
{
	for(ItemList::const_iterator it = itemlist.begin(); it != itemlist.end(); ++it)
		countMap[(*it)->getID()] += (*it)->getItemCount();

	return countMap;
}

void Container::postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
	int32_t index, CylinderLink_t/* link = LINK_OWNER*/)
{
	Cylinder* topParent = getTopParent();
	if(!topParent->getCreature())
	{
		if(topParent == this)
		{
			//let the tile class notify surrounding players
			if(topParent->getParent())
				topParent->getParent()->postAddNotification(actor, thing, oldParent, index, LINK_NEAR);
		}
		else
			topParent->postAddNotification(actor, thing, oldParent, index, LINK_PARENT);
	}
	else
		topParent->postAddNotification(actor, thing, oldParent, index, LINK_TOPPARENT);
}

void Container::postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
	int32_t index, bool isCompleteRemoval, CylinderLink_t/* link = LINK_OWNER*/)
{
	Cylinder* topParent = getTopParent();
	if(!topParent->getCreature())
	{
		if(topParent == this)
		{
			//let the tile class notify surrounding players
			if(topParent->getParent())
				topParent->getParent()->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_NEAR);
		}
		else
			topParent->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_PARENT);
	}
	else
		topParent->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_TOPPARENT);
}

void Container::__internalAddThing(Thing* thing)
{
	__internalAddThing(0, thing);
}

void Container::__internalAddThing(uint32_t
#ifdef __DEBUG_MOVESYS__
	index
#endif
	, Thing* thing)
{
#ifdef __DEBUG_MOVESYS__
	std::clog << "[Container::__internalAddThing] index: " << index << std::endl;
#endif
	if(!thing)
		return;

	Item* item = thing->getItem();
	if(item == NULL)
	{
#ifdef __DEBUG_MOVESYS__
		std::clog << "Failure: [Container::__internalAddThing] item == NULL" << std::endl;
#endif
		return;
	}

	itemlist.push_front(item);
	item->setParent(this);

	totalWeight += item->getWeight();
	if(Container* parentContainer = getParentContainer())
		parentContainer->updateItemWeight(item->getWeight());
}

void Container::__startDecaying()
{
	for(ItemList::const_iterator it = itemlist.begin(); it != itemlist.end(); ++it)
		(*it)->__startDecaying();
}

ContainerIterator Container::begin()
{
	ContainerIterator cit(this);
	if(!itemlist.empty())
	{
		cit.over.push(this);
		cit.current = itemlist.begin();
	}

	return cit;
}

ContainerIterator Container::end()
{
	ContainerIterator cit(this);
	return cit;
}

ContainerIterator Container::begin() const
{
	Container* evil = const_cast<Container*>(this);
	return evil->begin();
}

ContainerIterator Container::end() const
{
	Container* evil = const_cast<Container*>(this);
	return evil->end();
}

ContainerIterator::ContainerIterator():
base(NULL) {}

ContainerIterator::ContainerIterator(Container* _base):
base(_base) {}

ContainerIterator::ContainerIterator(const ContainerIterator& rhs):
base(rhs.base), over(rhs.over), current(rhs.current) {}

bool ContainerIterator::operator==(const ContainerIterator& rhs)
{
	return !(*this != rhs);
}

bool ContainerIterator::operator!=(const ContainerIterator& rhs)
{
	assert(base);
	if(base != rhs.base)
		return true;

	if(over.empty() && rhs.over.empty())
		return false;

	if(over.empty())
		return true;

	if(rhs.over.empty())
		return true;

	if(over.front() != rhs.over.front())
		return true;

	return current != rhs.current;
}

ContainerIterator& ContainerIterator::operator=(const ContainerIterator& rhs)
{
	this->base = rhs.base;
	this->current = rhs.current;
	this->over = rhs.over;
	return *this;
}

Item* ContainerIterator::operator*()
{
	assert(base);
	return *current;
}

Item* ContainerIterator::operator->()
{
	return *(*this);
}

ContainerIterator& ContainerIterator::operator++()
{
	assert(base);
	if(Item* item = *current)
	{
		Container* container = item->getContainer();
		if(container && !container->empty())
			over.push(container);
	}

	++current;
	if(current == over.front()->itemlist.end())
	{
		over.pop();
		if(over.empty())
			return *this;

		current = over.front()->itemlist.begin();
	}

	return *this;
}

ContainerIterator ContainerIterator::operator++(int32_t)
{
	ContainerIterator tmp(*this);
	++*this;
	return tmp;
}
