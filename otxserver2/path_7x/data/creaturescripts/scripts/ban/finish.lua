local config = {
	banLength = getConfigValue('banLength'),
	finalBanLength = getConfigValue('finalBanLength'),
	ipBanLength = getConfigValue('ipBanLength'),
	notationsToBan = getConfigValue('notationsToBan'),
	warningsToFinalBan = getConfigValue('warningsToFinalBan'),
	warningsToDeletion = getConfigValue('warningsToDeletion')
}

function onTextEdit(cid, item, text)
	unregisterCreatureEvent(cid, "Ban_Finish")
	if(item.itemid ~= 2599) then
		return true
	end

	local data = table.unserialize(getCreatureStorage(cid, "banConfig"))
	if(not data.type) then
		return true
	end

	if(text:len() == 0) then
		return false
	end

	text = text:explode("\n")
	if(not data.subType or isInArray({1, 5}, data.subType)) then
		if(text[1] ~= "Name:" or text[3] ~= "(Optional) Length:" or text[5] ~= "Comment:") then
			doPlayerSendCancel(cid, "Invalid format.")
			return false
		end

		local size = table.maxn(text)
		if(size > 6) then
			data.comment = ""
			for i = 6, size do
				data.comment = data.comment .. text[i] .. "\n"
			end
		else
			data.comment = text[6]
		end

		if(text[4]:len() > 0) then
			data.length = loadstring("return " .. text[4])()
		end
	elseif(text[1] ~= "Name:" or text[3] ~= "Comment:") then
		doPlayerSendCancel(cid, "Invalid format.")
		return false
	else
		data.comment = text[4]
	end

	data.name = text[2]
	if(data.type == 1) then
		errors(false)
		local player = getPlayerGUIDByName(data.name, true)

		errors(true)
		if(not player) then
			doPlayerSendCancel(cid, "Player not found.")
			return false
		end

		local account = getAccountIdByName(data.name)
		if(account == 0 or getAccountFlagValue(cid, PLAYERFLAG_CANNOTBEBANNED)) then
			doPlayerSendCancel(cid, "You cannot take action on this player.")
			return false
		end

		local warnings, warning = getAccountWarnings(account), 1
		if(data.subType == 1) then
			if(not tonumber(data.length)) then
				data.length = os.time() + config.banLength
				if((warnings + 1) >= config.warningsToDeletion) then
					data.length = -1
				elseif((warnings + 1) >= config.warningsToFinalBan) then
					data.length = os.time() + config.finalBanLength
				end
			else
				data.length = os.time() + data.length
			end

			doAddAccountBanishment(account, player, data.length, data.comment, getPlayerGUID(cid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (warnings: " .. (warnings + 1) .. ") has been banned.")
		elseif(data.subType == 2) then
			doAddAccountBanishment(account, player, config.finalBanLength, data.comment, getPlayerGUID(cid))
			if(warnings < config.warningsToFinalBan) then
				warning = config.warningsToFinalBan - warnings
			end

			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (warnings: " .. warning .. ") has been banned.")
		elseif(data.subType == 3) then
			doAddAccountBanishment(account, player, -1, data.comment, getPlayerGUID(cid))
			if(warnings < config.warningsToDeletion) then
				warning = config.warningsToDeletion - warnings
			end

			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (warnings: " .. warning .. ") has been deleted.")
		elseif(data.subType == 4) then
			local notations = getNotationsCount(account) + 1
			if(notations >= config.notationsToBan) then
				data.length = os.time() + config.banLength
				if((warnings + 1) >= config.warningsToDeletion) then
					data.length = -1
				elseif((warnings + 1) >= config.warningsToFinalBan) then
					data.length = os.time() + config.finalBanLength
				end

				doAddAccountBanishment(account, player, data.length, data.comment, getPlayerGUID(cid))
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (warnings: " .. (warnings + 1) .. ") has been banned reaching notations limit.")
			else
				doAddNotation(account, player, data.comment, getPlayerGUID(cid))
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (account notations: " .. notations .. ") has been noted.")
				warning = 0
			end
		end

		if(warning > 0) then
			doAddAccountWarnings(account, warning)
			doRemoveNotations(account)

			local pid = getPlayerByGUID(player)
			if(pid) then
				doPlayerSendTextMessage(pid, MESSAGE_STATUS_WARNING, "You have been banned.")
				doSendMagicEffect(getThingPosition(pid), CONST_ME_MAGIC_GREEN)
				addEvent(valid(doRemoveCreature), 1000, pid, true)
			end
		end
	elseif(data.type == 2) then
		errors(false)
		local player = getPlayerGUIDByName(data.name, true)

		errors(true)
		if(not player) then
			doPlayerSendCancel(cid, "Player not found.")
			return false
		end

		local account = getAccountIdByName(data.name)
		if(account == 0 or getAccountFlagValue(account, PLAYERFLAG_CANNOTBEBANNED)) then
			doPlayerSendCancel(cid, "You cannot take action on this player.")
			return false
		end

		data.subType = data.subType - 4
		if(data.subType == 1) then
			if(not tonumber(data.length)) then
				local warnings = getAccountWarnings(account) + 1
				data.length = os.time() + config.banLength
				if(warnings >= config.warningsToDeletion) then
					data.length = -1
				elseif(warnings >= config.warningsToFinalBan) then
					data.length = os.time() + config.finalBanLength
				end
			else
				data.length = os.time() + data.length
			end

			doAddPlayerBanishment(data.name, 3, data.length, data.comment, getPlayerGUID(cid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been banned.")

			local pid = getPlayerByGUID(player)
			if(pid) then
				doPlayerSendTextMessage(pid, MESSAGE_STATUS_WARNING, "You have been banned.")
				doSendMagicEffect(getThingPosition(pid), CONST_ME_MAGIC_GREEN)
				addEvent(valid(doRemoveCreature), 1000, pid, true)
			end
		elseif(data.subType == 2) then
			doAddPlayerBanishment(data.name, 3, -1, data.comment, getPlayerGUID(cid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been deleted.")
		elseif(data.subType == 3) then
			local warnings, notations = getAccountWarnings(account) + 1, getNotationsCount(account, player) + 1
			if(notations >= config.notationsToBan) then
				data.length = os.time() + config.banLength
				if(warnings >= config.warningsToDeletion) then
					data.length = -1
				elseif(warnings >= config.warningsToFinalBan) then
					data.length = os.time() + config.finalBanLength
				end

				doAddPlayerBanishment(account, 3, data.length, data.comment, getPlayerGUID(cid))
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been banned reaching notations limit.")

				local pid = getPlayerByGUID(player)
				if(pid) then
					doPlayerSendTextMessage(pid, MESSAGE_STATUS_WARNING, "You have been banned.")
					doSendMagicEffect(getThingPosition(pid), CONST_ME_MAGIC_GREEN)
					addEvent(valid(doRemoveCreature), 1000, pid, true)
				end
			else
				doAddNotation(account, player, data.comment, getPlayerGUID(cid))
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " (notations: " .. notations .. ") has been noted.")
			end
		elseif(data.subType == 4) then
			doAddPlayerBanishment(data.name, 1, -1, data.comment, getPlayerGUID(cid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been reported.")
		elseif(data.subType == 5) then
			doAddPlayerBanishment(data.name, 2, -1, data.comment, getPlayerGUID(cid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been namelocked.")

			local pid = getPlayerByGUID(player)
			if(pid) then
				doPlayerSendTextMessage(pid, MESSAGE_STATUS_WARNING, "You have been banned.")
				doSendMagicEffect(getThingPosition(pid), CONST_ME_MAGIC_GREEN)
				addEvent(valid(doRemoveCreature), 1000, pid, true)
			end
		end
	elseif(data.type == 3) then
		local ip = getIpByName(data.name)
		if(not ip) then
			doPlayerSendCancel(cid, "Player not found.")
			return false
		end

		local account = getAccountIdByName(data.name)
		if(account == 0 or getAccountFlagValue(account, PLAYERFLAG_CANNOTBEBANNED)) then
			doPlayerSendCancel(cid, "You cannot take action on this player.")
			return false
		end

		if(not tonumber(data.length)) then
			data.length = config.ipBanLength
		end

		doAddIpBanishment(ip, 4294967295, os.time() + data.length, data.comment, getPlayerGUID(cid))
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_RED, getPlayerNameByGUID(player) .. " has been banned on IP: " .. doConvertIntegerToIp(ip) .. ".")

		local pid = getPlayerByGUID(player)
		if(pid) then
			doPlayerSendTextMessage(pid, MESSAGE_STATUS_WARNING, "You have been banned.")
			doSendMagicEffect(getThingPosition(pid), CONST_ME_MAGIC_GREEN)
			addEvent(valid(doRemoveCreature), 1000, pid, true)
		end
	end

	return false
end
