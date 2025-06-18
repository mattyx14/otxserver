#ifndef __MOUNTS__
#define __MOUNTS__

struct Mount
{
	Mount(uint16_t id, uint16_t clientId, std::string name, bool premium) :
		name(std::move(name)), id(id), clientId(clientId), premium(premium) {}

	std::string name;
	uint16_t id, clientId;
	bool premium;
};

class Mounts
{
	public:
		static Mounts* getInstance()
		{
			static Mounts instance;
			return &instance;
		}

		bool loadFromXml();
		bool parseMountNode(xmlNodePtr p);
		Mount* getMountByID(uint8_t id);
		Mount* getMountByClientID(uint16_t clientId);

		const std::vector<Mount>& getMounts() const {
			return mounts;
		}

	private:
		std::vector<Mount> mounts;
};

#endif
