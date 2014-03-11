local config = {
	ingameGuilds = getBooleanFromString(getConfigValue('ingameGuildManagement'))
}

function onSay(cid, words, param, channel)
	local playerAccess, t = getPlayerAccess(cid), {}
	for i, talk in ipairs(getTalkActionList()) do
		if(not talk.hidden and playerAccess >= talk.access) then
			if(config.ingameGuilds or (talk.functionName ~= "guildjoin" and talk.functionName ~= "guildcreate")) then
				table.insert(t, talk)
			end
		end
	end

	table.sort(t, function(a, b) return a.access > b.access end)
	local lastAccess, str = -1, ""
	for i, talk in ipairs(t) do
		local line = ""
		if(lastAccess ~= talk.access) then
			if(i ~= 1) then
				line = "\n"
			end
			lastAccess = talk.access
		end
		str = str .. line .. talk.words .. "\n"
	end

	doShowTextDialog(cid, ITEM_ACTION_BOOK, str)
	return true
end
