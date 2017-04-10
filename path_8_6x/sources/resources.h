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

#ifndef __RESOURCES__
#define __RESOURCES__
#include "definitions.h"
#if defined(WINDOWS) && !defined(_CONSOLE)
	#define ID_KICK 100
	#define ID_ABOUT 101
	#define ID_LOG 102
	#define ID_ICON 103
	#define ID_TRAY_HIDE 104
	#define ID_TRAY_SHUTDOWN 105
	#define ID_STATUS_BAR 106

	#define ID_MENU 200

	#define ID_MENU_MAIN_ACCEPT 300
	#define ID_MENU_MAIN_REJECT 301
	#define ID_MENU_MAIN_CLEARLOG 302
	#define ID_MENU_MAIN_SHUTDOWN 303

	#define ID_MENU_SERVER_BROADCAST 400
	#define ID_MENU_SERVER_CLEAN 401
	#define ID_MENU_SERVER_REFRESH 402
	#define ID_MENU_SERVER_SAVE 403
	#define ID_MENU_SERVER_CLOSE 404
	#define ID_MENU_SERVER_OPEN 405
	#define ID_MENU_ADD_PREMIUM 406
	#define ID_MENU_KICK_PLAYER 407

	#define ID_MENU_RELOAD_ACTIONS 500
	#define ID_MENU_RELOAD_CHAT 501
	#define ID_MENU_RELOAD_CONFIG 502
	#define ID_MENU_RELOAD_CREATUREEVENTS 503
	#ifdef __LOGIN_SERVER__
	#define ID_MENU_RELOAD_GAMESERVERS 504
	#endif
	#define ID_MENU_RELOAD_GLOBALEVENTS 505
	#define ID_MENU_RELOAD_HIGHSCORES 506
	#define ID_MENU_RELOAD_MONSTERS 507
	#define ID_MENU_RELOAD_MOVEMENTS 508
	#define ID_MENU_RELOAD_OUTFITS 509
	#define ID_MENU_RELOAD_QUESTS 510
	#define ID_MENU_RELOAD_RAIDS 511
	#define ID_MENU_RELOAD_SPELLS 512
	#define ID_MENU_RELOAD_STAGES 513
	#define ID_MENU_RELOAD_TALKACTIONS 514
	#define ID_MENU_RELOAD_MODS 515

	#define ID_MENU_OTSERV 600
	#define ID_MENU_OTSERV2 601
	#define ID_MENU_OTSERV3 602
	#define ID_MENU_OT_SERVERLIST 603
	#define ID_MENU_BUG_FEATURE 604
	#define ID_MENU_SERVER_SOURCE 605

	#define ID_MENU_SERVER_WORLDTYPE_OPTIONAL 700
	#define ID_MENU_SERVER_WORLDTYPE_OPEN 701
	#define ID_MENU_SERVER_WORLDTYPE_HARDCORE 702

	#define ID_MENU_ABOUT_SERVER 800
	#define ID_MENU_ABOUT_DEVELOPERS 801
	#define ID_MENU_ABOUT_GUI_EXECUTABLE 802
#endif

// Compatible with 8.60
#define CLIENT_VERSION_MIN 860
#define CLIENT_VERSION_MAX 860
#define CLIENT_VERSION_ITEMS 20
#define CLIENT_VERSION_STRING "8.60"

#define SOFTWARE_NAME "OTX Server"
#define SOFTWARE_VERSION "2"
#define MINOR_VERSION "9"
#define SOFTWARE_CODENAME "OpenTibia"
#define SOFTWARE_DEVELOPERS "Matt Gomez, Leandro Brewster and The Forgotten Server Developers"
#define GIT_REPO "https://github.com/mattyx14/otxserver/tree/otxserv2/"

//#define CLIENT_VERSION_DATA
#define CLIENT_VERSION_DAT 0
#define CLIENT_VERSION_SPR 0
#define CLIENT_VERSION_PIC 0

#define VERSION_DATABASE 42
#endif
