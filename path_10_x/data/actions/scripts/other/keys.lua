REVERSE_DOORS, CHILD_DOORS = {}, {}
for k, v in pairs(DOORS) do
	REVERSE_DOORS[v] = k

	local tmp = getItemInfo(v)
	if(tmp.transformUseTo ~= 0) then
		CHILD_DOORS[tmp.transformUseTo] = k
	end
end

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(item.aid > 0 and itemEx.aid > 0) then
		if(isPlayerPzLocked(cid) and getTileInfo(toPosition).protection) then
			doPlayerSendDefaultCancel(cid, RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE)
			return true
		end

		local doors = DOORS[itemEx.itemid]
		if(not doors) then
			doors = REVERSE_DOORS[itemEx.itemid]
		end

		if(not doors) then
			doors = CHILD_DOORS[itemEx.itemid]
		end

		if(doors) then
			if(item.actionid ~= itemEx.actionid) then
				doPlayerSendCancel(cid, "The key does not match.")
			else
				doTransformItem(itemEx.uid, doors)
			end

			return true
		end
	end

	return false
end