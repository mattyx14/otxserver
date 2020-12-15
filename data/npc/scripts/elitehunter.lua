local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)
	npcHandler:onCreatureAppear(cid)
end
function onCreatureDisappear(cid)
	npcHandler:onCreatureDisappear(cid)
end
function onCreatureSay(cid, type, msg)
	npcHandler:onCreatureSay(cid, type, msg)
end
function onThink()
	npcHandler:onThink()
end

local voices = { {text = 'Gems and jewellery! Best prices in town!'} }
npcHandler:addModule(VoiceModule:new(voices))

local function greetCallback(cid)
	local player = Player(cid)
	if player:getStorageValue(JOIN_STOR) == -1 then
		npcHandler:setMessage(MESSAGE_GREET, 'Welcome |PLAYERNAME|. Would you like to join the \'Paw and Fur - Hunting Elite\'?')
	else
		npcHandler:setMessage(MESSAGE_GREET, 'Welcome back old chap. What brings you here this time?')
	end
	return true
end

local function joinTables(old, new)
	for k, v in pairs(new) do
		old[#old+1] = v
	end
	return old
end

local function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then return false end

	local player = Player(cid)
	msg = msg:gsub("(%l)(%w*)", function(a,b) return string.upper(a)..b end)
	if (msgcontains('join', msg) or msgcontains('yes', msg))
			and npcHandler.topic[cid] == 0
			and player:getStorageValue(JOIN_STOR) ~= 1 then
		player:setStorageValue(JOIN_STOR, 1)
		npcHandler:say('Great!, now you can start tasks.', cid) --I'm not sure if this is as real tibia. I let this piece of code because it was in the original file.
	elseif isInArray({'tasks', 'task', 'mission'}, msg:lower()) then
		local can = player:getTasks()
		if player:getStorageValue(JOIN_STOR) == -1 then
			return npcHandler:say('You\'ll have to {join}, to get any {tasks}.',cid)
		end
		if #can > 0 then
			local text = ''
			local sep = ', '
			table.sort(can, function(a, b) return a < b end)
			local t = 0
			local id
			for i = 1, #can do
				id = can[i]
				t = t + 1
				if t == #can - 1 then
					sep = ' and '
				elseif t == #can then
					sep = '.'
				end
				text = text .. '{' .. (tasks[id].name or tasks[id].raceName) .. '}' .. sep
			end
			npcHandler:say('The current task' .. (#can > 1 and 's' or '') .. ' that you can choose ' .. (#can > 1 and 'are' or 'is') .. ' ' .. text, cid)
			npcHandler.topic[cid] = 0
		else
			npcHandler:say('I don\'t have any task for you right now.', cid)
		end
	elseif msg ~= '' and player:canStartTask(msg) then
		if #player:getStartedTasks() >= tasksByPlayer then
			npcHandler:say('Sorry, but you already started ' .. tasksByPlayer .. ' tasks. You can check their {status}, {cancel} or {report} a task.', cid)
			return true
		end
		local task = getTaskByName(msg)
		if task and player:getStorageValue(QUESTSTORAGE_BASE + task) > 0 then
			return false
		end
		npcHandler:say('In this task you must defeat ' .. tasks[task].killsRequired .. ' ' .. tasks[task].raceName .. '. Are you sure that you want to start this task?', cid)
		choose[cid] = task
		npcHandler.topic[cid] = 1
	elseif msg:lower() == 'yes' and npcHandler.topic[cid] == 1 then
		player:setStorageValue(QUESTSTORAGE_BASE + choose[cid], 1)
		player:setStorageValue(KILLSSTORAGE_BASE + choose[cid], 0)
		npcHandler:say('Excellent! You can check the {status} of your task saying {report} to me. Also you can {cancel} tasks to.', cid)
		choose[cid] = nil
		npcHandler.topic[cid] = 0
	elseif msgcontains('status', msg) then
		local started = player:getStartedTasks()
		if started and #started > 0 then
			local text = ''
			local sep = ', '
			table.sort(started, (function(a, b) return (a < b) end))
			local t = 0
			local id
			for i = 1, #started do
				id = started[i]
				t = t + 1
				if t == #started - 1 then
					sep = ' and '
				elseif t == #started then
					sep = '.'
				end
				text = text .. 'Task name: ' .. tasks[id].raceName .. '. Current kills: ' .. player:getStorageValue(KILLSSTORAGE_BASE + id) .. '.\n'
			end
			npcHandler:say({'The status of your current tasks is:\n' .. text}, cid)
		else
			npcHandler:say('You haven\'t started any task yet.', cid)
		end
	elseif msgcontains('report', msg) then
		local started = player:getStartedTasks()
		local finishedAtLeastOne = false
		local finished = 0
		if started and #started > 0 then
			local id, reward
			for i = 1, #started do
				id = started[i]
				if player:getStorageValue(KILLSSTORAGE_BASE + id) >= tasks[id].killsRequired then
					for j = 1, #tasks[id].rewards do
						reward = tasks[id].rewards[j]
						local deny = false
						if reward.storage then
							if player:getStorageValue(reward.storage[1]) >= reward.storage[2] then
								deny = true
							end
						end
						if isInArray({REWARD_MONEY, 'money'}, reward.type:lower()) and not deny then
							player:addMoney(reward.value[1])
						elseif isInArray({REWARD_EXP, 'exp', 'experience'}, reward.type:lower()) and not deny then
							player:addExperience(reward.value[1], true)
						elseif isInArray({REWARD_ACHIEVEMENT, 'achievement', 'ach'}, reward.type:lower()) and not deny then
							player:addAchievement(reward.value[1])
						elseif isInArray({REWARD_STORAGE, 'storage', 'stor'}, reward.type:lower()) and not deny then
							player:setStorageValue(reward.value[1], reward.value[2])
						elseif isInArray({REWARD_POINT, 'points', 'point'}, reward.type:lower()) and not deny then
							player:setStorageValue(POINTSSTORAGE, getPlayerTasksPoints(cid) + reward.value[1])
						elseif isInArray({REWARD_ITEM, 'item', 'items', 'object'}, reward.type:lower()) and not deny then
							player:addItem(reward.value[1], reward.value[2])
						end

						if reward.storage then
							player:setStorageValue(reward.storage[1], reward.storage[2])
						end
					end

					player:setStorageValue(QUESTSTORAGE_BASE + id, (tasks[id].norepeatable and 2 or 0))
					player:setStorageValue(KILLSSTORAGE_BASE + id, -1)
					player:setStorageValue(REPEATSTORAGE_BASE + id, math.max(player:getStorageValue(REPEATSTORAGE_BASE + id), 0))
					player:setStorageValue(REPEATSTORAGE_BASE + id, player:getStorageValue(REPEATSTORAGE_BASE + id) + 1)
					finishedAtLeastOne = true
					finished = finished + 1
				end
			end
			if not finishedAtLeastOne then
				local started = player:getStartedTasks()
				if started and #started > 0 then
					local text = ''
					local sep = ', '
					table.sort(started, (function(a, b) return (a < b) end))
					local t = 0
					local id
					for i = 1, #started do
						id = started[i]
						t = t + 1
						if (t == #started - 1) then
							sep = ' and '
						elseif (t == #started) then
							sep = '.'
						end
						text = text .. '{' .. (tasks[id].name or tasks[id].raceName) .. '}' .. sep
					end
					npcHandler:say('The current task' .. (#started > 1 and 's' or '') .. ' that you started ' .. (#started > 1 and 'are' or 'is') .. ' ' .. text, cid)
				end
			else
				npcHandler:say('Awesome! you finished ' .. (finished > 1 and 'various' or 'a') .. ' task' .. (finished > 1 and 's' or '') .. '. Talk to me again if you want to start a {task}.', cid)
			end
		else
			npcHandler:say('You haven\'t started any task yet.', cid)
		end
	elseif msg:lower() == 'started' then
		local started = player:getStartedTasks()
		if started and #started > 0 then
			local text = ''
			local sep = ', '
			table.sort(started, (function(a, b) return (a < b) end))
			local t = 0
			local id
			for i = 1, #started do
				id = started[i]
				t = t + 1
				if t == #started - 1 then
					sep = ' and '
				elseif t == #started then
					sep = '.'
				end
				text = text .. '{' .. (tasks[id].name or tasks[id].raceName) .. '}' .. sep
			end

			npcHandler:say('The current task' .. (#started > 1 and 's' or '') .. ' that you started ' .. (#started > 1 and 'are' or 'is') .. ' ' .. text, cid)
		else
			npcHandler:say('You haven\'t started any task yet.', cid)
		end
	elseif msg:lower() == 'cancel' then
		local started = player:getStartedTasks()
		local task = getTaskByName(msg)
		local text = ''
		local sep = ', '
		table.sort(started, (function(a, b) return (a < b) end))
		local t = 0
		local id
		for i = 1, #started do
			id = started[i]
			t = t + 1
			if t == #started - 1 then
				sep = ' or '
			elseif t == #started then
				sep = '?'
			end
			text = text .. '{' .. (tasks[id].name or tasks[id].raceName) .. '}' .. sep
		end
		if started and #started > 0 then
			npcHandler:say('Cancelling a task will make the counter restart. Which of these tasks you want cancel?' .. (#started > 1 and '' or '') .. ' ' .. text, cid)
			npcHandler.topic[cid] = 2
		else
			npcHandler:say('You haven\'t started any task yet.', cid)
		end
	elseif getTaskByName(msg) and npcHandler.topic[cid] == 2 and isInArray(getPlayerStartedTasks(cid), getTaskByName(msg)) then
		local task = getTaskByName(msg)
		if player:getStorageValue(KILLSSTORAGE_BASE + task) > 0 then
			npcHandler:say('You currently killed ' .. player:getStorageValue(KILLSSTORAGE_BASE + task) .. '/' .. tasks[task].killsRequired .. ' ' .. tasks[task].raceName .. '. Cancelling this task will restart the count. Are you sure you want to cancel this task?', cid)
		else
			npcHandler:say('Are you sure you want to cancel this task?', cid)
		end
		npcHandler.topic[cid] = 3
		cancel[cid] = task
	elseif getTaskByName(msg) and npcHandler.topic[cid] == 1 and isInArray(getPlayerStartedTasks(cid), getTaskByName(msg)) then
		local task = getTaskByName(msg)
		if player:getStorageValue(KILLSSTORAGE_BASE + task) > 0 then
			npcHandler:say('You currently killed ' .. player:getStorageValue(KILLSSTORAGE_BASE + task) .. '/' .. tasks[task].killsRequired .. ' ' .. tasks[task].raceName .. '.', cid)
		else
			npcHandler:say('You currently killed 0/' .. tasks[task].killsRequired .. ' ' .. tasks[task].raceName .. '.', cid)
		end
		npcHandler.topic[cid] = 0
	elseif msg:lower() == 'yes' and npcHandler.topic[cid] == 3 then
		player:setStorageValue(QUESTSTORAGE_BASE + cancel[cid], -1)
		player:setStorageValue(KILLSSTORAGE_BASE + cancel[cid], -1)
		npcHandler:say('You have cancelled the task ' .. (tasks[cancel[cid]].name or tasks[cancel[cid]].raceName) .. '.', cid)
		npcHandler.topic[cid] = 0
	elseif isInArray({'points', 'rank'}, msg:lower()) then
		if player:getPawAndFurPoints() < 1 then
			npcHandler:say('At this time, you have ' .. player:getPawAndFurPoints() .. ' Paw & Fur points. You ' .. (player:getPawAndFurRank() == 6 and 'are an Elite Hunter' or player:getPawAndFurRank() == 5 and 'are a Trophy Hunter' or player:getPawAndFurRank() == 4 and 'are a Big Game Hunter' or player:getPawAndFurRank() == 3 and 'are a Ranger' or player:getPawAndFurRank() == 2 and 'are a Huntsman' or player:getPawAndFurRank() == 1 and 'are a Member'  or 'haven\'t been ranked yet') .. '.', cid)
		else
			npcHandler:say('At this time, you have ' .. player:getPawAndFurPoints() .. ' Paw & Fur points. You ' .. (player:getPawAndFurRank() == 6 and 'are an Elite Hunter' or player:getPawAndFurRank() == 5 and 'are a Trophy Hunter' or player:getPawAndFurRank() == 4 and 'are a Big Game Hunter' or player:getPawAndFurRank() == 3 and 'are a Ranger' or player:getPawAndFurRank() == 2 and 'are a Huntsman' or player:getPawAndFurRank() == 1 and 'are a Member'  or 'haven\'t been ranked yet') .. '.', cid)
		end
		npcHandler.topic[cid] = 0
	end
end


npcHandler:setMessage(MESSAGE_FAREWELL, 'Happy hunting, old chap!')
npcHandler:setCallback(CALLBACK_GREET, greetCallback)
npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
