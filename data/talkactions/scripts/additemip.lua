function onSay(cid, words, param, channel)

    local t = string.explode(param, ",")
    if t[1] ~= nil and t[2] ~= nil then
        doBroadcastMessage(getPlayerName(cid) .. " Acabou de dar: " .. t[2] .." ".. getItemNameById(t[1]) .. " a todos players online!")
        local list = {}
        for i, player in ipairs(getPlayersOnline()) do
            local playerIp = getPlayerIp(player)
            if not list[tostring(playerIp)] then
				list[tostring(playerIp)] = player
            end
        end
        if next(list) then
            for ip, pid in pairs(list) do
                if isPlayer(pid) then
                    doPlayerAddItem(pid, t[1], t[2])
                end
            end
        end
    else
        doPlayerPopupFYI(cid, "No param...\nSend:\n /additemip itemid, how_much_items\nexample:\n /additemip 2160, 10")
    end
    return true
end