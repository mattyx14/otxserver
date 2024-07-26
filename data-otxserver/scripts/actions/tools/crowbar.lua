local function onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
	for actionName, actionFunction in pairs(crowbarActions) do
		if actionFunction(player, item, target, toPosition) then
			return true
		end
	end
	return true
end

local crowbar = Action()

function crowbar.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	return onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
end

crowbar:id(3304)
crowbar:register()
