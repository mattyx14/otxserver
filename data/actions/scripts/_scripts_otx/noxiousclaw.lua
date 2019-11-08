local cursed = Condition(CONDITION_CURSED)
cursed:setParameter(CONDITION_PARAM_DELAYED, true) -- Condition will delay the first damage from when it's added
cursed:setParameter(CONDITION_PARAM_MINVALUE, -800) -- Minimum damage the condition can do at total
cursed:setParameter(CONDITION_PARAM_MAXVALUE, -1200) -- Maximum damage
cursed:setParameter(CONDITION_PARAM_STARTVALUE, -1) -- The damage the condition will do on the first hit
cursed:setParameter(CONDITION_PARAM_TICKINTERVAL, 40000) -- Delay between damages
cursed:setParameter(CONDITION_PARAM_FORCEUPDATE, true) -- Re-update condition when adding it(ie. min/max value)

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local player, useItem, depleteChance = player, item, 5
	if math.random(100) <= depleteChance then
		player:addCondition(cursed)
		useItem:transform(10312)
		useItem:decay()
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'Ouch! The serpent claw stabbed you.')
	else
		if player:getCondition(CONDITION_POISON) then
			player:removeCondition(CONDITION_POISON)
		end
		useItem:transform(10311)
		useItem:decay()
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_GREEN)
	end
	return true
end
