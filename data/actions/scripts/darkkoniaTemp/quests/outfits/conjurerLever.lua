local config = {
	{fromPosition = Position(1046, 181, 9), toPosition = Position(1232, 287, 8), sacrificePosition = Position(1046, 182, 9), sacrificeId = 2746},
	{fromPosition = Position(1051, 181, 9), toPosition = Position(1172, 345, 8), sacrificePosition = Position(1051, 182, 9), sacrificeId = 2744},
	{fromPosition = Position(1056, 181, 9), toPosition = Position(1237, 407, 8), sacrificePosition = Position(1056, 182, 9), sacrificeId = 2745},
	{fromPosition = Position(1051, 139, 9), toPosition = Position(1051, 148, 9), sacrificePosition = Position(1050, 137, 9), sacrificeId = 6560},
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
