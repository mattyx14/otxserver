/**
 * The OTXServer Project - based on TFS
 * Copyright (C) 2017  Mark Samman <mark.samman@gmail.com>

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

#ifndef OTX_GAMESTORE_H_A21098A1C2903794B635DDB0A3E7A914
#define OTX_GAMESTORE_H_A21098A1C2903794B635DDB0A3E7A914

#include "otpch.h"

enum Offer_t {
    DISABLED,
    ITEM,
    STACKABLE_ITEM,
    OUTFIT,
    OUTFIT_ADDON,
    MOUNT,
    NAMECHANGE,
    SEXCHANGE,
    PROMOTION,
    BLESSING,
    BOOST_XP,
    BOOST_STAMINA
};

enum CategoryState_t {
    NORMAL,
    NEW,
    SALE,
    LIMITED_TIME
};

enum GameStoreError_t{
    STORE_ERROR_PURCHASE,
    STORE_ERROR_NETWORK,
    STORE_ERROR_HISTORY,
    STORE_ERROR_TRANSFER,
    STORE_ERROR_INFORMATION
};

enum StoreService_t {
    SERVICE_STANDARD = 0,
    SERVICE_OUTFIT = 3,
    SERVICE_MOUNT = 4
};

struct BaseOffer{
    uint32_t id;
    std::string name;
    std::string description;
    uint32_t price;
    Offer_t type;
    std::vector<std::string> icons;
};

struct ItemOffer : BaseOffer{
    uint16_t productId;
    uint16_t count;
};

struct MountOffer: BaseOffer{
    uint16_t mountId;
};

struct OutfitOffer : BaseOffer {
    uint16_t maleLookType;
    uint16_t femaleLookType;
    uint8_t addonNumber;
};


struct StoreCategory{
    std::string name;
    std::string description;
    CategoryState_t state;
    std::vector<std::string> icons;
    std::vector<BaseOffer> offers;
};

class GameStore {
    public:
        bool reload();
        bool loadFromXml();
        const std::vector<StoreCategory>& getOffers() const{
            return storeOffers;
        };
    
    private:
        uint16_t offerCount=0;
        void instantiateIds();
        std::vector<StoreCategory> storeOffers;
};


#endif //OTX_GAMESTORE_H
