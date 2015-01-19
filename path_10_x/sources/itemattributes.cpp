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

#include "itemattributes.h"
#include "fileloader.h"

ItemAttributes::ItemAttributes(const ItemAttributes& o)
{
	if(o.attributes)
		attributes = new AttributeMap(*o.attributes);
}

void ItemAttributes::createAttributes()
{
	if(!attributes)
		attributes = new AttributeMap;
}

void ItemAttributes::eraseAttribute(const char* key)
{
	if(!attributes)
		return;

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		attributes->erase(it);
}

void ItemAttributes::setAttribute(const char* key, boost::any value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const char* key, const std::string& value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const char* key, int32_t value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const char* key, float value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const char* key, bool value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

boost::any ItemAttributes::getAttribute(const char* key) const
{
	if(!attributes)
		return boost::any();

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		return it->second.get();

	return boost::any();
}

std::string ItemAttributes::getStringAttribute(const std::string& key, bool &ok) const
{
	if(!attributes)
	{
		ok = false;
		return std::string();
	}

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		return it->second.getString(ok);

	ok = false;
	return std::string();
}

int32_t ItemAttributes::getIntegerAttribute(const std::string& key, bool &ok) const
{
	if(!attributes)
	{
		ok = false;
		return 0;
	}

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		return it->second.getInteger(ok);

	ok = false;
	return 0;
}

float ItemAttributes::getFloatAttribute(const std::string& key, bool &ok) const
{
	if(!attributes)
	{
		ok = false;
		return 0.0;
	}

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		return it->second.getFloat(ok);

	ok = false;
	return 0.0;
}

bool ItemAttributes::getBooleanAttribute(const std::string& key, bool &ok) const
{
	if(!attributes)
	{
		ok = false;
		return false;
	}

	AttributeMap::iterator it = attributes->find(key);
	if(it != attributes->end())
		return it->second.getBoolean(ok);

	ok = false;
	return false;
}

ItemAttribute& ItemAttribute::operator=(const ItemAttribute& o)
{
	if(&o == this)
		return *this;

	m_data = o.m_data;
	return *this;
}

std::string ItemAttribute::getString(bool &ok) const
{
	if(m_data.type() != typeid(std::string))
	{
		ok = false;
		return std::string();
	}

	ok = true;
	return boost::any_cast<std::string>(m_data);
}

int32_t ItemAttribute::getInteger(bool &ok) const
{
	if(m_data.type() != typeid(int32_t))
	{
		ok = false;
		return 0;
	}

	ok = true;
	return boost::any_cast<int32_t>(m_data);
}

float ItemAttribute::getFloat(bool &ok) const
{
	if(m_data.type() != typeid(float))
	{
		ok = false;
		return 0.0;
	}

	ok = true;
	return boost::any_cast<float>(m_data);
}

bool ItemAttribute::getBoolean(bool &ok) const
{
	if(m_data.type() != typeid(bool))
	{
		ok = false;
		return false;
	}

	ok = true;
	return boost::any_cast<bool>(m_data);
}

bool ItemAttributes::unserializeMap(PropStream& stream)
{
	uint16_t n;
	if(!stream.getShort(n))
		return true;

	createAttributes();
	while(n--)
	{
		std::string key;
		if(!stream.getString(key))
			return false;

		ItemAttribute attr;
		if(!attr.unserialize(stream))
			return false;

		(*attributes)[key] = attr;
	}

	return true;
}

void ItemAttributes::serializeMap(PropWriteStream& stream) const
{
	stream.addShort((uint16_t)std::min((size_t)0xFFFF, attributes->size()));
	AttributeMap::const_iterator it = attributes->begin();
	for(int32_t i = 0; it != attributes->end() && i <= 0xFFFF; ++it, ++i)
	{
		std::string key = it->first;
		if(key.size() > 0xFFFF)
			stream.addString(key.substr(0, 0xFFFF));
		else
			stream.addString(key);

		it->second.serialize(stream);
	}
}

bool ItemAttribute::unserialize(PropStream& stream)
{
	uint8_t type = 0;
	stream.getByte(type);
	switch(type)
	{
		case STRING:
		{
			std::string v;
			if(!stream.getLongString(v))
				return false;

			set(v);
			break;
		}
		case INTEGER:
		{
			uint32_t v;
			if(!stream.getLong(v))
				return false;

			set((int32_t)v);
			break;
		}
		case FLOAT:
		{
			float v;
			if(!stream.getFloat(v))
				return false;

			set(v);
			break;
		}
		case BOOLEAN:
		{
			uint8_t v;
			if(!stream.getByte(v))
				return false;

			set(v != 0);
		}
	}

	return true;
}

void ItemAttribute::serialize(PropWriteStream& stream) const
{
	bool ok;
	if(m_data.type() == typeid(std::string))
	{
		stream.addByte((uint8_t)STRING);
		stream.addLongString(getString(ok));
	}
	else if(m_data.type() == typeid(int32_t))
	{
		stream.addByte((uint8_t)INTEGER);
		stream.addLong(getInteger(ok));
	}
	else if(m_data.type() == typeid(float))
	{
		stream.addByte((uint8_t)FLOAT);
		stream.addLong(getFloat(ok));
	}
	else if(m_data.type() == typeid(bool))
	{
		stream.addByte((uint8_t)BOOLEAN);
		stream.addByte(getBoolean(ok));
	}
	else
		std::clog << "[ItemAttribute::serialize]: Invalid data type." << std::endl;
}
