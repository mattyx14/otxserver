local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_TELEPORT)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -1.3, -0, -1.7, 0)

local arr = {
{0, 1, 0},
{0, 1, 0},
{0, 1, 0},
{0, 1, 0},
{0, 1, 0},
{0, 1, 0},
{0, 1, 0},
{0, 3, 0},
}

local area = createCombatArea(arr)

setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end