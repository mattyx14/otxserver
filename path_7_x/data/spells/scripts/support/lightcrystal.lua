local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

local condition = createConditionObject(CONDITION_LIGHT)
setConditionParam(condition, CONDITION_PARAM_LIGHT_LEVEL, 8)
setConditionParam(condition, CONDITION_PARAM_LIGHT_COLOR, 215)
setConditionParam(condition, CONDITION_PARAM_TICKS, (11 * 60 + 35) * 1000)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
local crystalValue = getCreatureStorage(cid, 134)
if crystalValue >= os.time() then
	return doCombat(cid, combat, var)
else
	doPlayerSendCancel(cid, "Only VIP Character can use this spell.")
end
	return false
end
