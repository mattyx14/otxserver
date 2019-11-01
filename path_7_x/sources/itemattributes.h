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

#ifndef __ITEM_ATTRIBUTES__
#define __ITEM_ATTRIBUTES__
#include "otsystem.h"
#include <boost/any.hpp>
#include <unordered_map>

class PropWriteStream;
class PropStream;

class ItemAttribute
{
	public:
		ItemAttribute() {}
		ItemAttribute(const ItemAttribute& o) {*this = o;}
		~ItemAttribute() {}

		ItemAttribute(const std::string& s) {m_data = s;}
		ItemAttribute(int32_t i) {m_data = i;}
		ItemAttribute(float f) {m_data = f;}
		ItemAttribute(bool b) {m_data = b;}

		ItemAttribute& operator=(const ItemAttribute& o);

		void serialize(PropWriteStream& stream) const;
		bool unserialize(PropStream& stream);

		void set(const std::string& s) { m_data = s; }
		void set(int32_t i) { m_data = i; }
		void set(float f) { m_data = f; }
		void set(bool b) { m_data = b; }
		void set(boost::any a) { m_data = a; }

		std::string getString(bool &ok) const;
		int32_t getInteger(bool &ok) const;
		float getFloat(bool &ok) const;
		bool getBoolean(bool &ok) const;
		boost::any get() const { return m_data; }

	private:
		enum Type
		{
			NONE = 0,
			STRING = 1,
			INTEGER = 2,
			FLOAT = 3,
			BOOLEAN = 4
		};

		boost::any m_data;
};

class ItemAttributes
{
	public:
		ItemAttributes(): attributes(NULL) {}
		ItemAttributes(const ItemAttributes &i);
		virtual ~ItemAttributes() {delete attributes;}

		void serializeMap(PropWriteStream& stream) const;
		bool unserializeMap(PropStream& stream);

		void eraseAttribute(const char* key);
		void setAttribute(const char* key, boost::any value);
		boost::any getAttribute(const char* key) const;

		void setAttribute(const char* key, const std::string& value);
		void setAttribute(const char* key, int32_t value);
		void setAttribute(const char* key, float value);
		void setAttribute(const char* key, bool value);

		std::string getStringAttribute(const std::string& key, bool &ok) const;
		int32_t getIntegerAttribute(const std::string& key, bool &ok) const;
		float getFloatAttribute(const std::string& key, bool &ok) const;
		bool getBooleanAttribute(const std::string& key, bool &ok) const;

		bool hasStringAttribute(const std::string& key)  const { bool ok; getStringAttribute(key, ok); return ok; }
		bool hasIntegerAttribute(const std::string& key) const { bool ok; getIntegerAttribute(key, ok); return ok; }
		bool hasFloatAttribute(const std::string& key)   const { bool ok; getFloatAttribute(key, ok); return ok; }
		bool hasBooleanAttribute(const std::string& key) const { bool ok; getBooleanAttribute(key, ok); return ok; }

	protected:
		void createAttributes();

		typedef std::unordered_map<std::string, ItemAttribute> AttributeMap;
		AttributeMap* attributes;
};

#endif
