local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)		end
function onThink()				npcHandler:onThink()					end

local voices = { {text = 'Gems and jewellery! Best prices in town!'} }
npcHandler:addModule(VoiceModule:new(voices))

local config = {
	['hardened bones'] = {
		storageValue = 1,
		message = {
			wrongValue = 'Well, I\'ll give you a little hint. They can sometimes be extracted from creatures that consist only of - you guessed it, bones. You need an obsidian knife though.',
			deliever = 'How are you faring with your mission? Have you collected all 100 hardened bones?',
			success = 'I\'m surprised. That\'s pretty good for a man. Now, bring us the 100 turtle shells.'
		},
		itemId = 5925,
		count = 100
	},
	['turtle shells'] = {
		storageValue = 2,
		message = {
			wrongValue = 'Turtles can be found on some idyllic islands which have recently been discovered.',
			deliever = 'Did you get us 100 turtle shells so we can make new shields?',
			success = 'Well done - for a man. These shells are enough to build many strong new shields. Thank you! Now - show me fighting spirit.'
		},
		itemId = 5899,
		count = 100
	},
	['fighting spirit'] = {
		storageValue = 3,
		message = {
			wrongValue = 'You should have enough fighting spirit if you are a true hero. Sorry, but you have to figure this one out by yourself. Unless someone grants you a wish.',
			deliever = 'So, can you show me your fighting spirit?',
			success = 'Correct - pretty smart for a man. But the hardest task is yet to come: the claw from a lord among the dragon lords.'
		},
		itemId = 5884
	},
	['dragon claw'] = {
		storageValue = 4,
		message = {
			wrongValue = 'You cannot get this special red claw from any common dragon in Tibia. It requires a special one, a lord among the lords.',
			deliever = 'Have you actually managed to obtain the dragon claw I asked for?',
			success = 'You did it! I have seldom seen a man as courageous as you. I really have to say that you deserve to wear a spike. Go ask Cornelia to adorn your armour.'
		},
		itemId = 5919
	}
}

local message = {}

local function greetCallback(cid)
	npcHandler:setMessage(MESSAGE_GREET, 'Salutations, |PLAYERNAME|. What can I do for you?')
	message[cid] = nil
	return true
end

