function onSay(cid, words, param, channel)
	if getPlayerStorageValue(cid, 429833) > os.time() then
		return doPlayerSendCancel(cid, "You're exhausted.")
	end

	local sto = math.max(-1, getPlayerStorageValue(cid, 5556667) or -1)
	setPlayerStorageValue(cid, 5556667, (sto == -1 and 1 or -1))
	doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "[LIFE/MANA PERCENT]: You have "..(sto == -1 and "En" or "Dis").."able life and mana in percent!")
	doUpdatePlayerStats(cid)
	setPlayerStorageValue(cid, 429833, os.time() + 10)
	doCreatureAddHealth(cid, 1)
	doCreatureAddMana(cid, 1)
	return true
end
