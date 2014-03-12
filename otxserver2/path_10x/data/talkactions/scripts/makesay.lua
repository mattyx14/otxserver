function onSay(cid, words, param)
local param2 = string.explode(param, ",")
if(param2[1] ~= nil) then
	local pname = getPlayerByNameWildcard(param2[1])
	if(param2[2] ~= nil) then
		text = param2[2]
	end
		doCreatureSay(pname, text, TALKTYPE_SAY)
	else
		doPlayerSendCancel(cid, "Failed. Please try again.")
	end
	return true
end