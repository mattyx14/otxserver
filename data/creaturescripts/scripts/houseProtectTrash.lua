-- This script allows only GM+ and people invited to the house to move
-- and throw things into the house, uninvited people cannot move the items in the house
-- even if they enter it
function onMoveItem(moveItem, frompos, position, cid)
	if getPlayerAccess(cid) > 3 then
		return true
	end
	local house = getHouseFromPos(frompos) or getHouseFromPos(position)
	if type(house) == "number" then
		local owner = getHouseOwner(house)
		if owner == 0 then
			return false, doPlayerSendCancel(cid, "Sorry, not possible.")
		end

		if owner ~= getPlayerGUID(cid) then
			local sub = getHouseAccessList(house, 0x101):explode("\n")
			local guest = getHouseAccessList(house, 0x100):explode("\n")
			local isInvited = false
			if (#sub > 0) and isInArray(sub, getCreatureName(cid)) then
				isInvited = true
			end
			if (#guest > 0) and isInArray(guest, getCreatureName(cid)) then
				isInvited = true
			end
			if not isInvited then
				return false, doPlayerSendCancel(cid, "Sorry, not possible.")
			end
		end
	end

	return true
end
