local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_POISONDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)

local condition = createConditionObject(CONDITION_POISON)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
setConditionParam(condition, CONDITION_PARAM_MINVALUE, -200)
setConditionParam(condition, CONDITION_PARAM_MAXVALUE, -250)
setConditionParam(condition, CONDITION_PARAM_STARTVALUE, -50)
setConditionParam(condition, CONDITION_PARAM_TICKINTERVAL, 2000)
setConditionParam(condition, CONDITION_PARAM_FORCEUPDATE, true)
setCombatCondition(combat, condition)

function onGetFormulaValues(cid, level, maglevel)

if getPlayerLevel(cid) <= 99 then
	min = -(level * 1.1 + maglevel * 4.1) * 1.2
	max = -(level * 1.3 + maglevel * 5.4) * 1.5
elseif getPlayerLevel(cid) >= 100 and getPlayerLevel(cid) <= 129 then
	min = -(level * 1.1 + maglevel * 4.1) * 1.1
	max = -(level * 1.3 + maglevel * 5.4) * 1.4
elseif getPlayerLevel(cid) >= 130 then
	min = -(level * 1.1 + maglevel * 4.1) * 1.0
	max = -(level * 1.3 + maglevel * 5.4) * 1.3
else
	min = -(level * 1.1 + maglevel * 4.1) * 1.0
	max = -(level * 1.3 + maglevel * 5.4) * 1.3
end
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

arr = {
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
{1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1},
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
}

local area = createCombatArea(arr)

setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end