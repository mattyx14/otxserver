-- Banishment book, because a ban hammer didn't work :(
--
-- type,playerName : type can be account, ip or player
-- extraParameters : these are not neccesary.

local config = {
	accessNotation = 2,
	accessBan = 3,
	accessIpBan = 4,

	banLength = getConfigValue("banLength"),
	ipBanLength = getConfigValue("ipBanishmentLength"),
	finalBanLength = getConfigValue("finalBanLength"),
	
	notationsForBan = getConfigValue("notationsToBan"),
	warningsToFinalBan = getConfigValue("warningsToFinalBan"),
	warningsToDeletion = getConfigValue("warningsToDeletion"),
	nameReportActionType = getConfigValue("violationNameReportActionType"),
	broadcastBanishment = getConfigValue("broadcastBanishments"),

	defaultBanAction = ACTION_BANISHMENT,
	defaultPlayerBanType = PLAYERBAN_BANISHMENT
}

local function getAction(action)
	if(action == ACTION_NOTATION)then
		return "Notation"
	elseif(action == ACTION_NAMEREPORT)then
		return "Name Report"
	elseif(action == ACTION_BANISHMENT)then
		return "Banishment"
	elseif(action == ACTION_BANREPORT)then
		return "Name Report + Banishment"
	elseif(action == ACTION_BANFINAL)then
		return "Banishment + Final Warning"
	elseif(action == ACTION_BANREPORTFINAL)then
		return "Name Report + Banishment + Final Warning"
	elseif(action == ACTION_STATEMENT)then
		return "Statement Report"
	elseif(action == ACTION_DELETION)then
		return "Deletion"
	elseif(action == ACTION_NAMELOCK)then
		return "Name Lock"
	elseif(action == ACTION_BANLOCK)then
		return "Name Lock + Banishment"
	elseif(action == ACTION_BANLOCKFINAL)then
		return "Name Lock + Banishment + Final Warning"
	end

	return "Unknown"
end

