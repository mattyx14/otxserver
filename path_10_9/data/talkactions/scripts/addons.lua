local config = {
	["first citizen addon"] = {
		male = 128, female = 136, addon = 1, items = {{5878, 100}, {10568, 10}, {10567, 10}, {11189, 10}}
	},
	["second citizen addon"] = {
		male = 128, female = 136, addon = 2, items = {{5890, 100}, {5902, 50}, {2480, 1}, {11190, 1}}
	},
	["first hunter addon"] = {
		male = 129, female = 137, addon = 1, items = {{5947, 1}, {5876, 100}, {5948, 100}, {5891, 2}, {5887, 10}, {5889, 1}, {5888, 1}}
	},
	["second hunter addon"] = {
		male = 129, female = 137, addon = 2, items = {{5875, 1}, {7364, 250}}
	},
	["first knight addon"] = {
		male = 131, female = 139, addon = 1, items = {{5880, 100}, {5892, 1}, {2476, 5}, {2477, 10}}
	},
	["second knight addon"] = {
		male = 131, female = 139, addon = 2, items = {{5893, 100}, {5924, 1}, {5885, 1}, {5887, 10}, {5902, 20}}
	},
	["first mage addon"] = {
		male = 130, female = 141, addon = 1, items = {{2182, 1}, {2186, 1}, {2185, 1}, {2181, 1}, {2183, 1}, {2190, 1}, {2191, 1}, {2188, 1}, {2189, 1}, {2187, 1}, {5904, 3}, {2193, 20}, {5809, 1}}
	},
	["second mage addon"] = {
		male = 130, female = 141, addon = 2, items = {{5903, 1}, {7410, 1}, {2184, 1}}
	},
	["first nomble addon"] = {
		male = 132, female = 140, addon = 1, items = {{2160, 50}}
	},
	["second nomble addon"] = {
		male = 132, female = 140, addon = 2, items = {{2160, 50}}
	},
	["first summoner addon"] = {
		male = 133, female = 138, addon = 1, items = {{5958, 1}, {5945, 1}, {5944, 25}}
	},
	["second summoner addon"] = {
		male = 133, female = 138, addon = 2, items = {{5894, 70}, {5911, 20}, {5883, 10}, {5922, 50}, {5886, 10}, {5881, 50}, {5882, 50}, {5904, 3}, {5905, 20}}
	},
	["first warrior addon"] = {
		male = 134, female = 142, addon = 1, items = {{5925, 100}, {5899, 100}, {5884, 5}, {10020, 5}}
	},
	["second warrior addon"] = {
		male = 134, female = 142, addon = 2, items = {{5880, 100}, {5887, 10}, {2475, 10}}
	},
	["first barbarian addon"] = {
		male = 143, female = 147, addon = 1, items = {{5880, 100}, {5892, 1}, {5893, 50}, {5876, 50}}
	},
	["second barbarian addon"] = {
		male = 143, female = 147, addon = 2, items = {{5884, 1}, {5885, 1}, {5910, 50}, {5911, 50}, {5886, 10}}
	},
	["first druid addon"] = {
		male = 144, female = 148, addon = 1, items = {{5896, 50}, {5897, 50}, {10567, 15}}
	},
	["second druid addon"] = {
		male = 144, female = 148, addon = 2, items = {{5906, 100}, {5939, 1}, {5940, 1}}
	},
	["first wizard addon"] = {
		male = 145, female = 149, addon = 1, items = {{2536, 6}, {2492, 5}, {2488, 3}, {2470, 1}, {2123, 1}, {2436, 2}}
	},
	["second wizard addon"] = {
		male = 145, female = 149, addon = 2, items = {{5922, 25}, {9969, 5}}
	},
	["first oriental addon"] = {
		male = 146, female = 150, addon = 1, items = {{5945, 1}, {5944, 50}}
	},
	["second oriental addon"] = {
		male = 146, female = 150, addon = 2, items = {{5883, 50}, {5895, 100}, {5891, 2}, {5912, 50}}
	},
	["first pirate addon"] = {
		male = 151, female = 155, addon = 1, items = {{6098, 50}, {6126, 100}, {6097, 100}}
	},
	["second pirate addon"] = {
		male = 151, female = 155, addon = 2, items = {{6101, 1}, {6102, 1}, {6100, 1}, {6099, 1}}
	},
}

function onSay(player, words, param)
	local v = config[param:lower()]
	if not v then
		return false
	end

	local outfit = player:getSex() == PLAYERSEX_FEMALE and v["female"] or v["male"]
	if player:hasOutfit(outfit, 3) then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You have already obtained the " .. param .. " addons.")
		return false
	end

	local text = ""
	for i = 1, #v["items"] do
		local itemType = ItemType(v["items"][i][1])
		text = text .. (i ~= 1 and (i == #v["items"] and " and " or ", ") or "") .. (v["items"][i][2] > 1 and v["items"][i][2] or itemType:getArticle()) .. " " .. (v["items"][i][2] > 1 and itemType:getPluralName() or itemType:getName())
	end

	for i = 1, #v["items"] do
		if player:getItemCount(v["items"][i][1]) < v["items"][i][2] then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You need " .. text .. " for the whole " .. param .. " outfit.")
			return false
		end
	end

	for i = 1, #v["items"] do
		player:removeItem(unpack(v["items"][i]))
	end
	player:addOutfitAddon(v["female"], v["addon"])
	player:addOutfitAddon(v["male"], v["addon"])
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Enjoy your new addon to your " .. param .. " outfit!")
	player:getPosition():sendMagicEffect(CONST_ME_FIREWORK_YELLOW)
	return false
end
