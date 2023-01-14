local citizen = MoveEvent()

function citizen.onStepIn(creature, item, position, fromPosition)
	if item.actionid > 30020 and item.actionid < 30050 then
		local player = creature:getPlayer()
		if player == nil then
			return false
		end

		local town = Town(item.actionid - 30020)
		player:setTown(town)
		player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You are the newest resident of " .. town:getName(town) .. ".")
	end
	return true
end

citizen:type("stepin")
citizen:id(775, 1949, 1959)
citizen:register()
