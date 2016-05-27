/**
* This file doesn't belong to theforgottenserver developers.
*/

#ifndef FS_MODULE_H_73FCAF4608CB41399D53C919316646A9
#define FS_MODULE_H_73FCAF4608CB41399D53C919316646A9

#include "luascript.h"
#include "baseevents.h"
#include "networkmessage.h"

enum ModuleType_t {
	MODULE_TYPE_RECVBYTE,
	MODULE_TYPE_NONE,
};

class Module;

class Modules final : public BaseEvents
{
	public:
		Modules();
		~Modules();

		// non-copyable
		Modules(const Modules&) = delete;
		Modules& operator=(const Modules&) = delete;
	
		void executeOnRecvbyte(Player* player, NetworkMessage& msg, uint8_t byte);

	protected:
		LuaScriptInterface& getScriptInterface() final;
		std::string getScriptBaseName() const final;
		Event* getEvent(const std::string& nodeName) final;
		bool registerEvent(Event* event, const pugi::xml_node& node) final;
		Module* getEventByRecvbyte(uint8_t recvbyte, bool force);
		void clear() final;


		typedef std::map<uint8_t, Module*> ModulesList;
		ModulesList recvbyteList;

		LuaScriptInterface scriptInterface;
};

class Module final : public Event
{
	public:
		explicit Module(LuaScriptInterface* interface);

		bool configureEvent(const pugi::xml_node& node) final;

		ModuleType_t getEventType() const {
			return type;
		}
		bool isLoaded() const {
			return loaded;
		}

		void clearEvent();
		void copyEvent(Module* creatureEvent);

		//scripting
		void executeOnRecvbyte(Player* player, NetworkMessage& msg);
		//

		uint8_t getRecvbyte() {
			return recvbyte;
		}

		int16_t getDelay() {
			return delay;
		}
	protected:
		std::string getScriptEventName() const final;

		ModuleType_t type;
		uint8_t recvbyte;
		int16_t delay;
		bool loaded;
};

#endif
