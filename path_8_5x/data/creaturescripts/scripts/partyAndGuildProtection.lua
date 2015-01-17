function onCombat(cid, target)
	if(isPlayer(cid) and isPlayer(target)) then
		if(getConfigValue("noDamageToGuildMates")) then
			if(getPlayerGuildId(cid) == getPlayerGuildId(target)) then
				return false
			end
		end
		if(getConfigValue("noDamageToPartyMembers")) then
			if(getPlayerParty(cid) == getPlayerParty(target)) then
				return false
			end
		end
	end

	return true
end