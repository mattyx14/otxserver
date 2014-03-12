local default = {
	length = 24 * 60 * 60,
	comment = "Dont do illegal stuff.",
	reason = 4
}

function onLogin(cid)
	registerCreatureEvent(cid, SummonKill)
	if isPlayer(cid) and getPlayerClientVersion(cid) == 872 then
		doAddPlayerBanishment(getPlayerGUID(cid), 0, default.length, default.reason, ACTION_BANISHMENT, default.comment, getPlayerGUID(cid))
	end

	return true
end