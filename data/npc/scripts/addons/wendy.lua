local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)		end
function onThink()				npcHandler:onThink()					end

local voices = { {text = 'Courageous adventurers, come buy your weapons and armors here!'} }
npcHandler:addModule(VoiceModule:new(voices))

local function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then
		return false
	end

	local player = Player(cid)
	if msgcontains(msg, 'knight\'s sword') then
		if player:hasOutfit(player:getSex() == PLAYERSEX_FEMALE and 139 or 131, 1) then
			npcHandler:say('You already have this outfit!', cid)
			return true
		end

		if player:getStorageValue(Storage.OutfitQuest.Knight.AddonSword) < 1 then
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonSword, 1)
			npcHandler:say('Great! Simply bring me 100 Iron Ore and one Crude Iron and I will happily {forge} it for you.', cid)
		elseif player:getStorageValue(Storage.OutfitQuest.Knight.AddonSword) == 1 and npcHandler.topic[cid] == 1 then
			if player:getItemCount(5892) > 0 and player:getItemCount(5880) > 99 then
				player:removeItem(5892, 1)
				player:removeItem(5880, 100)
				player:addOutfitAddon(131, 1)
				player:addOutfitAddon(139, 1)
				player:setStorageValue(Storage.OutfitQuest.Knight.AddonSword, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				npcHandler:say('Alright! As a matter of fact, I have one in store. Here you go!', cid)
			else
				npcHandler:say('You do not have all the required items.', cid)
			end
			npcHandler.topic[cid] = 0
		end
	elseif msgcontains(msg, 'warrior\'s sword') then
		if player:hasOutfit(player:getSex() == PLAYERSEX_FEMALE and 142 or 134, 2) then
			npcHandler:say('You already have this outfit!', cid)
			return true
		end

		if player:getStorageValue(Storage.OutfitQuest.WarriorSwordAddon) < 1 then
			player:setStorageValue(Storage.OutfitQuest.WarriorSwordAddon, 1)
			npcHandler:say('Great! Simply bring me 100 iron ore and one royal steel and I will happily {forge} it for you.', cid)
		elseif player:getStorageValue(Storage.OutfitQuest.WarriorSwordAddon) == 1 and npcHandler.topic[cid] == 1 then
			if player:getItemCount(5887) > 0 and player:getItemCount(5880) > 99 then
				player:removeItem(5887, 1)
				player:removeItem(5880, 100)
				player:addOutfitAddon(134, 2)
				player:addOutfitAddon(142, 2)
				player:setStorageValue(Storage.OutfitQuest.WarriorSwordAddon, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				player:addAchievementProgress('Wild Warrior', 2)
				npcHandler:say('Alright! As a matter of fact, I have one in store. Here you go!', cid)
			else
				npcHandler:say('You do not have all the required items.', cid)
			end
			npcHandler.topic[cid] = 0
		end
	elseif msgcontains(msg, 'forge') then
		npcHandler:say('What would you like me to forge for you? A {knight\'s sword} or a {warrior\'s sword}?', cid)
		npcHandler.topic[cid] = 1
	end

	local addonProgress = player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmet)
	if msgcontains(msg, 'task') then
		if not player:isPremium() then
			npcHandler:say('Sorry, but our tasks are only for premium warriors.', cid)
			return true
		end

		if addonProgress < 1 then
			npcHandler:say('You mean you would like to prove that you deserve to wear such a helmet?', cid)
			npcHandler.topic[cid] = 1
		elseif addonProgress == 1 then
			npcHandler:say('Your current task is to bring me 100 perfect behemoth fangs, |PLAYERNAME|.', cid)
		elseif addonProgress == 2 then
			npcHandler:say('Your current task is to retrieve the helmet of Ramsay the Reckless from Anshara, |PLAYERNAME|.', cid)
		elseif addonProgress == 3 then
			npcHandler:say('Your current task is to obtain a flask of warrior\'s sweat, |PLAYERNAME|.', cid)
		elseif addonProgress == 4 then
			npcHandler:say('Your current task is to bring me royal steel, |PLAYERNAME|.', cid)
		elseif addonProgress == 5 then
			npcHandler:say('I\'m sure he will be glad to refine your helmet, |PLAYERNAME|.', cid)
		else
			npcHandler:say('You\'ve already completed the task and can consider yourself a mighty warrior, |PLAYERNAME|.', cid)
		end

	elseif msgcontains(msg, 'behemoth fang') then
		if addonProgress == 1 then
			npcHandler:say('Have you really managed to fulfil the task and brought me 100 perfect behemoth fangs?', cid)
			npcHandler.topic[cid] = 3
		else
			npcHandler:say('You\'re not serious asking that, are you? They come from behemoths, of course. Unless there are behemoth rabbits. Duh.', cid)
		end

	elseif msgcontains(msg, 'ramsay') then
		if addonProgress == 2 then
			npcHandler:say('Did you recover the helmet of Ramsay the Reckless?', cid)
			npcHandler.topic[cid] = 4
		else
			npcHandler:say('These pesky mino steal everything they can get their dirty hands on.', cid)
		end

	elseif msgcontains(msg, 'sweat') then
		if addonProgress == 3 then
			npcHandler:say('Were you able to get hold of a flask with pure warrior\'s sweat?', cid)
			npcHandler.topic[cid] = 5
		else
			npcHandler:say('Warrior\'s sweat can be magically extracted from headgear worn by a true warrior, but only in small amounts. Djinns are said to be good at magical extractions.', cid)
		end

	elseif msgcontains(msg, 'royal steel') then
		if addonProgress == 4 then
			npcHandler:say('Ah, have you brought the royal steel?', cid)
			npcHandler.topic[cid] = 6
		else
			npcHandler:say('Royal steel can only be refined by very skilled smiths.', cid)
		end

	elseif npcHandler.topic[cid] == 1 then
		if msgcontains(msg, 'yes') then
			npcHandler:say({
				'Well then, listen closely. First, you will have to prove that you are a fierce and restless warrior by bringing me 100 perfect behemoth fangs. ...',
				'Secondly, please retrieve a helmet for us which has been lost a long time ago. The famous Ramsay the Reckless wore it when exploring an mino settlement. ...',
				'Third, we need a new flask of warrior\'s sweat. We\'ve run out of it recently, but we need a small amount for the show battles in our arena. ...',
				'Lastly, I will have our smith refine your helmet if you bring me royal steel, an especially noble metal. ...',
				'Did you understand everything I told you and are willing to handle this task?'
			}, cid)
			npcHandler.topic[cid] = 2
		elseif msgcontains(msg, 'no') then
			npcHandler:say('Bah. Then you will have to wait for the day these helmets are sold in shops, but that will not happen before hell freezes over.', cid)
			npcHandler.topic[cid] = 0
		end

	elseif npcHandler.topic[cid] == 2 then
		if msgcontains(msg, 'yes') then
			player:setStorageValue(Storage.OutfitQuest.Ref, math.max(0, player:getStorageValue(Storage.OutfitQuest.Ref)) + 1)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 1)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 1)
			npcHandler:say('Alright then. Come back to me once you have collected 100 perfect behemoth fangs.', cid)
			npcHandler.topic[cid] = 0
		elseif msgcontains(msg, 'no') then
			npcHandler:say('Would you like me to repeat the task requirements then?', cid)
			npcHandler.topic[cid] = 1
		end

	elseif npcHandler.topic[cid] == 3 then
		if msgcontains(msg, 'yes') then
			if not player:removeItem(5893, 100) then
				npcHandler:say('Lying is not exactly honourable, |PLAYERNAME|. Shame on you.', cid)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 2)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 2)
			player:setStorageValue(Storage.OutfitQuest.Knight.RamsaysHelmetDoor, 1)
			npcHandler:say('I\'m deeply impressed, brave Knight |PLAYERNAME|. I expected nothing less from you. Now, please retrieve Ramsay\'s helmet.', cid)
		elseif msgcontains(msg, 'no') then
			npcHandler:say('There is no need to rush anyway.', cid)
		end
		npcHandler.topic[cid] = 0

	elseif npcHandler.topic[cid] == 4 then
		if msgcontains(msg, 'yes') then
			if not player:removeItem(5924, 1) then
				npcHandler:say('Lying is not exactly honourable, |PLAYERNAME|. Shame on you.', cid)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 3)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 3)
			npcHandler:say('Good work, brave Knight |PLAYERNAME|! Even though it is damaged, it has a lot of sentimental value. Now, please bring me warrior\'s sweat.', cid)
		elseif msgcontains(msg, 'no') then
			npcHandler:say('There is no need to rush anyway.', cid)
		end
		npcHandler.topic[cid] = 0

	elseif npcHandler.topic[cid] == 5 then
		if msgcontains(msg, 'yes') then
			if not player:removeItem(5885, 1) then
				npcHandler:say('Lying is not exactly honourable, |PLAYERNAME|. Shame on you.', cid)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 4)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 4)
			npcHandler:say('Now that is a pleasant surprise, brave Knight |PLAYERNAME|! There is only one task left now: Obtain royal steel to have your helmet refined.', cid)
		elseif msgcontains(msg, 'no') then
			npcHandler:say('There is no need to rush anyway.', cid)
		end
		npcHandler.topic[cid] = 0

	elseif npcHandler.topic[cid] == 6 then
		if msgcontains(msg, 'yes') then
			if not player:removeItem(5887, 1) then
				npcHandler:say('Lying is not exactly honourable, |PLAYERNAME|. Shame on you.', cid)
				return true
			end

			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 5)
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 5)
			npcHandler:say('You truly deserve to wear an adorned helmet, brave Knight |PLAYERNAME|. I\'m sure he will be glad to refine your {adorn} helmet.', cid)
		elseif msgcontains(msg, 'no') then
			npcHandler:say('There is no need to rush anyway.', cid)
		end
		npcHandler.topic[cid] = 0
	end

	if msgcontains(msg, 'adorn')
			or msgcontains(msg, 'outfit') then
		local addonProgress = player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmet)
		if addonProgress == 5 then
			player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 6)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 6)
			player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmetTimer, os.time() + 7200)
			npcHandler:say('Oh, Gregor sent you? I see. It will be my pleasure to adorn your helmet. Please give me some time to finish it.', cid)
		elseif addonProgress == 6 then
			if player:getStorageValue(Storage.OutfitQuest.Knight.AddonHelmetTimer) < os.time() then
				player:setStorageValue(Storage.OutfitQuest.Knight.MissionHelmet, 0)
				player:setStorageValue(Storage.OutfitQuest.Knight.AddonHelmet, 7)
				player:setStorageValue(Storage.OutfitQuest.Ref, math.min(0, player:getStorageValue(Storage.OutfitQuest.Ref) - 1))
				player:addOutfitAddon(131, 2)
				player:addOutfitAddon(139, 2)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				npcHandler:say('Just in time, |PLAYERNAME|. Your helmet is finished, I hope you like it.', cid)
			else
				npcHandler:say('Please have some patience, |PLAYERNAME|. Forging is hard work!', cid)
			end
		elseif addonProgress == 7 then
			npcHandler:say('I think it\'s one of my masterpieces.', cid)
		else
			npcHandler:say('Sorry, but without the permission of Gregor I cannot help you with this matter.', cid)
		end
	end
	return true
end

keywordHandler:addKeyword({'addon'}, StdModule.say, {npcHandler = npcHandler, text = 'I can forge the finest {weapons} for knights and warriors. They may wear them proudly and visible to everyone.'})
keywordHandler:addKeyword({'weapons'}, StdModule.say, {npcHandler = npcHandler, text = 'Would you rather be interested in a {knight\'s sword} or in a {warrior\'s sword} or a {task}?'})

npcHandler:setMessage(MESSAGE_GREET, 'Hello there.')
npcHandler:setMessage(MESSAGE_FAREWELL, 'Good bye.')

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
