function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item.actionid == 24877 then
		player:teleportTo(Position(1075, 959, 15))
		return true
	end

	if not player:getItemById(26406, true) then
		return false
	end

	if player:getStorageValue(Storage.ForgottenKnowledge.SilverKey) < 1 or not player:getItemById(26401, true) then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'You don\'t have the fitting key.')
		return true
	end

	player:teleportTo(Position(1095, 1080, 15))
	return true
end
