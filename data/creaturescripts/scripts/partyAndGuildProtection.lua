function onCombat(cid, target)
	if(isPlayer(cid) and isPlayer(target)) then
		if(getConfigValue("noDamageToGuildMates")) then
			if isPlayer(target) and getPlayerGuildId(cid) ~= 0 and getPlayerGuildId(cid) == getPlayerGuildId(target) then
				return false
			end
		end
		if(getConfigValue("noDamageToPartyMembers")) then
			if isInParty(cid) and isInParty(target) and (getPlayerParty(cid) == getPlayerParty(target)) then
				return false
			end
		end
	end

	return true
end
