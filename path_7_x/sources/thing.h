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

#ifndef __THING__
#define __THING__
#include "position.h"

enum ReturnValue
{
	RET_NOERROR = 0,
	RET_NOTPOSSIBLE = 1,
	RET_NOTENOUGHROOM = 2,
	RET_PLAYERISPZLOCKED = 3,
	RET_PLAYERISNOTINVITED = 4,
	RET_CANNOTTHROW = 5,
	RET_THEREISNOWAY = 6,
	RET_DESTINATIONOUTOFREACH = 7,
	RET_CREATUREBLOCK = 8,
	RET_NOTMOVABLE = 9,
	RET_DROPTWOHANDEDITEM = 10,
	RET_BOTHHANDSNEEDTOBEFREE = 11,
	RET_CANONLYUSEONEWEAPON = 12,
	RET_NEEDEXCHANGE = 13,
	RET_CANNOTBEDRESSED = 14,
	RET_PUTTHISOBJECTINYOURHAND = 15,
	RET_PUTTHISOBJECTINBOTHHANDS = 16,
	RET_TOOFARAWAY = 17,
	RET_FIRSTGODOWNSTAIRS = 18,
	RET_FIRSTGOUPSTAIRS = 19,
	RET_CONTAINERNOTENOUGHROOM = 20,
	RET_NOTENOUGHCAPACITY = 21,
	RET_CANNOTPICKUP = 22,
	RET_THISISIMPOSSIBLE = 23,
	RET_DEPOTISFULL = 24,
	RET_CREATUREDOESNOTEXIST = 25,
	RET_CANNOTUSETHISOBJECT = 26,
	RET_PLAYERWITHTHISNAMEISNOTONLINE = 27,
	RET_NOTREQUIREDLEVELTOUSERUNE = 28,
	RET_YOUAREALREADYTRADING = 29,
	RET_THISPLAYERISALREADYTRADING = 30,
	RET_YOUMAYNOTLOGOUTDURINGAFIGHT = 31,
	RET_DIRECTPLAYERSHOOT = 32,
	RET_NOTENOUGHLEVEL = 33,
	RET_NOTENOUGHMAGICLEVEL = 34,
	RET_NOTENOUGHMANA = 35,
	RET_NOTENOUGHSOUL = 36,
	RET_YOUAREEXHAUSTED = 37,
	RET_PLAYERISNOTREACHABLE = 38,
	RET_CANONLYUSETHISRUNEONCREATURES = 39,
	RET_ACTIONNOTPERMITTEDINPROTECTIONZONE = 40,
	RET_YOUMAYNOTATTACKTHISPLAYER = 41,
	RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE = 42,
	RET_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE = 43,
	RET_YOUMAYNOTATTACKTHISCREATURE = 44,
	RET_YOUCANONLYUSEITONCREATURES = 45,
	RET_CREATUREISNOTREACHABLE = 46,
	RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS = 47,
	RET_YOUNEEDPREMIUMACCOUNT = 48,
	RET_YOUNEEDTOLEARNTHISSPELL = 49,
	RET_YOURVOCATIONCANNOTUSETHISSPELL = 50,
	RET_YOUNEEDAWEAPONTOUSETHISSPELL = 51,
	RET_PLAYERISPZLOCKEDLEAVEPVPZONE = 52,
	RET_PLAYERISPZLOCKEDENTERPVPZONE = 53,
	RET_ACTIONNOTPERMITTEDINANOPVPZONE = 54,
	RET_YOUCANNOTLOGOUTHERE = 55,
	RET_YOUNEEDAMAGICITEMTOCASTSPELL = 56,
	RET_CANNOTCONJUREITEMHERE = 57,
	RET_TILEISFULL = 58,
	RET_NAMEISTOOAMBIGUOUS = 59,
	RET_CANONLYUSEONESHIELD = 60,
	RET_YOUARENOTTHEOWNER = 61,
	RET_YOUMAYNOTCASTAREAONBLACKSKULL = 62,
	RET_NOTENOUGHSKILL = 63,
	RET_YOUMAYNOTATTACKIMMEDIATELYAFTERLOGGINGIN = 64,
	RET_YOUHAVETOWAIT = 65,
	RET_YOUCANONLYTRADEUPTOX = 66
};

class Tile;
class Cylinder;
class Item;
class Creature;

class Thing
{
	protected:
		Thing(): parent(NULL), refCount(0) {}

	public:
		virtual ~Thing() {}

		void addRef() {++refCount;}
		void unRef()
		{
			--refCount;
			if(!refCount)
				delete this;
		}

		virtual std::string getDescription(int32_t lookDistance) const = 0;

		Cylinder* getParent() {return parent;}
		const Cylinder* getParent() const {return parent;}

		virtual void setParent(Cylinder* cylinder) {parent = cylinder;}

		Cylinder* getTopParent();
		const Cylinder* getTopParent() const;

		virtual Tile* getTile();
		virtual const Tile* getTile() const;

		virtual Position getPosition() const;
		virtual int32_t getThrowRange() const = 0;
		virtual bool isPushable() const = 0;

		virtual Item* getItem() {return NULL;}
		virtual const Item* getItem() const {return NULL;}
		virtual Creature* getCreature() {return NULL;}
		virtual const Creature* getCreature() const {return NULL;}

		virtual bool isRemoved() const;

	private:
		Cylinder* parent;
		int16_t refCount;
};
#endif
