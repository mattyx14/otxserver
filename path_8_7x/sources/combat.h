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

#ifndef __COMBAT__
#define __COMBAT__
#include "otsystem.h"

#include "baseevents.h"
#include "condition.h"
#include "map.h"

class Condition;
class Creature;
class Position;
class Item;

struct CombatEffects
{
	CombatEffects(bool _show): show(_show)
	{
		color = COLOR_UNKNOWN;
		distance = SHOOT_EFFECT_NONE;
		impact = MAGIC_EFFECT_NONE;
		hit = MAGIC_EFFECT_UNKNOWN;
	}

	CombatEffects()
	{
		color = COLOR_UNKNOWN;
		distance = SHOOT_EFFECT_NONE;
		impact = MAGIC_EFFECT_NONE;
		hit = MAGIC_EFFECT_UNKNOWN;
		show = true;
	}

	MagicEffect_t impact, hit;
	ShootEffect_t distance;
	Color_t color;
	bool show;
};

struct CombatElement
{
	CombatElement()
	{
		type = COMBAT_NONE;
		damage = 0;
	}

	CombatType_t type;
	int32_t damage;
};

class TargetCallback;
class ValueCallback;
class TileCallback;

struct CombatParams
{
	CombatParams()
	{
		blockedByArmor = blockedByShield = targetCasterOrTopMost = targetPlayersOrSummons = differentAreaDamage = false;
		isAggressive = useCharges = true;
		dispelType = CONDITION_NONE;
		combatType = COMBAT_NONE;
		itemId = 0;

		targetCallback = NULL;
		valueCallback = NULL;
		tileCallback = NULL;
	}

	bool blockedByArmor, blockedByShield, targetCasterOrTopMost, targetPlayersOrSummons, differentAreaDamage, useCharges, isAggressive;
	ConditionType_t dispelType;
	CombatType_t combatType, elementType;
	uint32_t itemId, elementDamage;

	TargetCallback* targetCallback;
	ValueCallback* valueCallback;
	TileCallback* tileCallback;
	CombatEffects effects;
	CombatElement element;

	std::list<const Condition*> conditionList;
};

struct Combat2Var
{
	int32_t minChange, maxChange, change;
	Combat2Var() {minChange = maxChange = change = 0;}
};

//for luascript callback
class ValueCallback : public CallBack
{
	public:
		ValueCallback(formulaType_t _type) {type = _type;}
		void getMinMaxValues(Player* player, CombatParams& params, int32_t& min, int32_t& max) const;

	protected:
		formulaType_t type;
};

class TileCallback : public CallBack
{
	public:
		TileCallback() {}
		void onTileCombat(Creature* creature, Tile* tile) const;

	protected:
		formulaType_t type;
};

class TargetCallback : public CallBack
{
	public:
		TargetCallback() {}
		void onTargetCombat(Creature* creature, Creature* target) const;

	protected:
		formulaType_t type;
};

typedef bool (*COMBATFUNC)(Creature*, Creature*, const CombatParams&, void*);
class MatrixArea
{
	public:
		MatrixArea(uint32_t _rows, uint32_t _cols)
		{
			centerX = centerY = 0;
			rows = _rows;
			cols = _cols;

			data_ = new bool*[rows];
			for(uint32_t row = 0; row < rows; ++row)
			{
				data_[row] = new bool[cols];
				for(uint32_t col = 0; col < cols; ++col)
					data_[row][col] = 0;
			}
		}

		MatrixArea(const MatrixArea& rhs)
		{
			centerX = rhs.centerX;
			centerY = rhs.centerY;
			rows = rhs.rows;
			cols = rhs.cols;

			data_ = new bool*[rows];
			for(uint32_t row = 0; row < rows; ++row)
			{
				data_[row] = new bool[cols];

				for(uint32_t col = 0; col < cols; ++col)
					data_[row][col] = rhs.data_[row][col];
			}
		}

