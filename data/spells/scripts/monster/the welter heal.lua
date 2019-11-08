local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)

function onCastSpell(cid, var)
            local summon = getCreatureSummons(cid)
            for i = 1, #summon do
                if getCreatureName(summon[i]) == "Egg" then
            	  doSendMagicEffect(getCreaturePosition(summon[i]), CONST_ME_HITBYPOISON)
                  local remov = doRemoveCreature(summon[i])
				         if remov == false then
                return false
            end
                        doCreatureSay(cid, "<the welter devours his spawn and heals himself>", TALKTYPE_ORANGE_1)
                        doCreatureAddHealth(cid, 25000)
                        doSendMagicEffect(getCreaturePosition(cid), 12)
                  elseif getCreatureName(summon[i]) == "Spawn Of The Welter" then
            	  doSendMagicEffect(getCreaturePosition(summon[i]), CONST_ME_DRAWBLOOD)
                  local remov1 = doRemoveCreature(summon[i])
                            if remov1 == false then
                return false
            end
                        doCreatureSay(cid, "<the welter devours his spawn and heals himself>", TALKTYPE_ORANGE_1)
                        doCreatureAddHealth(cid, 25000)
                        doSendMagicEffect(getCreaturePosition(cid), 12)
            end

    return doCombat(cid, combat, var)
        end
    end