function onCastSpell(cid, var)

	local center, center2 = {x=33529, y=32334, z=12, stackpos=255}, {x=33528, y=32334, z=12, stackpos=255}

	doCreatureSay(cid, "GET OVER HERE!", TALKTYPE_ORANGE_2, false, 0, center2)
	for x = 33519, 33538 do
	for y = 32327, 32342 do
	local a = getTopCreature({x=x, y=y, z=12}).uid
	if a ~= 0 and isCreature(a) == true and getCreatureName(a) == "Prince Drazzak" or isPlayer(a) then
	doTeleportThing(a, center)
	doTeleportThing(cid, center2)
	end
end
end
return true
end
