local msg = {
	[1] = "stayed until AFK",
	[2] = "stumbled and went base",
	[3] = "stopped in Series D of the championship",
	[4] = "turned into a nightclub carpet",
	[5] = "started moving backwards",
	[6] = "tripped over a banana peel",
	[7] = "was left without a finger",
}

function onDeath(cid, corpse, deathList, mostDamage)
	if(not isPlayer(cid)) then
		return true
	end

	if getConfigValue('displayDeathChannelMessages') ~= true then return true end
	
	local nameDamager = isPlayer(mostDamage) and getCreatureName(mostDamage) or ''

	local str = "the player "..getPlayerName(cid).." ["..getPlayerLevel(cid).."] " ..(getConfigValue('resetSystemEnable') and ("["..getPlayerResets(cid).."] ") or " ")  ..msg[math.random(1, #msg)].." after dying to: " .. (nameDamager ~= '' and (nameDamager .. "(damage)") or "")
	local attacker = ''
	local monster = false
	for _, target in ipairs(deathList) do
		if isCreature(target) and isPlayer(target) and getCreatureName(target) ~=  nameDamager then	
			attacker = attacker..getCreatureName(target).." ["..getPlayerLevel(target).."] "..(getConfigValue('resetSystemEnable') and ("["..getPlayerResets(cid).."] ") or " ")..(_ < #deathList and ", " or ".")
		elseif isCreature(target) and isMonster(target) then
			attacker = attacker..getCreatureName(target)..(_ < #deathList and ", " or ".")
		end
		if _ == 1 and isMonster(target) then
			attacker = getCreatureName(target)
			monster = true
			break
		end
	end

	str = str..(monster and ("o monster "..attacker..".") or attacker)
	for _, pid in ipairs(getPlayersOnline()) do
		doPlayerSendChannelMessage(pid, '', str, TALKTYPE_CHANNEL_ORANGE, 0xF)
	end

	return true
end
