function onUse(cid, item, fromPosition, itemEx, toPosition)
	local REACTION =
	{
		[{5080, 6568}] = {"Hug me!"}, -- panda teddy
		[{5669}] = {"It's not winning that matters, but winning in style.", "Today's your lucky day. Probably.", "Do not meddle in the affairs of dragons, for you are crunchy and taste good with ketchup.", "That is one stupid question.", "You'll need more rum for that.", "Do or do not. There is no try.", "You should do something you always wanted to.", "If you walk under a ladder and it falls down on you it probably means bad luck.", "Never say 'oops'. Always say 'Ah, interesting!'", "Five steps east, fourteen steps south, two steps north and seventeen steps west!"}, -- mysterious voodoo skull
		[{6512, 6567}] = {"Ho ho ho!", "Jingle bells, jingle bells...", "Have you been naughty?", "Have you been nice?", "Merry Christmas!", "Can you stop squeezing me now... I'm starting to feel a little sick."}, -- santa doll
		[{8977, 9002}] = {"Weirdo, you're a weirdo! Actually all of you are!", "Pie for breakfast, pie for lunch and pie for dinner!", "All hail the control panel!", "I own, Tibiacity owns, perfect match!", "Hug me! Feed me! Hail me!"}, -- tibiacity encyclopedia
		[{8981, 9004}] = {"It's news to me.", "News, updated as infrequently as possible!", "Extra! Extra! Read all about it!", "Fresh off the press!"}, -- golden newspaper
		[{8982, 8985}] = {"Hail TibiaNordic!", "So cold...", "Run, mammoth!"}, -- norseman doll
		[{10063, 10064}] = {"Hail " .. getCreatureName(cid) .. "! Hail Portal Tibia!", "Hauopa!", "WHERE IS MY HYDROMEL?!", "Yala Boom"}, -- epaminondas doll
		[{10107, 10108}] = {"Why so serious?", "These are not the puppets you are looking for...", "May the roleplay be with you.", "An age of roleplay, and all will know, that three hundred puppets gave their last breath to defend it!", "They stoles it! Sneaky little puppetses!", "There was a dream that was roleplay. You could only whisper it. Anything more than a whisper and it would vanish."}, -- hand puppets
		[{10318}] = {"Servant! Please call me a coach. - Yes, sir. You are a coach!", "Did your father help you do your homework? No, he did it all by himself.", "If big elephants have big trunks, do small elephants have suitcases?", "Do you want to hear a dirty joke? A white horse fell in the mud.", "What do you call a deer with no eyes? - I have no I-Deer!", "My friend said he knew a man with a wooden leg called Sam. So I asked him 'What was the name of his other leg?'", "Did you hear about the Lich who walked into frodo's bar? He ordered a cup of beer and a mup.", "If whistles are made of tin, what are fog horns made of?"}, -- the shield nevermourn
		[{10718, 10719}] = {"Hail " .. getCreatureName(cid) .. ", my friend!", "Friends forever!", "Look, how our friendship shines!", "Hail Tibiafriends!"} -- friendship amulet
	}
	local STUFFED_DRAGON =
	{
		[{0, 35}] = {"Fchhhhhh!"},
		[{35, 70}] = {"Zchhhhhh!"},
		[{70, 85}] = {"Groaaarrr.... *cough*"},
		[{85, 95}] = {"Aaa... CHOO!"},
		[{95, 100}] = {"You... will.... burn!!"}
	}
	local VOODOO_DOLL = {"You concentrate on your victim, hit the needle in the doll.......but nothing happens.", "You concentrate on your victim and hit the needle in the doll."}

	for k, v in pairs(REACTION) do
		local random = math.random(1, table.maxn(v))
		if(isInArray(k, item.itemid)) then
			doCreatureSay(cid, v[random], TALKTYPE_MONSTER, false, 0, fromPosition.x ~= CONTAINER_POSITION and fromPosition or getThingPosition(cid))
		end

		if(isInArray({5669}, item.itemid)) then
			doTransformItem(item.uid, 5670)
			doDecayItem(item.uid)
		elseif(isInArray({10063, 10064}, item.itemid) and random == 4) then
			doSendMagicEffect(fromPosition, CONST_ME_SOUND_RED)
		elseif(isInArray({10318}, item.itemid)) then
			doTransformItem(item.uid, 10364)
			doDecayItem(item.uid)
		end
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

	if(isInArray({10018}, item.itemid) and isCreature(itemEx.uid)) then
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, VOODOO_DOLL[math.random(1, #VOODOO_DOLL)])
	end

	return true
end
