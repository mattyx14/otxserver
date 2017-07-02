local rewards = {
	[SKILL_SWORD] = {
		{lvl = 150, items = {{2160, 2}, {2148, 1}}, storage = 54776},
		{lvl = 160, items = {{2365, 2}}, storage = 54777}
	},
	[SKILL_MAGLEVEL] = {
		{lvl = 100, items = {{2365, 2}}, storage = 54778},
	},
	[SKILL_LEVEL] = {
		{lvl = 480, items = {{2152, 2}}, storage = 54779},
	},
}

function onAdvance(player, skill, oldlevel, newlevel)
	local rewardstr = "Items received: "
	local reward_t = {}
	if rewards[skill] then
		for j = 1, #rewards[skill] do
			local r = rewards[skill][j]
			if not r then
				return true
			end

			if newlevel >= r.lvl then
				if player:getStorageValue(r.storage) < 1 then
					player:setStorageValue(r.storage, 1)
					for i = 1, #r.items do
						local itt = ItemType(r.items[i][1])
						if itt then
							player:addItem(r.items[i][1], r.items[i][2])
							table.insert(reward_t, itt:getName() .. (r.items[i][2] > 1 and " x" .. r.items[i][2] or ""))
						end
					end
				end
			end
		end
	
		if #reward_t > 0 then
			player:sendTextMessage(MESSAGE_STATUS_CONSOLE_ORANGE, rewardstr .. table.concat(reward_t, ", "))
		end
	end
	return true
end

function onLogin(player)
	player:registerEvent("onadvance_reward")
	return true
end