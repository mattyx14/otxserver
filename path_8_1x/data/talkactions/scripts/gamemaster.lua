local ignore = createConditionObject(CONDITION_GAMEMASTER, -1, false, GAMEMASTER_IGNORE, CONDITIONID_DEFAULT)
local teleport = createConditionObject(CONDITION_GAMEMASTER, -1, false, GAMEMASTER_TELEPORT, CONDITIONID_DEFAULT)
local light = createConditionObject(CONDITION_LIGHT, -1, false, 0, CONDITIONID_DEFAULT)
	setConditionParam(light, CONDITION_PARAM_LIGHT_LEVEL, 255)
	setConditionParam(light, CONDITION_PARAM_LIGHT_COLOR, 215)

function onSay(cid, words, param, channel)
	local condition, type, subId, name = ignore, CONDITION_GAMEMASTER, GAMEMASTER_IGNORE, "private messages ignoring"
	if(words:sub(2, 2) == "c") then
		condition, subId, name = teleport, GAMEMASTER_TELEPORT, "map click teleport"
	elseif(words:sub(2, 2) == "l") then
		condition, type, subId, name = light, CONDITION_LIGHT, 0, "full light"
	end

	local action = "off"
	if(not getCreatureCondition(cid, type, subId, CONDITIONID_DEFAULT)) then
		doAddCondition(cid, condition)
		action = "on"
	else
		doRemoveCondition(cid, type, subId, CONDITIONID_DEFAULT)
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You have turned " .. action .. " " .. name .. ".")
	return true
end
