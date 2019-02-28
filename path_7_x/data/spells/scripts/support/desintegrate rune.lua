local unremovable = {3058, 3059, 3060, 3061, 3062, 3063, 3064, 3065, 3066}
local function isRemovable(v)
	return type(v) == "table" and v.uid > 65535 and v.actionid == 0 and isMoveable(v.uid) and not isInArray(unremovable, v.itemid) and not isCreature(v.uid) or false
end

local function findFirstRemovable(pos)
	while getThingfromPos(pos).uid > 0 do
		if isRemovable(getThingfromPos(pos)) then
			return pos.stackpos
		end
		pos.stackpos = pos.stackpos + 1
	end
end
local function doRemoveObject(cid, pos)
	pos.stackpos = 1
	local i, first = 0, getThingfromPos(pos)
	if not isRemovable(first) then
		local k = findFirstRemovable(pos)
		if type(k) == "number" then
			pos.stackpos = k
		end
	end
	while i < 30 and isRemovable(getThingfromPos(pos)) do
		doRemoveItem(getThingfromPos(pos).uid)
		i = i + 1
		if not isRemovable(getThingfromPos(pos)) then
			local k = findFirstRemovable(pos)
			if type(k) == "number" then
				pos.stackpos = k
			end
		end
	end

	if i > 0 then
		doSendMagicEffect(pos, CONST_ME_BLOCKHIT)
		return LUA_NO_ERROR
	end

	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(getThingPos(cid), CONST_ME_POFF)
	return LUA_ERROR
end

function onCastSpell(cid, var)
	local pos = variantToPosition(var)
	if(pos.x ~= 0 and pos.y ~= 0 and pos.z ~= 0 and not getTileInfo(pos).protection) then
		return doRemoveObject(cid, pos)
	end

	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(getThingPos(cid), CONST_ME_POFF)
	return LUA_ERROR
end