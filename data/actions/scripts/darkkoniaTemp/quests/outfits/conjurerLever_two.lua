local config = {
	{fromPosition = Position(1240, 311, 9), toPosition = Position(857, 1114, 10), sacrificePosition = Position(1241, 311, 9), sacrificeId = 5942},
	{fromPosition = Position(1213, 259, 9), toPosition = Position(861, 1114, 10), sacrificePosition = Position(1212, 258, 9), sacrificeId = 5891},
	{fromPosition = Position(1253, 368, 9), toPosition = Position(865, 1114, 10), sacrificePosition = Position(1257, 372, 9), sacrificeId = 7314}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	item:transform(item.itemid == 1945 and 1946 or 1945)

	if item.itemid ~= 1945 then
		return true
	end

	local position = player:getPosition()

	local players = {}
	for i = 1, #config do
		local creature = Tile(config[i].fromPosition):getTopCreature()
		if not creature or not creature:isPlayer() then
			player:sendCancelMessage('You need more players for make this quest.')
			position:sendMagicEffect(CONST_ME_POFF)
			return true
		end

		local sacrificeItem = Tile(config[i].sacrificePosition):getItemById(config[i].sacrificeId)
		if not sacrificeItem then
			player:sendCancelMessage(creature:getName() .. ' is missing ' .. (creature:getSex() == PLAYERSEX_FEMALE and 'her' or 'his') .. ' sacrifice on the altar.')
			position:sendMagicEffect(CONST_ME_POFF)
			return true
		end

		players[#players + 1] = creature
	end

	for i = 1, #players do
		local sacrificeItem = Tile(config[i].sacrificePosition):getItemById(config[i].sacrificeId)
		if sacrificeItem then
			sacrificeItem:remove()
		end

		players[i]:getPosition():sendMagicEffect(CONST_ME_POFF)
		players[i]:teleportTo(config[i].toPosition)
		config[i].toPosition:sendMagicEffect(CONST_ME_TELEPORT)
	end
	return true
end
