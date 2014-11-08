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

#ifndef __PLAYERBOX__
#define __PLAYERBOX__
#include "otsystem.h"
#if defined(WINDOWS) && !defined(_CONSOLE)

class Player;
class PlayerBox
{
	static HWND playerBox;
	static HWND parent;

	static HWND kick;
	static HWND ban;
	static HWND online;

	static HINSTANCE m_instance;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	public:
		PlayerBox();
		virtual ~PlayerBox() {}

		void updatePlayersOnline();
		void addPlayer(Player* player);
		void removePlayer(Player* player);

		void setParent(HWND _parent) {parent = _parent;}
		bool popUp(LPCTSTR szCaption);
		static HWND list;
};
#endif
#endif
