local condition = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10 * 60 * 1000) -- 10 minutes
setConditionParam(condition, CONDITION_PARAM_SKILL_MELEE, 5)
setConditionParam(condition, CONDITION_PARAM_SKILL_SHIELD, -10)
setConditionParam(condition, CONDITION_PARAM_SUBID, 99)

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(not isKnight(cid)) then
		doCreatureSay(cid, "Only knights may drink this fluid.", TALKTYPE_MONSTER, cid)
		return true
	end

	if(doAddCondition(cid, condition)) then
		doSendMagicEffect(fromPosition, CONST_ME_MAGIC_RED)
		doRemoveItem(item.uid, 1)
	end

	return true
end
