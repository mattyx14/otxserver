local voc = {3, 7}

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
                local min = 4000
                local max = 8000
                if isPlayer(creatureTable[r]) == true and table.contains(voc, getPlayerVocation(creatureTable[r])) == true then
                    doTargetCombatHealth(cid, creatureTable[r], COMBAT_ENERGYDAMAGE, -min, -max, CONST_ME_NONE)
                elseif isMonster(creatureTable[r]) == true then
                    doTargetCombatHealth(cid, creatureTable[r], COMBAT_ENERGYDAMAGE, -min, -max, CONST_ME_NONE)
                end
            end
        end
    end
    doSendMagicEffect(pos, CONST_ME_ENERGYHIT)
    return true
end

setCombatCallback(combat, CALLBACK_PARAM_TARGETTILE, "onTargetTile")

local function delayedCastSpell(cid, var)
    if isCreature(cid) == true then
        doCombat(cid, combat, positionToVariant(getCreaturePosition(cid)))
	doCreatureSay(cid, "DIE!", TALKTYPE_ORANGE_1)
    end
end

function onCastSpell(cid, var)
    doCreatureSay(cid, "All PALADINS must DIE!", TALKTYPE_ORANGE_1)
    addEvent(delayedCastSpell, 4000, cid, var)
    return true
end