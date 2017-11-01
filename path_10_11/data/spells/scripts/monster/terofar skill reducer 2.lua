local combat = {}

for i = 1, 10 do
combat[i] = createCombatObject()
setCombatParam(combat[i], COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)

local condition = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition, CONDITION_PARAM_TICKS, 15000)
setConditionParam(condition, CONDITION_PARAM_STAT_MAGICPOINTSPERCENT, i)

	local area = createCombatArea({
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0}
	})

setCombatArea(combat[i], area)
addCombatCondition(combat[i], condition)

end

function onCastSpell(cid, var)
	return doCombat(cid, combat[math.random(1, 10)], var)
end
