local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_BATS)

local area = createCombatArea(AREA_SQUARE1X1)
setCombatArea(combat, area)

local maxsummons = 4

function onCastSpell(cid, var)
	doCreatureSay(cid, "Out of the dark I call you, fiend in the night!", TALKTYPE_ORANGE_1)
	local summoncount = getCreatureSummons(cid)
	if #summoncount < 4 then
		for i = 1, maxsummons - #summoncount do
		local e, f = math.random(-2, 2), math.random(-2, 2)
			local mid = doSummonCreature("Nightfiend", { x=getCreaturePosition(cid).x+e, y=getCreaturePosition(cid).y+f, z=getCreaturePosition(cid).z })
    			if mid == false then
				return false
			end
			doConvinceCreature(cid, mid)
		end
	end
	return doCombat(cid, combat, var)
end