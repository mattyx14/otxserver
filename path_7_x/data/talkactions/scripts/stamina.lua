local function formatStamina(miliseconds)
	if(miliseconds < 1000)then
		return "00:00:00";
	else
		local seconds = string.format("%02.f", math.floor(miliseconds / 1000))
		local hours = string.format("%02.f", math.floor(seconds / 3600))
		local mins = string.format("%02.f", math.floor(seconds / 60 - (hours * 60)))
		local secs = string.format("%02.f", math.floor(seconds - (hours * 3600) - (mins * 60)))
		return hours..":"..mins..":"..secs
	end
end

function onSay(cid, words, param)
	if(not(getConfigValue('useStamina')))then
		return false
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You have "..formatStamina(getPlayerStamina(cid)).." of stamina left.")
	return true
end