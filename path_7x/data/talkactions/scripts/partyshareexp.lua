local cfg = {
	inFightBlock = true,
	onlyLeader = true,
}

function onSay(cid, words, param, channel)
	local members = getPartyMembers(cid)
	if(not(members))then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You are not in a party.")
		return true
	end

	if(cfg.onlyLeader and not(isPartyLeader(cid)))then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Only party leader can enable/disable shared experience.")
		return true
	end

	if(cfg.inFightBlock and hasCreatureCondition(cid, CONDITION_INFIGHT))then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You need to be out of fight.")
		return true
	end

	local boolean = not(isPartySharedExperienceActive(cid))
	if(setPartySharedExperience(cid, boolean))then
		for _, pid in ipairs(members) do
			if(not(isPartyLeader(pid)))then
				doPlayerSendTextMessage(pid, MESSAGE_INFO_DESCR, "Shared Experience has been "..(boolean and "activated" or "deactivated")..".")
			end
		end
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "There was an error. Try again in few seconds.")
	end

	return true
end