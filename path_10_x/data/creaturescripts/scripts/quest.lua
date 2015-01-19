function onLook(cid, thing, position, lookDistance)
local quests = {
	2400, 2401, 2402, 2403 -- Change/add numbers to the unique ids on your quest chests.
}

local completed = {}
	if isPlayer(thing.uid) then
		for i = 1, #quests do
			if getPlayerStorageValue(thing.uid, quests[i]) > 0 then
					table.insert(completed, 1)
			end
		end
			doPlayerSetSpecialDescription(thing.uid, (getPlayerSex(thing.uid) == 0 and ".\nShe" or ".\nHe") .. " has completed ".. #completed .. "/" .. #quests .. " quests")
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, getPlayerName(thing.uid) .. " has completed " .. #completed .. "/" .. #quests .. " quests.")
		end
	return true
end