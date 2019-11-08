local condition = createConditionObject(CONDITION_REGENERATION)
setConditionParam(condition, CONDITION_PARAM_SUBID, 88888)
setConditionParam(condition, CONDITION_PARAM_TICKS, 15 * 60 * 1000)
setConditionParam(condition, CONDITION_PARAM_HEALTHGAIN, 0.01)
setConditionParam(condition, CONDITION_PARAM_HEALTHTICKS, 15 * 60 * 1000)

function onCastSpell(cid, var)
    if isCreature(cid) == true then
        if getCreatureHealth(cid) < getCreatureMaxHealth(cid) * 0.2 and getCreatureCondition(cid, CONDITION_REGENERATION, 88888) == false then
            doAddCondition(cid, condition)
	    local hp = math.random(5000, 7500)
            doCreatureAddHealth(cid, hp)
        else
            return false
        end
    else
        return false
    end
    return true
end