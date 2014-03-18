function onTargetTile(cid, position)
	position.stackpos = 255
	local corpse = getThingFromPos(position)
	if(corpse.uid == 0 or not isCorpse(corpse.uid) or not isMoveable(corpse.uid) or getCreatureSkullType(cid) == SKULL_BLACK) then
		return false
	end

	doRemoveItem(corpse.uid)
	doConvinceCreature(cid, doCreateMonster("Skeleton", position, false))
	doSendMagicEffect(position, CONST_ME_MAGIC_BLUE)
	return true
end

local area, combat = createCombatArea(AREA_CIRCLE3X3), createCombatObject()
setCombatArea(combat, area)

setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatCallback(combat, CALLBACK_PARAM_TARGETTILE, "onTargetTile")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end