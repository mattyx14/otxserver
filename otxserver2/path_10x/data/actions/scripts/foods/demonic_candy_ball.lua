local condition = {}

local conditions = {
	CONDITION_ATTRIBUTES,
	CONDITION_HASTE,
	CONDITION_LIGHT,
	CONDITION_INVISIBLE
}

local attributes = {
	CONDITION_PARAM_SKILL_SHIELD,
	CONDITION_PARAM_SKILL_DISTANCE,
	CONDITION_PARAM_SKILL_MELEE,
	CONDITION_PARAM_STAT_MAGICLEVEL
}

condition[CONDITION_HASTE] = createConditionObject(CONDITION_HASTE)
setConditionParam(condition[CONDITION_HASTE], CONDITION_PARAM_TICKS, 60 * 60 * 1000)
setConditionParam(condition[CONDITION_HASTE], CONDITION_PARAM_SUBID, 2)
setConditionParam(condition[CONDITION_HASTE], CONDITION_PARAM_SPEED, 60)

condition[CONDITION_ATTRIBUTES] = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition[CONDITION_ATTRIBUTES], CONDITION_PARAM_TICKS, 60 * 60 * 1000)

condition[CONDITION_LIGHT] = createConditionObject(CONDITION_LIGHT)
setConditionParam(condition[CONDITION_LIGHT], CONDITION_PARAM_LIGHT_LEVEL, 9)
setConditionParam(condition[CONDITION_LIGHT], CONDITION_PARAM_LIGHT_COLOR, 215)
setConditionParam(condition[CONDITION_LIGHT], CONDITION_PARAM_TICKS, 60 * 60 * 1000)

condition[CONDITION_INVISIBLE] = createConditionObject(CONDITION_INVISIBLE)
setConditionParam(condition[CONDITION_INVISIBLE], CONDITION_PARAM_TICKS, 10 * 60 * 1000)

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local food = SPECIAL_FOODS[item.itemid]
	if(food == nil) then
		return false
	end

	local random_condition = conditions[math.random(#conditions)]
	if(random_condition == CONDITION_ATTRIBUTES) then
		setConditionParam(condition[CONDITION_ATTRIBUTES], attributes[math.random(#attributes)], math.random(3,15))
	end

	if(not doAddCondition(cid, condition[random_condition])) then
		return true
	end

	doRemoveItem(item.uid, 1)
	doCreatureSay(cid, food, TALKTYPE_MONSTER)
	return true
end
