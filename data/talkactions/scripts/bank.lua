function Player.deposit(self, amount)
	if not self:removeMoney(amount) then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "[BankSystem]: You dont have money with you.")
		return false
	end

	self:setBankBalance(self:getBankBalance() + amount)
	return true
end

function Player.depositMoney(self, amount)
	if not self:removeMoney(amount) then
		return false
	end

	self:setBankBalance(self:getBankBalance() + amount)
	return true
end

function onSay(player, words, param)
	local split = param:split(",")
	local balance = player:getBankBalance()
	if split[1] == nil then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "[BankSystem]: the commands are:\n !bank depositall.")
		return
	end

	local storage = 54074 -- Make sure to select non-used storage. This is used to prevent SQL load attacks.
	local cooldown = 60 -- in seconds.

	if player:getStorageValue(storage) <= os.time() then
		player:setStorageValue(storage, os.time() + cooldown)
	--------------------------- Balance ---------------------------
		if split[1] == 'depositall' then
			local amount = player:getMoney()
			local amount = math.abs(amount)
			if amount > 0 and amount == player:getMoney() then
				player:deposit(amount)
				player:save()
				player:sendTextMessage(MESSAGE_STATUS_SMALL, "[BankSystem]: You added " .. amount .. " to your account, You can withdraw your money anytime you want to.\nYour account balance is " .. player:getBankBalance() .. ".")
			else
				player:sendTextMessage(MESSAGE_STATUS_SMALL, "[BankSystem]: You do not have enough money to deposit.")
			end
		else
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "[BankSystem]: Invalid param.")
		end
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Can only be executed once every " .. cooldown .. " seconds. Remaining cooldown: " .. player:getStorageValue(storage) - os.time())
	end

	return false
end
