local itemTeleports = MoveEvent()

function itemTeleports.onAddItem(moveitem, tileitem, position)
	local setting = ItemTeleports[tileitem.actionid]
	if not setting then
		return true
	end

	moveitem:moveTo(setting.destination)
	setting.destination:sendMagicEffect(setting.effect)
	return true
end

itemTeleports:type("additem")
itemTeleports:id(1387, 1397, 8058, 8632)
for idRange = 27715, 27726 do
	itemTeleports:id(idRange)
end
itemTeleports:register()
