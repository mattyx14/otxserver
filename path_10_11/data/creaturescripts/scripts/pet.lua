--By Igor Labanca
function onLogin(cid)
	local player = Player(cid)
	local STORAGE_PET = 60045

	vocationid = player:getVocation():getId()
	if vocationid == 5 and player:getStorageValue(STORAGE_PET) > os.time() then
		pet = "thundergiant"
		position = player:getPosition()
		summonpet = Game.createMonster(pet, position)
		player:addSummon(summonpet)
		position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	elseif vocationid == 6 and player:getStorageValue(STORAGE_PET) > os.time() then
		pet = "grovebeast"
		position = player:getPosition()
		summonpet = Game.createMonster(pet, position)
		player:addSummon(summonpet)
		position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	elseif vocationid == 7 and player:getStorageValue(STORAGE_PET) > os.time() then
		pet = "emberwing"
		position = player:getPosition()
		summonpet = Game.createMonster(pet, position)
		player:addSummon(summonpet)
		position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	elseif vocationid == 8 and player:getStorageValue(STORAGE_PET) > os.time() then
		pet = "skullfrost"
		position = player:getPosition()
		summonpet = Game.createMonster(pet, position)
		player:addSummon(summonpet)
		position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	end

	return true
end

function onThink(cid, interval, item, position, lastPosition, fromPosition, toPosition)
	local player = Player(cid)
	local STORAGE_PET = 60045
	vocationid = player:getVocation():getId()
	if vocationid == 5 and player:getStorageValue(STORAGE_PET) <= os.time() and player:getStorageValue(STORAGE_PET) > 0 then
		doRemoveCreature (getCreatureSummons(cid)[1])
		player:setStorageValue(STORAGE_PET,0)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE,"Summon Removido.")
	elseif vocationid == 6 and player:getStorageValue(STORAGE_PET) <= os.time() and player:getStorageValue(STORAGE_PET) > 0 then
		doRemoveCreature (getCreatureSummons(cid)[1])
		player:setStorageValue(STORAGE_PET,0)
	elseif vocationid == 7 and player:getStorageValue(STORAGE_PET) <= os.time() and player:getStorageValue(STORAGE_PET) > 0 then
		doRemoveCreature (getCreatureSummons(cid)[1])
		player:setStorageValue(STORAGE_PET,0)
	elseif vocationid == 8 and player:getStorageValue(STORAGE_PET) <= os.time() and player:getStorageValue(STORAGE_PET) > 0 then
		doRemoveCreature (getCreatureSummons(cid)[1])
		player:setStorageValue(STORAGE_PET,0)
	end

	return true
end
