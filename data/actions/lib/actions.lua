function destroyItem(player, target, toPosition)
	if target.uid <= 65535 or target.actionid > 0 then
		return false
	end

	--chest,crate,barrel...
	if (target.itemid >= 1724 and target.itemid <= 1741) or (target.itemid >= 2581 and target.itemid <= 2588) or target.itemid == 1770 or target.itemid == 2098 or target.itemid == 1774 or target.itemid == 1775 or target.itemid == 2064 or (target.itemid >= 1747 and target.itemid <= 1753) or (target.itemid >= 1714 and target.itemid <= 1717) or (target.itemid >= 1650 and target.itemid <= 1653) or (target.itemid >= 1666 and target.itemid <= 1677) or (target.itemid >= 1614 and target.itemid <= 1616) or (target.itemid >= 3813 and target.itemid <= 3820) or (target.itemid >= 3807 and target.itemid <= 3810) or (target.itemid >= 2080 and target.itemid <= 2085) or (target.itemid >= 2116 and target.itemid <= 2119) or target.itemid == 2094 or target.itemid == 2095 or target.itemid == 1619 or target.itemid == 2602 or target.itemid == 3805 or target.itemid == 3806 then
		if math.random(7) == 1 then
			if target.itemid == 1738 or target.itemid == 1739 or (target.itemid >= 2581 and target.itemid <= 2588) or target.itemid == 1770 or target.itemid == 2098 or target.itemid == 1774 or target.itemid == 1775 or target.itemid == 2064 then
				Game.createItem(2250, 1, toPosition)
			elseif (target.itemid >= 1747 and target.itemid <= 1749) or target.itemid == 1740 then
				Game.createItem(2251, 1, toPosition)
			elseif (target.itemid >= 1714 and target.itemid <= 1717) then
				Game.createItem(2252, 1, toPosition)
			elseif (target.itemid >= 1650 and target.itemid <= 1653) or (target.itemid >= 1666 and target.itemid <= 1677) or (target.itemid >= 1614 and target.itemid <= 1616) or (target.itemid >= 3813 and target.itemid <= 3820) or (target.itemid >= 3807 and target.itemid <= 3810) then
				Game.createItem(2253, 1, toPosition)
			elseif (target.itemid >= 1724 and target.itemid <= 1737) or (target.itemid >= 2080 and target.itemid <= 2085) or (target.itemid >= 2116 and target.itemid <= 2119) or target.itemid == 2094 or target.itemid == 2095 then
				Game.createItem(2254, 1, toPosition)
			elseif (target.itemid >= 1750 and target.itemid <= 1753) or target.itemid == 1619 or target.itemid == 1741 then
				Game.createItem(2255, 1, toPosition)
			elseif target.itemid == 2602 then
				Game.createItem(2257, 1, toPosition)
			elseif target.itemid == 3805 or target.itemid == 3806 then
				Game.createItem(2259, 1, toPosition)
			end
			target:remove(1)
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
		return true
	end
	--large amphora
	if target.itemid == 4996 then
		if math.random(3) == 1 then
			target:transform(4997)
			target:decay()
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
		return true
	end
	--spiderweb
	if target.itemid == 7538 or target.itemid == 7539 then
		if math.random(7) == 1 then
			if target.itemid == 7538 then
				target:transform(7544)
			elseif target.itemid == 7539 then
				target:transform(7545)
			end
			target:decay()
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
		return true
	end
	--wooden bar
	if target.itemid == 3798 or target.itemid == 3799 then
		if math.random(3) == 1 then
			if target.itemid == 3798 then
				target:transform(3959)
			elseif target.itemid == 3799 then
				target:transform(3958)
			end
			target:decay()
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
		return true
	end
	return false
end
