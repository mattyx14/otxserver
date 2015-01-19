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

#ifndef __POSITION__
#define __POSITION__
#include <stdlib.h>

#include <cmath>
#include <iostream>
#include <vector>

enum Direction
{
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
	SOUTHWEST = 4,
	SOUTHEAST = 5,
	NORTHWEST = 6,
	NORTHEAST = 7
};

typedef std::pair<int32_t, int32_t> PositionPair;
typedef std::vector<PositionPair> PairVector;
typedef std::vector<Direction> DirVector;

class Position
{
	public:
		Position(): x(0), y(0), z(0) {}
		~Position() {}

		template<uint16_t deltax, uint16_t deltay, uint16_t deltaz>
		inline static bool areInRange(const Position& p1, const Position& p2)
		{
			return !(std::abs(float(p1.x - p2.x)) > deltax || std::abs(float(p1.y - p2.y)) > deltay || std::abs(float(p1.z - p2.z)) > deltaz);
		}

		template<uint16_t deltax, uint16_t deltay>
		inline static bool areInRange(const Position& p1, const Position& p2)
		{
			return !(std::abs(float(p1.x - p2.x)) > deltax || std::abs(float(p1.y - p2.y)) > deltay);
		}

		static bool areInRange(const Position& r, const Position& p1, const Position& p2)
		{
			return !(std::abs(float(p1.x - p2.x)) > r.x || std::abs(float(p1.y - p2.y)) > r.y || std::abs(float(p1.z - p2.z)) > r.z);
		}

		Position(uint16_t _x, uint16_t _y, uint16_t _z): x(_x), y(_y), z(_z) {}
		uint16_t x, y, z;

		bool operator<(const Position& p) const
		{
			if(z < p.z)
				return true;

			if(z > p.z)
				return false;

			if(y < p.y)
				return true;

			if(y > p.y)
				return false;

			if(x < p.x)
				return true;

			if(x > p.x)
				return false;

			return false;
		}

		bool operator>(const Position& p) const
		{
			return !(*this < p);
		}

		bool operator==(const Position& p) const
		{
			return (p.x == x && p.y == y && p.z == z);
		}

		bool operator!=(const Position& p) const
		{
			return !(*this == p);
		}

		Position operator+(const Position& p1)
		{
			return Position(x + p1.x, y + p1.y, z + p1.z);
		}

		Position operator-(const Position& p1)
		{
			return Position(x - p1.x, y - p1.y, z - p1.z);
		}
};

std::ostream& operator<<(std::ostream&, const Position&);
std::ostream& operator<<(std::ostream&, const Direction&);

class PositionEx : public Position
{
	public:
		PositionEx() {}
		~PositionEx() {}

		PositionEx(uint16_t _x, uint16_t _y, uint16_t _z, int16_t _stackpos): Position(_x,_y,_z), stackpos(_stackpos) {}
		PositionEx(uint16_t _x, uint16_t _y, uint16_t _z): Position(_x,_y,_z), stackpos(0) {}

		PositionEx(Position p): Position(p.x, p.y, p.z), stackpos(0) {}
		PositionEx(Position p, int16_t _stackpos): Position(p.x, p.y, p.z), stackpos(_stackpos) {}

		int16_t stackpos;

		bool operator==(const PositionEx& p) const
		{
			return (p.x == x && p.y == y && p.z == z && p.stackpos == stackpos);
		}

		bool operator!=(const PositionEx& p) const
		{
			return !(p.x == x && p.y == y && p.z == z && p.stackpos != stackpos);
		}
};
#endif
