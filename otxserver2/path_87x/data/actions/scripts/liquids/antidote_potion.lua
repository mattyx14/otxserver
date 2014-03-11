-- NOTE: This needs update acccording to potions.lua!
local config = {
	removeOnUse = "no",
	usableOnTarget = "yes", -- can be used on target? (fe. healing friend)
	range = -1,
	realAnimation = "no", -- make text effect visible only for players in range 1x1
	flask = 7636,
	splash = 41
}

config.removeOnUse = getBooleanFromString(config.removeOnUse)
config.usableOnTarget = getBooleanFromString(config.usableOnTarget)
config.realAnimation = getBooleanFromString(config.realAnimation)

local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_TARGETCASTERORTOPMOST, true)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_POISON)

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(not isPlayer(itemEx.uid) or (not config.usableOnTarget and cid ~= itemEx.uid)) then
		if(config.splash < 1) then
			return true
		end

		if(toPosition.x == CONTAINER_POSITION) then
			toPosition = getThingPosition(item.uid)
		end

		doDecayItem(doCreateItem(POOL, config.splash, toPosition))
		doRemoveItem(item.uid, 1)
		if(not config.flask or config.removeOnUse) then
			return true
		end

		if(fromPosition.x ~= CONTAINER_POSITION) then
			doCreateItem(config.flask, fromPosition)
		else
			doPlayerAddItem(cid, config.flask, 1)
		end

		return true
	end

	if(config.range > 0 and cid ~= itemEx.uid and getDistanceBetween(getCreaturePosition(cid), getCreaturePosition(itemEx.uid)) > config.range) then
		return true
	end

	if(not doCombat(cid, combat, numberToVariant(itemEx.uid))) then
		return true
	end

	doSendMagicEffect(getThingPosition(itemEx.uid), CONST_ME_MAGIC_BLUE)
	if(not config.realAnimation) then
		doCreatureSay(itemEx.uid, "Aaaah...", TALKTYPE_ORANGE_1)
	else
		for i, tid in ipairs(getSpectators(getThingPosition(itemEx.uid), 1, 1)) do
			if(isPlayer(tid)) then
				doCreatureSay(itemEx.uid, "Aaaah...", TALKTYPE_ORANGE_1, false, tid)
			end
		end
	end

	doRemoveItem(item.uid, 1)
	if(not config.flask or config.removeOnUse) then
		return true
	end

	if(fromPosition.x ~= CONTAINER_POSITION) then
		doCreateItem(config.flask, fromPosition)
	else
		doPlayerAddItem(cid, config.flask, 1)
	end

	return true
end
