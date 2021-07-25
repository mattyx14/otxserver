local worldBoard = Action()

local communicates = {
	-- 
}

function worldBoard.onUse(player, item, fromPosition, target, toPosition, isHotkey)

for index, value in pairs(communicates) do
	if getGlobalStorageValue(value.globalStorage) > 0 then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, value.communicate)
	end
end
	return true
end


worldBoard:id(21570)
worldBoard:register()
