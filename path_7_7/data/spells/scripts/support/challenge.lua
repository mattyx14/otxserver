local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)

local area = createCombatArea(AREA_SQUARE1X1)
setCombatArea(combat, area)

function onTarget(cid, target) return doChallengeCreature(cid, target) end
setCombatCallback(combat, CALLBACK_PARAM_TARGETCREATURE, "onTarget")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
