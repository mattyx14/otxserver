local TYPES, ACCESS = {
	{
		event = "Ban_Action",
		actions = {
			[1] = "Banishment",
			[2] = "Banishment + Final Warning",
			[3] = "Deletion",
			[4] = "Notation"
		}
	},
	{
		event = "Ban_Action",
		actions = {
			[5] = "Banishment",
			[6] = "Deletion",
			[7] = "Notation",
			[8] = "Report",
			[9] = "Lock"
		}
	},
	{
		event = "Ban_Finish"
	}
},
{
	type = {
		[1] = { 1 },
		[2] = { 1 },
		[3] = { 1, 2 },
		[4] = { 1, 2 },
		[5] = { 1, 2, 3 }
	},
	action = {
		[1] = { 8 },
		[2] = { 8 },
		[3] = { 1, 4, 5, 7, 9 },
		[4] = { 1, 2, 4, 5, 7, 9 },
		[5] = { 1, 2, 3, 4, 5, 6, 7, 9 },
	}
}

function onChannelRequest(cid, channel, custom)
	unregisterCreatureEvent(cid, "Ban_Type")
	if(not custom or type(channel) ~= 'number') then
		doPlayerSendCancel(cid, "Invalid action.")
		return false
	end

	local type = TYPES[channel]
	if(not type) then
		doPlayerSendCancel(cid, "Invalid action.")
		return false
	end

	local access = getPlayerAccess(cid)
	if(not isInArray(ACCESS.type[access], channel)) then
		doPlayerSendCancel(cid, "You cannot do this action.")
		return false
	end

	registerCreatureEvent(cid, type.event)
	if(type.actions) then
		access = ACCESS.action[access]
		if(not access or table.maxn(access) == 0) then
			return false
		end

		local actions = {}
		for _, action in ipairs(access) do
			local tmp = type.actions[action]
			if(tmp) then
				actions[action] = tmp
			end
		end

		doPlayerSendChannels(cid, actions)
	else
		doShowTextDialog(cid, 2599, "Name:\n\n(Optional) Length:\n\nComment:\n", true, 1024)
		doCreatureSetStorage(cid, "banConfig", table.serialize({
			type = channel
		}))
	end

	return false
end