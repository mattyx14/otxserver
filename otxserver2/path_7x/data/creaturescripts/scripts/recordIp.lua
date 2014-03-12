function onLogin(cid)
local tmp = {playerName = getPlayerName(cid), ip = getPlayerIp(cid)}
	db.executeQuery("UPDATE `players` SET `ip` = '" .. doConvertIntegerToIp(tmp.ip) .. "' WHERE name = '"..tmp.playerName.."';")

	return true
end