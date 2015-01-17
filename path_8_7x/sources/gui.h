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

#ifndef __GUI__
#define __GUI__
#include "definitions.h"
#if defined(WINDOWS) && !defined(_CONSOLE)

#include "resources.h"
#include "playerbox.h"

class GUI
{
	public:
		static GUI* getInstance()
		{
			static GUI instance;
			return &instance;
		}

		void initTrayMenu();
		void initFont();

		uint64_t m_lineCount;
		std::string m_logText;

		bool m_connections, m_minimized;
		HWND m_mainWindow, m_statusBar, m_logWindow;

		HFONT m_font;
		PlayerBox m_pBox;
		HMENU m_trayMenu;
};
#endif
#endif
