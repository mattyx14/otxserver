function onTradeAccept(cid, target, item, targetItem)
	doPlayerSaveEx(target)
	doPlayerSaveEx(cid)

	return true
end
