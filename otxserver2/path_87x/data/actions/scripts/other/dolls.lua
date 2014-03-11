local DOLLS = {
	[5080] = {"Hug me."},
	[5669] = {
		"It's not winning that matters, but winning in style.",
		"Today's your lucky day. Probably.",
		"Do not meddle in the affairs of dragons, for you are crunchy and taste good with ketchup.",
		"That is one stupid question.",
		"You'll need more rum for that.",
		"Do or do not. There is no try.",
		"You should do something you always wanted to.",
		"If you walk under a ladder and it falls down on you it probably means bad luck.",
		"Never say 'oops'. Always say 'Ah, interesting!'",
		"Five steps east, fourteen steps south, two steps north and seventeen steps west!"
	},
	[6566] = {
		"Fchhhhhh!",
		"Zchhhhhh!",
		"Grooaaaaar*cough*",
		"Aaa... CHOO!",
		"You... will.... burn!!"
	},
	[6388] = {"Merry Christmas "},
	[6512] = {
		"Ho ho ho",
		"Jingle bells, jingle bells...",
		"Have you been naughty?",
		"Have you been nice?",
		"Merry Christmas!",
		"Can you stop squeezing me now... I'm starting to feel a little sick."
	},
	[8974] = {"ARE YOU PREPARED TO FACE YOUR DESTINY?"},
	[8977] = {
		"Weirdo, you're a weirdo! Actually all of you are!",
		"Pie for breakfast, pie for lunch and pie for dinner!",
		"All hail the control panel!",
		"I own, Tibiacity owns, perfect match!",
		"Hug me! Feed me! Hail me!"
	},
	[8981] = {
		"It's news to me.",
		"News, updated as infrequently as possible!",
		"Extra! Extra! Read all about it!",
		"Fresh off the press!"
	},
	[8982] = {
		"Hail TibiaNordic!",
		"So cold..",
		"Run, mammoth!"
	}
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local doll = DOLLS[item.itemid]
	if(doll == nil) then
		return false
	end

	if(fromPosition.x == CONTAINER_POSITION) then
		fromPosition = getThingPosition(cid)
	end

	local random = math.random(1, table.maxn(doll))
	local sound = doll[random]
	if(item.itemid == 6566) then
		if(random == 3) then
			doSendMagicEffect(fromPosition, CONST_ME_POFF)
		elseif(random == 4) then
			doSendMagicEffect(fromPosition, CONST_ME_FIREAREA)
		elseif(random == 5) then
			doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -1, -1, CONST_ME_EXPLOSIONHIT)
		end
	elseif(item.itemid == 5669) then
		doSendMagicEffect(fromPosition, CONST_ME_MAGIC_RED)
		doTransformItem(item.uid, item.itemid + 1)
		doDecayItem(item.uid)
	elseif(item.itemid == 6388) then
		doSendMagicEffect(fromPosition, CONST_ME_SOUND_YELLOW)
		sound = sound .. getCreatureName(cid) .. "."
	end

	doCreatureSay(cid, sound, TALKTYPE_MONSTER, false, 0, fromPosition)
	return true
end
