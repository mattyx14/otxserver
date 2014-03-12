local combat = createCombatObject()
local area = createCombatArea(AREA_CROSS5X5)
setCombatArea(combat, area)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

local condition = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition, CONDITION_PARAM_SUBID, 2)
setConditionParam(condition, CONDITION_PARAM_BUFF, true)
setConditionParam(condition, CONDITION_PARAM_TICKS, 2 * 60 * 1000)
setConditionParam(condition, CONDITION_PARAM_SKILL_SHIELD, 2)

local config = {
	baseMana = 90,
	hardcoreManaSpent = getConfigValue("addManaSpentInPvPZone")
}

function onCastSpell(cid, var)
	local pos, membersList = getCreaturePosition(cid), getPartyMembers(cid)
	if(membersList == nil or type(membersList) ~= 'table' or table.maxn(membersList) <= 1) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOPARTYMEMBERSINRANGE)
		doSendMagicEffect(pos, CONST_ME_POFF)
		return false
	end

	local affectedList = {}
	for _, pid in ipairs(membersList) do
		if(getDistanceBetween(getCreaturePosition(pid), pos) <= 36) then
			table.insert(affectedList, pid)
		end
	end

	local tmp = table.maxn(affectedList)
	if(tmp <= 1) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOPARTYMEMBERSINRANGE)
		doSendMagicEffect(pos, CONST_ME_POFF)
		return false
	end

	local mana = math.ceil((0.9 ^ (tmp - 1) * config.baseMana) * tmp)
	if(getCreatureMana(cid) < mana) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTENOUGHMANA)
		doSendMagicEffect(pos, CONST_ME_POFF)
		return false
	end

	if(not doCombat(cid, combat, var)) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		doSendMagicEffect(pos, CONST_ME_POFF)
		return false
	end

	doCreatureAddMana(cid, -(mana - config.baseMana), false)
	if(not getPlayerFlagValue(cid, PlayerFlag_NotGainMana) and (not getTileInfo(getThingPosition(cid)).hardcore or config.hardcoreManaSpent)) then
		doPlayerAddSpentMana(cid, (mana - config.baseMana))
	end

	for _, pid in ipairs(affectedList) do
		doAddCondition(pid, condition)
	end

	return true
end
