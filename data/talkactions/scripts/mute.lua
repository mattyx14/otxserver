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
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE,"Use /mute nome, tempo (muta total), /helpmute nome, tempo (muta somente no help).")
		return true
	end

	local t = string.explode(param, ', ')
	local target = getPlayerByNameWildcard(t[1])

	if not target or (isPlayerGhost(target) and getPlayerGhostAccess(target) > getPlayerGhostAccess(cid)) then
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." não está online.")
		return true
	end

	if getPlayerAccess(cid) <= getPlayerAccess(target) or getPlayerFlagValue(target, PLAYERFLAG_CANNOTBEMUTED) then
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, "Você não pode mutar este jogador.")
		return true
	end

	if words:sub(2,2) ~= "h" and getPlayerAccess(cid) < 4 then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Este comando só está disponível para GMs, CMs e GODs. Você só pode usar o /helpmute.")
		return true
	end

	if words:sub(2,2) == "h" then
		local time = tonumber(t[2])
		if not time or time < 1 then time = 60 end

		if time > 100000 and getPlayerAccess(cid) < 4 then
			doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE,"Você não pode mutar por mais de 100000 segundos.")
			return true
		end

		silence(target,time,true)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, " Você foi mutado no help por "..time.." segundos.")
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." foi mutado no help por "..time.." segundos.")

	elseif words:sub(2,2) == "m" then
		local time = tonumber(t[2])
		if not time or time < 1 then time = 60 end
		silence(target,time,false)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, " Você foi mutado por "..time.." segundos.")
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." foi mutado por "..time.." segundos.")

	elseif words:sub(2,2) == "u" then
		doRemoveCondition(target,CONDITION_MUTED)
		doPlayerSetStorageValue(target, 455010, -1)
		doPlayerSendTextMessage(target, MESSAGE_STATUS_CONSOLE_RED, "Você foi desmutado pelo: "..getCreatureName(cid))
		doPlayerSendTextMessage(cid,MESSAGE_STATUS_CONSOLE_BLUE, t[1].." foi desmutado.")
	end
	return true
end
