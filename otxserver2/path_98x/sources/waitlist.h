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

#ifndef __WAITLIST__
#define __WAITLIST__

struct Wait
{
	uint32_t ip;
	std::string name;
	bool premium;
	int64_t timeout;
};
typedef std::list<Wait*> WaitList;

class Player;
class WaitingList
{
	public:
		virtual ~WaitingList() {waitList.clear();}
		static WaitingList* getInstance()
		{
			static WaitingList waitingList;
			return &waitingList;
		}

		bool login(const Player* player);
		int32_t getSlot(const Player* player);

		static int32_t getTime(int32_t slot);

	protected:
		WaitingList() {}
		void cleanup();

		WaitList::iterator find(const Player* player, uint32_t& slot);
		int32_t getTimeout(int32_t slot) {return getTime(slot) + 15;}

		WaitList waitList;
};
#endif
