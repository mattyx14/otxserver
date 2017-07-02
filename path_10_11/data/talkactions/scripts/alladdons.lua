local looktypes = {
	128, 136, 129, 137, 130, 138, 131, 139, 132, 140, 133, 141, 134, 142,
	143, 147, 144, 148, 145, 149, 146, 150, 151, 155, 152, 156, 153, 157,
	154, 158, 251, 252, 268, 269, 273, 270, 278, 279, 289, 288, 325, 324,
	335, 336, 366, 367, 328, 329, 430, 431, 432, 433, 463, 464, 465, 466,
	471, 472, 512, 513, 514, 516, 541, 542, 574, 575, 577, 578, 618, 620,
	632, 635, 636, 664, 666, 683, 694, 696, 698, 724, 732, 745, 749, 759,
	845, 852, 610, 619, 633, 634, 637, 665, 667, 684, 695, 697, 699, 725,
	733, 746, 750, 760, 846, 853,
}

function onSay(player, words, param)
	if not player:getGroup():getAccess() then
		return true
	end

	local target
	if param == '' then
		target = player:getTarget()
		if not target then
			player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Gives players the ability to wear all addons. Usage: /addons <player name>')
			return false
		end
	else
		target = Player(param)
	end

	if not target then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Player ' .. param .. ' is currently not online.')
		return false
	end

	if player:getAccountType() < ACCOUNT_TYPE_GOD  then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Cannot perform action.')
		return false
	end

	for i = 1, #looktypes do
		target:addOutfitAddon(looktypes[i], 3)
	end

	player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'All addons unlocked for ' .. target:getName() .. '.')
	target:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, '[Server] All addons unlocked.')
	return false
end
