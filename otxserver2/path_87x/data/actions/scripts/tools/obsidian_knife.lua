local MINOTAUR_LEATHER, LIZARD_LEATHER, GREEN_DRAGON_LEATHER, RED_DRAGON_LEATHER, HARDENED_BONE, BEHEMOTH_FANG = 5878, 5876, 5877, 5948, 5925, 5893

local config = {
	[3090] = {25, MINOTAUR_LEATHER},
	[2871] = {25, MINOTAUR_LEATHER},
	[2866] = {25, MINOTAUR_LEATHER},
	[2876] = {25, MINOTAUR_LEATHER},
	[3104] = {25, GREEN_DRAGON_LEATHER},
	[2881] = {25, RED_DRAGON_LEATHER},
	[2931] = {25, BEHEMOTH_FANG},
	[4256] = {25, LIZARD_LEATHER},
	[4259] = {25, LIZARD_LEATHER},
	[4262] = {25, LIZARD_LEATHER},
	[11285] = {25, LIZARD_LEATHER},
	[11277] = {25, LIZARD_LEATHER},
	[11269] = {25, LIZARD_LEATHER},
	[11273] = {25, LIZARD_LEATHER},
	[11281] = {25, LIZARD_LEATHER},
	[3031] = {25, HARDENED_BONE}
}

local pumpkin_items = {
	[1] = {2683},
	[2] = {2688, 50},
	[3] = {6571},
	[4] = {6492},
	[5] = {6574},
	[6] = {6526},
	[7] = {2096},
	[8] = {9005, 20}
}

local sculpting = {
	[7441] = {randsize = 4, newid = 7442},
	[7442] = {randsize = 5, newid = 7444},
	[7444] = {randsize = 6, newid = 7445},
	[7445] = {randsize = 7, newid = 7446},
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if itemEx.itemid == 8961 then
		if getPlayerStorageValue(cid, 81279) <= 0 then
			doCreatureSay(cid, "Happy Halloween!", TALKTYPE_MONSTER)
			doSendMagicEffect(getCreaturePosition(cid), math.random(28,30))
			setPlayerStorageValue(cid, 81279, 1)
			local v = pumpkin_items[math.random(#pumpkin_items)]
			doPlayerAddItem(cid, v[1], v[2] or 1)
		else
			doCreatureSay(cid, "You already used your knife on the corpse.", TALKTYPE_MONSTER, false, cid)
		end
		return true
	end

	-- Sculpting
	local v = sculpting[itemEx.itemid]
	if v then
		if(math.random(v.randsize) == 1) then
			doTransformItem(itemEx.uid, v.newid)
		else
			doRemoveItem(itemEx.uid)
			doCreatureSay(cid, "The attempt at sculpting failed miserably.", TALKTYPE_MONSTER)
		end
		doSendMagicEffect(toPosition, CONST_ME_HITAREA)
		return true
	end

-- summons
if itemEx.actionid == 91347 then
	return doPlayerSendCancel(cid, "You cant skin a summon corpse.")
end
	-- Skinning
	v = config[itemEx.itemid]
	if not v then
		return false
	elseif math.random(100) <= v[1] then
		doPlayerAddItem(cid, v[2], 1)
		doSendMagicEffect(toPosition, CONST_ME_MAGIC_GREEN)
	else
		doSendMagicEffect(toPosition, CONST_ME_BLOCKHIT)
	end
	doTransformItem(itemEx.uid, itemEx.itemid + 1)
	doDecayItem(itemEx.uid)

	return true
end