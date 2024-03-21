function onSay(cid, words, param, channel)
	local guild = getPlayerGuildId(cid)
	if(not guild or getPlayerGuildLevel(cid) < GUILDLEVEL_LEADER) then
		doPlayerSendChannelMessage(cid, "", "You cannot execute this talkaction.", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
		return true
	end

	local t = string.explode(param, ",")
	if(not t[2]) then
		doPlayerSendChannelMessage(cid, "", "Not enough param(s).", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
		return true
	end

	local enemy = getGuildId(t[2])
	if(not enemy) then
		doPlayerSendChannelMessage(cid, "", "Guild \"" .. t[2] .. "\" does not exists.", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
		return true
	end

	if(enemy == guild) then
		doPlayerSendChannelMessage(cid, "", "You cannot perform war action on your own guild.", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
		return true
	end

	local enemyName, tmp = "", db.getResult("SELECT `name` FROM `guilds` WHERE `id` = " .. enemy)
	if(tmp:getID() ~= -1) then
		enemyName = tmp:getDataString("name")
		tmp:free()
	end

	if(isInArray({"accept", "reject", "cancel"}, t[1])) then
		local query = "`guild_id` = " .. enemy .. " AND `enemy_id` = " .. guild
		if(t[1] == "cancel") then
			query = "`guild_id` = " .. guild .. " AND `enemy_id` = " .. enemy
		end

		tmp = db.getResult("SELECT `id`, `begin`, `end`, `payment` FROM `guild_wars` WHERE " .. query .. " AND `status` = 0")
		if(tmp:getID() == -1) then
			doPlayerSendChannelMessage(cid, "", "Currently there's no pending invitation for a war with " .. enemyName .. ".", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
			return true
		end

		if(t[1] == "accept") then
			local _tmp = db.getResult("SELECT `balance` FROM `guilds` WHERE `id` = " .. guild)
			local state = _tmp:getID() < 0 or _tmp:getDataInt("balance") < tmp:getDataInt("payment")

			_tmp:free()
			if(state) then
				doPlayerSendChannelMessage(cid, "", "Your guild balance is too low to accept this invitation.", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
				return true
			end

			db.executeQuery("UPDATE `guilds` SET `balance` = `balance` - " .. tmp:getDataInt("payment") .. " WHERE `id` = " .. guild)
		end

		query = "UPDATE `guild_wars` SET "
		local msg = "accepted " .. enemyName .. " invitation to war."
		if(t[1] == "reject") then
			query = query .. "`end` = " .. os.time() .. ", `status` = 2"
			msg = "rejected " .. enemyName .. " invitation to war."
		elseif(t[1] == "cancel") then
			query = query .. "`end` = " .. os.time() .. ", `status` = 3"
			msg = "canceled invitation to a war with " .. enemyName .. "."
		else
			query = query .. "`begin` = " .. os.time() .. ", `end` = " .. (tmp:getDataInt("end") > 0 and (os.time() + ((tmp:getDataInt("begin") - tmp:getDataInt("end")) / 86400)) or 0) .. ", `status` = 1"
		end

		query = query .. " WHERE `id` = " .. tmp:getDataInt("id")
		if(t[1] == "accept") then
			doGuildAddEnemy(guild, enemy, tmp:getDataInt("id"), WAR_GUILD)
			doGuildAddEnemy(enemy, guild, tmp:getDataInt("id"), WAR_ENEMY)
		end

		tmp:free()
		db.executeQuery(query)
		doBroadcastMessage(getPlayerGuildName(cid) .. " has " .. msg, MESSAGE_EVENT_ADVANCE)
		return true
	end

	if(t[1] == "invite") then
		local str = ""
                tmp = db.getResult("SELECT `guild_id`, `status` FROM `guild_wars` WHERE `guild_id` IN (" .. guild .. "," .. enemy .. ") AND `enemy_id` IN (" .. enemy .. "," .. guild .. ") AND `status` IN (0, 1)")
                if(tmp:getID() ~= -1) then
                        if(tmp:getDataInt("status") == 0) then
                                if(tmp:getDataInt("guild_id") == guild) then
                                        str = "You have already invited " .. enemyName .. " to war."
                                else
                                        str = enemyName .. " have already invited you to war."
                                end
			else
				str = "You are already on a war with " .. enemyName .. "."
			end

			tmp:free()
		end

		if(str ~= "") then
			doPlayerSendChannelMessage(cid, "", str, TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
			return true
		end

		local frags = tonumber(t[3])
		if(frags ~= nil) then
			frags = math.max(10, math.min(1000, frags))
		else
			frags = 100
		end

		local payment = tonumber(t[4])
		if(payment ~= nil) then
			payment = math.max(100000, math.min(1000000000, payment))
			tmp = db.getResult("SELECT `balance` FROM `guilds` WHERE `id` = " .. guild)

			local state = tmp:getID() < 0 or tmp:getDataInt("balance") < payment
			tmp:free()
			if(state) then
				doPlayerSendChannelMessage(cid, "", "Your guild balance is too low for such payment.", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
				return true
			end

			db.executeQuery("UPDATE `guilds` SET `balance` = `balance` - " .. payment .. " WHERE `id` = " .. guild)
		else
			payment = 0
		end

		local begining, ending = os.time(), tonumber(t[5])
		if(ending ~= nil and ending ~= 0) then
			ending = begining + (ending * 86400)
		else
			ending = 0
		end

		db.executeQuery("INSERT INTO `guild_wars` (`guild_id`, `enemy_id`, `begin`, `end`, `frags`, `payment`) VALUES (" .. guild .. ", " .. enemy .. ", " .. begining .. ", " .. ending .. ", " .. frags .. ", " .. payment .. ");")
		doBroadcastMessage(getPlayerGuildName(cid) .. " has invited " .. enemyName .. " to war till " .. frags .. " frags.", MESSAGE_EVENT_ADVANCE)
		return true
	end

	if(not isInArray({"end", "finish"}, t[1])) then
		return false
	end

	local status = (t[1] == "end" and 1 or 4)
	tmp = db.getResult("SELECT `id` FROM `guild_wars` WHERE `guild_id` = " .. guild .. " AND `enemy_id` = " .. enemy .. " AND `status` = " .. status)
	if(tmp:getID() ~= -1) then
		local query = "UPDATE `guild_wars` SET `end` = " .. os.time() .. ", `status` = 5 WHERE `id` = " .. tmp:getDataInt("id")
		tmp:free()
		doGuildRemoveEnemy(guild, enemy)
		doGuildRemoveEnemy(enemy, guild)

		db.executeQuery(query)
		doBroadcastMessage(getPlayerGuildName(cid) .. " has " .. (status == 4 and "mend fences" or "ended up a war") .. " with " .. enemyName .. ".", MESSAGE_EVENT_ADVANCE)
		return true
	end

	if(status == 4) then
		doPlayerSendChannelMessage(cid, "", "Currently there's no pending war truce from " .. enemyName .. ".", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
		return true
	end

	tmp = db.getResult("SELECT `id`, `end` FROM `guild_wars` WHERE `guild_id` = " .. enemy .. " AND `enemy_id` = " .. guild .. " AND `status` = 1")
	if(tmp:getID() ~= -1) then
		if(tmp:getDataInt("end") > 0) then
			tmp:free()
			doPlayerSendChannelMessage(cid, "", "You cannot request ending for war with " .. enemyName .. ".", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
			return true
		end

		local query = "UPDATE `guild_wars` SET `status` = 4, `end` = " .. os.time() .. " WHERE `id` = " .. tmp:getDataInt("id")
		tmp:free()

		db.executeQuery(query)
		doBroadcastMessage(getPlayerGuildName(cid) .. " has signed an armstice declaration on a war with " .. enemyName .. ".", MESSAGE_EVENT_ADVANCE)
		return true
	end

	doPlayerSendChannelMessage(cid, "", "Currently there's no active war with " .. enemyName .. ".", TALKTYPE_CHANNEL_HIGHLIGHT, CHANNEL_GUILD)
	return true
end