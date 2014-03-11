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

#ifndef __SCRIPTMANAGER__
#define __SCRIPTMANAGER__
#include "otsystem.h"

struct ModBlock
{
	std::string name, description, author, version, contact, file;
	bool enabled;
};
typedef std::map<std::string, ModBlock> ModMap;

struct LibBlock
{
	std::string first, second;
};
typedef std::map<std::string, LibBlock> LibMap;

class ScriptManager
{
	public:
		static ScriptManager* getInstance()
		{
			static ScriptManager instance;
			return &instance;
		}

		ScriptManager();
		virtual ~ScriptManager() {clearMods();}

		bool loadSystem();
		bool loadMods();

		void clearMods();
		bool reloadMods();

		inline LibMap::iterator getFirstLib() {return libMap.begin();}
		inline LibMap::iterator getLastLib() {return libMap.end();}

		inline ModMap::iterator getFirstMod() {return modMap.begin();}
		inline ModMap::iterator getLastMod() {return modMap.end();}

	protected:
		bool loadFromXml(const std::string& file, bool& enabled);

	private:
		bool modsLoaded;

		LibMap libMap;
		ModMap modMap;
};
#endif
