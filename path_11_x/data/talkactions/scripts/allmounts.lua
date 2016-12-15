function onSay(player, words, param)
	if not player:getGroup():getAccess() then
		return true
	end

	local target
	if param == '' then
		target = player:getTarget()
		if not target then
			player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Unlocks all mounts for certain player. Usage: /mounts <player name>')
			return false
		end
	else
		target = Player(param)
	end

	if not target then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Player ' .. param .. ' is not currently online.')
		return false
	end

	if player:getAccountType() < ACCOUNT_TYPE_GOD  then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Cannot perform action.')
		return false
	end

	for i = 1, 90 do
		target:addMount(i)
	end

	player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'All mounts unlocked for: ' .. target:getName())
	target:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, '[Server] All mounts unlocked.')
	return false
end
