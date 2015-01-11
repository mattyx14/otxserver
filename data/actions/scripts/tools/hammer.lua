function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target.actionid == 50109 then
		if player:getItemCount(5901) >= 3 and player:getItemCount(10033) >= 1 and player:getItemCount(10034) >= 2 and player:getItemCount(8309) >= 6 then
			player:removeItem(5901, 3)
			player:removeItem(8309, 3)
			local bridge = Game.createItem(5779, 1, Position(32571, 31508, 9))
			bridge:setActionId(50110)
			player:say("KLING KLONG!", TALKTYPE_MONSTER_SAY, false, 0, pos)
		end
	elseif target.actionid == 50110 then
		player:removeItem(10033, 1)
		player:removeItem(10034, 2)
		player:removeItem(8309, 3)
		local rails = Game.createItem(7122, 1, Position(32571, 31508, 9))
		rails:setActionId(50111)
		player:say("KLING KLONG!", TALKTYPE_MONSTER_SAY, false, 0, pos)
	else
		return false
	end
	return true
end
