/**
 * The OTXServer Project - based on TFS
 * Copyright (C) 2017  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "gamestore.h"

#include "pugicast.h"
#include "tools.h"

#include <boost/algorithm/string.hpp>

std::vector<std::string> getIconsVector(std::string rawString){
    std::vector <std::string> icons;
    boost::split(icons, rawString, boost::is_any_of("|")); //converting the |-separated string to a vector of tokens

    return icons;
}

bool GameStore::reload() {
    for(auto category:storeOffers){
        for(auto offer:category.offers){
            offer.icons.clear();
        }
        category.offers.clear();
        category.icons.clear();
    }
    storeOffers.clear();
    return loadFromXml();
}

bool GameStore::loadFromXml() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("data/XML/gamestore.xml");
    if (!result) {
        printXMLError("Error - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
        return false;
    }

    for (auto categoryNode : doc.child("gamestore").children()){ //category iterator
        StoreCategory* cat = new StoreCategory();
        cat->name = categoryNode.attribute("name").as_string();
        cat->description = categoryNode.attribute("description").as_string("");
        std::string state = categoryNode.attribute("state").as_string("normal");

        if(boost::iequals(state,"normal")){ //reading state (defaults to normal)
            cat->state=CategoryState_t::NORMAL;
        }
        else if(boost::iequals(state,"new")){
            cat->state=CategoryState_t::NEW;
        }
        else if(boost::iequals(state,"sale")){
            cat->state=CategoryState_t::SALE;
        }
        else if(boost::iequals(state,"limitedtime")){
            cat->state=CategoryState_t::LIMITED_TIME;
        }

        cat->icons= getIconsVector(categoryNode.attribute("icons").as_string("default.png"));

        for(auto offerNode : categoryNode.children()){
            std::string type = categoryNode.attribute("state").as_string();
            if(boost::iequals(type,"namechange") || boost::iequals(type,"sexchange") || boost::iequals(type,"promotion")){
                BaseOffer* baseOffer = new BaseOffer();
            }
            else if(boost::iequals(type,"namechange")){
                //TODO: Continue here
            }

        }


    }

//    for (auto mountNode : doc.child("mounts").children()) {
//        mounts.emplace_back(
//                static_cast<uint8_t>(pugi::cast<uint16_t>(mountNode.attribute("id").value())),
//                pugi::cast<uint16_t>(mountNode.attribute("clientid").value()),
//                mountNode.attribute("name").as_string(),
//                pugi::cast<int32_t>(mountNode.attribute("speed").value()),
//                mountNode.attribute("premium").as_bool()
//        );
//    }
//    mounts.shrink_to_fit();
    return true;
}
