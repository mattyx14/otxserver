-- id of the item that is to decay if the player moves it from the depot
-- you can play this function in the LIB
local decay = {6300, 6301}
function getItemsInContainerById(container, itemid)
	local items = {}
	if isContainer(container) and getContainerSize(container) > 0 then
		for slot=0, (getContainerSize(container)-1) do
			local item = getContainerItem(container, slot)
			if isContainer(item.uid) then
				local itemsbag = getItemsInContainerById(item.uid, itemid)
				for i=0, #itemsbag do
					table.insert(items, itemsbag[i])
				end
			else
				if itemid == item.itemid then
					table.insert(items, item.uid)
				end
			end
		end
	end
	return items
end

function onThrow(cid, item, fromPosition, toPosition)
	if fromPosition.y >= 64 then
		for tamanho = 1, #decay do
			if isContainer(item.uid) then
				local search = getItemsInContainerById(item.uid, decay[tamanho])
				for i=1, #search do
					doDecayItem(search[i])
				end
			elseif isInArray(decay, item.itemid) then
				doDecayItem(item.uid)
			end
		end
	end

	return true
end

-- on accept trade with item stopped decaiyn, this will be start decay
function onTradeAccept(cid, target, item, targetItem)
	for tamanho = 1, #decay do
		if isContainer(item.uid) then
			local search = getItemsInContainerById(item.uid, decay[tamanho])
			for i=1, #search do
				doDecayItem(search[i])
			end
		elseif isInArray(decay, item.itemid) then
			doDecayItem(item.uid)
		end
		-- check target player item
		if isContainer(targetItem.uid) then
			local search2 = getItemsInContainerById(targetItem.uid, decay[tamanho])
			for i=1, #search2 do
				doDecayItem(search2[i])
			end
		elseif isInArray(decay, targetItem.itemid) then
			doDecayItem(targetItem.uid)
		end
	end	

	doPlayerSave(target)
	doPlayerSave(cid)
	return true
end
