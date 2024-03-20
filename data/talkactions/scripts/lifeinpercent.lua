function onSay(cid, words, param, channel)
	local sto = math.max(-1, getPlayerStorageValue(cid, 5556667) or -1)
	setPlayerStorageValue(cid, 5556667, (sto == -1 and 1 or -1))
	doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "[LIFE/MANA PERCENT]: You have "..(sto == -1 and "En" or "Dis").."able life and mana in percent!")
	doUpdatePlayerStats(cid)
	return true
end