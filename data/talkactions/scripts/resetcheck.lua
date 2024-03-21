function onSay(cid)
	if getCreatureStorage(cid, 67532) - os.time() > 0 then
		return false
	end
	doCreatureSetStorage(cid, 67532, os.time() + 2)

	local resetsCount = ResetSystem:getCount(cid)
	if resetsCount <= 0 then
		doPlayerPopupFYI(cid, "Reset System:\nRequirements to reset:\nLEVEL:"..ResetSystem.resets[1].needed_level ..".")
		return true
	end

	local bonus = ResetSystem.resets[resetsCount]
	if (bonus) then
		if (bonus.damage_percent) then
			message = "\n+" .. bonus.damage_percent .. "% damage"
		end

		if (bonus.hpmp_percent) then
			message = message .. "\n+" .. bonus.hpmp_percent .. "% health and mana"
		end

		if (bonus.exp_percent) then
			message = message .. "\n+" .. bonus.exp_percent .. "% EXP"
		end
	end

	if resetsCount >= 1 then
		doPlayerPopupFYI(cid, "Reset System:\n Your current reset is: "..resetsCount..".\n"..message.."\n Requirements to reset:\nLEVEL:"..ResetSystem.resets[resetsCount+1].needed_level ..".")
	else
		doPlayerPopupFYI(cid, "Reset System:\nRequirements to reset:\nLEVEL:"..ResetSystem.resets[1].needed_level ..".")
	end

	return true
end
