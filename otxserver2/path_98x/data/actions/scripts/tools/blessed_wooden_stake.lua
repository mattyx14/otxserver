local config = {
	level = 2
}

local DUSTS = {
	-- Demons
	[2956] = {25000, 5905},

	-- Vampires
	[2916] = {25000, 5906}
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(getPlayerLevel(cid) < config.level) then
		doPlayerSendCancel(cid, "You have to be at least Level " .. config.level .. " to use this tool.")
		return true
	end

	local dust = DUSTS[itemEx.itemid]
	if(not dust) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	local random = math.random(1, 100000)
	if(random <= dust[1]) then
		doSendMagicEffect(toPosition, CONST_ME_GROUNDSHAKER)
		doPlayerAddItem(cid, dust[2], 1)
	elseif(dust[3] and random >= dust[3]) then
		doSendMagicEffect(toPosition, CONST_ME_GROUNDSHAKER)
		doPlayerAddItem(cid, dust[4], 1)
	else
		doSendMagicEffect(toPosition, CONST_ME_POFF)
	end

	doTransformItem(itemEx.uid, itemEx.itemid + 1)
	return true
end
