#include "otpch.h"

#include "mounts.h"

#include "tools.h"

bool Mounts::parseMountNode(xmlNodePtr p)
{
	if(xmlStrcmp(p->name, (const xmlChar*)"mount"))
		return false;

	int32_t id;
	if(!readXMLInteger(p, "id", id))
	{
		std::clog << "[Error - Outfits::parseMountNode] Missing mount id, skipping" << std::endl;
		return false;
	}

	int32_t clientId;
	if(!readXMLInteger(p, "clientid", clientId))
	{
		std::clog << "[Error - Outfits::parseMountNode] Missing mount clientid, skipping" << std::endl;
		return false;
	}

	std::string name;
	if(!readXMLString(p, "name", name))
	{
		std::clog << "[Error - Outfits::parseMountNode] Missing mount name, skipping" << std::endl;
		return false;
	}

	bool premium = false;
	std::string strPremium;
	if(readXMLString(p, "premium", strPremium))
		premium = booleanString(strPremium);

	Mount* mount = new Mount(id, clientId, name, premium);
	mounts.push_back(*mount);
	return true;
}

bool Mounts::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML, "mounts.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Mounts::loadFromXml] Cannot load mounts file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr p, root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"mounts"))
	{
		std::clog << "[Error - Mounts::loadFromXml] Malformed mounts file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	p = root->children;
	while(p)
	{
		parseMountNode(p);
		p = p->next;
	}

	xmlFreeDoc(doc);
	return true;
}

Mount* Mounts::getMountByID(uint8_t id)
{
	auto it = std::find_if(mounts.begin(), mounts.end(), [id](const Mount& mount) {
		return mount.id == id;
	});

	return it != mounts.end() ? &*it : nullptr;
}

Mount* Mounts::getMountByClientID(uint16_t clientId)
{
	auto it = std::find_if(mounts.begin(), mounts.end(), [clientId](const Mount& mount) {
		return mount.clientId == clientId;
	});

	return it != mounts.end() ? &*it : nullptr;
}
