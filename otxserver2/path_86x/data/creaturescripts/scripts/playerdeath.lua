function onDeath(cid, corpse, deathList)

local strings = {""}
local t, position = 1, 1
local deathType = "killed"
local toSlain, toCrushed = 3, 9

if #deathList >= toSlain then
	deathType = "slain"
elseif #deathList >= toCrushed then
	deathType = "crushed"
end
for _, pid in ipairs(deathList) do
	strings[position] = t == 1 and "" or strings[position] .. ", "
	strings[position] = strings[position] .. getCreatureName(pid) .. ""
		t = t + 1
end
for i, str in ipairs(strings) do
	if(str:sub(str:len()) ~= ",") then
		str = str .. "."
end
	msg = getCreatureName(cid) .. " was " .. deathType .. " at level " .. getPlayerLevel(cid) .. " by " .. str
end
for _, oid in ipairs(getPlayersOnline()) do
	doPlayerSendChannelMessage(oid, "Death channel", msg, TALKTYPE_CHANNEL_O, CHANNEL_DEATH)
end

	return true
end