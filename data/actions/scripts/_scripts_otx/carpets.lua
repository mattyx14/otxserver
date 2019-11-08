local foldedCarpet = {
	[25393] = 25392, [25392] = 25393, --rift carpet
	[26192] = 26193, [26193] = 26192, --void carpet
	[26087] = 26109, --yalahahari carpet
	[26109] = 26087, --yalahahari carpet
	[26088] = 26110, --white fur carpet
	[26110] = 26088, --white fur carpet
	[26089] = 26111, --bamboo mat carpet
	[26111] = 26089, --bamboo matr carpet
	[26371] = 26363, --crimson carpet
	[26363] = 26371, --crimson carpet
	[26366] = 26372, --azure carpet
	[26372] = 26366, --azure carpet
	[26367] = 26373, --emerald carpet
	[26373] = 26367, --emerald carpet
	[26368] = 26374, --light parquet carpet
	[26374] = 26368, --light parquet carpet
	[26369] = 26375, --dark parquet carpet
	[26375] = 26369, --dark parquet carpet
	[26370] = 26376, --mable floor
	[26376] = 26370, --marble floor
	[27072] = 27080, --flowery carpet
	[27080] = 27072, --flowery carpet
	[27073] = 27081, --Colourful Carpet
	[27081] = 27073, --Colourful Carpet
	[27074] = 27082, --striped carpet
	[27082] = 27074, --striped carpet
	[27075] = 27083, --fur carpet
	[27083] = 27075, --fur carpet
	[27076] = 27084, --diamond carpet
	[27084] = 27076, --diamond carpet
	[27077] = 27085, --patterned carpet
	[27085] = 27077, --patterned carpet
	[27078] = 27086, --night sky carpet
	[27086] = 27078, --night sky carpet
	[27079] = 27087, --star carpet
	[27087] = 27079, --star carpet
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local carpet = foldedCarpet[item.itemid]
	if not carpet then
		return false
	end

	if fromPosition.x == CONTAINER_POSITION then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Put the item on the floor first.")
	elseif not fromPosition:getTile():getHouse() then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "You may use this only inside a house.")
	else
		item:transform(carpet)
	end

	return true
end
