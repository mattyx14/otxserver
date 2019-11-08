local voc = {1, 2, 3, 4, 5, 6, 7, 8}

local condition = createConditionObject(CONDITION_REGENERATION)
setConditionParam(condition, CONDITION_PARAM_SUBID, 88888)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10 * 60 * 1000)
setConditionParam(condition, CONDITION_PARAM_HEALTHGAIN, 0.01)
setConditionParam(condition, CONDITION_PARAM_HEALTHTICKS, 10 * 60 * 1000)

	arr = {
	{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
	{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
	}

local area = createCombatArea(arr)

local combat = createCombatObject()
setCombatArea(combat, area)

function onTargetTile(cid, pos)
    local creatureTable = {}
    local n, i = getTileInfo({x=pos.x, y=pos.y, z=pos.z}).creatures, 1
    if n ~= 0 then
        local v = getThingfromPos({x=pos.x, y=pos.y, z=pos.z, stackpos=i}).uid
        while v ~= 0 do
            if isCreature(v) == true then
                table.insert(creatureTable, v)
                if n == #creatureTable then
                    break
                end
            end
            i = i + 1
            v = getThingfromPos({x=pos.x, y=pos.y, z=pos.z, stackpos=i}).uid
        end
    end
    if #creatureTable ~= nil and #creatureTable > 0 then
        for r = 1, #creatureTable do
            if creatureTable[r] ~= cid then
                local min = 1500
                local max = 1700
                if isPlayer(creatureTable[r]) == true and table.contains(voc, getPlayerVocation(creatureTable[r])) == true then
                    doTargetCombatHealth(cid, creatureTable[r], COMBAT_FIREDAMAGE, -min, -max, CONST_ME_NONE)
                elseif isMonster(creatureTable[r]) == true then
                    doTargetCombatHealth(cid, creatureTable[r], COMBAT_FIREDAMAGE, -min, -max, CONST_ME_NONE)
                end
            end
        end
    end
    doSendMagicEffect(pos, CONST_ME_FIREAREA)
    return true
end

setCombatCallback(combat, CALLBACK_PARAM_TARGETTILE, "onTargetTile")

local function delayedCastSpell(cid, var)
    if isCreature(cid) == true then
        doCombat(cid, combat, positionToVariant(getCreaturePosition(cid)))
    end
end

function onCastSpell(cid, var)
    if isCreature(cid) == true then
        if getCreatureHealth(cid) < getCreatureMaxHealth(cid) * 0.1 and getCreatureCondition(cid, CONDITION_REGENERATION, 88888) == false then
            doAddCondition(cid, condition)
			addEvent(delayedCastSpell, 5000, cid, var)
			doCreatureSay(cid, "Better flee now.", TALKTYPE_ORANGE_1)
        else
            return false
        end
    else
        return false
    end
    return true
end