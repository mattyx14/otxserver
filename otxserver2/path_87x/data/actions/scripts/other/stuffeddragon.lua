--
-- Stuffed Dragon (sid: 5791)
--
-- TODO:
--	Make "You... will.... burn!!" more rare.
--

local SOUNDS = {"Fchhhhhh!", "Zchhhhhh!", "Grooaaaaar*cough*", "Aaa... CHOO!", "You... will.... burn!!"}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local random = math.random(1, table.maxn(SOUNDS))
	if(fromPosition.x ~= CONTAINER_POSITION) then
		doCreatureSay(cid, SOUNDS[random], TALKTYPE_MONSTER, false, 0, fromPosition)
	else
		doCreatureSay(cid, SOUNDS[random], TALKTYPE_MONSTER)
	end

	if(random == 5) then -- "You... will.... burn!!"
 		doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -1, -1, CONST_ME_EXPLOSIONHIT)
	end

	return true
end
