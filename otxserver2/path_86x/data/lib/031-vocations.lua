function isSorcerer(cid)
	return isInArray({1, 5}, getPlayerVocation(cid))
end

function isDruid(cid)
	return isInArray({2, 6}, getPlayerVocation(cid))
end

function isPaladin(cid)
	return isInArray({3, 7}, getPlayerVocation(cid))
end

function isKnight(cid)
	return isInArray({4, 8}, getPlayerVocation(cid))
end

function isRookie(cid)
	return isInArray({0}, getPlayerVocation(cid))
end
