local config = {
	pin = false, -- players can protect their money with pin code (like in cash machines) (true/false)
	pinMinLength = 4, -- minimum pin length
	pinMaxLength = 4, -- maximum pin length
	pinStorage = 3006, -- only if pin enabled (used to store player pin)
	transferDisabledVocations = {0} -- disable non vocation characters
}

local talkState = {}
local count = {}
local transfer = {}
local pin = {}

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)		end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)		end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)	end
function onThink()				npcHandler:onThink()				end

if(config.pin) then
	bank_pin = {
		get = function(cid)
			return getPlayerStorageValue(cid, config.pinStorage)
		end,

		set = function(cid, code)
			return setPlayerStorageValue(cid, config.pinStorage, code)
		end,

		logged = function(cid)
			return pin[cid] == bank_pin.get(cid)
		end,

		validate = function(code)
			if(not isNumber(code)) then
				return false
			end

			local length = tostring(code):len()
			return (length >= config.pinMinLength and length <= config.pinMaxLength)
		end
	}
end

if(not getPlayerBalance) then
	getPlayerBalance = function(cid)
		local result = db.getResult("SELECT `balance` FROM `players` WHERE `id` = " .. getPlayerGUID(cid))
		if(result:getID() == -1) then
			return false
		end

		local value = tonumber(result:getDataString("balance"))
		result:free()
		return value
	end

	doPlayerSetBalance = function(cid, balance)
		db.executeQuery("UPDATE `players` SET `balance` = " .. balance .. " WHERE `id` = " .. getPlayerGUID(cid))
		return true
	end

	doPlayerWithdrawMoney = function(cid, amount)
		local balance = getPlayerBalance(cid)
		if(amount > balance or not doPlayerAddMoney(cid, amount)) then
			return false
		end

		doPlayerSetBalance(cid, balance - amount)
		return true
	end

	doPlayerDepositMoney = function(cid, amount)
		if(not doPlayerRemoveMoney(cid, amount)) then
			return false
		end

		doPlayerSetBalance(cid, getPlayerBalance(cid) + amount)
		return true
	end

	doPlayerTransferMoneyTo = function(cid, target, amount)
		local balance = getPlayerBalance(cid)
		if(amount > balance) then
			return false
		end

		local tid = getPlayerByName(target)
		if(tid > 0) then
			doPlayerSetBalance(tid, getPlayerBalance(tid) + amount)
		else
			if(playerExists(target) == false) then
				return false
			end

			db.executeQuery("UPDATE `player_storage` SET `value` = `value` + '" .. amount .. "' WHERE `player_id` = (SELECT `id` FROM `players` WHERE `name` = '" .. escapeString(player) .. "') AND `key` = '" .. balance_storage .. "'")
		end

		doPlayerSetBalance(cid, getPlayerBalance(cid) - amount)
		return true
	end
end

if(not doPlayerSave) then
	local function doPlayerSave(cid)
		return true
	end
end

local function getPlayerVocationByName(name)
	local result = db.getResult("SELECT `vocation` FROM `players` WHERE `name` = " .. db.escapeString(name))
	if(result:getID() == -1) then
		return false
	end

	local value = result:getDataString("vocation")
	result:free()
	return value
end

local function isValidMoney(money)
	return (isNumber(money) and money > 0 and money < 4294967296)
end

local function getCount(string)
	local b, e = string:find("%d+")
	local money = b and e and tonumber(string:sub(b, e)) or -1
	if(isValidMoney(money)) then
		return money
	end
	return -1
end

function greetCallback(cid)
	talkState[cid], count[cid], transfer[cid], pin[cid] = 0, nil, nil, nil
	return true
end

function creatureSayCallback(cid, type, msg)

	if(not npcHandler:isFocused(cid)) then
		return false
	end

