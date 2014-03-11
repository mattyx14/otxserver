-- Advanced NPC System (Created by Jiddo),
-- Modified by TheForgottenServer Team,
-- Modified by The OTX Server Team.

if(NpcSystem == nil) then
	-- Loads the underlying classes of the npcsystem.
	dodirectory(getDataDir() .. 'npc/lib/npcsystem')

	-- Global npc constants:

	-- Keyword nestling behavior. For more information look at the top of keywordhandler.lua
	KEYWORD_BEHAVIOR = BEHAVIOR_NORMAL_EXTENDED

	-- Greeting and unGreeting keywords. For more information look at the top of modules.lua
	FOCUS_GREETWORDS = {'hi', 'hello'}
	FOCUS_FAREWELLWORDS = {'bye', 'farewell'}

	-- The word for requesting trade window. For more information look at the top of modules.lua
	SHOP_TRADEREQUEST = {'offer', 'trade'}

	-- The word for accepting/declining an offer. CAN ONLY CONTAIN ONE FIELD! For more information look at the top of modules.lua
	SHOP_YESWORD = {'yes'}
	SHOP_NOWORD = {'no'}

	-- Pattern used to get the amount of an item a player wants to buy/sell.
	PATTERN_COUNT = '%d+'

	-- Talkdelay behavior. For more information, look at the top of npchandler.lua.
	NPCHANDLER_TALKDELAY = TALKDELAY_ONTHINK

	-- Conversation behavior. For more information, look at the top of npchandler.lua.
	NPCHANDLER_CONVBEHAVIOR = CONVERSATION_PRIVATE

	-- Constant strings defining the keywords to replace in the default messages.
	--	For more information, look at the top of npchandler.lua...
	TAG_PLAYERNAME = '|PLAYERNAME|'
	TAG_ITEMCOUNT = '|ITEMCOUNT|'
	TAG_TOTALCOST = '|TOTALCOST|'
	TAG_ITEMNAME = '|ITEMNAME|'
	TAG_QUEUESIZE = '|QUEUESIZE|'

	NpcSystem = {}

	-- Gets an npcparameter with the specified key. Returns nil if no such parameter is found.
	function NpcSystem.getParameter(key)
		local ret = getNpcParameter(tostring(key))
		if((type(ret) == 'number' and ret == 0) or ret == nil) then
			return nil
		else
			return ret
		end
	end

	-- Parses all known parameters for the npc. Also parses parseable modules.
	function NpcSystem.parseParameters(npcHandler)
		local ret = NpcSystem.getParameter('idletime')
		if(ret ~= nil) then
			npcHandler.idleTime = tonumber(ret)
		end
		local ret = NpcSystem.getParameter('talkradius')
		if(ret ~= nil) then
			npcHandler.talkRadius = tonumber(ret)
		end
		local ret = NpcSystem.getParameter('message_greet')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_GREET, ret)
		end
		local ret = NpcSystem.getParameter('message_farewell')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_FAREWELL, ret)
		end
		local ret = NpcSystem.getParameter('message_decline')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_DECLINE, ret)
		end
		local ret = NpcSystem.getParameter('message_needmorespace')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_NEEDMORESPACE, ret)
		end
		local ret = NpcSystem.getParameter('message_needspace')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_NEEDSPACE, ret)
		end
		local ret = NpcSystem.getParameter('message_sendtrade')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_SENDTRADE, ret)
		end
		local ret = NpcSystem.getParameter('message_noshop')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_NOSHOP, ret)
		end
		local ret = NpcSystem.getParameter('message_oncloseshop')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_ONCLOSESHOP, ret)
		end
		local ret = NpcSystem.getParameter('message_onbuy')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_ONBUY, ret)
		end
		local ret = NpcSystem.getParameter('message_onsell')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_ONSELL, ret)
		end
		local ret = NpcSystem.getParameter('message_missingmoney')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_MISSINGMONEY, ret)
		end
		local ret = NpcSystem.getParameter('message_needmoney')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_NEEDMONEY, ret)
		end
		local ret = NpcSystem.getParameter('message_missingitem')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_MISSINGITEM, ret)
		end
		local ret = NpcSystem.getParameter('message_needitem')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_NEEDITEM, ret)
		end
		local ret = NpcSystem.getParameter('message_idletimeout')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_IDLETIMEOUT, ret)
		end
		local ret = NpcSystem.getParameter('message_walkaway')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_WALKAWAY, ret)
		end
		local ret = NpcSystem.getParameter('message_alreadyfocused')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_ALREADYFOCUSED, ret)
		end
		local ret = NpcSystem.getParameter('message_placedinqueue')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_PLACEDINQUEUE, ret)
		end
		local ret = NpcSystem.getParameter('message_buy')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_BUY, ret)
		end
		local ret = NpcSystem.getParameter('message_sell')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_SELL, ret)
		end
		local ret = NpcSystem.getParameter('message_bought')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_BOUGHT, ret)
		end
		local ret = NpcSystem.getParameter('message_sold')
		if(ret ~= nil) then
			npcHandler:setMessage(MESSAGE_SOLD, ret)
		end

		-- Parse modules.
		for parameter, module in pairs(Modules.parseableModules) do
			local ret = NpcSystem.getParameter(parameter)
			if(ret ~= nil) then
				local number = tonumber(ret)
				if(number ~= nil and number ~= 0) then
					npcHandler:addModule(module:new())
				end
			end
		end
	end
end
