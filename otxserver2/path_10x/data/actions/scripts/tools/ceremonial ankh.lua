local BLESSINGS = {"Wisdom of Solitude", "Spark of the Phoenix", "Fire of the Suns", "Spiritual Shielding", "Embrace of Tibia", "Twist of Fate"}
function onUse(cid, item, fromPosition, itemEx, toPosition)
	local result = ""
	for i = 1, (table.maxn(BLESSINGS) - 1) do
		result = (getPlayerBlessing(cid, i) and result .. (result:len() > 0 and ", " or "") .. BLESSINGS[i] or result)
	end

	if(getPlayerPVPBlessing(cid)) then
		result = result .. ", " .. BLESSINGS[table.maxn(BLESSINGS)]
	end

	return doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, result:len() > 0 and "Currently you have the following blessings: " .. result .. "." or "You do not have any blessing.")
end
