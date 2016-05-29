-- Parser
dofile('data/modules/scripts/gamestore/init.lua')

-- Config
GameStore.Categories = {
	{	name = "Mounts",
		icons = {"Category_Mounts.png"},
		offers = {
			{name = "Titanica", thingId = 7, type = GameStore.OfferTypes.OFFER_TYPE_MOUNT, price = 120, icons = {"mounts/Product_NewRumos.png"}, description = "This mount looks so hot!"}
		}
	},
}

--[[ This explanation is not so full
TODO: add full explanation!
]]
-- Explanaion
-- * means required
--[[ Category ------------
	name : category name (string)*
	description : a summary about the category (string)
	icons : table of strings, contains icon names (table->[string])*
	offers : table contains offers (table->[offer])
	state : none|new|sale|timed (GameStore.States)
]]
--[[ Offer ---------------
	name : Offer name (string)*
	type : item|outfit|mount|namechange|promotion (GameStore.OfferTypes)*
	thingId : itemid|mountid|outfitid|0(if namechange)|promotion_number || depend on type (number)*
	price : price of coins (number)*
	state : none|new|sale|timed (GameStore.States)
	disabled : offer is diabled or not (bool)
	icons : table of strings, contains icon names (table->[string])*
	purchaseFunction : function which is applied when player purchase (player[, offer])
]]

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
