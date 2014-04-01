function onSay(cid, words, param)
local parametres = string.explode(param, ",")
if(parametres[1] ~= nil) then
local accId = getAccountIdByName(parametres[1])
if(accId > 0) then
local comment = ""
if(parametres[2] == nil) then
doPlayerSendCancel(cid, "You must enter ban days.")
return true
elseif(isNumber(parametres[2]) == false) then
doPlayerSendCancel(cid, "Ban days use only numbers.")
return true
end
if(parametres[3] ~= nil) then
comment = parametres[3]
end
doAddAccountBanishment(accId, getPlayerGUIDByName(parametres[1]), os.time() + (86400 * parametres[2]), 4, 2, comment, getPlayerGUID(cid), '')
local player = getPlayerByNameWildcard(parametres[1])
if(isPlayer(player) == TRUE) then
doRemoveCreature(player)
end
else
doPlayerSendCancel(cid, "Player with name " .. parametres[1] .. " doesn't exist.")
end
else
doPlayerSendCancel(cid, "You must enter name.")
end
return true
end
