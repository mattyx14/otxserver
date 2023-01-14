local lionsRockSanctuaryPos = Position(33073, 32300, 9)
local lionsRockSanctuaryRockId = 1852
local lionsRockSanctuaryFountainId = 6389

local shrine = {
	-- ice shrine
	[3029] = {
		targetAction = 15001,
		-- shrinePosition = {x = 32194, y = 31418, z = 2}, -- read-only
		destination = {x = 33430, y = 32278, z = 7},
		effect = CONST_ME_ICEATTACK
	},
	-- fire shrine
	[3030] = {
		targetAction = 15002,
		-- shrinePosition = {x = 32910, y = 32338, z = 15}, -- read-only
		destination = {x = 33586, y = 32263, z = 7},
		effect = CONST_ME_MAGIC_RED
	},
	-- earth shrine
	[3032] = {
		targetAction = 15003,
		-- shrinePosition = {x = 32973, y = 32225, z = 7}, -- read-only
		destination = {x = 33539, y = 32209, z = 7},
		effect = CONST_ME_SMALLPLANTS
	},
	[3033] = {
		targetAction = 15004,
		-- shrinePosition = {x = 33060, y = 32713, z = 5}, -- read-only
		destination = {x = 33527, y = 32301, z = 4},
		effect = CONST_ME_ENERGYHIT
	}
}

local lionsRock = {
	[25006] = {
		itemId = 21442,
		itemPos = {x = 33069, y = 32298, z = 9},
		storage = Storage.LionsRock.Questline,
		value = 9,
		item = 3030,
		fieldId = 2123,
		message = "You place the ruby on the small socket. A red flame begins to burn.",
		effect = CONST_ME_MAGIC_RED
	},
	[25007] = {
		itemId = 21442,
		itemPos = {x = 33069, y = 32302, z = 9},
		storage = Storage.LionsRock.Questline,
		value = 9,
		item = 3029,
		fieldId = 21463,
		message = "You place the sapphire on the small socket. A blue flame begins to burn.",
		effect = CONST_ME_MAGIC_BLUE
	},
	[25008] = {
		itemId = 21440,
		itemPos = {x = 33077, y = 32302, z = 9},
		storage = Storage.LionsRock.Questline,
		value = 9,
		item = 3033,
		fieldId = 7465,
		message = "You place the amethyst on the small socket. A violet flame begins to burn.",
		effect = CONST_ME_PURPLESMOKE
	},
	[25009] = {
		itemId = 21437,
		itemPos = {x = 33077, y = 32298, z = 9},
		storage = Storage.LionsRock.Questline,
		value = 9,
		item = 9057,
		fieldId = 21465,
		message = "You place the topaz on the small socket. A yellow flame begins to burn.",
		effect = CONST_ME_BLOCKHIT
	}
}

local gems = Action()
function gems.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if player:getItemCount(3030) >= 1 and target.itemid == 3229 then
		target:transform(3230)
		target:decay()
		item:remove(1)
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		toPosition:sendMagicEffect(CONST_ME_MAGIC_RED)
		return true
	end
	return false
end

for index, value in pairs(shrine) do
	gems:id(index)
end

gems:id(9057)
gems:register()
