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
#include <random>
#include <cstdlib>
#include <cerrno>
#include <string>

inline int safeParseInt(const std::string& s, int defaultValue = 0)
{
	if (s.empty()) return defaultValue;
	errno = 0;
	char* endptr = nullptr;
	long val = std::strtol(s.c_str(), &endptr, 10);
	if (endptr == s.c_str() || *endptr != '\0' || errno == ERANGE) return defaultValue;
	return static_cast<int>(val);
}

inline std::mt19937& global_rng()
{
	static thread_local std::mt19937 rng(std::random_device{}());
	return rng;
}
