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

#ifndef __TEMPLATES__
#define __TEMPLATES__
#include "otsystem.h"

template<class T> class AutoList : public std::map<uint32_t, T*>
{
	public:
		AutoList() {}
		virtual ~AutoList() {}
};

class AutoId
{
	public:
		AutoId()
		{
			boost::recursive_mutex::scoped_lock lockClass(lock);
			++count;
			if(count >= 0xFFFFFF)
				count = 1000;

			while(list.find(count) != list.end())
			{
				if(count >= 0xFFFFFF)
					count = 1000;
				else
					++count;
			}

			list.insert(count);
			autoId = count;
		}

		virtual ~AutoId()
		{
			std::set<uint32_t>::iterator it = list.find(autoId);
			if(it != list.end())
				list.erase(it);
		}

		uint32_t autoId;

	protected:
		static uint32_t count;

		typedef std::set<uint32_t> List;
		static List list;

		static boost::recursive_mutex lock;
};
#endif