local function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then
		return false
	end

	local player, storage = Player(cid), Storage.OutfitQuest.WarriorShoulderAddon
	if npcHandler.topic[cid] == 0 then
		if isInArray({'outfit', 'addon'}, msg) then
			npcHandler:say('Are you talking about my spiky shoulder pad? You can\'t buy one of these. They have to be {earned}.', cid)
		elseif msgcontains(msg, 'earn') then
			if player:getStorageValue(storage) < 1 then
				npcHandler:say('I\'m not sure if you are enough of a hero to earn them. You could try, though. What do you think?', cid)
				npcHandler.topic[cid] = 1
			elseif player:getStorageValue(storage) >= 1 and player:getStorageValue(storage) < 5 then
				npcHandler:say('Before I can nominate you for an award, please complete your task.', cid)
			elseif player:getStorageValue(storage) == 5 then
				npcHandler:say('You did it! I have seldom seen a man as courageous as you. I really have to say that you deserve to wear a spike. Go ask Cornelia to adorn your armour.', cid)
			end
		elseif config[msg:lower()] then
			local targetMessage = config[msg:lower()]
			if player:getStorageValue(storage) ~= targetMessage.storageValue then
				npcHandler:say(targetMessage.message.wrongValue, cid)
				return true
			end

			npcHandler:say(targetMessage.message.deliever, cid)
			npcHandler.topic[cid] = 3
			message[cid] = targetMessage
		end
	elseif npcHandler.topic[cid] == 1 then
		if msgcontains(msg, 'yes') then
			npcHandler:say({
				'Okay, who knows, maybe you have a chance. A really small one though. Listen up: ...',
				'First, you have to prove your guts by bringing me 100 hardened bones. ...',
				'Next, if you actually managed to collect that many, please complete a small task for our guild and bring us 100 turtle shells. ...',
				'It is said that excellent shields can be created from these. ...',
				'Alright, um, afterwards show me that you have fighting spirit. Any true hero needs plenty of that. ...',
				'The last task is the hardest. You will need to bring me a claw from a mighty dragon king. ...',
				'Did you understand everything I told you and are willing to handle this task?'
			}, cid)
			npcHandler.topic[cid] = 2
		elseif msgcontains(msg, 'no') then
			npcHandler:say('I thought so. Train hard and maybe some day you will be ready to face this mission.', cid)
			npcHandler.topic[cid] = 0
		end
	elseif npcHandler.topic[cid] == 2 then
		if msgcontains(msg, 'yes') then
			player:setStorageValue(storage, 1)
			player:setStorageValue(Storage.OutfitQuest.DefaultStart, 1) --this for default start of Outfit and Addon Quests
			npcHandler:say('Excellent! Don\'t forget: Your first task is to bring me 100 hardened bones. Good luck!', cid)
			npcHandler.topic[cid] = 0
		elseif msgcontains(msg, 'no') then
			npcHandler:say('Would you like me to repeat the task requirements then?', cid)
			npcHandler.topic[cid] = 1
		end
	elseif npcHandler.topic[cid] == 3 then
		if msgcontains(msg, 'yes') then
			local targetMessage = message[cid]
			if not player:removeItem(targetMessage.itemId, targetMessage.count or 1) then
				npcHandler:say('Why do men always lie?', cid)
				return true
			end

			player:setStorageValue(storage, player:getStorageValue(storage) + 1)
			npcHandler:say(targetMessage.message.success, cid)
		elseif msgcontains(msg, 'no') then
			npcHandler:say('Don\'t give up just yet.', cid)
		end
		npcHandler.topic[cid] = 0
	end

	local playerAddon = Player(cid)

	if msgcontains(msg, 'adorn')
			or msgcontains(msg, 'outfit') then
		if playerAddon:getStorageValue(Storage.OutfitQuest.WarriorShoulderAddon) == 5 then
			playerAddon:setStorageValue(Storage.OutfitQuest.WarriorShoulderAddon, 6)
			playerAddon:setStorageValue(Storage.OutfitQuest.WarriorShoulderTimer, os.time() + (player:getSex() == PLAYERSEX_FEMALE and 3600 or 7200))
			npcHandler:say('Ah, you must be the hero Henrry talked about. I\'ll prepare the shoulder spikes for you. Please give me some time to finish.', cid)
		elseif playerAddon:getStorageValue(Storage.OutfitQuest.WarriorShoulderAddon) == 6 then
			if playerAddon:getStorageValue(Storage.OutfitQuest.WarriorShoulderTimer) > os.time() then
				npcHandler:say('I\'m not done yet. Please be as patient as you are courageous.', cid)
			elseif playerAddon:getStorageValue(Storage.OutfitQuest.WarriorShoulderTimer) > 0 and player:getStorageValue(Storage.OutfitQuest.WarriorShoulderTimer) < os.time() then
				playerAddon:addOutfitAddon(142, 1)
				playerAddon:addOutfitAddon(134, 1)
				playerAddon:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				playerAddon:setStorageValue(Storage.OutfitQuest.WarriorShoulderAddon, 7)
				playerAddon:addAchievementProgress('Wild Warrior', 2)
				npcHandler:say('Finished! Since you are a man, I thought you probably wanted two. Men always want that little extra status symbol. <giggles>', cid)
			else
				npcHandler:say('I\'m selling leather armor, chain armor, and brass armor. Ask me for a {trade} if you like to take a look.', cid)
			end
		end
	end
	return true
end

npcHandler:setMessage(MESSAGE_WALKAWAY, 'Be careful on your journeys.')
npcHandler:setMessage(MESSAGE_FAREWELL, 'Don\'t hurt yourself with that weapon, little one.')

npcHandler:setCallback(CALLBACK_GREET, greetCallback)
npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
