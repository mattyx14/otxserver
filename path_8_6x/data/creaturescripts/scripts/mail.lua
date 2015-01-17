function onMailReceive(cid, target, item, openBox)
	if(openBox) then
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "New mail has arrived.")
	end

	return true
end
