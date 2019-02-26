local config = {
	expiration = getConfigValue('reportsExpirationAfterReads')
}

function onSay(cid, words, param, channel)
	if(not checkExhausted(cid, 666, 10)) then
		return true
	end

	local t = { param }
	if(t[1] ~= nil) then
		t = string.explode(param, " ", 1)
	end

	local reportId = tonumber(t[1])
	if(reportId ~= nil) then
		local report = db.getResult("SELECT `r`.*, `p`.`name` AS `player_name` FROM `server_reports` r LEFT JOIN `players` p ON `r`.`player_id` = `p`.`id` WHERE `r`.`id` = " .. reportId)
		if(report:getID() == -1) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Report with no. " .. reportId .. " does not exists.")
			return true
		end

		if(t[2] ~= nil and isInArray({"delete", "remove"}, t[2])) then
			db.query("DELETE FROM `server_reports` WHERE `id` = " .. reportId)
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Report with no. " .. reportId .. " has been deleted.")

			result:free()
			return true
		end

		db.query("UPDATE `server_reports` SET `reads` = `reads` + 1 WHERE `id` = " .. reportId)
		doPlayerPopupFYI(cid, "Report no. " .. reportId .. "\n\nName: " .. report:getDataString("player_name") .. "\nPosition: [X: " .. report:getDataInt("posx") .. " | Y: " .. report:getDataInt("posy") .. " | Z: " .. report:getDataInt("posz") .. "]\nDate: " .. os.date("%c", report:getDataInt("timestamp")) .. "\nReads: " .. report:getDataInt("reads") .. "\nReport:\n\n" .. report:getDataString("report"))

		report:free()
		return true
	end

	local tmp = ";"
	if(config.expiration > 0) then
		tmp = " WHERE `r`.`reads` < " .. config.expiration .. ";"
	end

	local list = db.getResult("SELECT `r`.`id`, `r`.`player_id`, `p`.`name` AS `player_name` FROM `server_reports` r LEFT JOIN `players` p ON `r`.`player_id` = `p`.`id`" .. tmp)
	if(list:getID() == -1) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "There are no active reports.")
		return true
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Active reports:")
	repeat
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, list:getDataInt("id") .. ", by " .. list:getDataString("player_name") .. ".")
	until not list:next()

	list:free()
	return true
end
