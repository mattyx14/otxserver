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

#ifndef __BASEEVENTS__
#define __BASEEVENTS__
#include "otsystem.h"

#include "luascript.h"
#include <libxml/parser.h>

class Event;
class BaseEvents
{
	public:
		BaseEvents(): m_loaded(false) {}
		virtual ~BaseEvents() {}

		bool loadFromXml();
		bool reload();

		bool parseEventNode(xmlNodePtr p, std::string scriptsPath, bool override);
		bool isLoaded() const {return m_loaded;}

	protected:
		virtual std::string getScriptBaseName() const = 0;
		virtual void clear() = 0;

		virtual bool registerEvent(Event* event, xmlNodePtr p, bool override) = 0;
		virtual Event* getEvent(const std::string& nodeName) = 0;

		virtual LuaInterface& getInterface() = 0;

		bool m_loaded;
};

enum EventScript_t
{
	EVENT_SCRIPT_FALSE,
	EVENT_SCRIPT_BUFFER,
	EVENT_SCRIPT_TRUE
};

class Event
{
	public:
		Event(LuaInterface* _interface): m_interface(_interface),
			m_scripted(EVENT_SCRIPT_FALSE), m_scriptId(0), m_scriptData(NULL) {}
		Event(const Event* copy);
		virtual ~Event();

		virtual bool configureEvent(xmlNodePtr p) = 0;
		virtual bool isScripted() const {return m_scripted != EVENT_SCRIPT_FALSE;}

		bool loadBuffer(const std::string& buffer);
		bool checkBuffer(const std::string& base, const std::string& buffer) const;

		bool loadScript(const std::string& script, bool file);
		bool checkScript(const std::string& base, const std::string& script, bool file) const;

		virtual bool loadFunction(const std::string&) {return false;}

	protected:
		virtual std::string getScriptEventName() const = 0;
		virtual std::string getScriptEventParams() const = 0;

		LuaInterface* m_interface;
		EventScript_t m_scripted;

		int32_t m_scriptId;
		std::string* m_scriptData;
};

class CallBack
{
	public:
		CallBack();
		virtual ~CallBack() {}

		bool loadCallBack(LuaInterface* _interface, std::string name);

	protected:
		int32_t m_scriptId;
		LuaInterface* m_interface;

		bool m_loaded;
		std::string m_callbackName;
};
#endif
