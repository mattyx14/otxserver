local freeBlessMaxLevel = 50

function onLogin(cid)
	local player = Player(cid)
	if player:getLevel() <= freeBlessMaxLevel then
		for i = 1, 5 do
			player:addBlessing(i)
		end
		player:say('FREE BLESS LVL 50.', TALKTYPE_ORANGE_1)
		player:getPosition():sendMagicEffect(CONST_ME_HOLYDAMAGE)
	end
	return true
end