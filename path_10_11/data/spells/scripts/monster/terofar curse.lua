local combat = {}
local condition2 = createConditionObject(CONDITION_REGENERATION)
setConditionParam(condition2, CONDITION_PARAM_SUBID, 88888)
setConditionParam(condition2, CONDITION_PARAM_TICKS, 15 * 60 * 1000)
setConditionParam(condition2, CONDITION_PARAM_HEALTHGAIN, 0.01)
setConditionParam(condition2, CONDITION_PARAM_HEALTHTICKS, 15 * 60 * 1000)

for i = 1, 1 do
	combat[i] = createCombatObject()
	setCombatParam(combat[i], COMBAT_PARAM_TYPE, COMBAT_DEATHDAMAGE)
	setCombatParam(combat[i], COMBAT_PARAM_EFFECT, CONST_ME_SMALLCLOUDS)
	setCombatParam(combat[i], COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_DEATH)

	local condition = createConditionObject(CONDITION_CURSED)
	setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)

	local damage = i
	addDamageCondition(condition, 1, 4000, -damage)
	for j = 1, 36 do
		damage = damage * 1.2
		addDamageCondition(condition, 1, 4000, -damage)
	end

	setCombatCondition(combat[i], condition)
end

function onCastSpell(cid, var)
	    if isCreature(cid) == true then
        if  getCreatureCondition(cid, CONDITION_REGENERATION, 88888) == false then
            doAddCondition(cid, condition2)
			doCreatureSay(cid, "Terofar cast a greater death curse on you!", TALKTYPE_ORANGE_1)
        else
            return false
        end
    else
        return false
    end
	return doCombat(cid, combat[math.random(1, 1)], var)
end
