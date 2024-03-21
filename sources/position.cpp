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
#include <iomanip>
#include "position.h"

std::ostream& operator<<(std::ostream& os, const Position& pos)
{
	os << "( " << std::setw(5) << std::setfill('0') << pos.x;
	os << " / " << std::setw(5) << std::setfill('0') << pos.y;
	os << " / " << std::setw(3) << std::setfill('0') << pos.z;
	os << " )";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Direction& dir)
{
	switch(dir)
	{
		case NORTH:
			os << "North";
			break;
		case EAST:
			os << "East";
			break;
		case WEST:
			os << "West";
			break;
		case SOUTH:
			os << "South";
			break;
		case SOUTHWEST:
			os << "South-West";
			break;
		case SOUTHEAST:
			os << "South-East";
			break;
		case NORTHWEST:
			os << "North-West";
			break;
		case NORTHEAST:
			os << "North-East";
			break;
		default:
			break;
	}

	return os;
}
