local condition = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10 * 60 * 1000) -- 10 minutes
setConditionParam(condition, CONDITION_PARAM_STAT_MAGICLEVEL, 3)
setConditionParam(condition, CONDITION_PARAM_SKILL_SHIELD, -10)
setConditionParam(condition, CONDITION_PARAM_SUBID, 99)

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(not isSorcerer(cid) and not isDruid(cid)) then
		doCreatureSay(cid, "Only sorcerers and druids may drink this fluid.", TALKTYPE_ORANGE_1, cid)
		return true
	end

	if(doAddCondition(cid, condition)) then
		doSendMagicEffect(fromPosition, CONST_ME_MAGIC_RED)
		doRemoveItem(item.uid, 1)
		doCreatureSay(cid, "You feel smarter.", TALKTYPE_ORANGE_1, cid)
	end

	return true
end
