local rewards = {
	[SKILL_LEVEL] = {
		{lvl = 30, items = {{2160, 15}}, storage = Storage.Rewards.firstLevelReward},
		{lvl = 50, items = {{2160, 30}}, storage = Storage.Rewards.secondLevelReward},
		{lvl = 80, items = {{2160, 50}}, storage = Storage.Rewards.thirdLevelReward},
		{lvl = 110, items = {{2160, 80}}, storage = Storage.Rewards.fourthLevelReward},
		-- {lvl = 60, items = {{16101, 1}}, storage = 3103}, -- Premium Scrolls
	},
--[[
	[SKILL_SWORD] = {
		{lvl = 150, items = {{2160, 2}, {2148, 1}}, storage = 54776},
		{lvl = 160, items = {{2365, 2}}, storage = 54777}
	},
	[SKILL_MAGLEVEL] = {
		{lvl = 100, items = {{2365, 2}}, storage = 54778},
	},
]]
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
