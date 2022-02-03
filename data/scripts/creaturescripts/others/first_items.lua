local creatureevent = CreatureEvent("FirstItems")

local config = {
	[1] = { -- Sorcerer
		items = {
			{2175, 1}, -- spellbook
			{8819, 1}, -- magician's robe
			{8820, 1}, -- mage hat
			{2468, 1}, -- studded legs
			{2643, 1}, -- leather boots
			{2661, 1}  -- scarf
		},
		container = {
			{2120, 1}, -- rope
			{2554, 1}, -- shovel
			{7620, 10}  -- mana potion
		}
	},
	[2] = { -- Druid
		items = {
			{2175, 1}, -- spellbook
			{8819, 1}, -- magician's robe
			{8820, 1}, -- mage hat
			{2468, 1}, -- studded legs
			{2643, 1}, -- leather boots
			{2661, 1}  -- scarf
		},
		container = {
			{2120, 1}, -- rope
			{2554, 1}, -- shovel
			{7620, 10}  -- mana potion
		}
	},
	[3] = { -- Paladin
		items = {
			{2525, 1}, -- dwarven shield
			{2389, 5}, -- 5 spears
			{2660, 1}, -- ranger's cloak
			{8923, 1}, -- ranger legs
			{2643, 1}, -- leather boots
			{2661, 1}, -- scarf
			{2480, 1}  -- legion helmet
		},
		container = {
			{2120, 1},  -- rope
			{2554, 1},  -- shovel
			{7618, 10},  -- health potion
			{2456, 1},  -- bow
			{2544, 50}  -- 50 arrows
		}
	},
	[4] = { -- Knight
		items = {
			{2525, 1}, -- dwarven shield
			{2465, 1}, -- brass armor
			{2460, 1}, -- brass helmet
			{2478, 1}, -- brass legs
			{2643, 1}, -- leather boots
			{2661, 1}  -- scarf
		},
		container = {
			{2120, 1}, -- rope
			{2554, 1}, -- shovel
			{7618, 10}  -- health potion
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

	local backpack = player:addItem(1988) -- backpack
	if not backpack then
		return true
	end

	for i = 1, #targetVocation.container do
		backpack:addItem(targetVocation.container[i][1], targetVocation.container[i][2])
	end
	return true
end

creatureevent:register()