function onUse(cid, item, fromPosition, itemEx, toPosition)
	local REACTION =
	{
		[{5080, 6568}] = {"Hug me!"}, -- panda teddy
		[{5669}] = {"It's not winning that matters, but winning in style.", "Today's your lucky day. Probably.", "Do not meddle in the affairs of dragons, for you are crunchy and taste good with ketchup.", "That is one stupid question.", "You'll need more rum for that.", "Do or do not. There is no try.", "You should do something you always wanted to.", "If you walk under a ladder and it falls down on you it probably means bad luck.", "Never say 'oops'. Always say 'Ah, interesting!'", "Five steps east, fourteen steps south, two steps north and seventeen steps west!"}, -- mysterious voodoo skull
		[{6512, 6567}] = {"Ho ho ho!", "Jingle bells, jingle bells...", "Have you been naughty?", "Have you been nice?", "Merry Christmas!", "Can you stop squeezing me now... I'm starting to feel a little sick."} -- santa doll
	}
	local STUFFED_DRAGON =
	{
		[{0, 35}] = {"Fchhhhhh!"},
		[{35, 70}] = {"Zchhhhhh!"},
		[{70, 85}] = {"Groaaarrr.... *cough*"},
		[{85, 95}] = {"Aaa... CHOO!"},
		[{95, 100}] = {"You... will.... burn!!"}
	}

	for k, v in pairs(REACTION) do
		local random = math.random(1, table.maxn(v))
		if(isInArray(k, item.itemid)) then
			doCreatureSay(cid, v[random], TALKTYPE_MONSTER, false, 0, fromPosition.x ~= CONTAINER_POSITION and fromPosition or getThingPosition(cid))
		end

		if(isInArray({5669}, item.itemid)) then
			doTransformItem(item.uid, 5670)
			doDecayItem(item.uid)
	end

	if(isInArray({5791, 6566}, item.itemid)) then
		local random = math.random(1, 100)
		for k, v in pairs(STUFFED_DRAGON) do
			if(random > k[1] and random <= k[2]) then
				doCreatureSay(cid, v[1], TALKTYPE_MONSTER, false, 0, fromPosition.x ~= CONTAINER_POSITION and fromPosition or getThingPosition(cid))
			end

			if(random > 95 and random <= 100) then
				doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -1, -1, CONST_ME_EXPLOSIONHIT)
			end
		end
	end

	return true
end
