local HORN = 2079
local EMPTY_BIRD_CAGE = 2094
local BIRD_CAGE = 2095
local DIDGERIDOO = 3952
local WAR_DRUM = 3953
local CORNUCOPIA = 2369
local GREEN_NOTES = {2070, 2071, 2072, 2073, 2075, 2076, 2078, 2332, 2364, 2367, 2368, 2370, 2372, 2374, 3951, 3953, 3957}

local GREEN_NOTES = {2070, 2071, 2073, 2075, 2076, 2078, 2367, 2374}
function onUse(cid, item, fromPosition, itemEx, toPosition)
	local random = math.random(1, 5)
	if(item.itemid == BIRD_CAGE) then
		doSendMagicEffect(fromPosition, CONST_ME_SOUND_YELLOW)
	elseif(item.itemid == DIDGERIDOO) then
		if(random == 1) then
			doSendMagicEffect(fromPosition, CONST_ME_SOUND_BLUE)
			return true
		end
	elseif(item.itemid == CORNUCOPIA) then
		for i = 1, 11 do
			doPlayerAddItem(cid, 2681)
		end

		doRemoveItem(item.uid, 1)
		doSendMagicEffect(fromPosition, CONST_ME_SOUND_YELLOW)
	else
		local effect = CONST_ME_SOUND_BLUE
		if(item.itemid == HORN) then
			effect = CONST_ME_SOUND_PURPLE
		elseif(isInArray(GREEN_NOTES, item.itemid)) then
			effect = CONST_ME_SOUND_GREEN
		end

		doSendMagicEffect(fromPosition, effect)
	end

	return true
end
