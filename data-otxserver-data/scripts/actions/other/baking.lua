local liquidContainers = {2524, 2873, 2874, 2875, 2876, 2877, 2879, 2880, 2881, 2882, 2885, 2893, 2901, 2902, 2903}
local millstones = {1943, 1944, 1945, 1946}
local dough = {6276, 8018}
local oven = {2535, 2537, 2539, 2541 }

local baking = Action()

function baking.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item.itemid == 3603 and isInArray(liquidContainers, target.itemid) then
		if target.type == 1 then -- water
			item:transform(item.itemid, item.type - 1)
			player:addItem(3604, 1) -- lump of dough
			target:transform(target.itemid, 0)
		elseif target.type == 9 then -- milk
			item:transform(item.itemid, item.type - 1)
			player:addItem(6276, 1) -- lump of cake dough
			target:transform(target.itemid, 0)
		end
	elseif isInArray(dough, item.itemid) then
		if target.itemid == 2535 then -- oven
			item:transform(item.itemid + 1)
		elseif target.itemid == 6574 then -- bar of chocolate
			item:transform(8018) -- lump of chocolate dough
			target:remove()
		end
	elseif isInArray(oven, target.itemid) then
		item:transform(item.itemid, item.type - 1)
		player:addItem(3600, 1) -- bread
		elseif item.itemid == 5466 and target.itemid == 3605 then -- bunch of sugar cane, bunch of wheat
		item:transform(12802) -- sugar oat
		target:remove()
	elseif isInArray(millstones, target.itemid) then
		item:transform(item.itemid, item.type - 1)
		player:addItem(3603, 1) -- flour
	else
		return false
	end
	return true
end

baking:id(3603, 3604, 3605, 6276, 8018)
baking:register()
