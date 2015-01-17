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

#ifndef __BEDS__
#define __BEDS__
#include "item.h"

class House;
class Player;

class BedItem : public Item
{
	public:
		BedItem(uint16_t _type): Item(_type), house(NULL) {internalRemoveSleeper();}
		virtual ~BedItem() {}

		virtual BedItem* getBed() {return this;}
		virtual const BedItem* getBed() const {return this;}

		virtual Attr_ReadValue readAttr(AttrTypes_t attr, PropStream& propStream);
		virtual bool serializeAttr(PropWriteStream& propWriteStream) const;

		virtual bool canRemove() const {return house != NULL;}

		uint32_t getSleeper() const {return sleeper;}
		void setSleeper(uint32_t guid) {sleeper = guid;}

		House* getHouse() const {return house;}
		void setHouse(House* h) {house = h;}

		bool canUse(Player* player);

		void sleep(Player* player);
		void wakeUp();

		BedItem* getNextBedItem();

	protected:
		void updateAppearance(const Player* player);
		void regeneratePlayer(Player* player) const;

		void internalSetSleeper(const Player* player);
		void internalRemoveSleeper();

		uint32_t sleeper;
		House* house;
};

class Beds
{
	public:
		virtual ~Beds() {}
		static Beds* getInstance()
		{
			static Beds instance;
			return &instance;
		}

		BedItem* getBedBySleeper(uint32_t guid);
		void setBedSleeper(BedItem* bed, uint32_t guid) {BedSleepersMap[guid] = bed;}

	protected:
		Beds() {BedSleepersMap.clear();}
		std::map<uint32_t, BedItem*> BedSleepersMap;
};
#endif
