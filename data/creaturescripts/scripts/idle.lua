local config = {
	idleWarning = getConfigValue('idleWarningTime'),
	idleKick = getConfigValue('idleKickTime')
}

function onThink(cid, interval)
	if(not isPlayer(cid)) then
		return false
	end

	local position = getThingPosition(cid)
	local tile = getTileInfo(position)
	if(tile.noLogout or getCreatureNoMove(cid) or
		getPlayerCustomFlagValue(cid, PlayerCustomFlag_AllowIdle)) then
		return true
	end

	local idleTime = getPlayerIdleTime(cid) + interval
	doPlayerSetIdleTime(cid, idleTime)
	if(config.idleKick > 0 and idleTime > config.idleKick) then
		doRemoveCreature(cid)
		return true
	end

	if(config.idleWarning > 0 and idleTime == config.idleWarning) then
		local message = "There was no variation in your behaviour for " .. math.ceil(config.idleWarning / 60000) .. " minutes"
		if(config.idleKick > 0) then
			message = message .. ". You will be disconnected in "
			local diff = math.ceil((config.idleWarning - config.idleKick) / 60000)
			if(diff > 1) then
				message = message .. diff .. " minutes"
			else
				message = message .. "one minute"
			end

			message = message .. " if there is no change in your actions until then"
		end

		doPlayerSendTextMessage(cid, MESSAGE_STATUS_WARNING, message .. ".")
	end

	return true
end
