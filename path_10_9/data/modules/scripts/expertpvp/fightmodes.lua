function onRecvbyte(player, msg, byte)
	local fightMode = msggetByte()
	local chaseMode = msggetByte()
	local secureMode = msggetByte() ~= 0
	local pvpMode, oldPvpMode = msggetByte(), playergetPvpMode()
	local expertPvp = configManager.getBoolean(configKeys.EXPERT_PVP)

	playersetFightMode(fightMode)
	playersetChaseMode(chaseMode)
	if expertPvp then
		local worldType = Game.getWorldType()
		if worldType == WORLD_TYPE_NO_PVP and pvpMode == PVP_MODE_RED_FIST then
			playersetPvpMode(playergetPvpMode())
		elseif worldType == WORLD_TYPE_PVP_ENFORCED and pvpMode ~= PVP_MODE_RED_FIST then
			playersetPvpMode(PVP_MODE_RED_FIST)
		else
			playersetPvpMode(pvpMode)
		end

		-- SecureMode Formula!
		if worldType == WORLD_TYPE_NO_PVP and secureMode == false or (worldType == WORLD_TYPE_PVP_ENFORCED and secureMode == true) then
			playersetSecureMode(not secureMode)
		else
			if playergetPvpMode() == PVP_MODE_RED_FIST then
				-- force secureMode false while red fist is on!
				playersetSecureMode(false)
			else
				playersetSecureMode(secureMode)
			end

			if oldPvpMode == PVP_MODE_RED_FIST then
				playersetSecureMode(true)
			end
		end

		addPlayerEvent(Player.updateFightModes, 250, player)
	else
		playersetSecureMode(secureMode)
	end
end
