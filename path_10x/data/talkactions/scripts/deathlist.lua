local config = {
	deathAssistCount = getConfigValue('deathAssistCount') + 1,
	maxDeathRecords = getConfigValue('maxDeathRecords'),
	limit = ""
}
if(config.deathAssistCount > 0) then
	config.limit = " LIMIT 0, " .. config.deathAssistCount
end

function onSay(cid, words, param, channel)
	local target = db.getResult("SELECT `name`, `id` FROM `players` WHERE `name` = " .. db.escapeString(param) .. ";")
	if(target:getID() == -1) then
		doPlayerSendCancel(cid, "A player with that name does not exist.")
		return true
	end

	local targetName, targetId = target:getDataString("name"), target:getDataInt("id")
	target:free()

	local str, deaths = "", db.getResult("SELECT `id`, `date`, `level` FROM `player_deaths` WHERE `player_id` = " .. targetId .." ORDER BY `date` DESC LIMIT 0, " .. config.maxDeathRecords)
	if(deaths:getID() ~= -1) then
		repeat
			local killers = db.getResult("SELECT environment_killers.name AS monster_name, players.name AS player_name FROM killers LEFT JOIN environment_killers ON killers.id = environment_killers.kill_id LEFT JOIN player_killers ON killers.id = player_killers.kill_id LEFT JOIN players ON players.id = player_killers.player_id WHERE killers.death_id = " .. deaths:getDataInt("id") .. " ORDER BY killers.final_hit DESC, killers.id ASC" .. config.limit)
			if(killers:getID() ~= -1) then
				if(str ~= "") then
					str = str .. "\n" .. os.date("%d %B %Y %X ", deaths:getDataLong("date"))
				else
					str = os.date("%d %B %Y %X ", deaths:getDataLong("date"))
				end

				local count, i = killers:getRows(false), 0
				repeat
					local monster = killers:getDataString("monster_name")
					if(i == 0 or i == (count - 1)) then
						monster = string.gsub(monster:gsub("an ", ""), "a ", "")
					end

					if(killers:getDataString("player_name") ~= "") then
						if(i == 0) then
							str = str .. "Killed at level " .. deaths:getDataInt("level") .. " by:\n  "
						elseif(i == count) then
							str = str .. " and by "
						elseif(i % 4 == 0) then
							str = str .. ",\n  "
						else
							str = str .. ", "
						end

						if(monster ~= "") then
							str = str .. monster .. " summoned by "
						end

						str = str .. killers:getDataString("player_name")
					else
						if(i == 0) then
							str = str .. "Died at level " .. deaths:getDataInt("level") .. " by:\n  "
						elseif(i == count) then
							str = str .. " and by "
						elseif(i % 4 == 0) then
							str = str .. ",\n  "
						else
							str = str .. ", "
						end

						str = str .. monster
					end

					i = i + 1
					if(i == count) then
						str = str .. "."
					end
				until not(killers:next())
				killers:free()
			end
		until not(deaths:next())
		deaths:free()
	else
		str = "No deaths recorded."
	end

	doPlayerPopupFYI(cid, "Deathlist for player: " .. targetName .. ".\n\n" .. str)
	return true
end
