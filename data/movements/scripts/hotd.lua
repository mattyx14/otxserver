function onEquip(cid, item, slot, boolean)
	--[[ the point of this is to call event ONLY when the duration was started,
	so it means the helmet was 'filled up' with the coconut shrimp bake, testing around ]]--
	if(boolean) then
		return true
	end

	if(isUnderWater(cid)) then
		if(item.itemid ~= 5461 or not getItemAttribute(item.uid, "duration")) then
			callFunction(cid, item.uid, slot, false)
		else
			doTransformItem(item.uid, 12541)
		end
	end

	return true
end
