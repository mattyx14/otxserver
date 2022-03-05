local internalNpcName = "Wendy"
local npcType = Game.createNpcType(internalNpcName)
local npcConfig = {}

npcConfig.name = internalNpcName
npcConfig.description = internalNpcName

npcConfig.health = 100
npcConfig.maxHealth = npcConfig.health
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 2

npcConfig.outfit = {
	lookType = 139, 
	lookHead = 132, 
	lookBody = 79, 
	lookLegs = 97, 
	lookFeet = 132, 
	lookAddons = 3
}

npcConfig.flags = {
	floorchange = false
}

npcConfig.voices = {
	interval = 15000, 
	chance = 50, 
	{text = 'I buy swords, clubs, axes, helmets, boots, legs, shields and armors.'}
}

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)

npcType.onThink = function(npc, interval)
	npcHandler:onThink(npc, interval)
end

npcType.onAppear = function(npc, creature)
	npcHandler:onAppear(npc, creature)
end

npcType.onDisappear = function(npc, creature)
	npcHandler:onDisappear(npc, creature)
end

npcType.onMove = function(npc, creature, fromPosition, toPosition)
	npcHandler:onMove(npc, creature, fromPosition, toPosition)
end

npcType.onSay = function(npc, creature, type, message)
	npcHandler:onSay(npc, creature, type, message)
end

npcType.onCloseChannel = function(npc, creature)
	npcHandler:onCloseChannel(npc, creature)
end

keywordHandler:addKeyword({'smithy'}, StdModule.say, {npcHandler = npcHandler, text = "I am a trader. I don't buy everything, though. And not from everyone, for that matter, ask me for a {trade}."})
keywordHandler:addKeyword({'first weapons'}, StdModule.say, {npcHandler = npcHandler, text = "If need {first club}, {first sword} or {first axe}."})

local club = {
	[VOCATION.BASE_ID.KNIGHT] = 3327,
}
local sword = {
	[VOCATION.BASE_ID.KNIGHT] = 7774,
}
local axe = {
	[VOCATION.BASE_ID.KNIGHT] = 7773,
}

