-- Add positions here for which you do not want to count frags
local areas = {
	[1] = {from = {x = 91, y = 122, z = 7}, to = {x = 98, y = 127, z = 7}},
}

local onlyKillerInArea = false
function noCountFragArea(cid, target)
	if not isCreature(cid) or not isCreature(target) then
		return true
	end

	local posKiller = getPlayerPosition(cid)
	local posTarget = getPlayerPosition(target)
	for i = 1, #areas do
		local area = areas[i]
		if isInArea(posKiller, area.from, area.to) then
			if onlyKillerInArea then
				return false
			elseif isInArea(posTarget, area.from, area.to) then
				return false
			end
		end
	end

	return true
end
