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

#ifndef __COMBAT_HELPERS__
#define __COMBAT_HELPERS__

#include <cstdint>
#include <limits>

#include "enums.h"

// Return true if m is a single-bit mask (power of two) and non-zero.
inline bool isSingleBitMask(uint32_t m) noexcept {
	return m != 0u && ((m & (m - 1u)) == 0u);
}

// Return zero-based index of a single-bit mask (1<<n -> n).
// Returns std::numeric_limits<uint32_t>::max() on error.
inline uint32_t combatMaskToIndex(uint32_t mask) noexcept {
	if (!isSingleBitMask(mask)) return std::numeric_limits<uint32_t>::max();
	uint32_t idx = 0u;
	while ((mask & 1u) == 0u) {
		mask >>= 1u;
		++idx;
	}
	return idx;
}

// Convert index -> single-bit mask (1u << idx).
inline uint32_t combatIndexToMask(uint32_t idx) noexcept {
	return (1u << idx);
}

// Next / previous single-bit mask helpers.
inline uint32_t combatNextMask(uint32_t mask) noexcept { return (mask << 1u); }
inline uint32_t combatPrevMask(uint32_t mask) noexcept { return (mask >> 1u); }

// Convenience bounds for iteration using project enums.
inline uint32_t combatFirstMask() noexcept { return static_cast<uint32_t>(COMBAT_PHYSICALDAMAGE); }
inline uint32_t combatLastMask() noexcept  { return static_cast<uint32_t>(COMBAT_LAST); }

// Validate mask is single-bit and within defined range.
inline bool combatMaskValid(uint32_t mask) noexcept {
	return isSingleBitMask(mask) && mask <= combatLastMask();
}

#endif // __COMBAT_HELPERS__