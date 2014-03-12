local TYPE_ACCESS = {
	[1] = { "Player" },
	[2] = { "Player" },
	[3] = { "Account", "Player" },
	[4] = { "Account", "Player" },
	[5] = { "Account", "Player", "IP" }
}

function onSay(cid, words, param, channel)
	unregisterCreatureEventType(cid, "channelrequest")
	unregisterCreatureEventType(cid, "textedit")

	doPlayerSendChannels(cid, TYPE_ACCESS[getPlayerAccess(cid)])
	registerCreatureEvent(cid, "Ban_Type")
	return true
end