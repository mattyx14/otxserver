local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_HITBYPHYSICAL)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_PHYSICAL)

function onGetFormulaValues(cid)
local condition = createConditionObject(CONDITION_PHYSICAL)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
addDamageCondition(condition, 10, 20000, -10)
setCombatCondition(combat, condition)
end

setCombatCallback(combat,COMBAT_FORMULA_LEVELMAGIC, "onGetFormulaValues")
function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end