local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

function onCastSpell(cid, var)
if getCreatureCondition(cid,CONDITION_POISON) then
doRemoveCondition(cid, CONDITION_POISON)
end
	return doCombat(cid, combat, var)
end