---------------------------- pin -------------------------
	if(config.pin) then
		if(talkState[cid] == "verify-pin") then
			talkState[cid] = 0
			pin[cid] = getCount(msg)
			if(not bank_pin.logged(cid)) then
				selfSay("Invalid pin code entered. Please try again.", cid)
				return true
			end

			selfSay("You have been successfully logged in.", cid)
		elseif(talkState[cid] == "new-pin") then
			talkState[cid] = 0

			if(bank_pin.get(cid) ~= -1 and not bank_pin.logged(cid)) then
				selfSay("Please login before attempting to change your pin code.", cid)
				talkState[cid] = "verify-pin"
				return true
			end

			if(msgcontains(msg, 'reset') or msgcontains(msg, 'remove') or msgcontains(msg, 'clear')) then
				selfSay("Pin code has been removed.", cid)
				pin[cid] = nil
				bank_pin.set(cid, -1)
				return true
			end

			pin[cid] = getCount(msg)
			if(bank_pin.validate(pin[cid])) then
				selfSay("Pin code successfully changed.", cid)
				bank_pin.set(cid, pin[cid])
			else
				local str = ""
				if(config.pinMinLength ~= config.pinMaxLength) then
					str = config.pinMinLength .. " - " .. config.pinMaxLength
				else
					str = config.pinMinLength
				end

				selfSay("Invalid pin code entered. Your pin should contain " .. str .. " digits", cid)
			end

			return true
		elseif(msgcontains(msg, 'balance') or
			msgcontains(msg, 'deposit') or
			msgcontains(msg, 'withdraw') or
			msgcontains(msg, 'transfer')) then
				if(bank_pin.get(cid) ~= -1 and not bank_pin.logged(cid)) then
					selfSay("Please tell me your bank pin code before making any transactions.", cid)
					talkState[cid] = "verify-pin"
					return true
				end

				talkState[cid] = 0
		elseif(msgcontains(msg, 'login')) then
			talkState[cid] = "verify-pin"
			return true
		elseif(msgcontains(msg, 'pin')) then
			selfSay("Please tell me your new pin code.", cid)
			talkState[cid] = "new-pin"
			return true
		end
	end
---------------------------- help ------------------------
	if msgcontains(msg, 'advanced') then
		if isInArray(config.transferDisabledVocations, getPlayerVocation(cid)) then
			selfSay("Once you are on the Tibian mainland, you can access new functions of your bank account, such as transferring money to other players safely or taking part in house auctions.", cid)
		else
			selfSay("Renting a house has never been this easy. Simply make a bid for an auction. We will check immediately if you have enough money.", cid)
		end
		talkState[cid] = 0
	elseif msgcontains(msg, 'help') or msgcontains(msg, 'functions') then
		selfSay("You can check the {balance} of your bank account, {deposit} money or {withdraw} it. You can also {transfer} money to other characters, provided that they have a vocation.", cid)
		talkState[cid] = 0
	elseif msgcontains(msg, 'bank') then
		npcHandler:say("We can change money for you. You can also access your bank account.", cid)
		talkState[cid] = 0
	elseif msgcontains(msg, 'job') then
		npcHandler:say("I work in this bank. I can change money for you and help you with your bank account.", cid)
		talkState[cid] = 0
---------------------------- balance ---------------------
	elseif msgcontains(msg, 'balance') then
		selfSay("Your account balance is " .. getPlayerBalance(cid) .. " gold.", cid)
		talkState[cid] = 0
---------------------------- deposit ---------------------
	elseif msgcontains(msg, 'deposit all') and getPlayerMoney(cid) > 0 then
		count[cid] = getPlayerMoney(cid)
		if not isValidMoney(count[cid]) then
			selfSay("Sorry, but you can't deposit that much.", cid)
			talkState[cid] = 0
			return false
		end

		if count[cid] < 1 then
			selfSay("You don't have any money to deposit in you inventory..", cid)
			talkState[cid] = 0
		else
			selfSay("Would you really like to deposit " .. count[cid] .. " gold?", cid)
			talkState[cid] = 2
		end
	elseif msgcontains(msg, 'deposit') then
		selfSay("Please tell me how much gold it is you would like to deposit.", cid)
		talkState[cid] = 1
	elseif talkState[cid] == 1 then
		count[cid] = getCount(msg)
		if isValidMoney(count[cid]) then
			selfSay("Would you really like to deposit " .. count[cid] .. " gold?", cid)
			talkState[cid] = 2
		else
			selfSay("Is isnt valid amount of gold to deposit.", cid)
			talkState[cid] = 0
		end
	elseif talkState[cid] == 2 then
		if msgcontains(msg, 'yes') then
			if not doPlayerDepositMoney(cid, count[cid]) then
				selfSay("You don\'t have enough gold.", cid)
			else
				selfSay("Alright, we have added the amount of " .. count[cid] .. " gold to your balance. You can withdraw your money anytime you want to. Your account balance is " .. getPlayerBalance(cid) .. ".", cid)
				doPlayerSave(cid)
			end
		elseif msgcontains(msg, 'no') then
			selfSay("As you wish. Is there something else I can do for you?", cid)
		end
		talkState[cid] = 0
