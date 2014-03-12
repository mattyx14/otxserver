local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid) 			end
function onCreatureDisappear(cid) 			npcHandler:onCreatureDisappear(cid) 		end
function onCreatureSay(cid, type, msg) 		npcHandler:onCreatureSay(cid, type, msg) 	end
function onThink() 							npcHandler:onThink() 						end
function onPlayerEndTrade(cid)				npcHandler:onPlayerEndTrade(cid)			end
function onPlayerCloseChannel(cid)			npcHandler:onPlayerCloseChannel(cid)		end

function oracle(cid, message, keywords, parameters, node)
	local npcHandler = parameters.npcHandler
	if(npcHandler == nil) then
		print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'oracle - Call without any npcHandler instance.')
		return false
	end

	if(not npcHandler:isFocused(cid)) then
		return false
	end

	local cityNode, vocationNode = node:getParent():getParent(), node:getParent()
	local params = {
		['vocation'] = vocationNode:getParameters().vocation,
		['town'] = cityNode:getParameters().town,
		['premium'] = cityNode:getParameters().premium or false,
		['items'] = vocationNode:getParameters().items or nil
	}

	if(params['town'] ~= nil and params['vocation'] ~= nil) then
		if(getPlayerLevel(cid) < parameters.level) then
			npcHandler:say('You must first reach level {' .. parameters.level .. '}!', cid)
		elseif(getPlayerVocation(cid) > 0) then
			npcHandler:say('Sorry, you already have a vocation!', cid)
		elseif(params['premium'] and not isPremium(cid)) then
			npcHandler:say('Sorry, this town is reserved only for premium players!', cid)
		else
			doPlayerSetVocation(cid, params['vocation'])
			doPlayerSetTown(cid, params['town'])
			if(params['items'] ~= nil) then
				local parcel, time = doCreateItemEx(2595), os.time()
				local label, target = doAddContainerItem(parcel, 2599), getCreatureName(cid)

				doItemSetAttribute(label, "text", target .. "\n" .. getTownName(params['town']))
				doItemSetAttribute(label, "date", time)
				doItemSetAttribute(label, "writer", getCreatureName(getNpcId()))

				for _, item in ipairs(params['items']) do
					if(type(item[2]) == 'string') then
						local tmp = doCreateItemEx(item[1])

						doItemSetAttribute(tmp, "text", item[2])
						doItemSetAttribute(tmp, "date", time)
						doItemSetAttribute(tmp, "writer", getCreatureName(getNpcId()))

						doAddContainerItemEx(parcel, tmp)
					else
						doAddContainerItem(parcel, item[1], item[2] or 1)
					end
				end

				doPlayerSendMailByName(target, parcel, params['town'])
			end

			local tmp, temple = getThingPosition(cid), getTownTemplePosition(params['town'])
			npcHandler:say('SO BE IT!', cid)
			doTeleportThing(cid, temple)

			doSendMagicEffect(tmp, CONST_ME_POFF)
			doSendMagicEffect(temple, CONST_ME_TELEPORT)
		end
	else
		npcHandler:resetNpc(cid)
		error('Player: ' .. getCreatureName(cid) .. ', Params: ' .. table.serialize(params))
	end

	return true

end

function greetCallback(cid)
	if(getPlayerLevel(cid) >= getConfigValue('rookLevelToLeaveRook')) then
		return true
	end

	npcHandler:say('COME BACK WHEN YOU GROW UP, CHILD!')
	return false
end

npcHandler:setCallback(CALLBACK_GREET, greetCallback)
npcHandler:setMessage(MESSAGE_GREET, 'Hello |PLAYERNAME|. Are you prepared to face your destiny?')

local yesNode = KeywordNode:new({'yes'}, oracle, {level = getConfigValue('rookLevelToLeaveRook')})
local noNode = KeywordNode:new({'no'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, moveup = 1, text = 'Then what vocation do you want to become?'})

local node1 = keywordHandler:addKeyword({'yes'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'What city do you wish to live in? {Rhyves}, {Varak} or {Jorvik}?'})
	local node2 = node1:addChildKeyword({'varak'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, town = 1, text = 'Varak, eh? So what vocation do you wish to become? {Sorcerer}, {druid}, {paladin} or {knight}?'})
		local node3 = node2:addChildKeyword({'sorcerer'}, StdModule.say, {npcHandler = npcHandler, vocation = 1, onlyFocus = true, text = 'So, you wish to be a powerful magician? Are you sure about that? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'druid'}, StdModule.say, {npcHandler = npcHandler, vocation = 2, onlyFocus = true, text = 'Are you sure that a druid is what you wish to become? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'paladin'}, StdModule.say, {npcHandler = npcHandler, vocation = 3, onlyFocus = true, text = 'A ranged marksman. Are you sure? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'knight'}, StdModule.say, {npcHandler = npcHandler, vocation = 4, onlyFocus = true, text = 'A mighty warrior. Is that your final decision? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
	node2 = node1:addChildKeyword({'rhyves'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, town = 2, premium = true, text = 'Rhyves, eh? So what vocation do you wish to become? {Sorcerer}, {druid}, {paladin} or {knight}?'})
		node3 = node2:addChildKeyword({'sorcerer'}, StdModule.say, {npcHandler = npcHandler, vocation = 1, onlyFocus = true, text = 'So, you wish to be a powerful magician? Are you sure about that? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'druid'}, StdModule.say, {npcHandler = npcHandler, vocation = 2, onlyFocus = true, text = 'Are you sure that a druid is what you wish to become? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'paladin'}, StdModule.say, {npcHandler = npcHandler, vocation = 3, onlyFocus = true, text = 'A ranged marksman. Are you sure? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
		node3 = node2:addChildKeyword({'knight'}, StdModule.say, {npcHandler = npcHandler, vocation = 4, onlyFocus = true, text = 'A mighty warrior. Is that your final decision? This decision is irreversible!'})
			node3:addChildKeywordNode(yesNode)
			node3:addChildKeywordNode(noNode)
keywordHandler:addKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'Then come back when you are ready.'})

npcHandler:addModule(FocusModule:new())
