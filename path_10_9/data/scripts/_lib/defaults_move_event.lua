-- default callbacks
function defaultStepIn(creature, item, position, fromPosition)
	return true
end

function defaultStepOut(creature, item, position, fromPosition)
	return true
end

function defaultAddItem(moveitem, tileitem, pos)
	return true
end

function defaultRemoveItem(moveitem, tileitem, pos)
	return true
end

function defaultEquip(player, item, slot)
	return true
end

function defaultDeEquip(player, item, slot)
	return true
end
