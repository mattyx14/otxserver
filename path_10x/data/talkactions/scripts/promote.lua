local config = {
	broadcast = false
}

function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local t = string.explode(param, ",", 1)
	local pid = getPlayerByNameWildcard(t[1])
	if(not pid) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[1] .. " not found.")
		return true
	end

	if(getPlayerAccess(pid) >= getPlayerAccess(cid)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cannot perform action.")
		return true
	end

	local g, group = 1, getPlayerGroupId(pid)
	if(words:sub(2, 2) == "d") then
		g = -1
	end

	local newGroup = group + g
	if(t[2] ~= nil) then
		for i, id in ipairs(getGroupList()) do
			local tmp = getGroupInfo(id)
			if(isInArray({tmp.id, tmp.name}, t[2])) then
				newGroup = id
				break
			end
		end
	end

	if(newGroup <= 0 or newGroup == group or not setPlayerGroupId(pid, newGroup)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cannot perform action.")
		return true
	end

	local str = "been " .. (group < newGroup and "promoted" or "demoted") .. " to " .. getGroupInfo(newGroup).name .. "."
	if(not config.broadcast) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, param .. " has " .. str)
	else
		doBroadcastMessage(param .. " has " .. str, MESSAGE_EVENT_ADVANCE)
	end

	doPlayerSendTextMessage(pid, MESSAGE_EVENT_ADVANCE, "You have " .. str)
	return true
end
