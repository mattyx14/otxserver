local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, 0)

local area = createCombatArea(AREA_CROSS5X5)
combat:setArea(area)

local condition = Condition(CONDITION_ATTRIBUTES)
condition:setParameter(CONDITION_PARAM_SUBID, 1)
condition:setParameter(CONDITION_PARAM_BUFF_SPELL, 1)
condition:setParameter(CONDITION_PARAM_TICKS, 2 * 60 * 1000)
condition:setParameter(CONDITION_PARAM_SKILL_MELEE, 3)
condition:setParameter(CONDITION_PARAM_SKILL_DISTANCE, 3)

function onCastSpell(creature, var)
	local position = creature:getPosition()
	local party = creature:getParty()
	if not party or party:getMemberCount() < 1 then
		creature:sendCancelMessage('No party members in range.')
		position:sendMagicEffect(CONST_ME_POFF)
		return false
	end

	local affectedList = {}
	for _, members in ipairs(party:getMembers()) do
		if members:getPosition():getDistance(position) <= 36 then
			table.insert(affectedList, members)
		end
	end

	local tmp = #affectedList
	if tmp < 1 then
		creature:sendCancelMessage('No party members in range.')
		position:sendMagicEffect(CONST_ME_POFF)
		return false
	end

	local mana = math.ceil((0.9 ^ (tmp - 1) * 60) * tmp)
	if creature:getMana() < mana then
		creature:sendCancelMessage(RETURNVALUE_NOTENOUGHMANA)
		position:sendMagicEffect(CONST_ME_POFF)
		return false
	end

	if not combat:execute(creature, var) then
		creature:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		position:sendMagicEffect(CONST_ME_POFF)
		return false
	end

	creature:addMana(-(mana - 60), false)
	creature:addManaSpent((mana - 60) * configManager.getNumber(configKeys.RATE_MAGIC))
	creature:addCondition(condition)
	for _, members in ipairs(affectedList) do
		members:addCondition(condition)
	end

	return true
end
