local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)

function onCastSpell(cid, var)
        doCreatureSay(cid, "THE POWER OF HIS INTERNAL FIRE RENEWS OMRAFIR!", TALKTYPE_ORANGE_2)
	return doCombat(cid, combat, var)
end
