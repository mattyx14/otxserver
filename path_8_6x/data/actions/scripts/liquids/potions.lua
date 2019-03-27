local config = {
	endless = "no",
	removeOnUse = "no",
	usableOnTarget = "yes", -- can be used on a target? (eg. healing a friend)
	checkTargetRequirements = "yes",
	splashable = "yes",
	sameContainer = "yes",
	range = 7,
	area = {3, 3} -- if not set correctly, the message will be sent only to user of the item
}

local multiplier = {
	health = 1.0,
	mana = 1.0
}

local POTIONS = {
	[8704] = {empty = 7636, splash = 42, health = {50, 100}}, -- small health potion
	[7618] = {empty = 7636, splash = 42, health = {100, 200}}, -- health potion
	[7588] = {empty = 7634, splash = 42, health = {200, 400}, level = 50, vocations = {isKnight, isPaladin}, vocStr = "knights and paladins"}, -- strong health potion
	[7591] = {empty = 7635, splash = 42, health = {500, 700}, level = 80, vocations = {isKnight}, vocStr = "knights"}, -- great health potion
	[8473] = {empty = 7635, splash = 42, health = {800, 1000}, level = 130, vocations = {isKnight}, vocStr = "knights"}, -- ultimate health potion

	[7620] = {empty = 7636, splash = 47, mana = {70, 130}}, -- mana potion
	[7589] = {empty = 7634, splash = 47, mana = {110, 190}, level = 50, vocations = {isSorcerer, isDruid, isPaladin}, vocStr = "sorcerers, druids and paladins"}, -- strong mana potion
	[7590] = {empty = 7635, splash = 47, mana = {200, 300}, level = 80, vocations = {isSorcerer, isDruid}, vocStr = "sorcerers and druids"}, -- great mana potion

	[8472] = {empty = 7635, splash = 43, health = {200, 400}, mana = {110, 190}, level = 80, vocations = {isPaladin}, vocStr = "paladins"} -- great spirit potion
}

for i, potion in pairs(POTIONS) do
	if(type(i) == 'number') then
		for k, v in pairs(config) do
			if(not potion[k]) then
				potion[k] = v
			end
		end

		if(potion.endless) then
			potion.endless = getBooleanFromString(potion.endless)
		end

		if(potion.removeOnUse) then
			potion.removeOnUse = getBooleanFromString(potion.removeOnUse)
		end

		if(potion.usableOnTarget) then
			potion.usableOnTarget = getBooleanFromString(potion.usableOnTarget)
		end

		if(potion.checkTargetRequirements) then
			potion.checkTargetRequirements = getBooleanFromString(potion.checkTargetRequirements)
		end

		if(potion.splashable) then
			potion.splashable = getBooleanFromString(potion.splashable)
		end

		if(potion.sameContainer) then
			potion.sameContainer = getBooleanFromString(potion.sameContainer)
		end

		if(type(potion.health) == 'table' and table.size(potion.health) > 1) then
			potion.health[1] = math.ceil(potion.health[1] * multiplier.health)
			potion.health[2] = math.ceil(potion.health[2] * multiplier.health)
		else
			potion.health = nil
		end

		if(type(potion.mana) == 'table' and table.size(potion.mana) > 1) then
			potion.mana[1] = math.ceil(potion.mana[1] * multiplier.mana)
			potion.mana[2] = math.ceil(potion.mana[2] * multiplier.mana)
		else
			potion.mana = nil
		end

		POTIONS[i] = potion
	end
