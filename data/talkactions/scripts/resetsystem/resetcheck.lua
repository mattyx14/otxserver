function onSay(cid)
if getCreatureStorage(cid, 67532) - os.time() > 0 then
return false
end
doCreatureSetStorage(cid, 67532, os.time() + 2)
local resetsCount = ResetSystem:getCount(cid)
if resetsCount <= 0 then
doPlayerPopupFYI(cid, "Reset System:\nRequisitos para resetar:\nLEVEL:"..ResetSystem.resets[1].needed_level ..".")
return true
end
local bonus = ResetSystem.resets[resetsCount]
if (bonus) then
if (bonus.damage_percent) then
message = "\n+" .. bonus.damage_percent .. "% de dano"
end
if (bonus.hpmp_percent) then
message = message .. "\n+" .. bonus.hpmp_percent .. "% de vida e mana"
end
if (bonus.exp_percent) then
message = message .. "\n+" .. bonus.exp_percent .. "% de EXP"
end end
if resetsCount >= 1 then
doPlayerPopupFYI(cid, "Reset System:\n Seu reset atual é: "..resetsCount..".\n"..message.."\n Requisitos para resetar:\nLEVEL:"..ResetSystem.resets[resetsCount+1].needed_level ..".")
else
doPlayerPopupFYI(cid, "Reset System:\nRequisitos para resetar:\nLEVEL:"..ResetSystem.resets[1].needed_level ..".")
end
return 1
end
