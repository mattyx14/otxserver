local combat, area = createCombatObject(), createCombatArea(AREA_SQUARE1X1)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
setCombatArea(combat, area)

local condition = createConditionObject(CONDITION_OUTFIT)
setConditionParam(condition, CONDITION_PARAM_TICKS, 20000)
for i = 230, 247 do
	addOutfitCondition(condition, {lookTypeEx = i})
end

setCombatCondition(combat, condition)
function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