		virtual ~MatrixArea()
		{
			for(uint32_t row = 0; row < rows; ++row)
				delete[] data_[row];

			delete[] data_;
		}

		void setValue(uint32_t row, uint32_t col, bool value) const {data_[row][col] = value;}
		bool getValue(uint32_t row, uint32_t col) const {return data_[row][col];}

		void setCenter(uint16_t y, uint16_t x) {centerX = x; centerY = y;}
		void getCenter(uint16_t& y, uint16_t& x) const {x = centerX; y = centerY;}

		size_t getRows() const {return rows;}
		size_t getCols() const {return cols;}

		inline const bool* operator[](uint32_t i) const { return data_[i]; }
		inline bool* operator[](uint32_t i) { return data_[i]; }

	protected:
		uint16_t centerX, centerY;
		uint32_t rows, cols;
		bool** data_;
};

typedef std::map<Direction, MatrixArea* > CombatAreas;
class CombatArea
{
	public:
		CombatArea() {hasExtArea = false;}
		virtual ~CombatArea() {clear();}

		CombatArea(const CombatArea& rhs);

		ReturnValue doCombat(Creature* attacker, const Position& pos, const Combat& combat) const;
		bool getList(const Position& centerPos, const Position& targetPos, std::list<Tile*>& list) const;

		void setupArea(const std::list<uint32_t>& list, uint32_t rows);
		void setupArea(int32_t length, int32_t spread);
		void setupArea(int32_t radius);
		void setupExtArea(const std::list<uint32_t>& list, uint32_t rows);
		void clear();

	protected:
		enum MatrixOperation_t
		{
			MATRIXOPERATION_COPY,
			MATRIXOPERATION_MIRROR,
			MATRIXOPERATION_FLIP,
			MATRIXOPERATION_ROTATE90,
			MATRIXOPERATION_ROTATE180,
			MATRIXOPERATION_ROTATE270,
		};

		MatrixArea* createArea(const std::list<uint32_t>& list, uint32_t rows);
		void copyArea(const MatrixArea* input, MatrixArea* output, MatrixOperation_t op) const;

		MatrixArea* getArea(const Position& centerPos, const Position& targetPos) const
		{
			int32_t dx = targetPos.x - centerPos.x, dy = targetPos.y - centerPos.y;
			Direction dir = NORTH;
			if(dx < 0)
				dir = WEST;
			else if(dx > 0)
				dir = EAST;
			else if(dy < 0)
				dir = NORTH;
			else
				dir = SOUTH;

			if(hasExtArea)
			{
				if(dx < 0 && dy < 0)
					dir = NORTHWEST;
				else if(dx > 0 && dy < 0)
					dir = NORTHEAST;
				else if(dx < 0 && dy > 0)
					dir = SOUTHWEST;
				else if(dx > 0 && dy > 0)
					dir = SOUTHEAST;
			}

			CombatAreas::const_iterator it = areas.find(dir);
			if(it != areas.end())
				return it->second;

			return NULL;
		}

		CombatAreas areas;
		bool hasExtArea;
};

class Combat
{
	public:
		Combat();
		virtual ~Combat();

		static void doCombatHealth(Creature* caster, Creature* target,
			int32_t minChange, int32_t maxChange, const CombatParams& params, bool check = true);
		static void doCombatHealth(Creature* caster, const Position& pos,
			const CombatArea* area, int32_t minChange, int32_t maxChange, const CombatParams& params);

		static void doCombatMana(Creature* caster, Creature* target,
			int32_t minChange, int32_t maxChange, const CombatParams& params, bool check = true);
		static void doCombatMana(Creature* caster, const Position& pos,
			const CombatArea* area, int32_t minChange, int32_t maxChange, const CombatParams& params);

		static void doCombatCondition(Creature* caster, Creature* target,
			const CombatParams& params, bool check = true);
		static void doCombatCondition(Creature* caster, const Position& pos,
			const CombatArea* area, const CombatParams& params);

