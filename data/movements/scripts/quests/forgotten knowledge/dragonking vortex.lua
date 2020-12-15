local function functionBack()
	local dragonking = Tile(Position(381, 357, 11)):getTopCreature()
	local soul, diference, health = false, 0, 0
	local spectators, spectator = Game.getSpectators(Position(381, 356, 10), false, false, 17, 17, 17, 17)
	for v = 1, #spectators do
		spectator = spectators[v]
		if spectator:getName():lower() == 'soul of dragonking zyrtarch' then
			soul = true
		end
	end
	if not soul then
		dragonking:remove()
		return true
	end
	local specs, spec = Game.getSpectators(Position(381, 356, 10), false, false, 17, 17, 17, 17)
	for i = 1, #specs do
		spec = specs[i]
		if spec:isPlayer() then
			spec:teleportTo(Position(381, 365, 9))
		elseif spec:isMonster() and spec:getName():lower() == 'soul of dragonking zyrtarch' then
			spec:teleportTo(Position(381, 357, 11))
			health = spec:getHealth()
			diference = dragonking:getHealth() - health
		end
	end
	dragonking:addHealth( - diference)
	dragonking:teleportTo(Position(381, 351, 9))
end

local function removeVortex(position)
	local vortex = Tile(position):getItemById(26396)
	if vortex then
		vortex:remove()
	end
end
function onStepIn(creature, item, position, fromPosition)
	if creature:isMonster() and creature:getName():lower() ~= 'dragonking zyrtarch' then
		return true
	end

	if creature:isPlayer() then
		creature:teleportTo(Position(378, 355, 10))
	end

	if creature:getName():lower() == 'dragonking zyrtarch' then
		local soul = Tile(Position(381, 357, 11)):getTopCreature()
		creature:teleportTo(Position(381, 357, 11))
		soul:teleportTo(Position(381, 356, 10))
		addEvent(functionBack, 30 * 1000)
		addEvent(removeVortex, 15 * 1000, position)
	end

	return true
end
