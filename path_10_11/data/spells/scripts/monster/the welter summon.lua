local function delayedCastSpell(cid, mon)
            local summon = getCreatureSummons(cid)
            for i = 1, #summon do
                if getCreatureName(summon[i]) == "Egg" then
        local newmon = doSummonCreature("Spawn Of The Welter", getCreaturePosition(summon[i]))
        if newmon ~= false then
            doConvinceCreature(cid, newmon)
            doSendMagicEffect(getCreaturePosition(summon[i]), CONST_ME_POISONAREA)
                    doRemoveCreature(summon[i])
                end
            end
        end
    end

local maxsummons = 1
function onCastSpell(cid, var)
  local summoncount = getCreatureSummons(cid)
  if #summoncount < 1 then
        local mon = doSummonCreature("Egg", getCreaturePosition(cid))
        if mon == false then
            return false
      end
      doConvinceCreature(cid, mon)
  end
  addEvent(delayedCastSpell, 10000, cid, mon)
  return true
end