		static void doCombatDispel(Creature* caster, Creature* target,
			const CombatParams& params, bool check = true);
		static void doCombatDispel(Creature* caster, const Position& pos,
			const CombatArea* area, const CombatParams& params);

		static void getCombatArea(const Position& centerPos, const Position& targetPos,
			const CombatArea* area, std::list<Tile*>& list);

		static bool isInPvpZone(const Creature* attacker, const Creature* target);
		static bool isProtected(Player* attacker, Player* target);

		static CombatType_t ConditionToDamageType(ConditionType_t type);
		static ConditionType_t DamageToConditionType(CombatType_t type);

		static ReturnValue canTargetCreature(const Player* attacker, const Creature* target);
		static ReturnValue canDoCombat(const Creature* caster, const Tile* tile, bool isAggressive, bool createItem);
		static ReturnValue canDoCombat(const Creature* attacker, const Creature* target, bool isAggressive);

		static void postCombatEffects(Creature* caster, const Position& pos, const CombatParams& params);
		static void addDistanceEffect(Creature* caster, const Position& fromPos, const Position& toPos, ShootEffect_t effect);

		void doCombat(Creature* caster, Creature* target) const;
		void doCombat(Creature* caster, const Position& pos) const;

		bool setCallback(CallBackParam_t key);
		CallBack* getCallback(CallBackParam_t key);

		void setArea(CombatArea* _area)
		{

			delete area;

			area = _area;
		}
		bool hasArea() const {return area != NULL;}

		bool setParam(CombatParam_t param, uint32_t value);
		void setCondition(const Condition* _condition) {params.conditionList.push_back(_condition);}
		void setPlayerCombatValues(formulaType_t _type, double _mina, double _minb, double _maxa,
			double _maxb, double _minl, double _maxl, double _minm, double _maxm, int32_t _minc,
			int32_t _maxc);

		void postCombatEffects(Creature* caster, const Position& pos) const
			{Combat::postCombatEffects(caster, pos, params);}

	protected:
		static void doCombatDefault(Creature* caster, Creature* target, const CombatParams& params);
		static void CombatFunc(Creature* caster, const Position& pos,
			const CombatArea* area, const CombatParams& params, COMBATFUNC func, void* data);

		static bool CombatHealthFunc(Creature* caster, Creature* target, const CombatParams& params, void* data);
		static bool CombatManaFunc(Creature* caster, Creature* target, const CombatParams& params, void* data);
		static bool CombatConditionFunc(Creature* caster, Creature* target, const CombatParams& params, void* data);
		static bool CombatDispelFunc(Creature* caster, Creature* target, const CombatParams& params, void* data);
		static bool CombatNullFunc(Creature* caster, Creature* target, const CombatParams& params, void* data);

		static void combatTileEffects(const SpectatorVec& list, Creature* caster, Tile* tile, const CombatParams& params);
		bool getMinMaxValues(Creature* creature, Creature* target, CombatParams& params, int32_t& min, int32_t& max) const;

		//configureable
		CombatParams params;

		//formula variables
		formulaType_t formulaType;
		double mina, minb, maxa, maxb, minl, maxl, minm, maxm;
		int32_t minc, maxc;

		CombatArea* area;
};

class MagicField : public Item
{
	public:
		MagicField(uint16_t _type) : Item(_type) {createTime = OTSYS_TIME();}
		virtual ~MagicField() {}

		virtual MagicField* getMagicField() {return this;}
		virtual const MagicField* getMagicField() const {return this;}

		virtual bool isBlocking(const Creature* creature) const;

		bool isReplacable() const {return Item::items[id].replacable;}
		bool isUnstepable() const {return id == ITEM_MAGICWALL_SAFE || id == ITEM_WILDGROWTH_SAFE;}
		CombatType_t getCombatType() const
		{
			const ItemType& it = items[id];
			return it.combatType;
		}

		void onStepInField(Creature* creature);

	private:
		uint64_t createTime;
};
#endif
