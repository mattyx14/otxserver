local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)

function onCastSpell(cid, var)
  local summoncount = getCreatureSummons(cid)
  if #summoncount < 3 then
	doSummonCreature("Egg2", getCreaturePosition(cid))
end
	return doCombat(cid, combat, var)
end