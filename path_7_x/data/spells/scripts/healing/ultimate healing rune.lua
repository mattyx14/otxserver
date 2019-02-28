local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_TARGETCASTERORTOPMOST, 1)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

function onGetFormulaValues(cid, level, maglevel)
if getPlayerVocation(cid) == 2 or getPlayerVocation(cid) == 4 then
min = (level * 2.2 + maglevel * 3.9) * 2.2
max = (level * 2.5 + maglevel * 4.1) * 2.8 
if min < 250 then
min = 250
	end
else
min = (level * 2.2 + maglevel * 3.5) * 2.2
max = (level * 2.5 + maglevel * 3.6) * 2.8 
if min < 250 then
min = 250
	end
end
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
if getCreatureCondition(cid,CONDITION_PARALYZE) then
doRemoveCondition(cid, CONDITION_PARALYZE)
end
	return doCombat(cid, combat, var)
end
