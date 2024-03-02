local stage = configManager.getNumber(configKeys.FREE_QUEST_STAGE)

local questTable = {
	-- 
}

-- from Position: (33201, 31762, 1)
-- to Position: (33356, 31309, 4)
local function playerFreeQuestStart(playerId, index)
	local player = Player(playerId)
	if not player then
		return
	end

	for i = 1, 5 do
		index = index + 1
		if not questTable[index] then
			player:sendTextMessage(MESSAGE_LOOK, "Adding free quests completed.")
			player:setStorageValue(Storage.FreeQuests, stage)
			return
		end

		if player:getStorageValue(questTable[index].storage) ~= questTable[index].storageValue then
			player:setStorageValue(questTable[index].storage, questTable[index].storageValue)
		end
	end

	addEvent(playerFreeQuestStart, 500, playerId, index)
end

local freeQuests = CreatureEvent("FreeQuests")

function freeQuests.onLogin(player)
	if not configManager.getBoolean(configKeys.TOGGLE_FREE_QUEST) or player:getStorageValue(Storage.FreeQuests) == stage then
		return true
	end

	player:sendTextMessage(MESSAGE_LOOK, "Adding free acccess quests to your character.")
	addEvent(playerFreeQuestStart, 500, player:getId(), 0)
	player:addOutfit(251, 0)
	player:addOutfit(252, 0)

	return true
end

freeQuests:register()
