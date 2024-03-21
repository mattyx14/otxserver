function onLogin(cid)
	registerCreatureEvent(cid, SummonKill)

	return true
end

function onKill(cid, target)
	if isMonster(target) and isPlayer(getCreatureMaster(target)) then
		registerCreatureEvent(target, ObMonsterCheck)
	end

	return true
end

function onDeath(cid, corpse)
	doItemSetAttribute(corpse.uid, aid, 91347)

	return true
end