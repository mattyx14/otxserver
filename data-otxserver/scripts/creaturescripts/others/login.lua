local playerLogin = CreatureEvent("PlayerLogin")

function playerLogin.onLogin(player)
	-- Premium Ends, change addon (citizen) houseless
	if not player:isPremium() then
		player:sendTextMessage(MESSAGE_FAILURE, "Your premium time has expired.")

		if sex == 1 then
			player:setOutfit({ lookType = 128, lookFeet = 114, lookLegs = 134, lookHead = 114, lookAddons = 0 })
		elseif sex == 0 then
			player:setOutfit({ lookType = 136, lookFeet = 114, lookLegs = 134, lookHead = 114, lookAddons = 0 })
		end

		if home and not player:isPremium() then
			setHouseOwner(home, 0)
			player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "You've lost your house because you are not premium anymore.")
			player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "Your items from house are send to your inbox.")
		end
	end

	-- Open channels
	if table.contains({ TOWNS_LIST.DAWNPORT, TOWNS_LIST.DAWNPORT_TUTORIAL }, player:getTown():getId()) then
		player:openChannel(3) -- World chat
	else
		player:openChannel(3) -- World chat
		player:openChannel(5) -- Advertsing main
		if player:getGuild() then
			player:openChannel(0x00) -- guild
		end
	end
	return true
end

playerLogin:register()
