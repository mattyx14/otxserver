function isSorcerer(cid, promoted)
	local arr = {1, 5}
	if(promoted) then
		table.remove(arr, 1)
	end

	return isInArray(arr, getPlayerVocation(cid))
end

function isDruid(cid, promoted)
	local arr = {2, 6}
	if(promoted) then
		table.remove(arr, 1)
	end

	return isInArray(arr, getPlayerVocation(cid))
end

function isPaladin(cid, promoted)
	local arr = {3, 7}
	if(promoted) then
		table.remove(arr, 1)
	end

	return isInArray(arr, getPlayerVocation(cid))
end

function isKnight(cid, promoted)
	local arr = {4, 8}
	if(promoted) then
		table.remove(arr, 1)
	end

	return isInArray(arr, getPlayerVocation(cid))
end

function isRookie(cid, promoted)
	return not promoted and getPlayerVocation(cid) == 0
end
