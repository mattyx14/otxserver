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
    icons.shrink_to_fit();
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
    offerCount=0;
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
        if(!cat->name.length()){
            printXMLError("Error parsing XML category name  - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
            return false;
        }

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
            std::string type = offerNode.attribute("type").as_string();
            BaseOffer *offer = nullptr;
            if(boost::iequals(type,"namechange")){
                offer = new BaseOffer();
                offer->type = NAMECHANGE;
            }
            else if (boost::iequals(type,"sexchange")){
                offer = new BaseOffer();
                offer->type = SEXCHANGE;
            }
            else if(boost::iequals(type,"promotion")){
                offer = new BaseOffer();
                offer->type = PROMOTION;
            }
            else if(boost::iequals(type,"outfit")) {
                OutfitOffer* tmp = new OutfitOffer();
                tmp->type = OUTFIT;
                tmp->maleLookType = (uint16_t)offerNode.attribute("malelooktype").as_uint();
                tmp->femaleLookType = (uint16_t)offerNode.attribute("femalelooktype").as_uint();
                tmp->addonNumber = (uint8_t) offerNode.attribute("addon").as_uint(0);
                if(!tmp->femaleLookType || !tmp->maleLookType || tmp->addonNumber >3){
                    printXMLError("Error parsing XML outfit offer  - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
                    return false;
                }
                else{
                    offer = tmp;
                }
            }
            else if(boost::iequals(type,"addon")){
                OutfitOffer* tmp = new OutfitOffer();
                tmp->type = OUTFIT_ADDON;
                tmp->maleLookType = (uint16_t)offerNode.attribute("malelooktype").as_uint();
                tmp->femaleLookType = (uint16_t)offerNode.attribute("femalelooktype").as_uint();
                tmp->addonNumber = (uint8_t) offerNode.attribute("addon").as_uint(0);
                if(!tmp->femaleLookType || !tmp->maleLookType || !tmp->addonNumber || tmp->addonNumber >3){
                    printXMLError("Error parsing XML addon offer - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
                    return false;
                }
                else{
                    offer =tmp;
                }
            }
            else if(boost::iequals(type,"mount")){
                MountOffer* tmp = new MountOffer();
                tmp->type = MOUNT;
                tmp->mountId = (uint16_t) offerNode.attribute("mountid").as_uint();
                if(!tmp->mountId){
                    printXMLError("Error parsing XML mountID number not specified for an mount offer - GameStore::loadFromXml",
                                  "data/XML/gamestore.xml", result);
                    return false;
                }
                else{
                    offer=tmp;
                }
            }
            else if(boost::iequals(type,"item")){
                ItemOffer* tmp = new ItemOffer();
                tmp->type = ITEM;
                tmp->productId = (uint16_t ) offerNode.attribute("productid").as_uint();
                tmp->count = (uint16_t) offerNode.attribute("count").as_uint();

                if(!tmp->productId || !tmp->count){
                    printXMLError("Error parsing XML Item Offer - GameStore::loadFromXml",
                                  "data/XML/gamestore.xml", result);
                    return false;
                }
                else{
                    offer=tmp;
                }
            }
            else if(boost::iequals(type,"item")){
                ItemOffer* tmp = new ItemOffer();
                tmp->type = STACKABLE_ITEM;
                tmp->productId = (uint16_t ) offerNode.attribute("productid").as_uint();
                tmp->count = (uint16_t) offerNode.attribute("count").as_uint();

                if(!tmp->productId || !tmp->count){
                    printXMLError("Error parsing XML Stackable Item Offer - GameStore::loadFromXml",
                                  "data/XML/gamestore.xml", result);
                    return false;
                }
                else{
                    offer=tmp;
                }
            }

            if(!offer)
            {
                printXMLError("Error parsing XML invalid offer type - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
                return false;
            }
            else{
                offer->name = offerNode.attribute("name").as_string();
                offer->price = offerNode.attribute("price").as_uint();
                offer->description = offerNode.attribute("description").as_string("");
                offer->icons = getIconsVector(offerNode.attribute("icons").as_string("default.png"));

                if( !offer->name.length() || !offer->price)
                {
                    printXMLError("Error parsing XML - One or more required offer params are missing - GameStore::loadFromXml", "data/XML/gamestore.xml", result);
                    return false;
                }
                offerCount++;
                offer->id = offerCount;
                cat->offers.push_back(*offer);
            }
        }
        cat->offers.shrink_to_fit();
        storeOffers.push_back(*cat);
    }
    storeOffers.shrink_to_fit();
    return true;
}
