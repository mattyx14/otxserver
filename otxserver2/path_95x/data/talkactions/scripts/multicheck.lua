function onSay(cid, words, param, channel)
	local _ip = nil
	if(param ~= '') then
		_ip = tonumber(param)
		if(not _ip or _ip == 0) then
			local revertIp = doRevertIp(param)
			if(not revertIp) then
				local tid = getPlayerByNameWildcard(param)
				if(not tid) then
					_ip = nil
				else
					_ip = getPlayerIp(tid)
					if(_ip == 0) then
						_ip = nil
					end
				end
			else
				_ip = doConvertIpToInteger(revertIp)
			end
		end
	end

	local list, ips, players = {}, {}, getPlayersOnline()
	for i, pid in ipairs(players) do
		local ip = getPlayerIp(pid)
		if(ip ~= 0) then
			local tmp = table.find(ips, ip)
			if(tmp ~= nil and (not _ip or _ip == ip)) then
				if(table.countElements(list, ip) == 0) then
					list[players[tmp]] = ip
				end

				list[pid] = ip
			end

			table.insert(ips, ip)
		end
	end

	local total = table.maxn(list)
	if(total > 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Currently online players with same IP address(es):")
		for pid, ip in pairs(list) do
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, getCreatureName(pid) .. " (" .. doConvertIntegerToIp(ip) .. ")")
		end
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Total: " .. total .. " clients with more than one connection.")
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Currently there aren't any players with same IP address(es).")
	end

	return true
end
