local dead_human = {
	3058, 3059, 3060, 3061, 3064, 3065, 3066
}
function onCastSpell(creature, var, isHotkey)
	local position = Variant.getPosition(var)
	local tile = position:getTile()
	local object = tile and tile:getTopVisibleThing()
	if object and not object:isCreature() then
		local desintegrate = false
		while not isInArray(dead_human, object.itemid)
			and object:getType():isMovable()
			and object.uid > 65535
			and object.actionid == 0
		do
			object:remove()
			desintegrate = true
			object = tile:getTopVisibleThing()
		end
		if desintegrate then
			position:sendMagicEffect(CONST_ME_BLOCKHIT)
			return true
		end
	end

	creature:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
	creature:getPosition():sendMagicEffect(CONST_ME_POFF)
	return false
end
