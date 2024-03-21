function onSay(cid, words, param, channel)
	if(not checkExhausted(cid, 666, 10)) then
		return true
	end

	if(not getBooleanFromString(getConfigValue('useFragHandler'))) then
		return false
	end

	local time = os.time()
	local times = {today = (time - 86400), week = (time - (7 * 86400))}

	local contents, result = {day = {}, week = {}, month = {}}, db.getResult("SELECT `pd`.`date`, `pd`.`level`, `p`.`name` FROM `player_killers` pk LEFT JOIN `killers` k ON `pk`.`kill_id` = `k`.`id` LEFT JOIN `player_deaths` pd ON `k`.`death_id` = `pd`.`id` LEFT JOIN `players` p ON `pd`.`player_id` = `p`.`id` WHERE `pk`.`player_id` = " .. getPlayerGUID(cid) .. " AND `k`.`unjustified` = 1 AND `k`.`war` = 0 AND `pd`.`date` >= " .. (time - (30 * 86400)) .. " ORDER BY `pd`.`date` DESC")
	if(result:getID() ~= -1) then
		repeat
			local content = {
				name = result:getDataString("name"),
				level = result:getDataInt("level"),
				date = result:getDataInt("date")
			}
			if(content.date > times.today) then
				table.insert(contents.day, content)
			elseif(content.date > times.week) then
				table.insert(contents.week, content)
			else
				table.insert(contents.month, content)
			end
		until not result:next()
		result:free()
	end

	local size = {
		day = table.maxn(contents.day),
		week = table.maxn(contents.week),
		month = table.maxn(contents.month)
	}

	if(getBooleanFromString(getConfigValue('advancedFragList'))) then
		local result = "Frags gained today: " .. size.day .. "."
		if(size.day > 0) then
			for i, content in ipairs(contents.day) do
				if i > 5 then
					break
				end
				result = result .. "\n* " .. os.date("%d %B %Y %X at ", content.date) .. content.name .. " on level " .. content.level
			end

			result = result .. "\n"
		end

		result = result .. "\nFrags gained this week: " .. (size.day + size.week) .. "."
		if(size.week > 0) then
			for i, content in ipairs(contents.week) do
				if i > 5 then
					break
				end
				result = result .. "\n* " .. os.date("%d %B %Y %X at ", content.date) .. content.name .. " on level " .. content.level
			end

			result = result .. "\n"
		end

		result = result .. "\nFrags gained this month: " .. (size.day + size.week + size.month) .. "."
		if(size.month > 0) then
			for i, content in ipairs(contents.month) do
				if i > 5 then
					break
				end
				result = result .. "\n* " .. os.date("%d %B %Y %X at ", content.date) .. content.name .. " on level " .. content.level
			end

			result = result .. "\n"
		end

		local skullEnd = getPlayerSkullEnd(cid)
		if(skullEnd > 0) then
			result = result .. "\nYour " .. (getCreatureSkullType(cid) == SKULL_RED and "red" or "black") .. " skull will expire at " .. os.date("%d %B %Y %X", skullEnd)
		end

		doPlayerPopupFYI(cid, result)
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You currently have " .. size.day .. " frags today, " .. (size.day + size.week) .. " this week and " .. (size.day + size.week + size.month) .. " this month.")
		if(size.day > 0) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Last frag at " .. os.date("%d %B %Y %X", contents.day[1].date) .. " on level " .. contents.day[1].level .. " (" .. contents.day[1].name .. ").")
		end

		local skullEnd = getPlayerSkullEnd(cid)
		if(skullEnd > 0) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your " .. (getCreatureSkullType(cid) == SKULL_RED and "red" or "black") .. " skull will expire at " .. os.date("%d %B %Y %X", skullEnd))
		end
	end

	return true
end
