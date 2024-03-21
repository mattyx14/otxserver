function onSay(cid, words, param, channel)
	local sto = getPlayerStorageValue(cid, 212121)
	setPlayerStorageValue(cid, 212121, (tonumber(sto) == 1 and -1 or 1))
	doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "[MW System]: Did you "..(tonumber(sto) == 1 and "Deac" or "Ac").."tivate, now you see "..(tonumber(sto) == 1 and "Old" or "New").." MW")
	return true
end
