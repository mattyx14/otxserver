local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_SOUND_RED)

local maxsummons = 2

function onCastSpell(cid, var)
	doCreatureSay(cid, "Minions! Follow my call!", TALKTYPE_ORANGE_1)
	local summoncount = getCreatureSummons(cid)
	if #summoncount < 2 then
		for i = 1, maxsummons - #summoncount do
		local e, f = math.random(-2, 2), math.random(-2, 2)
			local mid = doSummonCreature("minion of Gaz'haragoth", { x=getCreaturePosition(cid).x+e, y=getCreaturePosition(cid).y+f, z=getCreaturePosition(cid).z })
    			if mid == false then
				return false
			end
		end
	end
	return doCombat(cid, combat, var)
end