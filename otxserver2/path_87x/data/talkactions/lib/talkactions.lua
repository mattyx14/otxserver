-- Prevent spamm commands --
function checkExhausted(cid, storage, seconds)
	local v = exhaustion.get(cid, storage)
	if(not v) then
		exhaustion.set(cid, storage, seconds)
	else
		doPlayerSendTextMessage(cid, MESSAGE_EVENT_DEFAULT, "Please wait " .. v .. " seconds before trying this command again.")
		return false
	end

	return true
end
