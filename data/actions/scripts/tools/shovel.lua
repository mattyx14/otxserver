local holes = {468, 481, 483, 7932}
function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if isInArray(holes, target.itemid) then
		target:transform(target.itemid + 1)
		target:decay()
	elseif target.itemid == 231 or target.itemid == 9059 then
		local rand = math.random(100)
		if target.actionid == 100 and rand <= 20 then
			target:transform(489)
			target:decay()
		elseif rand == 1 then
			Game.createItem(2159, 1, toPosition)
		elseif rand > 95 then
			Game.createMonster("Scarab", toPosition)
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
	elseif target.itemid == 22674 then
		if not player:removeItem(5091, 1) then
			return false
		end

		target:transform(5731)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_POFF)
	end

	return true
end