function onTextEdit(cid, item, text)
	unregisterCreatureEvent(cid, "BanBook")

	local t = text:explode("\n")
	while(#t < 4)do
		table.insert(t, "")
	end

	for k, v in ipairs(t) do
		t[k] = v:explode(",")
	end

	local message, kick, _type, value = "", 3, t[1][1], t[1][2]
	local player, account = getPlayerByName(value), getAccountIdByName(value)
	if(getPlayerAccess(cid) <= getPlayerAccess(player))then
		doPlayerSendCancel(cid, "You may not give banishment, notations or statements to " .. getPlayerName(player) .. ".")
		return false
	end

	if(isInArray({"account", "acc", "a"}, _type))then
		if(getPlayerAccess(cid) < config.accessBan)then
			doPlayerSendCancel(cid, "You do not have enough access to banish anyone.")
			return false
		end

		local length, action = os.time() + (tonumber(t[2][1]) or config.banLength), (tonumber(t[2][2]) or config.defaultBanAction)
		message = "taken the action \"" .. getAction(action) .. "\" on player " .. getPlayerName(player) .. "."
		doAddAccountBanishment(account, getPlayerGUIDByName(value), length, 21, action, t[3][1] or "", getPlayerGUID(cid), "")
	elseif(isInArray({"ip", "i"}, _type))then
		if(getPlayerAccess(cid) < config.accessIpBan)then
			doPlayerSendCancel(cid, "You do not have enough access to ip banish anyone.")
			return false
		end

		local ip, length = getIpByName(value), os.time() + (tonumber(t[2][2]) or config.ipBanLength)
		message = "taken the action \"Ip Banishment\" on \"" .. getPlayerName(player) .. "\"."
		doAddIpBanishment(ip, tonumber(t[2][1]) or 0xFFFFFFFF, length, 21, t[3][1] or "", getPlayerGUID(cid), "")
	elseif(isInArray({"player", "p"}, _type))then
		local length, bantype, action, comment, forced = os.time(), (tonumber(t[2][1]) or config.defaultPlayerBanType),
			(tonumber(t[3][1]) or config.defaultBanAction), (t[3][2] or ""), false
		if(action == ACTION_NAMEREPORT)then
			bantype = bantype == PLAYERBAN_NONE and config.nameReportActionType or bantype
			if(bantype == PLAYERBAN_BANISHMENT)then
				if(tonumber(t[2][2]))then
					length = length + tonumber(t[2][2])
				else
					length = length + config.banLength
				end
			end

			if(not doAddPlayerBanishment(value, bantype, length, 21, action, comment, getPlayerGUID(cid), ""))then
				doPlayerSendCancel(cid, "Player has already been reported.")
				return false
			elseif(bantype == PLAYERBAN_BANISHMENT)then
				doAddAccountWarnings(account)
			end

			kick = bantype
		elseif(action == ACTION_NOTATION or action == ACTION_BANISHMENT or action == ACTION_BANREPORT)then
			for _ = 1, 1 do
				if(action == ACTION_NOTATION)then
					if(getPlayerAccess(cid) < config.accessNotation)then
						doPlayerSendCancel(cid, "You do not have enough access to give anyone a notation.")
						return false
					end

					local guid = getPlayerGUIDByName(value)
					doAddNotation(account, guid, 21, comment, getPlayerGUID(cid), "")
					if(getNotationsCount(account) < config.notationsForBan)then
						kick = 1
						break
					end

					action = ACTION_BANISHMENT
				end

				if(getPlayerAccess(cid) < config.accessBan)then
					doPlayerSendCancel(cid, "You do not have enough access to banish anyone.")
					return false
				end

				local deny = action ~= ACTION_BANREPORT
				doAddAccountWarnings(account)
				if(getAccountWarnings(account) >= config.warningsToDeletion)then
					action = ACTION_DELETION
				elseif(tonumber(t[2][2]))then
					length = length + tonumber(t[2][2])
				elseif(getAccountWarnings(account) >= config.warningsToFinalBan)then
					length = length + config.finalBanLength
				else
					length = length + config.banLength
				end

				if(not doAddAccountBanishment(account, getPlayerGUIDByName(value), length, 21, action, comment, getPlayerGUID(cid), ""))then
					doRemoveAccountWarnings(account)
					doPlayerSendCancel(cid, "Account already banned.")
					return false
				end

				if(deny)then
					break
				end
				
				doAddPlayerBanishment(value, bantype, length, 21, action, comment, getPlayerGUID(cid), "")
			end
		elseif(action == ACTION_BANFINAL or action == ACTION_BANREPORTFINAL)then
			local allow = (action == ACTION_BANREPORTFINAL)
			doAddAccountWarnings(account)
			if(getAccountWarnings(account) >= config.warningsToDeletion)then
				action = ACTION_DELETION
			elseif(tonumber(t[2][2]))then
				length = length + tonumber(t[2][2])
			else
				length = length + config.finalBanLength
			end

			if(not doAddAccountBanishment(account, getPlayerGUIDByName(value), length, 21, action, comment, getPlayerGUID(cid), ""))then
				doRemoveAccountWarnings(account)
				doPlayerSendCancel(cid, "Account already banned.")
				return false
			end

			if(action ~= ACTION_DELETION)then
				doAddAccountWarnings(account, config.warningsToFinalBan - 1)
			end
			
			if(allow)then
				doAddPlayerBanishment(value, config.nameReportActionType, -1, 21, action, comment, getPlayerGUID(cid), "")
			end
		elseif(action == ACTION_DELETION)then
			doAddAccountWarnings(account)
			if(not doAddAccountBanishment(account, getPlayerGUIDByName(value), -1, 21, action, comment, getPlayerGUID(cid), ""))then
				doRemoveAccountWarnings(account)
				doPlayerSendCancel(cid, "Account currently banned or already deleted.")
				return false
			end
		end

		message = "taken the action \"" .. getAction(action, false) .. "\""
		if(action == ACTION_NOTATION)then
			message = message .. " (" .. (config.notationsToBan - getNotationsCount(account, getPlayerGUIDByName(value))) .. " left to banishment)"
		end

		message = message .. " against: " .. value .. " (Warnings: " .. getAccountWarnings(account) .. "), with comment: \"" .. comment .. "\"."
	else
		doPlayerSendCancel(cid, "No such ban type.")
		return false
	end

	if(kick >= 3)then
		doRemoveNotations(account)
	end

	if(config.broadcastBanishment)then
		doBroadcastMessage(getPlayerName(cid) .. " has " .. message)
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, "You have " .. message)
	end

	if(isPlayer(player) and kick > 1)then
		doSendMagicEffect(getCreaturePosition(player), CONST_ME_MAGIC_GREEN)
		doRemoveCreature(player)
	end

	return false
end
