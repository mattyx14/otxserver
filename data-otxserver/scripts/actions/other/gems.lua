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
