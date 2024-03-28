local muted = createConditionObject(CONDITION_MUTED)
	
local function silence(cid,time,help)
	if help then
		storagehelp = 455010
		doPlayerSetStorageValue(cid, storagehelp, os.time()+time)
		return true
	end
	setConditionParam(muted,CONDITION_PARAM_TICKS,(time*1000))
	doAddCondition(cid,muted)
end

function onSay(cid, words, param)
	if param == "" then
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE,"Use /mute name, time (change total), /helpmute name, time (change only in help).")
		return true
	end

	local t = string.explode(param, ', ')
	local target = getPlayerByNameWildcard(t[1])

	if not target or (isPlayerGhost(target) and getPlayerGhostAccess(target) > getPlayerGhostAccess(cid)) then
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." it's not online.")
		return true
	end

	if getPlayerAccess(cid) <= getPlayerAccess(target) or getPlayerFlagValue(target, PLAYERFLAG_CANNOTBEMUTED) then
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, "You cannot mutate this player.")
		return true
	end

	if words:sub(2,2) ~= "h" and getPlayerAccess(cid) < 4 then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "This command is only available for GMs, CMs and GODs. You can only use the /helpmute.")
		return true
	end

	if words:sub(2,2) == "h" then
		local time = tonumber(t[2])
		if not time or time < 1 then time = 60 end

		if time > 100000 and getPlayerAccess(cid) < 4 then
			doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE,"You cannot mutate for more than 100000 seconds.")
			return true
		end

		silence(target,time,true)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, "You were mutated in help by "..time.." seconds.")
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." was changed in help by "..time.." seconds.")

	elseif words:sub(2,2) == "m" then
		local time = tonumber(t[2])
		if not time or time < 1 then time = 60 end
		silence(target,time,false)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, " You have been mutated by "..time.." seconds.")
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." was mutated by "..time.." seconds.")

	elseif words:sub(2,2) == "u" then
		doRemoveCondition(target,CONDITION_MUTED)
		doPlayerSetStorageValue(target, 455010, -1)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, "You were demuted by: "..getCreatureName(cid))
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." was unmuted.")
	end
	return true
end
