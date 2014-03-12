function onSay(cid, words, param, channel)
	param = tonumber(param)
	if(not param or param < 0 or param > CONST_ME_LAST) then
		doPlayerSendCancel(cid, "Numeric param may not be lower than 0 and higher than " .. CONST_ME_LAST .. ".")
		return true
	end

	doSendMagicEffect(getCreaturePosition(cid), param)
	return true
end
