local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_FIREAREA)

function getSpellDamage(cid, min1, max1)

    local min1, max1 = -7000, -10000

    return min1, max1
end

setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")

 arr1 = {
{0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}
   
local area1 = createCombatArea(arr1)
setCombatArea(combat, area1)

-----------------------------------------------------------------------------

local combat2 = createCombatObject()
setCombatParam(combat2, COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
setCombatParam(combat2, COMBAT_PARAM_EFFECT, CONST_ME_FIREAREA)

function getSpellDamage(cid, min2, max2)

    local min2, max2 = -7000, -10000

    return min2, max2
end

setCombatCallback(combat2, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")

 arr2 = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}
   
local area2 = createCombatArea(arr2)
setCombatArea(combat2, area2)

--------------------------------------------------------------------------

local combat3 = createCombatObject()
setCombatParam(combat3, COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
setCombatParam(combat3, COMBAT_PARAM_EFFECT, CONST_ME_FIREAREA)

function getSpellDamage(cid, min3, max3)

    local min3, max3 = -7000, -10000

    return min3, max3
end

setCombatCallback(combat3, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")

 arr3 = {
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}
   
local area3 = createCombatArea(arr3)
setCombatArea(combat3, area3)

--------------------------------------------------------------------------

local combat4 = createCombatObject()
setCombatParam(combat4, COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
setCombatParam(combat4, COMBAT_PARAM_EFFECT, CONST_ME_FIREAREA)

function getSpellDamage(cid, min4, max4)

    local min4, max4 = -7000, -10000

    return min4, max4
end

setCombatCallback(combat4, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")

 arr4 = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}
   
local area4 = createCombatArea(arr4)
setCombatArea(combat4, area4)

--------------------------------------------------------------

local function delayedCastSpell(cid, var)
    local creature = Creature(cid)
    if isCreature(cid) == true then
        if creature:getDirection() == 0 then  
            doCombat(cid, combat, positionToVariant(getCreaturePosition(cid)))
        elseif creature:getDirection() == 1 then  
            doCombat(cid, combat2, positionToVariant(getCreaturePosition(cid)))
        elseif creature:getDirection() == 2 then  
            doCombat(cid, combat3, positionToVariant(getCreaturePosition(cid)))
        elseif creature:getDirection() == 3 then  
            doCombat(cid, combat4, positionToVariant(getCreaturePosition(cid)))
        end
        doCreatureSay(cid, "OMRAFIR BREATHES INFERNAL FIRE", TALKTYPE_ORANGE_2)
    end
end

function onCastSpell(cid, var)
    doCreatureSay(cid, "OMRAFIR INHALES DEEPLY!", TALKTYPE_ORANGE_2)
    return true
end