---------------------------- withdraw --------------------
	elseif msgcontains(msg, 'withdraw') then
		selfSay("Please tell me how much gold you would like to withdraw.", cid)
		talkState[cid] = 6
	elseif talkState[cid] == 6 then
		count[cid] = getCount(msg)
		if isValidMoney(count[cid]) then
			selfSay("Are you sure you wish to withdraw " .. count[cid] .. " gold from your bank account?", cid)
			talkState[cid] = 7
		else
			selfSay("Is isnt valid amount of gold to withdraw.", cid)
			talkState[cid] = 0
		end
	elseif talkState[cid] == 7 then
		if msgcontains(msg, 'yes') then
			if not doPlayerWithdrawMoney(cid, count[cid]) then
				selfSay("There is not enough gold on your account. Your account balance is " .. getPlayerBalance(cid) .. ". Please tell me the amount of gold coins you would like to withdraw.", cid)
				talkState[cid] = 0
			else
				selfSay("Here you are, " .. count[cid] .. " gold. Please let me know if there is something else I can do for you.", cid)
				talkState[cid] = 0
				doPlayerSave(cid)
			end
		elseif msgcontains(msg, 'no') then
			selfSay("As you wish. Is there something else I can do for you?", cid)
			talkState[cid] = 0
		end
---------------------------- transfer --------------------
	elseif msgcontains(msg, 'transfer') then
		selfSay("Please tell me the amount of gold you would like to transfer.", cid)
		talkState[cid] = 11
	elseif talkState[cid] == 11 then
		count[cid] = getCount(msg)
		if getPlayerBalance(cid) < count[cid] then
			selfSay("You dont have enough money on your bank account.", cid)
			talkState[cid] = 0
			return true
		end

		if isValidMoney(count[cid]) then
			selfSay("Who would you like transfer " .. count[cid] .. " gold to?", cid)
			talkState[cid] = 12
		else
			selfSay("Is isnt valid amount of gold to transfer.", cid)
			talkState[cid] = 0
		end
	elseif talkState[cid] == 12 then
		transfer[cid] = msg

		if getCreatureName(cid) == transfer[cid] then
			selfSay("Ekhm, You want transfer money to yourself? Its impossible!", cid)
			talkState[cid] = 0
			return true
		end

		if isInArray(config.transferDisabledVocations, getPlayerVocation(cid)) then
			selfSay("Your vocation cannot transfer money.", cid)
			talkState[cid] = 0
		end

		if playerExists(transfer[cid]) then
			selfSay("So you would like to transfer " .. count[cid] .. " gold to \"" .. transfer[cid] .. "\" ?", cid)
			talkState[cid] = 13
		else
			selfSay("Player with name \"" .. transfer[cid] .. "\" doesnt exist.", cid)
			talkState[cid] = 0
		end
	elseif talkState[cid] == 13 then
		if msgcontains(msg, 'yes') then
			local targetVocation = getPlayerVocationByName(transfer[cid])
			if not targetVocation or isInArray(config.transferDisabledVocations, targetVocation) or not doPlayerTransferMoneyTo(cid, transfer[cid], count[cid]) then
				selfSay("This player does not exist on this world or have no vocation.", cid)
			else
				selfSay("You have transferred " .. count[cid] .. " gold to \"" .. transfer[cid] .."\".", cid)
				transfer[cid] = nil
				doPlayerSave(cid)
			end
		elseif msgcontains(msg, 'no') then
			selfSay("As you wish. Is there something else I can do for you?", cid)
		end
		talkState[cid] = 0
