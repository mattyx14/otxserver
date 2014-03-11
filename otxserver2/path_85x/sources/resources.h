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
	#define ID_KICK 101
	#define ID_ABOUT 104
	#define ID_LOG 105
	#define ID_ICON 106

	#define ID_TRAY_HIDE 118
	#define ID_TRAY_SHUTDOWN 119
	#define ID_STATUS_BAR 120

	#define ID_MENU 200

	#define ID_MENU_MAIN_REJECT 201
	#define ID_MENU_MAIN_ACCEPT 202
	#define ID_MENU_MAIN_CLEARLOG 203
	#define ID_MENU_MAIN_SHUTDOWN 204

	#define ID_MENU_SERVER_WORLDTYPE_OPTIONAL 205
	#define ID_MENU_SERVER_WORLDTYPE_OPEN 206
	#define ID_MENU_SERVER_WORLDTYPE_HARDCORE 207

	#define ID_MENU_SERVER_SAVE 209
	#define ID_MENU_SERVER_CLEAN 210
	#define ID_MENU_SERVER_REFRESH 211
	#define ID_MENU_SERVER_CLOSE 212
	#define ID_MENU_SERVER_OPEN 213

	#define ID_MENU_RELOAD_ACTIONS 214
	#define ID_MENU_RELOAD_CHAT 215
	#define ID_MENU_RELOAD_CONFIG 216
	#define ID_MENU_RELOAD_CREATUREEVENTS 217
	#ifdef __LOGIN_SERVER__
	#define ID_MENU_RELOAD_GAMESERVERS 218
	#endif
	#define ID_MENU_RELOAD_GLOBALEVENTS 219
	#define ID_MENU_RELOAD_MONSTERS 221
	#define ID_MENU_RELOAD_MOVEMENTS 222
	#define ID_MENU_RELOAD_QUESTS 223
	#define ID_MENU_RELOAD_RAIDS 224
	#define ID_MENU_RELOAD_SPELLS 225
	#define ID_MENU_RELOAD_STAGES 226
	#define ID_MENU_RELOAD_TALKACTIONS 227
	#define ID_MENU_RELOAD_MODS 228

	#define ID_MENU_ABOUT_SERVER 230
	#define ID_MENU_ABOUT_DEVELOPERS 231
	#define ID_MENU_ABOUT_GUI_EXECUTABLE 232

	#define ID_MENU_OTSERV 233
	#define ID_MENU_OT_SERVERLIST 234
	#define ID_MENU_BUG_FEATURE 235
	#define ID_MENU_SERVER_SOURCE 236
	#define ID_MENU_OTSERV2 237
	#define ID_MENU_OTSERV3 238

	#define ID_MENU_SERVER_BROADCAST 240
	#define ID_MENU_ADD_PREMIUM 241
	#define ID_MENU_KICK_PLAYER 242
#endif

// Compatible with 8.54 to 8.57
#define CLIENT_VERSION_MIN 854
#define CLIENT_VERSION_MAX 857
#define CLIENT_VERSION_ITEMS 17
#define CLIENT_VERSION_STRING "Only clients with protocol 8.5x allowed!"

#define SOFTWARE_NAME "OTX Server"
#define SOFTWARE_VERSION "2"
#define MINOR_VERSION "6"
#define PATCH_VERSION "0"
#define REVISION_VERSION ""
#define SOFTWARE_CODENAME "Alissow"
#define SOFTWARE_DEVELOPERS "Kaiser, Comedinhasss, Emma, Kazbin and The Forgotten Server Developers"
#define SOFTWARE_PROTOCOL "8.57"
#define FORUMS "www.blacktibia.org - www.tibiaking.com - www.otland.net"

//#define CLIENT_VERSION_DATA
#define CLIENT_VERSION_DAT 0
#define CLIENT_VERSION_SPR 0
#define CLIENT_VERSION_PIC 0

#define VERSION_DATABASE 37
#endif
