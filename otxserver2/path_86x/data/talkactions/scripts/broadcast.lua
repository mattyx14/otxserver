function onSay(cid, words, param, channel)
	if(param == '') then
		return true
	end

	doPlayerBroadcastMessage(cid, param)
	return true
end
