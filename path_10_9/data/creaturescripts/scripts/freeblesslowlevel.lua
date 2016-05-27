local freeBlessMaxLevel = 100

function onLogin(player)
	if player:getLevel() <= freeBlessMaxLevel then
		for i = 1, 6 do
			player:addBlessing(i)
			player:getPosition():sendMagicEffect(CONST_ME_HOLYDAMAGE)
			player:say('FREE BLESS LVL ' .. freeBlessMaxLevel .. '.', TALKTYPE_ORANGE_1)
		end
	end

	if player:getStorageValue(Storage.FreeBless) == 1 then
		for i = 1, 6 do
			player:addBlessing(i)
			player:getPosition():sendMagicEffect(CONST_ME_HOLYDAMAGE)
		end
	end
	return true
end