end

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local potion = POTIONS[item.itemid]
	if(not potion) then
		return false
	end

	if(not isPlayer(itemEx.uid) or (not potion.usableOnTarget and cid ~= itemEx.uid)) then
		if(not potion.splashable or not potion.splash) then
			return false
		end

		if(toPosition.x == CONTAINER_POSITION) then
			toPosition = getThingPosition(item.uid)
		end

		doDecayItem(doCreateItem(POOL, potion.splash, toPosition))
		doRemoveItem(item.uid, 1)
		if(not potion.empty or potion.removeOnUse) then
			return true
		end

		local flask = doCreateItemEx(potion.empty, 1)
		if(fromPosition.x ~= CONTAINER_POSITION) then
			doTileAddItemEx(fromPosition, flask)
		elseif(potion.sameContainer) then
			errors(false)
			local parent = getItemParent(item.uid)
			if(doAddContainerItemEx(parent.uid, flask) ~= RETURNVALUE_NOERROR) then
				doPlayerAddItemEx(cid, flask, true)
				errors(true)
			end
		else
			doPlayerAddItemEx(cid, flask, true)
		end

		return true
	end

	local check = itemEx.uid
	if(not potion.checkTargetRequirements) then
		check = cid
	end

	local vocation = not potion.vocations or type(potion.vocations) ~= 'table' or table.size(potion.vocations) == 0
	if(not vocation) then
		for _, func in ipairs(potion.vocations) do
			if(func(check)) then
				vocation = true
				break
			end
		end

		if(vocation and potion.noVocations and type(potion.noVocations) == 'table' and table.size(potion.noVocations) > 0) then
			for _, func in ipairs(potion.noVocations) do
				if(func(check)) then
					vocation = false
					break
				end
			end
		end
	end

	if((not vocation or (potion.level and getPlayerLevel(check) < potion.level)) and not getPlayerCustomFlagValue(cid, PLAYERCUSTOMFLAG_GAMEMASTERPRIVILEGES)) then
		doCreatureSay(cid, "Only " .. potion.vocStr .. (potion.level and (" of level " .. potion.level) or "") .. " or above may drink this fluid.", TALKTYPE_MONSTER, false, cid)
		return true
	end

	if(potion.range > 0 and cid ~= itemEx.uid and getDistanceBetween(getThingPosition(cid), getThingPosition(itemEx.uid)) > potion.range and not getPlayerCustomFlagValue(cid, PLAYERCUSTOMFLAG_CANUSEFAR)) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_TOOFARAWAY)
		return true
	end

	if(potion.health and not doTargetCombatHealth(cid, itemEx.uid, COMBAT_HEALING, potion.health[1], potion.health[2], CONST_ME_MAGIC_BLUE, false)) then
		return false
	end

	if(potion.mana and not doTargetCombatMana(cid, itemEx.uid, potion.mana[1], potion.mana[2], CONST_ME_MAGIC_BLUE, false)) then
		return false
	end

	if(type(potion.area) == 'table' and table.size(potion.area) > 1) then
		for i, tid in ipairs(getSpectators(getThingPosition(itemEx.uid), potion.area[1], potion.area[2])) do
			if(isPlayer(tid)) then
				doCreatureSay(itemEx.uid, "Aaaah...", TALKTYPE_MONSTER, false, tid)
			end
		end
	else
		doCreatureSay(itemEx.uid, "Aaaah...", TALKTYPE_MONSTER, false, itemEx.uid)
		if(itemEx.uid ~= cid) then
			doCreatureSay(itemEx.uid, "Aaaah...", TALKTYPE_MONSTER, false, cid)
		end
	end

	if(potion.endless) then
		return true
	end

	doRemoveItem(item.uid, 1)
	if(not potion.empty or potion.removeOnUse) then
		return true
	end

	local flask = doCreateItemEx(potion.empty, 1)
	if(fromPosition.x ~= CONTAINER_POSITION) then
		doTileAddItemEx(fromPosition, flask)
	elseif(potion.sameContainer) then
		errors(false)
		local parent = getItemParent(item.uid)
		if(doAddContainerItemEx(parent.uid, flask) ~= RETURNVALUE_NOERROR) then
			doPlayerAddItemEx(cid, flask, true)
			errors(true)
		end
	else
		doPlayerAddItemEx(cid, flask, true)
	end

	return true
end
