function onSay(cid, words, param, channel)
local storage = 6707
local delaytime = 600
local a = "data/logs/bugs.txt"

local f = io.open(a, "a+")
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local exhaust = exhaustion.get(cid, storage)
	if(not exhaust) then
		exhaustion.set(cid, storage, delaytime)
			doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "Your report has been received successfully!")
			f:write(""..getPlayerName(cid).." reported a bug at " .. os.date("%d %B %Y - %X ", os.time()) .."\n"..param.." [x="..getPlayerPosition(cid).x..", y="..getPlayerPosition(cid).y..", z="..getPlayerPosition(cid).z.."].\n\n----------------------------------------------------------\n")
		f:close()
	else
		doPlayerSendCancel(cid, "You are exhausted.")
	end

	return true
end
