local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_NONE)

local area = createCombatArea(AREA_CIRCLE3X3)
setCombatArea(combat, area)


function onCastSpell(cid, var)
	doCreatureSay(cid, "CRUSH THEM ALL!", TALKTYPE_ORANGE_2)
			local mid = doSummonCreature("Demon", { x=33528, y=32330, z=12 })
				    doSummonCreature("Demon", { x=33523, y=32338, z=12 })
				    doSummonCreature("Demon", { x=33532, y=32337, z=12 })
    			if mid == false then
				return false
			end
	return doCombat(cid, combat, var)
end