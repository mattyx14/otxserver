local config = {
    sep = '\n\n----------------------------------------------------------\n',
    file = "data/logs/bans.txt"
}
function onSay(cid, words, param)
	local file = io.open(config.file, "a+")
	if not file then
	return print("Warning: Could not open ".. config.file)
	end
    local player = Player(cid)
    if player:getAccountType() < ACCOUNT_TYPE_GOD then
        return false
    end
 
    local split = param:split(",")
    if split[1] ~= nil then
        local accountId = getAccountNumberByPlayerName(split[1])
        if accountId > 0 then
            local comment = ""
            if split[2] == nil then
                player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Usage: /ban Nick, days, reason\nFor example: /ban PlayerName, 1, Bug abuse")
                return false
            elseif isNumber(split[2]) == false then
                player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Usage: /ban Nick, days, reason\nFor example: /ban PlayerName, 1, Bug abuse")
                return false
            end
            if split[3] == nil then
			 player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Usage: /ban Nick, days, reason\nFor example: /ban PlayerName, 1, Bug abuse")
             comment = split[3]
                return false
            end
        else
            player:sendCancelMessage("Player with name " .. split[1] .. " doesn't exist.")
            return false
        end
     
        local resultId = db.storeQuery("SELECT 1 FROM `account_bans` WHERE `account_id` = " .. accountId)
        if resultId ~= false then
            result.free(resultId)
            return false
        end
     
        local targetCid = getPlayerByName(split[1])
        local timeNow = os.time()
        local queryBool = db:query("INSERT INTO `account_bans` (`account_id`, `reason`, `banned_at`, `expires_at`, `banned_by`) VALUES (" .. accountId .. "," .. "'" .. tostring(split[3]) .. "'" ..", " .. timeNow .. ", " .. timeNow + (split[2] * 86400) .. ", " .. getPlayerGUIDByName(getCreatureName(cid)) .. ")")
 
        if queryBool == true then
            if targetCid ~= false then
                doRemoveCreature(targetCid)
            end
       player:sendTextMessage(MESSAGE_EVENT_ADVANCE,"" .. split[1] .. " was banished because of " .. split[3] .. " for " .. split[2] .. " days by " .. player:getName() .. ".")
			file:write(string.format('Date [%s] |%s Banished by %s because of%s%s', os.date("%d/%m/%y %H:%M"),  split[1], player:getName(), split[3], config.sep))
			file:close()
        else
            player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Usage: /ban Nick, days, reason\nFor example: /ban PlayerName, 1, Bug abuse")
        end
    end  
end
