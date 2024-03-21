local specialQuests = {
	[2001] = 30015 --Annihilator
}

local questsExperience = {
	[30015] = 10000
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(getBooleanFromString(getConfigValue('enableProtectionQuestForGM'))) then
		if(getPlayerCustomFlagValue(cid, PLAYERCUSTOMFLAG_GAMEMASTERPRIVILEGES)) then
			doSendMagicEffect(getCreaturePosition(cid), CONST_ME_POFF, cid)
			return true
		end
	end

	local storage = specialQuests[item.actionid]
	if(not storage) then
		storage = item.uid
		if(storage > 65535) then
			return false
		end
	end

	if(getCreatureStorage(cid, storage) > 0) then
	local ret = getItemDescriptions(item.uid)
		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "The " .. ret.name .. " is empty.")
		return true
	end

	local items = {}
	local reward = 0

	local size = isContainer(item.uid) and getContainerSize(item.uid) or 0
	if(size == 0) then
		reward = doCopyItem(item, false)
	else
		for i = 0, size do
			local tmp = getContainerItem(item.uid, i)
			if(tmp.itemid > 0) then
				table.insert(items, tmp)
			end
		end
	end

	size = table.maxn(items)
	if(size == 1) then
		reward = doCopyItem(items[1], true)
	end

	local result = ""
	if(reward ~= 0) then
		local ret = getItemDescriptions(reward.uid)
		if(reward.type > 0 and isItemRune(reward.itemid)) then
			result = reward.type .. " charges " .. ret.name
		elseif(reward.type > 0 and isItemStackable(reward.itemid)) then
			result = reward.type .. " " .. ret.plural
		else
			result = ret.article .. " " .. ret.name
		end
	else
		if(size > 20) then
			reward = doCopyItem(item, false)
		elseif(size > 8) then
			reward = getThing(doCreateItemEx(1988, 1))
		else
			reward = getThing(doCreateItemEx(1987, 1))
		end

		for i = 1, size do
			local tmp = doCopyItem(items[i], true)
			if(doAddContainerItemEx(reward.uid, tmp.uid) ~= RETURNVALUE_NOERROR) then
				print("[Warning] QuestSystem:", "Could not add quest reward")
			else
				local ret = ", "
				if(i == 2) then
					ret = " and "
				elseif(i == 1) then
					ret = ""
				end

				result = result .. ret
				ret = getItemDescriptions(tmp.uid)
				if(tmp.type > 0 and isItemRune(tmp.itemid)) then
					result = result .. tmp.type .. " charges " .. ret.name
				elseif(tmp.type > 0 and isItemStackable(tmp.itemid)) then
					result = result .. tmp.type .. " " .. ret.plural
				else
					result = result .. ret.article .. " " .. ret.name
				end
			end
		end
	end

	local ret = getItemDescriptions(reward.uid)
	result = "You have found " .. ret.article .. " " .. ret.name
	if(doPlayerAddItemEx(cid, reward.uid, false) == RETURNVALUE_NOERROR)then
		result = result .. "."
		doCreatureSetStorage(cid, storage, 1)
	else
		result = (getPlayerFreeCap(cid) < getItemWeight(reward.uid)) and result .. ". Weighing " .. getItemWeight(reward.uid) .. " oz it is too heavy." or result .. ", but you have no room to take it."
		doPlayerSaveEx(cid)
	end	
		if(questsExperience[storage] ~= nil) then
			doPlayerAddExpEx(cid, questsExperience[storage])
		end

	doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, result)
	return true
end
