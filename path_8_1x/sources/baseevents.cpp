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
#include <libxml/xmlmemory.h>

#include "baseevents.h"
#include "tools.h"

bool BaseEvents::loadFromXml()
{
	std::string scriptsName = getScriptBaseName();
	if(m_loaded)
	{
		std::clog << "[Error - BaseEvents::loadFromXml] " << scriptsName << " interface already loaded!" << std::endl;
		return false;
	}

	std::string path = getFilePath(FILE_TYPE_OTHER, std::string(scriptsName + "/lib/"));
	if(!getInterface().loadDirectory(path, false, true))
		std::clog << "[Warning - BaseEvents::loadFromXml] Cannot load " << path << std::endl;

	path = getFilePath(FILE_TYPE_OTHER, std::string(scriptsName + "/" + scriptsName + ".xml"));
	xmlDocPtr doc = xmlParseFile(path.c_str());
	if(!doc)
	{
		std::clog << "[Warning - BaseEvents::loadFromXml] Cannot open " << path << " file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name, (const xmlChar*)scriptsName.c_str()))
	{
		std::clog << "[Error - BaseEvents::loadFromXml] Malformed " << path << " file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	path = getFilePath(FILE_TYPE_OTHER, std::string(scriptsName + "/scripts/"));
	for(xmlNodePtr p = root->children; p; p = p->next)
		parseEventNode(p, path, false);

	xmlFreeDoc(doc);
	m_loaded = true;
	return m_loaded;
}

bool BaseEvents::parseEventNode(xmlNodePtr p, std::string scriptsPath, bool override)
{
	Event* event = getEvent((const char*)p->name);
	if(!event)
		return false;

	if(!event->configureEvent(p))
	{
		std::clog << "[Warning - BaseEvents::loadFromXml] Cannot configure an event" << std::endl;
		delete event;
		return false;
	}

	bool success = true, skip = false;
	std::string strValue, tmpStrValue;
	if(readXMLString(p, "event", strValue))
	{
		skip = true;
		tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "script")
		{
			bool file = readXMLString(p, "value", strValue);
			if(!file)
				success = parseXMLContentString(p->children, strValue);
			else
				strValue = scriptsPath + strValue;

			if(success)
				success = event->checkScript(getScriptBaseName(), strValue, file) && event->loadScript(strValue, file);
		}
		else if(tmpStrValue == "buffer")
		{
			if(!readXMLString(p, "value", strValue))
				success = parseXMLContentString(p->children, strValue);

			if(success)
				success = event->checkBuffer(getScriptBaseName(), strValue) && event->loadBuffer(strValue);
		}
		else if(tmpStrValue == "function")
		{
			if(readXMLString(p, "value", strValue))
				success = event->loadFunction(strValue);
			else
				success = false;
		}
		else
			skip = false;
	}

	if(!skip)
	{
		if(readXMLString(p, "script", strValue))
		{
			bool file = asLowerCaseString(strValue) != "cdata";
			if(!file)
				success = parseXMLContentString(p->children, strValue);
			else
				strValue = scriptsPath + strValue;

			if(success)
				success = event->checkScript(getScriptBaseName(), strValue, file) && event->loadScript(strValue, file);
		}
		else if(readXMLString(p, "buffer", strValue))
		{
			if(asLowerCaseString(strValue) == "cdata")
				success = parseXMLContentString(p->children, strValue);

			if(success)
				success = event->checkBuffer(getScriptBaseName(), strValue) && event->loadBuffer(strValue);
		}
		else if((readXMLString(p, "function", strValue) && event->loadFunction(strValue))
			|| (parseXMLContentString(p->children, strValue) && event->checkBuffer(
			getScriptBaseName(), strValue) && event->loadBuffer(strValue)))
			success = true;
		else
			success = false;
	}

	if(!success)
	{
		delete event;
		return false;
	}

	if(!override && readXMLString(p, "override", strValue) && booleanString(strValue))
		override = true;

	if(registerEvent(event, p, override))
		return true;

	if(!event)
		return false;

	delete event;
	return false;
}

bool BaseEvents::reload()
{
	m_loaded = false;
	clear();
	return loadFromXml();
}

Event::Event(const Event* copy)
{
	m_interface = copy->m_interface;
	m_scripted = copy->m_scripted;
	m_scriptId = copy->m_scriptId;

	/*std::string* oldScript = m_scriptData;
		This needs a reference system
		to safely free the string
		if its used in other events*/
	m_scriptData = copy->m_scriptData;
	/*if(oldScript)
		delete oldScript;*/
}

Event::~Event()
{
	/*if(m_scriptData)
		delete m_scriptData;*/
}

bool Event::loadBuffer(const std::string& buffer)
{
	if(!m_interface)
	{
		std::clog << "[Error - Event::loadBuffer] m_interface = NULL" << std::endl;
		return false;
	}

	if(m_scriptData)
	{
		std::clog << "[Error - Event::loadBuffer] m_scriptData != NULL" << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_BUFFER;
	m_scriptData = new std::string(buffer);
	return true;
}

bool Event::checkBuffer(const std::string& base, const std::string& script) const
{
	LuaInterface testInterface("Test Interface");
	testInterface.initState();

	std::string path = getFilePath(FILE_TYPE_OTHER, std::string(base + "/lib/"));
	if(!testInterface.loadDirectory(path, false, true))
		std::clog << "[Warning - Event::checkBuffer] Cannot load " << path << std::endl;

	if(m_scriptData)
	{
		std::clog << "[Error - Event::checkBuffer] m_scriptData != NULL" << std::endl;
		return false;
	}

	//TODO: is it really the way we should do it?...
	std::string buffer = script;
	trimString(buffer);

	std::stringstream scriptstream;
	scriptstream << "function " << getScriptEventName() << "(" << getScriptEventParams() << ")" << std::endl << buffer << std::endl << "end";

	buffer = scriptstream.str();
	if(testInterface.loadBuffer(buffer))
		return true;

	std::clog << "[Error - Event::checkBuffer] Cannot load buffer (" << script << ")"
		<< std::endl << testInterface.getLastError() << std::endl;
	return false;
}

bool Event::loadScript(const std::string& script, bool file)
{
	if(!m_interface)
	{
		std::clog << "[Error - Event::loadScript] m_interface = NULL" << std::endl;
		return false;
	}

	if(m_scriptId)
	{
		std::clog << "[Error - Event::loadScript] scriptId = " << m_scriptId << std::endl;
		return false;
	}

	bool result = false;
	if(!file)
	{
		std::string buffer = script, function = "function " + getScriptEventName();
		trimString(buffer);
		if(buffer.find(function) == std::string::npos)
		{
			std::stringstream scriptstream;
			scriptstream << function << "(" << getScriptEventParams() << ")" << std::endl << buffer << std::endl << "end";
			buffer = scriptstream.str();
		}

		result = m_interface->loadBuffer(buffer);
	}
	else
		result = m_interface->loadFile(script);

	if(!result)
	{
		std::clog << "[Warning - Event::loadScript] Cannot load script (" << script << ")"
			<< std::endl << m_interface->getLastError() << std::endl;
		return false;
	}

	int32_t id = m_interface->getEvent(getScriptEventName());
	if(id == -1)
	{
		std::clog << "[Warning - Event::loadScript] Event " << getScriptEventName() << " not found (" << script << ")" << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_TRUE;
	m_scriptId = id;
	return true;
}

bool Event::checkScript(const std::string& base, const std::string& script, bool file) const
{
	LuaInterface testInterface("Test Interface");
	testInterface.initState();

	std::string path = getFilePath(FILE_TYPE_OTHER, std::string(base + "/lib/"));
	if(!testInterface.loadDirectory(path, false, true))
		std::clog << "[Warning - Event::checkScript] Cannot load " << path << std::endl;

	if(m_scriptId)
	{
		std::clog << "[Error - Event::checkScript] scriptId = " << m_scriptId << std::endl;
		return false;
	}

	bool result = false;
	if(!file)
	{
		std::string buffer = script, function = "function " + getScriptEventName();
		trimString(buffer);
		if(buffer.find(function) == std::string::npos)
		{
			std::stringstream scriptstream;
			scriptstream << function << "(" << getScriptEventParams() << ")" << std::endl << buffer << std::endl << "end";
			buffer = scriptstream.str();
		}

		result = testInterface.loadBuffer(buffer);
	}
	else
		result = testInterface.loadFile(script);

	if(!result)
	{
		std::clog << "[Error - Event::checkScript] Cannot load script (" << script << ")"
			<< std::endl << testInterface.getLastError() << std::endl;
		return false;
	}

	if(testInterface.getEvent(getScriptEventName()) != -1)
		return true;

	std::clog << "[Error - Event::checkScript] Event " << getScriptEventName() << " not found (" << script << ")" << std::endl;
	return false;
}

CallBack::CallBack()
{
	m_scriptId = 0;
	m_interface = NULL;
	m_loaded = false;
}

bool CallBack::loadCallBack(LuaInterface* _interface, std::string name)
{
	if(!_interface)
	{
		std::clog << "[Error - CallBack::loadCallBack] m_interface = NULL" << std::endl;
		return false;
	}

	m_interface = _interface;
	int32_t id = m_interface->getEvent(name);
	if(id == -1)
	{
		std::clog << "[Warning - CallBack::loadCallBack] Event " << name << " not found." << std::endl;
		return false;
	}

	m_callbackName = name;
	m_scriptId = id;

	m_loaded = true;
	return true;
}