local function creatureSayCallback(npc, creature, type, message)
	local player = Player(creature)
	local playerId = player:getId()

	if not npcHandler:checkInteraction(npc, creature) then
		return false
	end

	local itemIdClub = club[player:getVocation():getBaseId()]
	local itemIdSword = sword[player:getVocation():getBaseId()]
	local itemIdAxe = axe[player:getVocation():getBaseId()]

	if MsgContains(message, 'knight\'s sword') then
		if player:hasOutfit(player:getSex() == PLAYERSEX_FEMALE and 139 or 131, 1) then
			npcHandler:say('You already have this outfit!', npc, creature)
			return true
		end

		if player:getStorageValue(Storage.OutfitQuest.Knight.AddonSword) < 1 then
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonSword, 1)
			npcHandler:say('Great! Simply bring me 100 Iron Ore and one Crude Iron and I will happily {forge} it for you.', npc, creature)
		elseif player:getStorageValue(Storage.OutfitQuest.Knight.AddonSword) == 1 and npcHandler:getTopic(playerId) == 1 then
			if player:getItemCount(5892) > 0 and player:getItemCount(5880) > 99 then
				player:removeItem(5892, 1)
				player:removeItem(5880, 100)
				player:addOutfitAddon(131, 1)
				player:addOutfitAddon(139, 1)
				player:setStorageValue(Storage.OutfitQuest.Knight.AddonSword, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				npcHandler:say('Alright! As a matter of fact, I have one in store. Here you go!', npc, creature)
			else
				npcHandler:say('You do not have all the required items.', npc, creature)
			end
			npcHandler:setTopic(playerId, 0)
		end
	elseif MsgContains(message, 'warrior\'s sword') then
		if player:hasOutfit(player:getSex() == PLAYERSEX_FEMALE and 142 or 134, 2) then
			npcHandler:say('You already have this outfit!', npc, creature)
			return true
		end

		if player:getStorageValue(Storage.OutfitQuest.WarriorSwordAddon) < 1 then
			player:setStorageValue(Storage.OutfitQuest.WarriorSwordAddon, 1)
			npcHandler:say('Great! Simply bring me 100 iron ore and one royal steel and I will happily {forge} it for you.', npc, creature)
		elseif player:getStorageValue(Storage.OutfitQuest.WarriorSwordAddon) == 1 and npcHandler:getTopic(playerId) == 1 then
			if player:getItemCount(5887) > 0 and player:getItemCount(5880) > 99 then
				player:removeItem(5887, 1)
				player:removeItem(5880, 100)
				player:addOutfitAddon(134, 2)
				player:addOutfitAddon(142, 2)
				player:setStorageValue(Storage.OutfitQuest.WarriorSwordAddon, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				player:addAchievementProgress('Wild Warrior', 2)
				npcHandler:say('Alright! As a matter of fact, I have one in store. Here you go!', npc, creature)
			else
				npcHandler:say('You do not have all the required items.', npc, creature)
			end
			npcHandler:setTopic(playerId, 0)
		end
	elseif MsgContains(message, 'forge') then
		npcHandler:say("What would you like me to forge for you? A {knight's sword} or a {warrior's sword}?", npc, creature)
		npcHandler:setTopic(playerId, 1)
	end

	local addonProgress = player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmet)
	if MsgContains(message, "task") then
		if not player:isPremium() then
			npcHandler:say("Sorry, but our tasks are only for premium warriors.", npc, creature)
			return true
		end

		if addonProgress < 1 then
			npcHandler:say("You mean you would like to prove that you deserve to wear such a helmet?", npc, creature)
			npcHandler:setTopic(playerId, 1)
		elseif addonProgress == 1 then
			npcHandler:say("Your current task is to bring me 100 perfect behemoth fangs, |PLAYERNAME|.", npc, creature)
		elseif addonProgress == 2 then
			npcHandler:say("Your current task is to retrieve the helmet of Ramsay the Reckless from Banuta, |PLAYERNAME|.", npc, creature)
		elseif addonProgress == 3 then
			npcHandler:say("Your current task is to obtain a flask of warrior's sweat, |PLAYERNAME|.", npc, creature)
		elseif addonProgress == 4 then
			npcHandler:say("Your current task is to bring me royal steel, |PLAYERNAME|.", npc, creature)
		elseif addonProgress == 5 then
			npcHandler:say("Please talk to Sam and tell him I sent you. \z
				I'm sure he will be glad to refine your helmet, |PLAYERNAME|.", npc, creature)
		else
			npcHandler:say("You've already completed the task and can consider yourself a mighty warrior, |PLAYERNAME|.", npc, creature)
		end

	elseif MsgContains(message, "behemoth fang") then
		if addonProgress == 1 then
			npcHandler:say("Have you really managed to fulfil the task and brought me 100 perfect behemoth fangs?", npc, creature)
			npcHandler:setTopic(playerId, 3)
		else
			npcHandler:say("You're not serious asking that, are you? They come from behemoths, of course. \z
				Unless there are behemoth rabbits. Duh.", npc, creature)
		end

	elseif MsgContains(message, "ramsay") then
		if addonProgress == 2 then
			npcHandler:say("Did you recover the helmet of Ramsay the Reckless?", npc, creature)
			npcHandler:setTopic(playerId, 4)
		else
			npcHandler:say("These pesky apes steal everything they can get their dirty hands on.", npc, creature)
		end

	elseif MsgContains(message, "sweat") then
		if addonProgress == 3 then
			npcHandler:say("Were you able to get hold of a flask with pure warrior's sweat?", npc, creature)
			npcHandler:setTopic(playerId, 5)
		else
			npcHandler:say("Warrior's sweat can be magically extracted from headgear worn by a true warrior, \z
				but only in small amounts. Djinns are said to be good at magical extractions.", npc, creature)
		end

	elseif MsgContains(message, "royal steel") then
		if addonProgress == 4 then
			npcHandler:say("Ah, have you brought the royal steel?", npc, creature)
			npcHandler:setTopic(playerId, 6)
		else
			npcHandler:say("Royal steel can only be refined by very skilled smiths.", npc, creature)
		end

	elseif npcHandler:getTopic(playerId) == 1 then
		if MsgContains(message, "yes") then
			npcHandler:say(
				{
					"Well then, listen closely. First, you will have to prove that you are a fierce and \z
						restless warrior by bringing me 100 perfect behemoth fangs. ...",
					"Secondly, please retrieve a helmet for us which has been lost a long time ago. \z
						The famous Ramsay the Reckless wore it when exploring an ape settlement. ...",
					"Third, we need a new flask of warrior's sweat. We've run out of it recently, \z
						but we need a small amount for the show battles in our arena. ...",
					"Lastly, I will have our smith refine your helmet if you bring me royal steel, an especially noble metal. ...",
					"Did you understand everything I told you and are willing to handle this task?"
				},
			npc, creature, 100)
			npcHandler:setTopic(playerId, 2)
		elseif MsgContains(message, "no") then
			npcHandler:say("Bah. Then you will have to wait for the day these helmets are sold in shops, \z
				but that will not happen before hell freezes over.", npc, creature)
			npcHandler:setTopic(playerId, 0)
		end

	elseif npcHandler:getTopic(playerId) == 2 then
		if MsgContains(message, "yes") then
			player:setStorageValue(Storage.OutfitQuest.Ref, math.max(0, player:getStorageValue(Storage.OutfitQuest.Ref)) + 1)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 1)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 1)
			npcHandler:say("Alright then. Come back to me once you have collected 100 perfect behemoth fangs.", npc, creature)
			npcHandler:setTopic(playerId, 0)
		elseif MsgContains(message, "no") then
			npcHandler:say("Would you like me to repeat the task requirements then?", npc, creature)
			npcHandler:setTopic(playerId, 1)
		end

	elseif npcHandler:getTopic(playerId) == 3 then
		if MsgContains(message, "yes") then
			if not player:removeItem(5893, 100) then
				npcHandler:say("Lying is not exactly honourable, |PLAYERNAME|. Shame on you.", npc, creature)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 2)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 2)
			player:setStorageValue(Storage.OutfitQuest.Knight.RamsaysHelmetDoor, 1)
			npcHandler:say("I'm deeply impressed, brave Knight |PLAYERNAME|. I expected nothing less from you. \z
				Now, please retrieve Ramsay's helmet.", npc, creature)
		elseif MsgContains(message, "no") then
			npcHandler:say("There is no need to rush anyway.", npc, creature)
		end
		npcHandler:setTopic(playerId, 0)

	elseif npcHandler:getTopic(playerId) == 4 then
		if MsgContains(message, "yes") then
			if not player:removeItem(3351, 1) then
				npcHandler:say("Lying is not exactly honourable, |PLAYERNAME|. Shame on you.", npc, creature)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 3)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 3)
			npcHandler:say("Good work, brave Knight |PLAYERNAME|! Even though it is damaged, \z
				it has a lot of sentimental value. Now, please bring me warrior's sweat.", npc, creature)
		elseif MsgContains(message, "no") then
			npcHandler:say("There is no need to rush anyway.", npc, creature)
		end
		npcHandler:setTopic(playerId, 0)

	elseif npcHandler:getTopic(playerId) == 5 then
		if MsgContains(message, "yes") then
			if not player:removeItem(5885, 1) then
				npcHandler:say("Lying is not exactly honourable, |PLAYERNAME|. Shame on you.", npc, creature)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 4)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 4)
			npcHandler:say("Now that is a pleasant surprise, brave Knight |PLAYERNAME|! \z
				There is only one task left now: Obtain royal steel to have your helmet refined.", npc, creature)
		elseif MsgContains(message, "no") then
			npcHandler:say("There is no need to rush anyway.", npc, creature)
		end
		npcHandler:setTopic(playerId, 0)

	elseif npcHandler:getTopic(playerId) == 6 then
		if MsgContains(message, "yes") then
			if not player:removeItem(5887, 1) then
				npcHandler:say("Lying is not exactly honourable, |PLAYERNAME|. Shame on you.", npc, creature)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 5)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 5)
			npcHandler:say("You truly deserve to wear an adorned helmet, brave Knight |PLAYERNAME|. \z
				Please talk to Sam and tell him I sent you. I'm sure he will be glad to refine your helmet.", npc, creature)
		elseif MsgContains(message, "no") then
			npcHandler:say("There is no need to rush anyway.", npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	end

	if MsgContains(message, 'adorn')
			or MsgContains(message, 'outfit')
			or MsgContains(message, 'addon') then
		local addonProgress = player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmet)
		if addonProgress == 5 then
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 6)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 6)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmetTimer, os.time() + 7200)
			npcHandler:say('Oh, I see. It will be my pleasure to adorn your helmet. Please give me some time to finish it.', npc, creature)
		elseif addonProgress == 6 then
			if player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmetTimer) < os.time() then
				player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 0)
				player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 7)
				player:setStorageValue(Storage.OutfitQuest.Ref, math.min(0, player:getStorageValue(Storage.OutfitQuest.Ref) - 1))
				player:addOutfitAddon(131, 2)
				player:addOutfitAddon(139, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				npcHandler:say('Just in time, |PLAYERNAME|. Your helmet is finished, I hope you like it.', npc, creature)
			else
				npcHandler:say('Please have some patience, |PLAYERNAME|. Forging is hard work!', npc, creature)
			end
		elseif addonProgress == 7 then
			npcHandler:say('I think it\'s one of my masterpieces.', npc, creature)
		else
			npcHandler:say('Sorry, I cannot help you with this matter.', npc, creature)
		end
	elseif MsgContains(message, "no") then
		if npcHandler:getTopic(playerId) == 1 then
			npcHandler:say("Then no.", npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	end

	if MsgContains(message, 'first club') then
		if player:isKnight() then
			if player:getStorageValue(Storage.FirstQuest.FirstWeaponClub) == -1 then
				npcHandler:say('You ask me you begin your adventure with the {'.. ItemType(itemIdClub):getName() ..'}, repeat the name ok... ', npc, creature)
				npcHandler:setTopic(playerId, 8)
			else
				npcHandler:say('What? I have already gave you one {' .. ItemType(itemIdClub):getName() .. '}!', npc, creature)
			end
		else
			npcHandler:say('Sorry, you aren\'t a knight.', npc, creature)
		end
	elseif MsgContains(message, 'daramian mace') then
		if npcHandler:getTopic(playerId) == 8 then
			player:addItem(itemIdClub, 1)
			player:setStorageValue(Storage.FirstQuest.FirstWeaponClub, 1)
			npcHandler:say('Here you are young adept, take care yourself.', npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	elseif MsgContains(message, "no") and npcHandler:getTopic(playerId) == 8 then
		npcHandler:say("Ok then.", npc, creature)
		npcHandler:setTopic(playerId, 0)
	end

	if MsgContains(message, 'first sword') then
		if player:isKnight() then
			if player:getStorageValue(Storage.FirstQuest.FirstWeaponSword) == -1 then
				npcHandler:say('You ask me you begin your adventure with the {'.. ItemType(itemIdSword):getName() ..'}, repeat the name ok... ', npc, creature)
				npcHandler:setTopic(playerId, 9)
			else
				npcHandler:say('What? I have already gave you one {' .. ItemType(itemIdSword):getName() .. '}!', npc, creature)
			end
		else
			npcHandler:say('Sorry, you aren\'t a knight.', npc, creature)
		end
	elseif MsgContains(message, 'jagged sword') then
		if npcHandler:getTopic(playerId) == 9 then
			player:addItem(itemIdSword, 1)
			player:setStorageValue(Storage.FirstQuest.FirstWeaponSword, 1)
			npcHandler:say('Here you are young adept, take care yourself.', npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	elseif MsgContains(message, "no") and npcHandler:getTopic(playerId) == 9 then
		npcHandler:say("Ok then.", npc, creature)
		npcHandler:setTopic(playerId, 0)
	end

	if MsgContains(message, 'first axe') then
		if player:isKnight() then
			if player:getStorageValue(Storage.FirstQuest.FirstWeaponAxe) == -1 then
				npcHandler:say('You ask me you begin your adventure with the {'.. ItemType(itemIdAxe):getName() ..'}, repeat the name ok... ', npc, creature)
				npcHandler:setTopic(playerId, 10)
			else
				npcHandler:say('What? I have already gave you one {' .. ItemType(itemIdAxe):getName() .. '}!', npc, creature)
			end
		else
			npcHandler:say('Sorry, you aren\'t a knight.', npc, creature)
		end
	elseif MsgContains(message, 'steel axe') then
		if npcHandler:getTopic(playerId) == 10 then
			player:addItem(itemIdAxe, 1)
			player:setStorageValue(Storage.FirstQuest.FirstWeaponAxe, 1)
			npcHandler:say('Here you are young adept, take care yourself.', npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	elseif MsgContains(message, "no") and npcHandler:getTopic(playerId) == 10 then
		npcHandler:say("Ok then.", npc, creature)
		npcHandler:setTopic(playerId, 0)
	end
	return true
end

keywordHandler:addKeyword(
	{'addon'},
	StdModule.say,
	{
		npcHandler = npcHandler,
		text = 'I can forge the finest {weapons} for knights and warriors. \
		They may wear them proudly and visible to everyone.'
	}
)
keywordHandler:addKeyword(
	{'weapons'},
	StdModule.say,
	{
		npcHandler = npcHandler,
		text = "Would you rather be interested in a {knight's sword} or in a {warrior's sword}?"
	}
)

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:setMessage(MESSAGE_GREET, "Ah, a customer! Be greeted, |PLAYERNAME|! Welcome to my {smithy} or do you need {first weapons} for knights.")
npcHandler:setMessage(MESSAGE_FAREWELL, "Farewell, |PLAYERNAME|, may the winds guide your way.")
npcHandler:setMessage(MESSAGE_WALKAWAY, "Come back soon!")
npcHandler:setMessage(MESSAGE_SENDTRADE, "Take all the time you need to decide what you want!")
npcHandler:addModule(FocusModule:new())

npcConfig.shop = {
	-- Great Items
	{ itemName = "abyss hammer", clientId = 7414, sell = 10000 },
	{ itemName = "albino plate", clientId = 19358, sell = 750 },
	{ itemName = "amber staff", clientId = 7426, sell = 400 },
	{ itemName = "ancient amulet", clientId = 3025, sell = 100 },
	{ itemName = "assassin dagger", clientId = 7404, sell = 10000 },
	{ itemName = "bandana", clientId = 5917, sell = 75 },
	{ itemName = "beastslayer axe", clientId = 3344, sell = 750 },
	{ itemName = "beetle necklace", clientId = 10457, sell = 750 },
	{ itemName = "berserker", clientId = 7403, sell = 20000 },
	{ itemName = "blacksteel sword", clientId = 7406, sell = 3000 },
	{ itemName = "blessed sceptre", clientId = 7429, sell = 20000 },
	{ itemName = "bone shield", clientId = 3441, sell = 40 },
	{ itemName = "bonelord helmet", clientId = 3408, sell = 3750 },
	{ itemName = "brutetamer's staff", clientId = 7379, sell = 750 },
	{ itemName = "buckle", clientId = 17829, sell = 3500 },
	{ itemName = "castle shield", clientId = 3435, sell = 2500 },
	{ itemName = "chain bolter", clientId = 8022, sell = 20000 },
	{ itemName = "chaos mace", clientId = 7427, sell = 4500 },
	{ itemName = "cobra crown", clientId = 11674, sell = 25000 },
	{ itemName = "coconut shoes", clientId = 9017, sell = 250 },
	{ itemName = "composite hornbow", clientId = 8027, sell = 12500 },
	{ itemName = "cranial basher", clientId = 7415, sell = 15000 },
	{ itemName = "crocodile boots", clientId = 3556, sell = 500 },
	{ itemName = "crystal crossbow", clientId = 16163, sell = 17500 },
	{ itemName = "crystal mace", clientId = 3333, sell = 6000 },
	{ itemName = "crystal necklace", clientId = 3008, sell = 200 },
	{ itemName = "crystal ring", clientId = 6093, sell = 125 },
	{ itemName = "crystal sword", clientId = 7449, sell = 300 },
	{ itemName = "crystalline armor", clientId = 8050, sell = 8000 },
	{ itemName = "daramian mace", clientId = 3327, sell = 55 },
	{ itemName = "daramian waraxe", clientId = 3328, sell = 500 },
	{ itemName = "dark shield", clientId = 3421, sell = 200 },
	{ itemName = "death ring", clientId = 6299, sell = 500 },
	{ itemName = "demon shield", clientId = 3420, sell = 15000 },
	{ itemName = "demonbone amulet", clientId = 3019, sell = 16000 },
	{ itemName = "demonrage sword", clientId = 7382, sell = 18000 },
	{ itemName = "devil helmet", clientId = 3356, sell = 500 },
	{ itemName = "diamond sceptre", clientId = 7387, sell = 1500 },
	{ itemName = "divine plate", clientId = 8057, sell = 22500 },
	{ itemName = "djinn blade", clientId = 3339, sell = 7500 },
	{ itemName = "doll", clientId = 2991, sell = 100 },
	{ itemName = "dragon scale mail", clientId = 3386, sell = 20000 },
	{ itemName = "dragon slayer", clientId = 7402, sell = 7500 },
	{ itemName = "dragonbone staff", clientId = 7430, sell = 1500 },
	{ itemName = "dwarven armor", clientId = 3397, sell = 15000 },
	{ itemName = "elvish bow", clientId = 7438, sell = 1000 },
	{ itemName = "emerald bangle", clientId = 3010, sell = 400 },
	{ itemName = "epee", clientId = 3326, sell = 4000 },
	{ itemName = "flower dress", clientId = 9015, sell = 500 },
	{ itemName = "flower wreath", clientId = 9013, sell = 250 },
	{ itemName = "fur boots", clientId = 7457, sell = 1000 },
	{ itemName = "furry club", clientId = 7432, sell = 500 },
	{ itemName = "glacier amulet", clientId = 815, sell = 750 },
	{ itemName = "glacier kilt", clientId = 823, sell = 5500 },
	{ itemName = "glacier mask", clientId = 829, sell = 1250 },
	{ itemName = "glacier robe", clientId = 824, sell = 5500 },
	{ itemName = "glacier shoes", clientId = 819, sell = 1250 },
	{ itemName = "gold ring", clientId = 3063, sell = 4000 },
	{ itemName = "golden armor", clientId = 3360, sell = 10000 },
	{ itemName = "golden legs", clientId = 3364, sell = 15000 },
	{ itemName = "goo shell", clientId = 19372, sell = 2000 },
	{ itemName = "griffin shield", clientId = 3433, sell = 1500 },
	{ itemName = "guardian halberd", clientId = 3315, sell = 5500 },
	{ itemName = "hammer of wrath", clientId = 3332, sell = 15000 },
	{ itemName = "headchopper", clientId = 7380, sell = 3000 },
	{ itemName = "heavy mace", clientId = 3340, sell = 25000 },
	{ itemName = "heavy machete", clientId = 3330, sell = 45 },
	{ itemName = "heavy trident", clientId = 12683, sell = 1000 },
	{ itemName = "helmet of the lost", clientId = 17852, sell = 1000 },
	{ itemName = "heroic axe", clientId = 7389, sell = 15000 },
	{ itemName = "hibiscus dress", clientId = 8045, sell = 1500 },
	{ itemName = "hieroglyph banner", clientId = 12482, sell = 250 },
	{ itemName = "horn", clientId = 19359, sell = 150 },
	{ itemName = "jade hammer", clientId = 7422, sell = 12500 },
	{ itemName = "krimhorn helmet", clientId = 7461, sell = 100 },
	{ itemName = "lavos armor", clientId = 8049, sell = 8000 },
	{ itemName = "leaf legs", clientId = 9014, sell = 250 },
	{ itemName = "leopard armor", clientId = 3404, sell = 500 },
	{ itemName = "leviathan's amulet", clientId = 9303, sell = 1500 },
	{ itemName = "light shovel", clientId = 5710, sell = 150 },
	{ itemName = "lightning boots", clientId = 820, sell = 1250 },
	{ itemName = "lightning headband", clientId = 828, sell = 1250 },
	{ itemName = "lightning legs", clientId = 822, sell = 5500 },
	{ itemName = "lightning pendant", clientId = 816, sell = 750 },
	{ itemName = "lightning robe", clientId = 825, sell = 5500 },
	{ itemName = "lunar staff", clientId = 7424, sell = 2500 },
	{ itemName = "magic plate armor", clientId = 3366, sell = 45000 },
	{ itemName = "magma amulet", clientId = 817, sell = 750 },
	{ itemName = "magma boots", clientId = 818, sell = 1250 },
	{ itemName = "magma coat", clientId = 826, sell = 5500 },
	{ itemName = "magma legs", clientId = 821, sell = 5500 },
	{ itemName = "magma monocle", clientId = 827, sell = 1250 },
	{ itemName = "mammoth fur cape", clientId = 7463, sell = 3000 },
	{ itemName = "mammoth fur shorts", clientId = 7464, sell = 400 },
	{ itemName = "mammoth whopper", clientId = 7381, sell = 150 },
	{ itemName = "mastermind shield", clientId = 3414, sell = 25000 },
	{ itemName = "medusa shield", clientId = 3436, sell = 4500 },
	{ itemName = "mercenary sword", clientId = 7386, sell = 6000 },
	{ itemName = "model ship", clientId = 2994, sell = 500 },
	{ itemName = "mycological bow", clientId = 16164, sell = 17500 },
	{ itemName = "mystic blade", clientId = 7384, sell = 15000 },
	{ itemName = "naginata", clientId = 3314, sell = 1000 },
	{ itemName = "nightmare blade", clientId = 7418, sell = 17500 },
	{ itemName = "noble axe", clientId = 7456, sell = 5000 },
	{ itemName = "norse shield", clientId = 7460, sell = 750 },
	{ itemName = "onyx pendant", clientId = 22195, sell = 1750 },
	{ itemName = "orcish maul", clientId = 7392, sell = 3000 },
	{ itemName = "oriental shoes", clientId = 21981, sell = 7500 },
	{ itemName = "pair of iron fists", clientId = 17828, sell = 2000 },
	{ itemName = "paladin armor", clientId = 8063, sell = 7500 },
	{ itemName = "patched boots", clientId = 3550, sell = 1000 },
	{ itemName = "pharaoh banner", clientId = 12483, sell = 500 },
	{ itemName = "pharaoh sword", clientId = 3334, sell = 11500 },
	{ itemName = "pirate boots", clientId = 5461, sell = 1500 },
	{ itemName = "pirate hat", clientId = 6096, sell = 500 },
	{ itemName = "pirate knee breeches", clientId = 5918, sell = 100 },
	{ itemName = "pirate shirt", clientId = 6095, sell = 250 },
	{ itemName = "pirate voodoo doll", clientId = 5810, sell = 250 },
	{ itemName = "platinum amulet", clientId = 3055, sell = 1250 },
	{ itemName = "ragnir helmet", clientId = 7462, sell = 200 },
	{ itemName = "relic sword", clientId = 7383, sell = 12500 },
	{ itemName = "rift bow", clientId = 22866, sell = 22500 },
	{ itemName = "rift crossbow", clientId = 22867, sell = 22500 },
	{ itemName = "rift lance", clientId = 22727, sell = 15000 },
	{ itemName = "rift shield", clientId = 22726, sell = 25000 },
	{ itemName = "ring of the sky", clientId = 3006, sell = 15000 },
	{ itemName = "royal axe", clientId = 7434, sell = 20000 },
	{ itemName = "ruby necklace", clientId = 3016, sell = 1000 },
	{ itemName = "ruthless axe", clientId = 6553, sell = 22500 },
	{ itemName = "sacred tree amulet", clientId = 9302, sell = 1500 },
	{ itemName = "sapphire hammer", clientId = 7437, sell = 3500 },
	{ itemName = "scarab amulet", clientId = 3018, sell = 100 },
	{ itemName = "scarab shield", clientId = 3440, sell = 1000 },
	{ itemName = "shockwave amulet", clientId = 9304, sell = 1500 },
	{ itemName = "silver brooch", clientId = 3017, sell = 75 },
	{ itemName = "silver dagger", clientId = 3290, sell = 250 },
	{ itemName = "skull helmet", clientId = 5741, sell = 20000 },
	{ itemName = "skullcracker armor", clientId = 8061, sell = 9000 },
	{ itemName = "spiked squelcher", clientId = 7452, sell = 2500 },
	{ itemName = "steel boots", clientId = 3554, sell = 15000 },
	{ itemName = "swamplair armor", clientId = 8052, sell = 8000 },
	{ itemName = "taurus mace", clientId = 7425, sell = 250 },
	{ itemName = "tempest shield", clientId = 3442, sell = 17500 },
	{ itemName = "terra amulet", clientId = 814, sell = 750 },
	{ itemName = "terra boots", clientId = 813, sell = 1250 },
	{ itemName = "terra hood", clientId = 830, sell = 1250 },
	{ itemName = "terra legs", clientId = 812, sell = 5500 },
	{ itemName = "terra mantle", clientId = 811, sell = 5500 },
	{ itemName = "the justice seeker", clientId = 7390, sell = 20000 },
	{ itemName = "tortoise shield", clientId = 6131, sell = 75 },
	{ itemName = "vile axe", clientId = 7388, sell = 15000 },
	{ itemName = "voodoo doll", clientId = 3002, sell = 200 },
	{ itemName = "war axe", clientId = 3342, sell = 6000 },
	{ itemName = "war horn", clientId = 2958, sell = 4000 },
	{ itemName = "witch hat", clientId = 9653, sell = 2500 },
	{ itemName = "wyvern fang", clientId = 7408, sell = 750 },

	-- Simple Items
	{ itemName = "axe", clientId = 3274, buy = 20, sell = 7 },
	{ itemName = "barbarian axe", clientId = 3317, buy = 590, sell = 185 },
	{ itemName = "battle axe", clientId = 3266, buy = 235, sell = 80 },
	{ itemName = "battle hammer", clientId = 3305, buy = 350, sell = 120 },
	{ itemName = "battle shield", clientId = 3413, sell = 95 },
	{ itemName = "bone club", clientId = 3337, sell = 5 },
	{ itemName = "bone sword", clientId = 3338, buy = 75, sell = 20 },
	{ itemName = "brass armor", clientId = 3359, buy = 450, sell = 150 },
	{ itemName = "brass helmet", clientId = 3354, buy = 120, sell = 30 },
	{ itemName = "brass legs", clientId = 3372, buy = 195, sell = 49 },
	{ itemName = "brass shield", clientId = 3411, buy = 65, sell = 25 },
	{ itemName = "carlin sword", clientId = 3283, buy = 473, sell = 118 },
	{ itemName = "chain armor", clientId = 3358, buy = 200, sell = 70 },
	{ itemName = "chain helmet", clientId = 3352, buy = 52, sell = 17 },
	{ itemName = "chain legs", clientId = 3558, buy = 80, sell = 25 },
	{ itemName = "clerical mace", clientId = 3311, buy = 540, sell = 170 },
	{ itemName = "club", clientId = 3270, buy = 5, sell = 1 },
	{ itemName = "coat", clientId = 3562, buy = 8, sell = 1 },
	{ itemName = "copper shield", clientId = 3430, sell = 50 },
	{ itemName = "crimson sword", clientId = 7385, buy = 610 },
	{ itemName = "crowbar", clientId = 3304, buy = 260, sell = 50 },
	{ itemName = "dagger", clientId = 3267, buy = 5, sell = 2 },
	{ itemName = "double axe", clientId = 3275, sell = 260 },
	{ itemName = "doublet", clientId = 3379, buy = 16, sell = 3 },
	{ itemName = "durable exercise axe", clientId = 35280, buy = 945000, count = 1800 },
	{ itemName = "durable exercise club", clientId = 35281, buy = 945000, count = 1800 },
	{ itemName = "durable exercise sword", clientId = 35279, buy = 945000, count = 1800 },
	{ itemName = "dwarven shield", clientId = 3425, buy = 500, sell = 100 },
	{ itemName = "exercise axe", clientId = 28553, buy = 262500, count = 500 },
	{ itemName = "exercise club", clientId = 28554, buy = 262500, count = 500 },
	{ itemName = "exercise sword", clientId = 28552, buy = 262500, count = 500 },
	{ itemName = "halberd", clientId = 3269, sell = 400 },
	{ itemName = "hand axe", clientId = 3268, buy = 8, sell = 4 },
	{ itemName = "hatchet", clientId = 3276, sell = 25 },
	{ itemName = "iron helmet", clientId = 3353, buy = 390, sell = 150 },
	{ itemName = "jacket", clientId = 3561, buy = 12, sell = 1 },
	{ itemName = "katana", clientId = 3300, sell = 35 },
	{ itemName = "lasting exercise axe", clientId = 35286, buy = 7560000, count = 14400 },
	{ itemName = "lasting exercise club", clientId = 35287, buy = 7560000, count = 14400 },
	{ itemName = "lasting exercise sword", clientId = 35285, buy = 7560000, count = 14400 },
	{ itemName = "leather armor", clientId = 3361, buy = 35, sell = 12 },
	{ itemName = "leather boots", clientId = 3552, buy = 10, sell = 2 },
	{ itemName = "leather helmet", clientId = 3355, buy = 12, sell = 4 },
	{ itemName = "leather legs", clientId = 3559, buy = 10, sell = 9 },
	{ itemName = "legion helmet", clientId = 3374, sell = 22 },
	{ itemName = "longsword", clientId = 3285, buy = 160, sell = 51 },
	{ itemName = "mace", clientId = 3286, buy = 90, sell = 30 },
	{ itemName = "morning star", clientId = 3282, buy = 430, sell = 100 },
	{ itemName = "orcish axe", clientId = 3316, sell = 350 },
	{ itemName = "plate armor", clientId = 3357, buy = 1200, sell = 400 },
	{ itemName = "plate legs", clientId = 3557, sell = 115 },
	{ itemName = "plate shield", clientId = 3410, buy = 125, sell = 45 },
	{ itemName = "rapier", clientId = 3272, buy = 15, sell = 5 },
	{ itemName = "sabre", clientId = 3273, buy = 35, sell = 12 },
	{ itemName = "scale armor", clientId = 3377, buy = 260, sell = 75 },
	{ itemName = "short sword", clientId = 3294, buy = 26, sell = 10 },
	{ itemName = "sickle", clientId = 3293, buy = 7 },
	{ itemName = "small axe", clientId = 3462, sell = 5 },
	{ itemName = "soldier helmet", clientId = 3375, buy = 110, sell = 16 },
	{ itemName = "steel helmet", clientId = 3351, buy = 580, sell = 293 },
	{ itemName = "steel shield", clientId = 3409, buy = 240, sell = 80 },
	{ itemName = "studded armor", clientId = 3378, buy = 90, sell = 25 },
	{ itemName = "studded helmet", clientId = 3376, buy = 63 },
	{ itemName = "studded legs", clientId = 3362, buy = 50 },
	{ itemName = "studded shield", clientId = 3426, buy = 50 },
	{ itemName = "sword", clientId = 3264, buy = 85 },
	{ itemName = "two handed sword", clientId = 3265, buy = 950 },
	{ itemName = "viking helmet", clientId = 3367, buy = 265 },
	{ itemName = "viking shield", clientId = 3431, buy = 260 },
	{ itemName = "wooden shield", clientId = 3412, buy = 15 },

	-- Medium List Items
	{ itemName = "angelic axe", clientId = 7436, sell = 5000 },
	{ itemName = "blue robe", clientId = 3567, sell = 10000 },
	{ itemName = "bonelord shield", clientId = 3418, buy = 7000, sell = 1200 },
	{ itemName = "boots of haste", clientId = 3079, sell = 30000 },
	{ itemName = "broadsword", clientId = 3301, sell = 500 },
	{ itemName = "butcher's axe", clientId = 7412, sell = 18000 },
	{ itemName = "crown armor", clientId = 3381, sell = 12000 },
	{ itemName = "crown helmet", clientId = 3385, sell = 2500 },
	{ itemName = "crown legs", clientId = 3382, sell = 12000 },
	{ itemName = "crown shield", clientId = 3419, sell = 8000 },
	{ itemName = "crusader helmet", clientId = 3391, sell = 6000 },
	{ itemName = "dragon lance", clientId = 3302, sell = 9000 },
	{ itemName = "dragon shield", clientId = 3416, sell = 4000 },
	{ itemName = "fire axe", clientId = 3320, sell = 8000 },
	{ itemName = "fire sword", clientId = 3280, sell = 4000 },
	{ itemName = "glorious axe", clientId = 7454, sell = 3000 },
	{ itemName = "guardian shield", clientId = 3415, sell = 2000 },
	{ itemName = "ice rapier", clientId = 3284, sell = 1000 },
	{ itemName = "noble armor", clientId = 3380, buy = 8000, sell = 900 },
	{ itemName = "obsidian lance", clientId = 3313, buy = 3000, sell = 500 },
	{ itemName = "phoenix shield", clientId = 3439, sell = 16000 },
	{ itemName = "queen's sceptre", clientId = 7410, sell = 20000 },
	{ itemName = "royal helmet", clientId = 3392, sell = 30000 },
	{ itemName = "shadow sceptre", clientId = 7451, sell = 10000 },
	{ itemName = "spike sword", clientId = 3271, buy = 8000, sell = 1000 },
	{ itemName = "thaian sword", clientId = 7391, sell = 16000 },
	{ itemName = "war hammer", clientId = 3279, buy = 10000, sell = 1200 },
	-- Continue
	{ itemName = "ancient shield", clientId = 3432, buy = 5000, sell = 900 },
	{ itemName = "black shield", clientId = 3429, sell = 800 },
	{ itemName = "bonebreaker", clientId = 7428, sell = 10000 },
	{ itemName = "dark armor", clientId = 3383, buy = 1500, sell = 400 },
	{ itemName = "dark helmet", clientId = 3384, buy = 1000, sell = 250 },
	{ itemName = "dragon hammer", clientId = 3322, sell = 2000 },
	{ itemName = "dreaded cleaver", clientId = 7419, sell = 15000 },
	{ itemName = "giant sword", clientId = 3281, sell = 17000 },
	{ itemName = "haunted blade", clientId = 7407, sell = 8000 },
	{ itemName = "knight armor", clientId = 3370, sell = 5000 },
	{ itemName = "knight axe", clientId = 3318, sell = 2000 },
	{ itemName = "knight legs", clientId = 3371, sell = 5000 },
	{ itemName = "mystic turban", clientId = 3574, sell = 150 },
	{ itemName = "onyx flail", clientId = 7421, sell = 22000 },
	{ itemName = "ornamented axe", clientId = 7411, sell = 20000 },
	{ itemName = "poison dagger", clientId = 3299, sell = 50 },
	{ itemName = "scimitar", clientId = 3307, sell = 150 },
	{ itemName = "serpent sword", clientId = 3297, buy = 6000, sell = 900 },
	{ itemName = "skull staff", clientId = 3324, sell = 6000 },
	{ itemName = "strange helmet", clientId = 3373, sell = 500 },
	{ itemName = "titan axe", clientId = 7413, sell = 4000 },
	{ itemName = "tower shield", clientId = 3428, sell = 8000 },
	{ itemName = "vampire shield", clientId = 3434, sell = 15000 },
	{ itemName = "warrior helmet", clientId = 3369, sell = 5000 }
}

-- On buy npc shop message
npcType.onBuyItem = function(npc, player, itemId, subType, amount, inBackpacks, name, totalCost)
	npc:sellItem(player, itemId, amount, subType, true, inBackpacks, 2854)
	player:sendTextMessage(MESSAGE_INFO_DESCR, string.format("Bought %ix %s for %i %s.", amount, name, totalCost, ItemType(npc:getCurrency()):getPluralName():lower()))
end
-- On sell npc shop message
npcType.onSellItem = function(npc, player, clientId, subtype, amount, name, totalCost)
	player:sendTextMessage(MESSAGE_INFO_DESCR, string.format("Sold %ix %s for %i gold.", amount, name, totalCost))
end
-- On check npc shop message (look item)
npcType.onCheckItem = function(npc, player, clientId, subType)
end

npcType:register(npcConfig)
