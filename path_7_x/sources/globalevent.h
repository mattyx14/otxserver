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

#ifndef __GLOBALEVENT__
#define __GLOBALEVENT__
#include "baseevents.h"

#include "const.h"
#include "scheduler.h"

#define TIMER_INTERVAL 1000

enum GlobalEvent_t
{
	GLOBALEVENT_NONE,
	GLOBALEVENT_TIMER,

	GLOBALEVENT_STARTUP,
	GLOBALEVENT_SHUTDOWN,
	GLOBALEVENT_GLOBALSAVE,
	GLOBALEVENT_RECORD
};

class GlobalEvent;
typedef std::map<std::string, GlobalEvent*> GlobalEventMap;

class GlobalEvents : public BaseEvents
{
	public:
		GlobalEvents();
		virtual ~GlobalEvents();
		void startup();

		void timer();
		void think();
		void execute(GlobalEvent_t type);

		GlobalEventMap getEventMap(GlobalEvent_t type);
		void clearMap(GlobalEventMap& map);

	protected:
		virtual std::string getScriptBaseName() const {return "globalevents";}
		virtual void clear();

		virtual Event* getEvent(const std::string& nodeName);
		virtual bool registerEvent(Event* event, xmlNodePtr p, bool override);

		virtual LuaInterface& getInterface() {return m_interface;}
		LuaInterface m_interface;

		GlobalEventMap thinkMap, serverMap, timerMap;
};

class GlobalEvent : public Event
{
	public:
		GlobalEvent(LuaInterface* _interface);
		virtual ~GlobalEvent() {}

		virtual bool configureEvent(xmlNodePtr p);

		int32_t executeRecord(uint32_t current, uint32_t old, Player* player);
		int32_t executeEvent();

		GlobalEvent_t getEventType() const {return m_eventType;}
		std::string getName() const {return m_name;}

		uint32_t getInterval() const {return m_interval;}

		int64_t getLastExecution() const {return m_lastExecution;}
		void setLastExecution(int64_t time) {m_lastExecution = time;}

	protected:
		GlobalEvent_t m_eventType;

		virtual std::string getScriptEventName() const;
		virtual std::string getScriptEventParams() const;

		std::string m_name;
		int64_t m_lastExecution;
		uint32_t m_interval;
};
#endif
