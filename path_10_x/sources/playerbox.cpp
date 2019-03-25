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
#include "playerbox.h"
#if defined(WINDOWS) && !defined(_CONSOLE)

#include "gui.h"
#include "player.h"
#include "ioban.h"

#include "game.h"
extern Game g_game;

HWND PlayerBox::playerBox = NULL;
HWND PlayerBox::parent = NULL;

HWND PlayerBox::ban = NULL;
HWND PlayerBox::kick = NULL;
HWND PlayerBox::list = NULL;
HWND PlayerBox::online = NULL;

HINSTANCE PlayerBox::m_instance = NULL;

PlayerBox::PlayerBox()
{
	HINSTANCE hInst = GetModuleHandle(NULL);
	WNDCLASSEX wcex;
	if(!GetClassInfoEx(hInst, "PlayerBox", &wcex))
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = (WNDPROC)WndProc;

		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;

		wcex.lpszMenuName = NULL;
		wcex.hIcon = NULL;
		wcex.hIconSm = NULL;

		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);

		wcex.lpszClassName = "PlayerBox";
		if(!RegisterClassEx(&wcex))
			MessageBox(NULL, "Cannot create PlayerBox!", "Error", MB_OK);
	}
}

void PlayerBox::updatePlayersOnline()
{
	int32_t playersOnline = SendMessage(list, CB_GETCOUNT, 0, 0);
	char playersOnlineBuffer[50];

	sprintf(playersOnlineBuffer, "%d player%s online", playersOnline, (playersOnline != 1 ? "s" : ""));
	SendMessage(online, WM_SETTEXT, 0, (LPARAM)playersOnlineBuffer);
}

void PlayerBox::addPlayer(Player* player)
{
	SendMessage(list, CB_ADDSTRING, 0, (LPARAM)player->getName().c_str());
	updatePlayersOnline();
}

void PlayerBox::removePlayer(Player* player)
{
	DWORD index = SendMessage(list, CB_FINDSTRING, 0,(LPARAM)player->getName().c_str());
	if((signed)index != CB_ERR)
		SendMessage(list, CB_DELETESTRING, index, 0);

	updatePlayersOnline();
}

LRESULT CALLBACK PlayerBox::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
		{
			int32_t playersOnline = g_game.getPlayersOnline();
			char playersOnlineBuffer[50];
			sprintf(playersOnlineBuffer, "%d player%s online", playersOnline, (playersOnline != 1 ? "s" : ""));
			m_instance = GetModuleHandle(NULL);

			// ban = CreateWindowEx(0, "button", "Ban permamently", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 5, 35, 115, 25, hWnd, NULL, m_instance, NULL);
			kick = CreateWindowEx(0, "button", "Kick", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 125, 35, 90, 25, hWnd, NULL, m_instance, NULL);
			list = CreateWindowEx(0, "combobox", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL/* | CBS_DROPDOWNLIST*/ | CBS_SORT, 5, 5, 210, 25, hWnd, NULL, m_instance, NULL);
			online = CreateWindowEx(WS_EX_STATICEDGE, "static", playersOnlineBuffer, WS_VISIBLE | WS_CHILD | WS_TABSTOP, 5, 65, 210, 20, hWnd, NULL, m_instance, NULL);

			// SendMessage(ban, WM_SETFONT, (WPARAM)GUI::getInstance()->m_font, 0);
			SendMessage(kick, WM_SETFONT, (WPARAM)GUI::getInstance()->m_font, 0);
			SendMessage(list, WM_SETFONT, (WPARAM)GUI::getInstance()->m_font, 0);
			SendMessage(online, WM_SETFONT, (WPARAM)GUI::getInstance()->m_font, 0);
			for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
				SendMessage(list, CB_ADDSTRING, 0, (LPARAM)it->second->getName().c_str());

			break;
		}
		case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
				{
					char name[30];
					GetWindowText(list, name, sizeof(name));
					if(Player* player = g_game.getPlayerByName(name))
					{
						char buffer[150];
						sprintf(buffer, "Are you sure you want to %s %s?", ((HWND)lParam == kick ? "kick" : "ban permamently"), player->getName().c_str());
						if(MessageBox(hWnd, buffer, "Player management", MB_YESNO) == IDYES)
						{
							if((HWND)lParam == ban)
								//IOBan::getInstance()->addPlayerBanishment(player->getID(), -1, 21, ACTION_DELETION, "Permament banishment.", 0, PLAYERBAN_BANISHMENT);

							g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_GREEN);
							Scheduler::getInstance().addEvent(createSchedulerTask(1000, boost::bind(
								&Game::kickPlayer, &g_game, player->getID(), false)));
						}
					}
					else
						MessageBox(hWnd, "A player with this name is not online", "Player management", MB_OK);

					break;
				}
				default:
					break;
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(parent, true);
			SetForegroundWindow(parent);
			DestroyWindow(hWnd);

			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool PlayerBox::popUp(LPCTSTR szCaption)
{
	RECT r;
	GetWindowRect(GetDesktopWindow(), &r);

	playerBox = CreateWindowEx(WS_EX_TOOLWINDOW, "PlayerBox", szCaption, WS_POPUPWINDOW|WS_CAPTION|WS_TABSTOP, (r.right-200)/2, (r.bottom-115)/2, 225, 115, parent, NULL, m_instance, NULL);
	if(!playerBox)
		return false;

	SetForegroundWindow(playerBox);
	EnableWindow(parent, FALSE);
	ShowWindow(playerBox, SW_SHOW);
	UpdateWindow(playerBox);

	BOOL ret = 0;
	MSG msg;

	SendMessage(list, WM_KEYDOWN, VK_DOWN, 0);
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
		{
			SendMessage(playerBox, WM_DESTROY, 0, 0);
			ret = 0;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return ret == TRUE;
}
#endif
