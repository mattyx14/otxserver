-- Parser
dofile('data/modules/scripts/gamestore/init.lua')
-- Config
GameStore.Categories = {
	{
		name = "Sale Products",
		state = GameStore.States.STATE_NEW,
		rookgaard = false,
		icons = {"New_Products.png"},
		offers = {
			{name = "Jackalope", basePrice = 500, price = 375, state = GameStore.States.STATE_SALE, validUntil = 30, thingId = 103, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, icons = {"Product_PremiumTime30.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
		}
	},

	{
		name = "Premium Time",
		state = GameStore.States.STATE_NEW,
		rookgaard = false,
		icons = {"New_Products.png"},
		offers = {
			{name = "30 Days", basePrice = 500, price = 375, state = GameStore.States.STATE_SALE, validUntil = 30, thingId = 30, type = GameStore.OfferTypes.OFFER_TYPE_PREMIUM, icons = {"Product_PremiumTime30.png"}, description = "Premium Account for 30 days."},
			{name = "60 Days", basePrice = 1000, price = 500, state = GameStore.States.STATE_SALE, validUntil = 30, thingId = 60, type = GameStore.OfferTypes.OFFER_TYPE_PREMIUM, icons = {"Product_PremiumTime60.png"}, description = "Premium Account for 60 days."},
			{name = "180 Days", basePrice = 1600, price = 800, state = GameStore.States.STATE_SALE, validUntil = 30, thingId = 180, type = GameStore.OfferTypes.OFFER_TYPE_PREMIUM, icons = {"Product_PremiumTime180.png"}, description = "Premium Account for 180 days."}
		}
	},

	{
		name = "New Products",
		state = GameStore.States.STATE_NEW,
		icons = {"New_Products.png"},
		rookgaard = false,
		offers = {
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Grove Keeper", thingId = {male=908,female=909}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f909.png", "f908.png"}},
			{name = "Festive Outfit", thingId = {male=931,female=929}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 40, icons = {"f931.png", "f929.png"}},
			{name = "Pharaoh", thingId = {male=955,female=956}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f955.png", "f956.png"}},

			-- Mount Example : thingId = mountId
			{name = "Jackalope", thingId = 103, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o905.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Dreadhare", thingId = 104, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o906.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Wolpertinger", thingId = 105, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o907.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Stone Rhino", thingId = 106, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o937.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Gold Sphinx", thingId = 107, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o950.png"}, description = "Here you can purchase the Mount for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Emerald Sphinx", thingId = 108, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o951.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			{name = "Shadow Sphinx", thingId = 109, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o952.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
		}
	},

	{
		name = "Overcharged items",
		state = GameStore.States.STATE_NEW,
		rookgaard = false,
		icons = {"New_Products.png"},
		offers = {
			-- Item Example : thingId = itemId
			{name = "Blade of Carving Overcharged", thingId = 26270, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25192.png"}, description = "Sword Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Blade of Mayhem Overcharged", thingId = 26235, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25193.png"}, description = "Sword Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Blade of Remedy Overcharged", thingId = 26256, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25191.png"}, description = "Sword Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Slayer of Carving Overcharged", thingId = 26277, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25190.png"}, description = "Sword Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Slayer of Mayhem Overcharged", thingId = 26238, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18390.png"}, description = "Sword Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Slayer of Remedy Overcharged", thingId = 26260, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"8918.png"}, description = "Sword Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Axe of Carving Overcharged", thingId = 26284, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Axe of Mayhem Overcharged", thingId = 26241, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Axe of Remedy Overcharged", thingId = 26267, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Chopper of Carving Overcharged", thingId = 26293, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Chopper of Mayhem Overcharged", thingId = 26244, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Chopper of Remedy Overcharged", thingId = 26273, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Axe Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Hammer of Carving Overcharged", thingId = 26305, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Hammer of Mayhem Overcharged", thingId = 26250, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Hammer of Remedy Overcharged", thingId = 26283, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Mace of Carving Overcharged", thingId = 26300, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Mace of Mayhem Overcharged", thingId = 26247, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Mace of Remedy Overcharged", thingId = 26278, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Club Fighting +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Rod of Carving Overcharged", thingId = 26317, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Magic Level +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Rod of Mayhem Overcharged", thingId = 26323, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "	Magic Level +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Rod of Remedy Overcharged", thingId = 26302, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Magic Level +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Wand of Carving Overcharged", thingId = 26314, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Magic Level +1, Mana Leech chance +100%, mana leech amount +9%"},
			{name = "Wand of Mayhem Overcharged", thingId = 26320, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "	Magic Level +1, Critical Hit chance +10%, critical extra damage +60%"},
			{name = "Wand of Remedy Overcharged", thingId = 26297, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Magic Level +1, Hit Points Leech chance +100%, hit points leech amount +9%"},
			{name = "Bow of Carving Overcharged", thingId = 26308, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			{name = "Bow of Mayhem Overcharged", thingId = 26254, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			{name = "Bow of Remedy Overcharged", thingId = 26288, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			{name = "Crossbow of Carving Overcharged", thingId = 26311, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			{name = "Crossbow of Mayhem Overcharged", thingId = 26268, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			{name = "Crossbow of Remedy Overcharged", thingId = 26292, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
			
		
		}
	},

	{
		name = "Mounts",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Mounts.png"},
		rookgaard = false,
		offers = {
			-- Mount Example : thingId = mountId
			{name = "Widow Queen", thingId = 1, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o368.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Racing Bird", thingId = 2, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o369.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "War Bear", thingId = 3, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o370.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Black Sheep", thingId = 4, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o371.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Midnight Panther", thingId = 5, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o372.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Draptor", thingId = 6, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o373.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Titanica", thingId = 7, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o374.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tin Lizzard", thingId = 8, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o375.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "Blazebringer", thingId = 9, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o376.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Rapid Boar", thingId = 10, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o377.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "Stampor", thingId = 11, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o378.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Undead Cavebear", thingId = 12, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o379.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Donkey", thingId = 13, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o387.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tiger Slug", thingId = 14, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o388.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Uniwheel", thingId = 15, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o389.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Crystal Wolf", thingId = 16, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o390.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "War Horse", thingId = 17, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o392.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Kingly Deer", thingId = 18, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o401.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tamed Panda", thingId = 19, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o402.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Dromedary", thingId = 20, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o405.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Scorpion King", thingId = 21, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o406.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Rented Horse", thingId = 22, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o421.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Armoured War Horse", thingId = 23, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o426.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Shadow Draptor", thingId = 24, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o427.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Rented Horse", thingId = 25, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o437.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Rented Horse", thingId = 26, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o438.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Ladybug", thingId = 27, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o447.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Manta Ray", thingId = 28, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o450.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Ironblight", thingId = 29, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o502.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Magma Crawler", thingId = 30, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o503.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Dragonling", thingId = 31, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o506.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "Gnarlhound", thingId = 32, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 530, icons = {"o515.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Crimson Ray", thingId = 33, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o521.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Steelbeak", thingId = 34, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o522.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Water Buffalo", thingId = 35, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 30, icons = {"o526.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tombstinger", thingId = 36, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o546.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Platesaurian", thingId = 37, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o547.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Ursagrondon", thingId = 38, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 560, icons = {"o548.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "The Hellgrip", thingId = 39, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o559.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Noble Lion", thingId = 40, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o571.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Desert King", thingId = 41, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o572.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Shock Head", thingId = 42, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o580.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Walker", thingId = 43, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o606.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Azudocus", thingId = 44, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o621.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Carpacosaurus", thingId = 45, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o622.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Death Crawler", thingId = 46, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o624.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Flamesteed", thingId = 47, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o626.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Jade Lion", thingId = 48, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o627.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Jade Pincer", thingId = 49, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o628.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Nethersteed", thingId = 50, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o629.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tempest", thingId = 51, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o630.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Winter King", thingId = 52, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o631.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Doombringer", thingId = 53, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o644.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Woodland Prince", thingId = 54, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o647.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Hailtorm Fury", thingId = 55, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o648.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Siegebreaker", thingId = 56, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o649.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Poisonbane", thingId = 57, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o650.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Blackpelt", thingId = 58, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o651.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Golden Dragonfly", thingId = 59, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o669.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Steel Bee", thingId = 60, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o670.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Copper Fly", thingId = 61, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o671.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Tundra Rambler", thingId = 62, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o672.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Highland Yak", thingId = 63, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o673.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Glacier Vagabond", thingId = 64, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o674.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Flying Divan", thingId = 65, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o688.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Magic Carpet", thingId = 66, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o689.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Floating Kashmir", thingId = 67, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o690.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Ringtail Waccoon", thingId = 68, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o691.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Night Waccoon", thingId = 69, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o692.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Emerald Waccoon", thingId = 70, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o693.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "Glooth Glider", thingId = 71, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o682.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Shadow Hart", thingId = 72, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o685.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Black Stag", thingId = 73, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o686.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Emperor Deer", thingId = 74, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o687.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Flitterkatzen", thingId = 75, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o726.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Venompaw", thingId = 76, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o727.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Batcat", thingId = 77, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o728.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Sea Devil", thingId = 78, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o734.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Coralripper", thingId = 79, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o735.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Plumfish", thingId = 80, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o736.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Gorongra", thingId = 81, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o738.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Noctungra", thingId = 82, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o739.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Silverneck", thingId = 83, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o740.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Slagsnare", thingId = 84, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o761.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Nightstinger", thingId = 85, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o762.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Razorcreep", thingId = 86, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o763.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			--{name = "Rift Runner", thingId = 87, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o848.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Nightdweller", thingId = 88, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o849.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Frostflare", thingId = 89, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o850.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Cinderhoof", thingId = 90, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o851.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Mouldpincer", thingId = 91, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o868.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Bloodcurl", thingId = 92, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o869.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Leafscuttler", thingId = 93, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o870.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Sparkion", thingId = 94, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o883.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Swamp Snapper", thingId = 95, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o886.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Mould Shell", thingId = 96, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o887.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Reed Lurker", thingId = 97, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o888.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Neon Sparkid", thingId = 98, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o889.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Vortexion", thingId = 99, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o890.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Ivory Fang", thingId = 100, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o901.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Shadow Claw", thingId = 101, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o902.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
			-- Mount Example : thingId = mountId
			{name = "Snow Pelt", thingId = 102, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 60, icons = {"o903.png"}, description = "Here you can purchase the Mount  for your character. Riding on a mount is not only cool, but also gives your character a speed boost."},
		}
	},

	{
		name = "Outfits",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Outfits.png"},
		rookgaard = false,
		offers = {
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Citizen Addon", thingId = {male=128,female=136}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f128.png", "f136.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Hunter Addon", thingId = {male=129,female=137}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f129.png", "f137.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Mage Addon", thingId = {male=130,female=138}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 100, icons = {"f130.png", "f138.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Knight Addon", thingId = {male=131,female=139}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f131.png", "f139.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Nobleman Addon", thingId = {male=132,female=140}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 5, icons = {"f132.png", "f140.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Summoner Addon", thingId = {male=133,female=141}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 100, icons = {"f133.png", "f141.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Warrior Addon", thingId = {male=134,female=142}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f134.png", "f142.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Barbarian Addon", thingId = {male=143,female=147}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 50, icons = {"f143.png", "f147.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Druid Addon", thingId = {male=144,female=148}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f144.png", "f148.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Wizard Addon", thingId = {male=145,female=149}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f145.png", "f149.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "oriental Addon", thingId = {male=146,female=150}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f146.png", "f150.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Pirate Addon", thingId = {male=151,female=155}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f151.png", "f155.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			-- Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Shaman Addon", thingId = {male=154,female=158}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f154.png", "f158.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Norseman Addon", thingId = {male=251,female=252}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 10, icons = {"f251.png", "f252.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Nightmare Addon", thingId = {male=268,female=269}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 10, icons = {"f268.png", "f269.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Jester Addon", thingId = {male=273,female=270}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 5, icons = {"f273.png", "f270.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Brotherhood Addon", thingId = {male=278,female=279}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 10, icons = {"f278.png", "f279.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Demonhunter Addon", thingId = {male=289,female=288}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 10, icons = {"f289.png", "f288.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Yalaharian Addon", thingId = {male=325,female=324}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 5, icons = {"f325.png", "f324.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Newly Wed", thingId = {male=328,female=329}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 30, icons = {"f328.png", "f329.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Warmaster Addon", thingId = {male=335,female=336}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 30, icons = {"f335.png", "f336.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Wayfarer Addon", thingId = {male=367,female=366}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f367.png", "f366.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Afflicted Addon", thingId = {male=430,female=431}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 5, icons = {"f430.png", "f431.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Elementalist Addon", thingId = {male=432,female=433}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 80, icons = {"f432.png", "f433.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Deepling Addon", thingId = {male=463,female=464}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f463.png", "f464.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Insectoid Addon", thingId = {male=465,female=466}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 30, icons = {"f465.png", "f466.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Entrepreneur Addon", thingId = {male=472,female=471}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f472.png", "f471.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Crystal Warlord Addon", thingId = {male=512,female=513}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f512.png", "f513.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Soil Guardian Addon", thingId = {male=516,female=514}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f516.png", "f514.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Demon Addon", thingId = {male=541,female=542}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f541.png", "f542.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Cave Explorer Addon", thingId = {male=544,female=575}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 20, icons = {"f544.png", "f575.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Dream Warden Addon", thingId = {male=577,female=578}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f577.png", "f578.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Glooth Engineer Addon", thingId = {male=610,female=618}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f610.png", "f618.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Jersey", thingId = {male=619,female=620}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f619.png", "f620.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Champion Addon", thingId = {male=633,female=632}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f633.png", "f632.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Conjurer Addon", thingId = {male=634,female=635}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f634.png", "f635.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Beastmaster Addon", thingId = {male=637,female=636}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f637.png", "f636.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Chaos Acolyte Addon", thingId = {male=665,female=664}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f665.png", "f664.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Death Herald Addon", thingId = {male=667,female=666}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f667.png", "f666.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Ranger Addon", thingId = {male=684,female=683}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f684.png", "f683.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Ceremonial Garb Addon", thingId = {male=695,female=694}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f695.png", "f694.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Puppeteer Addon", thingId = {male=697,female=696}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f697.png", "f696.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Spirit Caller Addon", thingId = {male=699,female=698}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f699.png", "f698.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Evoker Addon", thingId = {male=725,female=724}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f725.png", "f724.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Seaweaver Addon", thingId = {male=733,female=732}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f733.png", "f732.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Recruiter Addon", thingId = {male=746,female=745}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f746.png", "f745.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Sea Dog Addon", thingId = {male=750,female=749}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f750.png", "f749.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Royal Pumpkin Addon", thingId = {male=760,female=759}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f760.png", "f759.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			--{name = "Rift Warrior Addon", thingId = {male=846,female=845}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f846.png", "f845.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Winter Warden Addon", thingId = {male=853,female=852}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f853.png", "f852.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Philosopher Addon", thingId = {male=874,female=873}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f874.png", "f873.png"}},
			-- Addon Example : thingId = lookType, addon = ( 1 = addon 1, 2 = addon 2, 3 = both addons)
			{name = "Arena Champion Addon", thingId = {male=884,female=885}, addon = 3, type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON, price = 60, icons = {"f884.png", "f885.png"}},
		}
	},

	{
		name = "Items",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Items.png"},
		rookgaard = false,
		offers = {
			{name = "crystalline token", thingId = 18423, count = 5, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"18423.png"}, description = "Become rich!"},
			-- Item Example : thingId = itemId
			{name = "Yalahari Mask", thingId = 9778, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Yalahari_Mask.png"}, description = "Become rich!"},
			{name = "Golden Helmet", thingId = 2471, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 100, icons = {"Golden_Helmet.png"}, description = "Become rich!"},
			{name = "Prismatic Helmet", thingId = 18403, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Prismatic_Helmet.png"}, description = "Become rich!"},
			{name = "Gill Gugel", thingId = 18398, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Gill_Gugel.png"}, description = "Become rich!"},
			{name = "Elite Draken Helmet", thingId = 12645, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Elite_Draken_Helmet.png"}, description = "Become rich!"},
			{name = "Werewolf Helmet", thingId = 24718, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Werewolf_Helmet.png"}, description = "Become rich!"},
			{name = "Depth Galea", thingId = 15408, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Depth_Galea.png"}, description = "Become rich!"},
			--Armorid--
			{name = "Demon Armor", thingId = 2494, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Demon_Armor.png"}, description = "Become rich!"},
			{name = "Fire Born Giant Armor", thingId = 8881, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 15, icons = {"Fireborn_Giant_Armor.png"}, description = "Become rich!"},
			{name = "Royal Draken Mail", thingId = 12642, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 15, icons = {"Royal_Draken_Mail.png"}, description = "Become rich!"},
			{name = "Gill Coat", thingId = 18399, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Gill_Coat.png"}, description = "Become rich!"},
			{name = "Depth Lorica", thingId = 15407, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 30, icons = {"Depth_Lorica.png"}, description = "Become rich!"},
			{name = "Furios Frock", thingId = 21725, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Furious_Frock.png"}, description = "Become rich!"},
			{name = "Ornate Chestplate", thingId = 15406, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Ornate_Chestplate.png"}, description = "Become rich!"},
			{name = "Earthborn Titan Armor", thingId = 8882, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Earthborn_Titan_Armor.png"}, description = "Become rich!"},
			{name = "Wind Born Colossus Armor", thingId = 8883, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Windborn_Colossus_Armor.png"}, description = "Become rich!"},
			{name = "Master Acher's Armor", thingId = 8888, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 15, icons = {"Master_Archer's_Armor.png"}, description = "Become rich!"},
			--Legsid--
			{name = "Demon Legs", thingId = 2495, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 30, icons = {"Demon_Legs.png"}, description = "Become rich!"},
			{name = "Grasshopper Legs", thingId = 15490, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Grasshopper_Legs.png"}, description = "Become rich!"},
			{name = "Depth Ocrea", thingId = 15409, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Depth_Ocrea.png"}, description = "Become rich!"},
			{name = "Primatic Legs", thingId = 18405, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Prismatic_Legs.png"}, description = "Become rich!"},
			{name = "Gill Legs", thingId = 18400, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Gill_Legs.png"}, description = "Become rich!"},
			{name = "Ornate Legs", thingId = 15412, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Ornate_Legs.png"}, description = "Become rich!"},
			{name = "Dwarven Legs", thingId = 2504, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Dwarven_Legs.png"}, description = "Become rich!"},
			{name = "Icy Culotte", thingId = 21700, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Icy_Culottes.png"}, description = "Become rich!"},
			--Bootsid--
			{name = "Boots of Haste", thingId = 2195, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 3, icons = {"Boots_of_Haste.png"}, description = "Become rich!"},
			{name = "Golden Boots", thingId = 2646, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 50, icons = {"Golden_Boots.png"}, description = "Become rich!"},
			{name = "Pair Soft Boots", thingId = 2640, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 15, icons = {"Pair_of_Soft_Boots.png"}, description = "Become rich!"},
			--Shieldid--
			{name = "Depth Scutum", thingId = 15411, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 3, icons = {"Depth_Scutum.png"}, description = "Become rich!"},
			{name = "Shield of Corruption", thingId = 12644, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 30, icons = {"Shield_of_Corruption.png"}, description = "Become rich!"},
			{name = "Prismatic Shield", thingId = 18410, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Prismatic_Shield.png"}, description = "Become rich!"},
			{name = "Ornate Shield", thingId = 15413, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"Ornate_Shield.png"}, description = "Become rich!"},
			{name = "Spelbook of Vigilance", thingId = 18401, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"Spellbook_of_Vigilance.png"}, description = "Become rich!"},
			{name = "Spellbook of Ancient Arcana", thingId = 16112, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 30, icons = {"Spellbook_of_Ancient_Arcana.png"}, description = "Become rich!"},
			{name = "Umbral spellbook", thingId = 22423, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 15, icons = {"Umbral_Spellbook.png"}, description = "Become rich!"},
			{name = "Umbral Master Spellbook", thingId = 22424, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"Umbral_Master_Spellbook.png"}, description = "Become rich!"},
			{name = "Great Shield", thingId = 2522, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 80, icons = {"Great_Shield.png"}, description = "Become rich!"},
			{name = "Blessed Shield", thingId = 2523, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 100, icons = {"Blessed_Shield.png"}, description = "Become rich!"},
			{name = "Gold Pounch", state = GameStore.States.STATE_NEW, thingId = 26377, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 450, icons = {"Product_MagicCoinPurse.png"}, description="This item can't be moved from Store Inbox.\nYou can only put inside money.\nWarning: this item drop when you die red or black skull!"},
			{name = "Blood Herb", thingId = 2798, count = 10, type = GameStore.OfferTypes.OFFER_TYPE_STACKABLE, price = 30, icons = {"BloodHerb.png"}, description = "Become rich!"}
		}
	},

	{
		name = "Items for Sorcerers",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Sorc.png"},
		offers = {
			-- Item Example : thingId = itemId
			{name = "Thundermind Raiment", thingId = 25192, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25192.png"}, description = "Become rich!"},
			{name = "Frostmind Raiment", thingId = 25193, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25193.png"}, description = "Become rich!"},
			{name = "Earthmind Raiment", thingId = 25191, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25191.png"}, description = "Become rich!"},
			{name = "Firemind Raiment", thingId = 25190, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25190.png"}, description = "Become rich!"},
			{name = "Wand of Defiance", thingId = 18390, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18390.png"}, description = "Become rich!"},
			{name = "Spelbook of Dark Mysteries", thingId = 8918, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"8918.png"}, description = "Become rich!"},
			{name = "Wand of Everblazing", thingId = 18409, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18409.png"}, description = "Become rich!"},
		}
	},

	{
		name = "Items for Knights",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Knight.png"},
		rookgaard = false,
		offers = {
			-- Item Example : thingId = itemId
			{name = "Earthmind Raiment", thingId = 25191, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25191.png"}, description = "Become rich!"},
			{name = "Earthheart Platemail", thingId = 25179, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25179.png"}, description = "Become rich!"},
			{name = "Earthheart Hauberk", thingId = 25178, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25178.png"}, description = "Become rich!"},
			{name = "Frostheart Platemail", thingId = 25185, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25185.png"}, description = "Become rich!"},
			{name = "Frostheart Hauberk", thingId = 25184, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25184.png"}, description = "Become rich!"},
			{name = "Frostheart Cuirass", thingId = 25183, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25183.png"}, description = "Become rich!"},
			{name = "Thunderheart Platemail", thingId = 25182, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25182.png"}, description = "Become rich!"},
			{name = "Thunderheart Hauberk", thingId = 25181, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25181.png"}, description = "Become rich!"},
			{name = "Thunderheart Cuirass", thingId = 25180, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25180.png"}, description = "Become rich!"},
			{name = "Fireheart Platemail", thingId = 25176, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25176.png"}, description = "Become rich!"},
			{name = "Fireheart Hauberk", thingId = 25175, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25175.png"}, description = "Become rich!"},
			{name = "Fireheart Cuirass", thingId = 25174, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25174.png"}, description = "Become rich!"},
			{name = "Shiny Blade", thingId = 18465, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 20, icons = {"18465.png"}, description = "Become rich!"},
			{name = "Crystaline Axe ", thingId = 18451, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 20, icons = {"18451.png"}, description = "Become rich!"},
			{name = "Mycological Mace", thingId = 18452, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 20, icons = {"18452.png"}, description = "Become rich!"},
			{name = "Umbral Blade", thingId = 22399, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22399.png"}, description = "Become rich!"},
			{name = "Umbral Masterblade", thingId = 22400, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22400.png"}, description = "Become rich!"},
			{name = "umbral Slayer", thingId = 22402, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22402.png"}, description = "Become rich!"},
			{name = "Umbral Hammer", thingId = 22414, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22414.png"}, description = "Become rich!"},
			{name = "Umbral Master Hammer", thingId = 22415, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22415.png"}, description = "Become rich!"},
			{name = "Umbral Mace", thingId = 22411, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22411.png"}, description = "Become rich!"},
			{name = "Umbral master Chopper", thingId = 22409, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22409.png"}, description = "Become rich!"},
			{name = "Umbral Mater Axe", thingId = 22406, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22406.png"}, description = "Become rich!"},
			{name = "Umbral Axe", thingId = 22405, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22405.png"}, description = "Become rich!"},
			{name = "Umbral Master Slayer", thingId = 22403, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22403.png"}, description = "Become rich!"},
			{name = "Umbral Chopper", thingId = 22408, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22408.png"}, description = "Become rich!"},
		
		}
	},

	{
		name = "Items for Paladins",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Pally.png"},
		rookgaard = false,
		offers = {
			-- Item Example : thingId = itemId
			{name = "Firesoul Tabard", thingId = 25186, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25186.png"}, description = "Become rich!"},
			{name = "Frostsoul Tabard", thingId = 25189, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25189.png"}, description = "Become rich!"},
			{name = "Thundersoul Tabard", thingId = 25188, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25188.png"}, description = "Become rich!"},
			{name = "Earthsoul Tabard", thingId = 25187, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25187.png"}, description = "Become rich!"},
			{name = "Crystal Crossbow", thingId = 18453, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 10, icons = {"18453.png"}, description = "Become rich!"},
			{name = "Mycological Bow", thingId = 18454, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 20, icons = {"18454.png"}, description = "Become rich!"},
			{name = "Umbral bow", thingId = 22417, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22417.png"}, description = "Become rich!"},
			{name = "Umbral Master bow", thingId = 22418, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22418.png"}, description = "Become rich!"},
			{name = "Umbral Crossbow", thingId = 22420, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 25, icons = {"22420.png"}, description = "Become rich!"},
			{name = "Umbral Master Crossbow", thingId = 22421, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 60, icons = {"22421.png"}, description = "Become rich!"},
		}
	},

	{
		name = "Items for Druids",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_Druid.png"},
		rookgaard = false,
		offers = {
			-- Item Example : thingId = itemId
			{name = "Thundermind Raiment", thingId = 25192, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25192.png"}, description = "Become rich!"},
			{name = "Frostmind Raiment", thingId = 25193, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25193.png"}, description = "Become rich!"},
			{name = "Earthmind Raiment", thingId = 25191, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25191.png"}, description = "Become rich!"},
			{name = "Firemind Raiment", thingId = 25190, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 90, icons = {"25190.png"}, description = "Become rich!"},
			{name = "Glacial Rod", thingId = 18412, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18412.png"}, description = "Become rich!"},
			{name = "Muck Rod", thingId = 18411, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"18411.png"}, description = "Become rich!"},
			{name = "Spelbook of Dark Mysteries", thingId = 8918, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_ITEM, price = 5, icons = {"8918.png"}, description = "Become rich!"},
		}
	},

	{
		name = "House Equipment",
		state = GameStore.States.STATE_NEW,
		icons = {"Category_House_Equipment.png"},
		rookgaard = false,
		offers = {
			-- Item Example : thingId = itemId
			{name = "Parrot", thingId = 26890, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_STACKABLE, price = 180, icons = {"26890.png"}, description = "Become rich!"},
			{name = "Magnificent Cabinet", thingId = 26075, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 50, icons = {"MagnificantCabinet.png"}, description = "Become rich!"},
			{name = "Magnificent Chair", thingId = 26062, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 30, icons = {"Product_HouseEquipment_BaroqueFurniture_Chair.png"}, description = "Become rich!"},
			{name = "Magnificent Trunk", thingId = 26086, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 35, icons = {"Product_HouseEquipment_BaroqueFurniture_Chest.png"}, description = "Become rich!"},
			{name = "Magnificent Table", thingId = 26074, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 30, icons = {"Product_HouseEquipment_BaroqueFurniture_Table.png"}, description = "Become rich!"},
			{name = "Ferocious Cabinet", thingId = 26078, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 55, icons = {"Product_HouseEquipment_TortureChamberFurniture_Cabinet.png"}, description = "Become rich!"},
			{name = "Ferocious Chair", thingId = 26066, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 100, icons = {"Product_HouseEquipment_TortureChamberFurniture_Chair.png"}, description = "Become rich!"},
			{name = "Ferocious Trunk", thingId = 26082, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 40, icons = {"Product_HouseEquipment_TortureChamberFurniture_Chest.png"}, description = "Become rich!"},
			{name = "Ferocious Table", thingId = 26070, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 25, icons = {"Product_HouseEquipment_TortureChamberFurniture_Table.png"}, description = "Become rich!"},
			{name = "Rustic Cabinet", thingId = 26357, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 50, icons = {"Product_HouseEquipment_RusticFurniture_Cabinet.png"}, description = "Become rich!"},
			{name = "Rustic Chair", thingId = 26352, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 25, icons = {"Product_HouseEquipment_RusticFurniture_Chair.png"}, description = "Become rich!"},
			{name = "Rustic Trunk", thingId = 26362, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 40, icons = {"Product_HouseEquipment_RusticFurniture_Chest.png"}, description = "Become rich!"},
			{name = "Rustic Table", thingId = 26355, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 25, icons = {"Product_HouseEquipment_RusticFurniture_Table.png"}, description = "Become rich!"},
			{name = "Yalaharian Carpet", thingId = 26109, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 17, icons = {"Product_HouseEquipment_Carpet1.png"}, description = "Become rich!"},
			{name = "White Fur Carpet", thingId = 26110, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 15, icons = {"Product_HouseEquipment_Carpet2.png"}, description = "Become rich!"},
			{name = "Bamboo Mat", thingId = 26111, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 12, icons = {"Product_HouseEquipment_Carpet3.png"}, description = "Become rich!"},
			{name = "Crimson Carpet", thingId = 26371, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 17, icons = {"Product_HouseEquipment_Carpet_04.png"}, description = "Become rich!"},
			{name = "Azure Carpet", thingId = 26372, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 17, icons = {"Product_HouseEquipment_Carpet_05.png"}, description = "Become rich!"},
			{name = "Emerald Carpet", thingId = 26373, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 17, icons = {"Product_HouseEquipment_Carpet_06.png"}, description = "Become rich!"},
			{name = "Light Parquet", thingId = 26374, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 15, icons = {"Product_HouseEquipment_Carpet_07.png"}, description = "Become rich!"},
			{name = "Dark Parquet", thingId = 26375, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 15, icons = {"Product_HouseEquipment_Carpet_08.png"}, description = "Become rich!"},
			{name = "Marble Parquet", thingId = 26376, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 15, icons = {"Product_HouseEquipment_Carpet_09.png"}, description = "Become rich!"},
			{name = "Fish Tank", thingId = 26347, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 90, icons = {"Product_HouseEquipment_Housepet_FishTank.png"}, description = "Become rich!"},
			{name = "Dog House", thingId = 26353, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 75, icons = {"Product_HouseEquipment_Housepet_DogHouse.png"}, description = "Become rich!"},
			{name = "Baby Dragon", thingId = 26099, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 125, icons = {"Product_HouseEquipment_Housepet_BabyDragon.png"}, description = "Become rich!"},
			{name = "Cat in a Basket", thingId = 26108, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 75, icons = {"Product_HouseEquipment_Housepet_Cat.png"}, description = "Become rich!"},
			{name = "Hamster in a Wheel", thingId = 26101, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 90, icons = {"Product_HouseEquipment_Housepet_Hamster.png"}, description = "Become rich!"},
			{name = "Protectress Lamp", thingId = 26097, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 45, icons = {"Product_HouseEquipment_Lamp_Goddess.png"}, description = "Become rich!"},
			{name = "Predator Lamp", thingId = 26093, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 30, icons = {"Product_HouseEquipment_Lamp_Leopard.png"}, description = "Become rich!"},
			{name = "Ornate Mailbox", thingId = 26058, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 100, icons = {"Product_HouseEquipment_Mailbox_Golden.png"}, description = "Become rich!"},
			{name = "Royal Mailbox", thingId = 26056, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 75, icons = {"Product_HouseEquipment_Mailbox_Standard.png"}, description = "Become rich!"},
			{name = "Lordly Tapestry", thingId = 26104, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 25, icons = {"Product_HouseEquipment_Tapestry_01.png"}, description = "Become rich!"},
			{name = "Menacing Tapestry", thingId = 26105, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 35, icons = {"Product_HouseEquipment_Tapestry_02.png"}, description = "Become rich!"},
			{name = "AllSeeing Tapestry", thingId = 26106, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 30, icons = {"Product_HouseEquipment_Tapestry_03.png"}, description = "Become rich!"},
			{name = "Golden Dragon Tapestry", thingId = 26379, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 35, icons = {"Product_HouseEquipment_Tapestry_04.png"}, description = "Become rich!"},
			{name = "Sword Tapestry", thingId = 26380, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 30, icons = {"Product_HouseEquipment_Tapestry_05.png"}, description = "Become rich!"},
			{name = "Brocade Tapestry", thingId = 26381, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_HOUSE, price = 25, icons = {"Product_HouseEquipment_Tapestry_06.png"}, description = "Become rich!"},
			}
	},
	{
		name = "Blessings",
		state = GameStore.States.STATE_NEW,
		icons = {"Category_Blessings.png"},
		rookgaard = false,
		offers = {
			-- NameChange example
			{name = "Twist of Fate", thingId = 1, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "The Wisdom of Solitude", thingId = 2, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "The Spark of the Phoenix", thingId = 3, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 20, icons = {"Product_Wisdom.png"}},
			{name = "The Fire of the Suns", thingId = 4, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "The Spiritual Shielding", thingId = 5, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "The Embrace of Tibia", thingId = 6, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "The Heart of the Mountain", thingId = 7, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			{name = "Blood of the Mountain", thingId = 8, type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS, price = 15, icons = {"Product_Wisdom.png"}},
			-- Promotion example
			--{name = "First Promotion", thingId = 1--[[ed/ms/rp/ek]], type = GameStore.OfferTypes.OFFER_TYPE_PROMOTION, price = 100, icons = {"Product_FirstPromotion.png"}}
		}
	},
	{
		name = "Extra Services",
		state = GameStore.States.STATE_NONE,
		icons = {"Category_ExtraServices.png"},
		rookgaard = false,
		offers = {
			-- NameChange example
			{name = "Character Name Change", type = GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE, price = 20, icons = {"Product_CharacterNameChange.png"}},
			-- Sexchange example
			{name = "Character Sex Change", type = GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE, price = 20, icons = {"Product_CharacterSexChange.png"}},
			-- Services Pay to win example
			{name = "XP Boost 50%", state = GameStore.States.STATE_NEW, type = GameStore.OfferTypes.OFFER_TYPE_EXPBOOST, price = 15, icons = {"xpboost.png"}, description="50% XP Boost for 1 hour!"},
			{name = "Prey Bonus Reroll", state = GameStore.States.STATE_NEW, count = 1, type = GameStore.OfferTypes.OFFER_TYPE_PREYBONUS, price = 5, icons = {"Product_UsefulThings_PreyBonusReroll.png"}},
			{name = "5x Prey Bonus Reroll", state = GameStore.States.STATE_NEW, count = 5, type = GameStore.OfferTypes.OFFER_TYPE_PREYBONUS, price = 25, icons = {"Product_UsefulThings_PreyBonusReroll.png"}},
			{name = "Permanent Prey Slot", state = GameStore.States.STATE_NEW, type = GameStore.OfferTypes.OFFER_TYPE_PREYSLOT, price = 450, icons = {"Product_UsefulThings_PermanentPreySlot.png"}},
			{name = "Temple Teleport", type = GameStore.OfferTypes.OFFER_TYPE_TEMPLE, price = 25, icons = {"Product_Transportation_TempleTeleport.png"}},
			-- Promotion example
			--{name = "First Promotion", thingId = 1--[[ed/ms/rp/ek]], type = GameStore.OfferTypes.OFFER_TYPE_PROMOTION, price = 100, icons = {"Product_FirstPromotion.png"}}
		}
	},
}

-- For Explanation and information
-- view the readme.md file in github or via markdown viewer.

-- Non-Editable
local runningId = 1
for k, category in ipairs(GameStore.Categories) do
	if category.offers then
		for m, offer in ipairs(category.offers) do
			offer.id = runningId
			runningId = runningId + 1
			
			if not offer.type then
				offer.type = GameStore.OfferTypes.OFFER_TYPE_NONE
			end
		end
	end
end
