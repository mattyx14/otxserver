local config = {
	heal = true,
	save = true
}

function onAdvance(player, skill, oldLevel, newLevel)
	if skill ~= 8 or newLevel <= oldLevel then
		return true
	end

	if config.heal then
		player:addHealth(player:getMaxHealth())
	end

	if config.save then
		player:save()
	end
	return true
end
