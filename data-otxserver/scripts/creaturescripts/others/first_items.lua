local creatureevent = CreatureEvent("FirstItems")

local config = {
	[1] = { -- Sorcerer
		items = {
			{3059, 1}, -- spellbook
			{7991, 1}, -- magician's robe
			{7992, 1}, -- mage hat
			{3362, 1}, -- studded legs
			{3552, 1}, -- leather boots
			{3572, 1}  -- scarf
		},
		container = {
			{3003, 1}, -- rope
			{3457, 1}, -- shovel
			{268, 10}  -- mana potion
		}
	},
	[2] = { -- Druid
		items = {
			{3059, 1}, -- spellbook
			{7991, 1}, -- magician's robe
			{7992, 1}, -- mage hat
			{3362, 1}, -- studded legs
			{3552, 1}, -- leather boots
			{3572, 1}  -- scarf
		},
		container = {
			{3003, 1}, -- rope
			{3457, 1}, -- shovel
			{268, 10}  -- mana potion
		}
	},
	[3] = { -- Paladin
		items = {
			{3425, 1}, -- dwarven shield
			{3277, 5}, -- 5 spears
			{3571, 1}, -- ranger's cloak
			{8095, 1}, -- ranger legs
			{3552, 1}, -- leather boots
			{3572, 1}, -- scarf
			{3354, 1} -- brass helmet
		},
		container = {
			{3003, 1},  -- rope
			{3457, 1},  -- shovel
			{266, 10},  -- health potion
			{3350, 1},  -- bow
			{3447, 50}  -- 50 arrows
		}
	},
	[4] = { -- Knight
		items = {
			{3425, 1}, -- dwarven shield
			{3359, 1}, -- brass armor
			{3354, 1}, -- brass helmet
			{3372, 1}, -- brass legs
			{3552, 1}, -- leather boots
			{3572, 1}  -- scarf
		},
		container = {
			{3003, 1}, -- rope
			{3457, 1}, -- shovel
			{266, 10}  -- health potion
		}
	}
}

function creatureevent.onLogin(player)
	local targetVocation = config[player:getVocation():getId()]
	if not targetVocation then
		return true
	end

	if player:getLastLoginSaved() ~= 0 then
		return true
	end

	for i = 1, #targetVocation.items do
		player:addItem(targetVocation.items[i][1], targetVocation.items[i][2])
	end

	local backpack = player:addItem(2854) -- backpack
	if not backpack then
		return true
	end

	for i = 1, #targetVocation.container do
		backpack:addItem(targetVocation.container[i][1], targetVocation.container[i][2])
	end
	return true
end

creatureevent:register()