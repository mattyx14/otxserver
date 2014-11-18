function onUse(player, item, fromPosition, itemEx, toPosition, isHotkey)
	local str = {"\nSpiritual Shielding", "\nEmbrace of Tibia", "\nFire of the Suns", "\nSpark of the Phoenix", "\nWisdom of Solitude", "\nTwist of Fate"}
	local result = "Received blessings:"
	for i = 1, 6 do
		result = player:hasBlessing(i) and result .. str[i] or result
	end

	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 20 > result:len() and "No blessings received." or result)
	return true
end
