local playerLogin = CreatureEvent("PlayerLogin")

function playerLogin.onLogin(player)
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
