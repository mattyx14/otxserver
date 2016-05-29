/**
* This file doesn't belong to theforgottenserver developers.
*/

#include "otpch.h"

#include "modules.h"
#include "player.h"

Modules::Modules() :
	scriptInterface("Modules Interface")
{
	scriptInterface.initState();
}

Modules::~Modules()
{
	for (const auto& it : recvbyteList) {
		delete it.second;
	}
}

void Modules::clear()
{
	//clear recvbyte list
	for (const auto& it : recvbyteList) {
		it.second->clearEvent();
	}

	//clear lua state
	scriptInterface.reInitState();
}

LuaScriptInterface& Modules::getScriptInterface()
{
	return scriptInterface;
}

std::string Modules::getScriptBaseName() const
{
	return "modules";
}

Event* Modules::getEvent(const std::string& nodeName)
{
	if (strcasecmp(nodeName.c_str(), "module") != 0) {
		return nullptr;
	}
	return new Module(&scriptInterface);
}

bool Modules::registerEvent(Event* event, const pugi::xml_node&)
{
	Module* module = static_cast<Module*>(event);
	if (module->getEventType() == MODULE_TYPE_NONE) {
		std::cout << "Error: [Modules::registerEvent] Trying to register event without type!" << std::endl;
		return false;
	}

	Module* oldModule = getEventByRecvbyte(module->getRecvbyte(), false);
	if (oldModule) {
		if (!oldModule->isLoaded() && oldModule->getEventType() == module->getEventType()) {
			oldModule->copyEvent(module);
		}
		return false;
	} else {
		recvbyteList[module->getRecvbyte()] = module;
		return true;
	}
}

Module* Modules::getEventByRecvbyte(uint8_t recvbyte, bool force)
{
	ModulesList::iterator it = recvbyteList.find(recvbyte);
	if (it != recvbyteList.end()) {
		if (!force || it->second->isLoaded()) {
			return it->second;
		}
	}
	return nullptr;
}

void Modules::executeOnRecvbyte(Player* player, NetworkMessage& msg, uint8_t byte)
{
	for (const auto& it : recvbyteList) {
		Module* module = it.second;
		if (module->getEventType() == MODULE_TYPE_RECVBYTE && module->getRecvbyte() == byte && player->canRunModule(module->getRecvbyte())) {
			player->setModuleDelay(module->getRecvbyte(), module->getDelay());
			module->executeOnRecvbyte(player, msg);
			return;
		}
	}
}
/////////////////////////////////////

Module::Module(LuaScriptInterface* interface) :
	Event(interface), type(MODULE_TYPE_NONE), loaded(false) {}

bool Module::configureEvent(const pugi::xml_node& node)
{
	delay = 0;

	pugi::xml_attribute typeAttribute = node.attribute("type");
	if (!typeAttribute) {
		std::cout << "[Error - Modules::configureEvent] Missing type for module." << std::endl;
		return false;
	}

	std::string tmpStr = asLowerCaseString(typeAttribute.as_string());
	if (tmpStr == "recvbyte") {
		pugi::xml_attribute byteAttribute = node.attribute("byte");
		if (!byteAttribute) {
			std::cout << "[Error - Modules::configureEvent] Missing byte for module typed recvbyte." << std::endl;
			return false;
		}

		recvbyte = static_cast<uint8_t>(byteAttribute.as_int());
		type = MODULE_TYPE_RECVBYTE;
	} else {
		std::cout << "[Error - Modules::configureEvent] Invalid type for module." << std::endl;
		return false;
	}

	pugi::xml_attribute delayAttribute = node.attribute("delay");
	if (delayAttribute) {
		delay = static_cast<uint16_t>(delayAttribute.as_uint());
	}

	loaded = true;
	return true;
}

std::string Module::getScriptEventName() const
{
	switch (type) {
		case MODULE_TYPE_RECVBYTE:
			return "onRecvbyte";
		default:
			return std::string();
	}
}

void Module::copyEvent(Module* module)
{
	scriptId = module->scriptId;
	scriptInterface = module->scriptInterface;
	scripted = module->scripted;
	loaded = module->loaded;
}

void Module::clearEvent()
{
	scriptId = 0;
	scriptInterface = nullptr;
	scripted = false;
	loaded = false;
}

void Module::executeOnRecvbyte(Player* player, NetworkMessage& msg)
{
	//onAdvance(player, skill, oldLevel, newLevel)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - CreatureEvent::executeAdvance] Call stack overflow" << std::endl;
		return;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);
	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");
	
	LuaScriptInterface::pushUserdata<NetworkMessage>(L, const_cast<NetworkMessage*>(&msg));
	LuaScriptInterface::setWeakMetatable(L, -1, "NetworkMessage");

	lua_pushnumber(L, recvbyte);

	scriptInterface->callVoidFunction(3);
}
