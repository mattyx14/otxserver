--[[
Items have been updated so that if the offer type is not one of the types: OFFER_TYPE_OUTFIT, OFFER_TYPE_OUTFIT_ADDON,
OFFER_TYPE_MOUNT, OFFER_TYPE_NAMECHANGE, OFFER_TYPE_SEXCHANGE, OFFER_TYPE_PROMOTION, OFFER_TYPE_EXPBOOST,
OFFER_TYPE_PREYSLOT, OFFER_TYPE_PREYBONUS, OFFER_TYPE_TEMPLE, OFFER_TYPE_BLESSINGS, OFFER_TYPE_PREMIUM,
OFFER_TYPE_ALLBLESSINGS
]]

-- Parser
dofile('data/modules/scripts/gamestore/init.lua')
-- Config

HomeBanners = {
    images = { "home/banner_darkkonia.png" , "home/banner_retrooutfits.png" },
    delay = 10
}

GameStore.Categories = {
 {
   icons = { "Category_Consumables.png" },
   name = "Consumables",
   rookgaard = true,
   subclasses = {"Blessings"},
 },
 -- Blessings
 {
   icons = { "Category_Blessings.png" },
   name = "Blessings",
   parent = "Consumables",
   rookgaard = true,
   state = GameStore.States.STATE_NONE,
   offers = {
        {
          icons = { "Heart_of_the_Mountain.png" },
          name = "Heart of the Mountain",
          price = 25,
          blessid = 7,
          count = 1,
          id = 9,
          description = "<i>Reduces your character's chance to lose any items as well as the amount of your character's experience and skill loss upon death:</i>\n\n&#8226; 1 blessing = 8.00% less Skill / XP loss, 30% equipment protection\n&#8226; 2 blessing = 16.00% less Skill / XP loss, 55% equipment protection\n&#8226; 3 blessing = 24.00% less Skill / XP loss, 75% equipment protection\n&#8226; 4 blessing = 32.00% less Skill / XP loss, 90% equipment protection\n&#8226; 5 blessing = 40.00% less Skill / XP loss, 100% equipment protection\n&#8226; 6 blessing = 48.00% less Skill / XP loss, 100% equipment protection\n&#8226; 7 blessing = 56.00% less Skill / XP loss, 100% equipment protection\n\n{character} \n{limit|5} \n{info} added directly to the Record of Blessings \n{info} characters with a red or black skull will always lose all equipment upon death",
          type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS,
        },
        {
          icons = { "Blood_of_the_Mountain.png" },
          name = "Blood of the Mountain",
          price = 25,
          blessid = 8,
          count = 1,
          id = 10,
          description = "<i>Reduces your character's chance to lose any items as well as the amount of your character's experience and skill loss upon death:</i>\n\n&#8226; 1 blessing = 8.00% less Skill / XP loss, 30% equipment protection\n&#8226; 2 blessing = 16.00% less Skill / XP loss, 55% equipment protection\n&#8226; 3 blessing = 24.00% less Skill / XP loss, 75% equipment protection\n&#8226; 4 blessing = 32.00% less Skill / XP loss, 90% equipment protection\n&#8226; 5 blessing = 40.00% less Skill / XP loss, 100% equipment protection\n&#8226; 6 blessing = 48.00% less Skill / XP loss, 100% equipment protection\n&#8226; 7 blessing = 56.00% less Skill / XP loss, 100% equipment protection\n\n{character} \n{limit|5} \n{info} added directly to the Record of Blessings \n{info} characters with a red or black skull will always lose all equipment upon death",
          type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS,
        },
        {
          icons = { "Death_Redemption.png" },
          name = "Death Redemption",
          price = 260,
          blessid = 10,
          count = 1,
          description = "<i>Reduces the penalty of your character's most recent death.</i>\n\n{character}\n{info} can only be used for the most recent death and only within 24 hours after this death",
          type = GameStore.OfferTypes.OFFER_TYPE_BLESSINGS,
        },
      },
 }, 
 -- Cosmetics
 {
   icons = { "Category_Cosmetics.png" },
   name = "Cosmetics",
   rookgaard = true,
   subclasses = {"Mounts", "Outfits"},
 },
  -- Mounts
 { 
   icons = { "Category_Mounts.png" },
   name = "Mounts",
   parent = "Cosmetics", 
   rookgaard = true,
   offers = { 
        {
          icons = { "Savanna_Ostrich.png" },
          name = "Savanna Ostrich",
          price = 500,
          id = 168,
          description = "{character}\n{speedboost}\n\n<i>These birds have a strong maternal instinct since their fledglings are completely dependent on their parents for protection. Do not expect them to abandon their brood only because they are carrying you around. In fact, if you were to separate them from their chick, the Savanna Ostrich, Coral Rhea and Eventide Nandu would turn into vicious beings, so don't even try it!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
        },
        {			
          icons = { "Coral_Rhea.png" },
          name = "Coral Rhea",
          price = 500,
          id = 169,
          description = "{character}\n{speedboost}\n\n<i>These birds have a strong maternal instinct since their fledglings are completely dependent on their parents for protection. Do not expect them to abandon their brood only because they are carrying you around. In fact, if you were to separate them from their chick, the Savanna Ostrich, Coral Rhea and Eventide Nandu would turn into vicious beings, so don't even try it!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
        },
        {			
          icons = { "Eventide_Nandu.png" },
          name = "Eventide Nandu",
          price = 500,
          id = 170,
          description = "{character}\n{speedboost}\n\n<i>These birds have a strong maternal instinct since their fledglings are completely dependent on their parents for protection. Do not expect them to abandon their brood only because they are carrying you around. In fact, if you were to separate them from their chick, the Savanna Ostrich, Coral Rhea and Eventide Nandu would turn into vicious beings, so don't even try it!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
        },
        {
          icons = { "Void_Watcher.png" },
          name = "Void Watcher",
          price = 870,
          id = 179,
          description = "{character}\n{speedboost}\n\n<i>If you are looking for a vigilant and faithful companion, look no further! Glide through every realm and stare into the darkest abyss on the back of a Void Watcher. They already know everything about you anyway for they have been watching you from the shadows!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
          home = true,
        },
        {
          icons = { "Rune_Watcher.png" },
          name = "Rune Watcher",
          price = 870,
          id = 180,
          description = "{character}\n{speedboost}\n\n<i>If you are looking for a vigilant and faithful companion, look no further! Glide through every realm and stare into the darkest abyss on the back of a Rune Watcher. They already know everything about you anyway for they have been watching you from the shadows!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
          home = true,
        },
        {
          icons = { "Rift_Watcher.png" },
          name = "Rift Watcher",
          price = 870,
          id = 181,
          description = "{character}\n{speedboost}\n\n<i>If you are looking for a vigilant and faithful companion, look no further! Glide through every realm and stare into the darkest abyss on the back of a Rift Watcher. They already know everything about you anyway for they have been watching you from the shadows!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_MOUNT,
          home = true,
        },
      },
   rookgaard = true,
   state = GameStore.States.STATE_NONE,
  }, 
 -- Base outfit has addon = 0 or no defined addon. By default addon is set to 0.
 {
   icons = { "Category_Outfits.png" },
   name = "Outfits",
   parent = "Cosmetics", 
   offers = { 
        {
          icons = { "Outfit_Retro_Citizen_Male.png", "Outfit_Retro_Citizen_Female.png" },
          name = "Retro Citizen",
          price = 870,
          sexId = {female = 975,male = 974},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>Do you still remember your first stroll through the streets of Thais? For old times' sake, walk the paths of Nostalgia as a Retro Citizen!</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Hunter_Male.png", "Outfit_Retro_Hunter_Female.png" },
          name = "Retro Hunter",
          price = 870,
          sexId = {female = 973,male = 972},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>Whenever you pick up your bow and spears, you walk down memory lane and think of your early days? Treat yourself with the fashionable Retro Hunter outfit and hunt some good old monsters from your childhood.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Knight_Male.png", "Outfit_Retro_Knight_Female.png" },
          name = "Retro Knight",
          price = 870,
          sexId = {female = 971,male = 970},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>Who needs a fancy looking sword with bling-bling and ornaments? Back in the days, we survived without such unnecessary accessories! Time to show those younkers what a Retro Knight is made of.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Mage_Male.png", "Outfit_Retro_Mage_Female.png" },
          name = "Retro Wizzard",
          price = 870,
          sexId = {female = 969, male = 968},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>Dress up as a Retro Mage and you will always cut a fine figure on the battleground while eliminating your enemies with your magical powers the old-fashioned way.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Nobleman_Male.png", "Outfit_Retro_Nobleman_Female.png" },
          name = "Retro Noblewoman",
          price = 870,
          sexId = { female = 967, male = 966},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>King Tibianus has invited you to a summer ball and you have nothing to wear for this special event? Do not worry, the Retro Noble(wo)man outfit makes you a real eye catcher on every festive occasion.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Summoner_Male.png", "Outfit_Retro_Summoner_Female.png" },
          name = "Retro Summoner",
          price = 870,
          sexId = {female = 965, male = 964},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>While the Retro Mage usually throws runes and mighty spells directly at the enemies, the Retro Summoner outfit might be the better choice for Tibians that prefer to send mighty summons to the battlefield to keep their enemies at distance.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
        {
          icons = { "Outfit_Retro_Warrior_Male.png", "Outfit_Retro_Warrior_Female.png" },
          name = "Retro Warrior",
          price = 870,
          sexId = {female = 963, male = 962},
          description = "{character}\n{info} colours can be changed using the Outfit dialog\n\n<i>You are fearless and strong as a behemoth but have problems finding the right outfit for your adventures? The Retro Warrior outfit is a must-have for all fashion-conscious old-school Tibians out there.</i>",
          type = GameStore.OfferTypes.OFFER_TYPE_OUTFIT,
        },
      },
   rookgaard = true,
   state = GameStore.States.STATE_NONE,
  },
-- Boost
 {   
   icons = { "Category_Boosts.png" },
   name = "Boosts",
   offers = { 
        {
          icons = { "XP_Boost.png" },
          name = "XP Boost",
          price = 30,
          id = 65010,
          description = "<i>Purchase a boost that increases the experience points your character gains from hunting by 50%!</i>\n\n{character}\n{info} lasts for 1 hour hunting time\n{info} paused if stamina falls under 14 hours\n{info} can be purchased up to 5 times between 2 server saves\n{info} price increases with every purchase\n{info} cannot be purchased if an XP boost is already active",
          type = GameStore.OfferTypes.OFFER_TYPE_EXPBOOST,			
        },
      },
   rookgaard = true,
   state = GameStore.States.STATE_NONE,
  },
-- Extras
 {
   icons = { "Category_Extras.png" },
   name = "Extras",
   rookgaard = true,
	 subclasses = {"Extra Services", "Useful Things"},
 }, 
 -- Extras Services
 {   
   icons = { "Category_ExtraServices.png" },
   name = "Extra Services",
   parent = "Extras",
   rookgaard = true,
   state = GameStore.States.STATE_NONE, 
   offers = {       
        {
          icons = { "Name_Change.png" },
          name = "Character Name Change",
          price = 250,
          id = 65002,
          description = "<i>Tired of your current character name? Purchase a new one!</i>\n\n{character}\n{info} relog required after purchase to finalise the name change",
          type = GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE,
        },
        {
          icons = { "Sex_Change.png" },
          name = "Character Sex Change",
          price = 120,
          id = 65003,
          description = "<i>Turns your female character into a male one - or vice versa.</i>\n\n{character}\n{activated}\n{info} you will keep all outfits you have purchased or earned in quest",
          type = GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE,
        },
      },
  },
-- Usefull Things  
  {   
   icons = { "Category_UsefulThings.png" },
   name = "Useful Things",
   parent = "Extras",
   rookgaard = true,
   state = GameStore.States.STATE_NONE,
   offers = { 
        {
          icons = { "Gold_Pouch.png" },
          name = "Gold Pouch",
          price = 900,
          itemtype = 26377,
          count = 1,
          description = "<i>Carries as many gold, platinum or crystal coins as your capacity allows, however, no other items.</i>\n\n{character}\n{storeinbox}\n{once}\n{useicon} use it to open it\n{info} always placed on the first position of your Store inbox",
          type = GameStore.OfferTypes.OFFER_TYPE_POUNCH,
        },
        {                
          icons = { "Charm_Expansion_Offer.png" },
          name = "Charm Expansion",
          price = 450,
          id = 65005,
          description = "<i>Assign as many of your unlocked Charms as you like and get a 25% discount whenever you are removing a Charm from a creature!</i>\n\n{character}\n{once}",
          type = GameStore.OfferTypes.OFFER_TYPE_CHARMS,
        }, 
        {
          icons = { "Permanent_Prey_Slot.png" },
          name = "Permanent Prey Slot",
          price = 900,
          id = 65008,
          description = "<i>Get an additional prey slot to activate additional prey!</i>\n\n{character}\n{info} maximum amount that can be owned by character: 3\n{info} added directly to Prey dialog",
          type = GameStore.OfferTypes.OFFER_TYPE_PREYSLOT,
        },
        {
          icons = { "Prey_Bonus_Reroll.png" },
          name = "Prey Wildcard",
          price = 50,
          id = 1,
          count = 5,
          description = "<i>Use Prey Wildcards to reroll the bonus of an active prey, to lock your active prey or to select a prey of your choice.</i>\n\n{character}\n{info} added directly to Prey dialog\n{info} maximum amount that can be owned by character: 50",
          type = GameStore.OfferTypes.OFFER_TYPE_PREYBONUS,
        },
        {
          icons = { "Prey_Bonus_Reroll.png" },
          name = "Prey Wildcard",
          price = 200,
          count = 20,
          description = "<i>Use Prey Wildcards to reroll the bonus of an active prey, to lock your active prey or to select a prey of your choice.</i>\n\n{character}\n{info} added directly to Prey dialog\n{info} maximum amount that can be owned by character: 50",
          type = GameStore.OfferTypes.OFFER_TYPE_PREYBONUS,
        }, 
	},
 },
}   


-- Each outfit must be uniquely identified to distinguish between addons.
-- Here we dynamically assign ids for outfits. These ids must be unique.
local runningId = 45000
for k, category in ipairs(GameStore.Categories) do
	if category.offers then
		for m, offer in ipairs(category.offers) do
			if not offer.id then
				if type(offer.count) == "table" then
					for i = 1, #offer.price do
						offer.id[i] = runningId
						runningId = runningId + 1
					end
				else
					offer.id = runningId
					runningId = runningId + 1
				end
			end
			if not offer.type then
				offer.type = GameStore.OfferTypes.OFFER_TYPE_NONE
			end
			if not offer.coinType then
				offer.coinType = GameStore.CointType.Coin
			end
		end
	end
end