---------------------------- money exchange --------------
	elseif msgcontains(msg, 'change gold') then
		npcHandler:say("How many platinum coins would you like to get?", cid)
		talkState[cid] = 14
	elseif talkState[cid] == 14 then
		if getCount(msg) == -1 or getCount(msg) == 0 then
			npcHandler:say("Hmm, can I help you with something else?", cid)
			talkState[cid] = 0
		else
			count[cid] = getCount(msg)
			npcHandler:say("So you would like me to change " .. count[cid] * 100 .. " of your gold coins into " .. count[cid] .. " platinum coins?", cid)
			talkState[cid] = 15
		end
	elseif talkState[cid] == 15 then
		if msgcontains(msg, 'yes') then
			if doPlayerRemoveItem(cid, 2148, count[cid] * 100) then
				doPlayerAddItem(cid, 2152, count[cid])
				npcHandler:say("Here you are.", cid)
			else
				npcHandler:say("Sorry, you do not have enough gold coins.", cid)
			end
		else
			npcHandler:say("Well, can I help you with something else?", cid)
		end
		talkState[cid] = 0
	elseif msgcontains(msg, 'change platinum') then
		npcHandler:say("Would you like to change your platinum coins into gold or crystal?", cid)
		talkState[cid] = 16
	elseif talkState[cid] == 16 then
		if msgcontains(msg, 'gold') then
			npcHandler:say("How many platinum coins would you like to change into gold?", cid)
			talkState[cid] = 17
		elseif msgcontains(msg, 'crystal') then
			npcHandler:say("How many crystal coins would you like to get?", cid)
			talkState[cid] = 19
		else
			npcHandler:say("Well, can I help you with something else?", cid)
			talkState[cid] = 0
		end
	elseif talkState[cid] == 17 then
		if getCount(msg) == -1 or getCount(msg) == 0 then
			npcHandler:say("Hmm, can I help you with something else?", cid)
			talkState[cid] = 0
		else
			count[cid] = getCount(msg)
			npcHandler:say("So you would like me to change " .. count[cid] .. " of your platinum coins into " .. count[cid] * 100 .. " gold coins for you?", cid)
			talkState[cid] = 18
		end
	elseif talkState[cid] == 18 then
		if msgcontains(msg, 'yes') then
			if doPlayerRemoveItem(cid, 2152, count[cid]) then
				npcHandler:say("Here you are.", cid)
				doPlayerAddItem(cid, 2148, count[cid] * 100)
			else
				npcHandler:say("Sorry, you do not have enough platinum coins.", cid)
			end
		else
			npcHandler:say("Well, can I help you with something else?", cid)
		end
		talkState[cid] = 0
	elseif talkState[cid] == 19 then
		if getCount(msg) == -1 or getCount(msg) == 0 then
			npcHandler:say("Hmm, can I help you with something else?", cid)
			talkState[cid] = 0
		else
			count[cid] = getCount(msg)
			npcHandler:say("So you would like me to change " .. count[cid] * 100 .. " of your platinum coins into " .. count[cid] .. " crystal coins for you?", cid)
			talkState[cid] = 20
		end
	elseif talkState[cid] == 20 then
		if msgcontains(msg, 'yes') then
			if doPlayerRemoveItem(cid, 2152, count[cid] * 100) then
				npcHandler:say("Here you are.", cid)
				doPlayerAddItem(cid, 2160, count[cid])
			else
				npcHandler:say("Sorry, you do not have enough platinum coins.", cid)
			end
		else
			npcHandler:say("Well, can I help you with something else?", cid)
		end
		talkState[cid] = 0
	elseif msgcontains(msg, 'change crystal') then
		npcHandler:say("How many crystal coins would you like to change into platinum?", cid)
		talkState[cid] = 21
	elseif talkState[cid] == 21 then
		if getCount(msg) == -1 or getCount(msg) == 0 then
			npcHandler:say("Hmm, can I help you with something else?", cid)
			talkState[cid] = 0
		else
			count[cid] = getCount(msg)
			npcHandler:say("So you would like me to change " .. count[cid] .. " of your crystal coins into " .. count[cid] * 100 .. " platinum coins for you?", cid)
			talkState[cid] = 22
		end
	elseif talkState[cid] == 22 then
		if msgcontains(msg, 'yes') then
			if doPlayerRemoveItem(cid, 2160, count[cid])  then
				npcHandler:say("Here you are.", cid)
				doPlayerAddItem(cid, 2152, count[cid] * 100)
			else
				npcHandler:say("Sorry, you do not have enough crystal coins.", cid)
			end
		else
			npcHandler:say("Well, can I help you with something else?", cid)
		end
		talkState[cid] = 0
	elseif msgcontains(msg, 'change') then
		npcHandler:say("There are three different coin types in Tibia: 100 gold coins equal 1 platinum coin, 100 platinum coins equal 1 crystal coin. So if you'd like to change 100 gold into 1 platinum, simply say '{change gold}' and then '1 platinum'.", cid)
		talkState[cid] = 0
	end

	return true
end

npcHandler:setCallback(CALLBACK_GREET, greetCallback)
npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
