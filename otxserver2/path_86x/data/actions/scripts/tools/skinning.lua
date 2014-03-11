local SKINS = {
	[5908] = {
		-- Minotaurs
		[2830] = {25000, 5878},
		[2871] = {25000, 5878},
		[2866] = {25000, 5878},
		[2876] = {25000, 5878},
		[3090] = {25000, 5878},

		-- Low Class Lizards
		[4259] = {25000, 5876},
		[4262] = {25000, 5876},
		[4256] = {25000, 5876},

		-- High Class Lizards
		[11288] = {25000, 5876},
		[11280] = {25000, 5876},
		[11272] = {25000, 5876},
		[11284] = {25000, 5876},

		-- Dragons
		[3104] = {25000, 5877},
		[2844] = {25000, 5877},

		-- Dragon Lords
		[2881] = {25000, 5948},

		-- Behemoths
		[2931] = {25000, 5896},
		[2931] = {15000, 5930},

		-- Bone Beasts
		[3031] = {25000, 5925},

		-- The Mutated Pumpkin
		[8961] = { { 5000, 7487 }, { 10000, 7737 }, { 20000, 6492 }, { 30000, 8860 }, { 45000, 2683 }, { 60000, 2096 }, { 90000, 9005, 50 } }
	},
	[5942] = {
		-- Demon
		[2916] = {25000, 5906},

		-- Vampire
		[2956] = {25000, 5905}
	}
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local skin = SKINS[item.itemid][itemEx.itemid]
	if(skin == nil or getItemAttribute(itemEx.uid, "summon") ~= nil) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	local random, effect, transform = math.random(1, 100000), CONST_ME_MAGIC_GREEN, true
	if(type(skin[1]) == 'table') then
		local added = false
		for _, _skin in ipairs(skin) do
			if(random <= _skin[1]) then
				doPlayerAddItem(cid, _skin[2], _skin[3] or 1)
				added = true
				break
			end
		end

		if(not added and itemEx.itemid == 8961) then
			effect = CONST_ME_POFF
			transform = false
		end
	elseif(random <= skin[1]) then
		doPlayerAddItem(cid, skin[2], skin[3] or 1)
	else
		effect = CONST_ME_POFF
	end

	doSendMagicEffect(toPosition, effect)
	if(transform) then
		doTransformItem(itemEx.uid, itemEx.itemid + 1)
	end

	return true
end
