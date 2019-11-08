function onCastSpell(cid, var)
    local health, hp, cpos = math.random(7500, 9000), (getCreatureHealth(cid)/getCreatureMaxHealth(cid))*100, getCreaturePosition(cid)
	
    if isCreature(cid) == true and getCreatureName(cid) == "Omrafir" and (hp < 99.99) then
	if getTileItemById(cpos,1487).uid ~= 0 or getTileItemById(cpos,1492).uid ~= 0 or getTileItemById(cpos,1500).uid ~= 0 then
        doCreatureAddHealth(cid, health)
    doSendMagicEffect(cpos, CONST_ME_MAGIC_BLUE)
    doCreatureSay(cid, "Omrafir gains new strength from the fire", TALKTYPE_ORANGE_1)
		end
	end
end