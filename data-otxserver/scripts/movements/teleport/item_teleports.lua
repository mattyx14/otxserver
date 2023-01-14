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
itemTeleports:id(775, 1949, 7804, 21739, 21740, 22106, 22747, 33791)
for idRange = 25047, 25058 do
	itemTeleports:id(idRange)
end
itemTeleports:register()
