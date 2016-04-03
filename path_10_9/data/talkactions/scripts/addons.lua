local config = {
	["first citizen addon"] = {
		male = 128, female = 136, addon = 1, items = {{5878, 100}, {10568, 10}, {10567, 10}, {11189, 10}}
	},
	["second citizen addon"] = {
		male = 128, female = 136, addon = 2, items = {{5890, 100}, {5902, 50}, {2480, 1}, {11190, 1}}
	},
	["first hunter addon"] = {
		male = 129, female = 137, addon = 1, items = {{5947, 1}, {5876, 100}, {5948, 100}, {5891, 5}, {5887, 1}, {5889, 1}, {5888, 1}}
	},
	["second hunter addon"] = {
		male = 129, female = 137, addon = 2, items = {{5875, 1}, {7364, 250}}
	},
	["first mage addon"] = {
		male = 129, female = 137, addon = 2, items = {{5875, 1}, {7364, 250}}
	},
	["second mage addon"] = {
		male = 129, female = 137, addon = 2, items = {{5875, 1}, {7364, 250}}
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
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Enjoy your new addons to your " .. param .. " outfit!")
	player:getPosition():sendMagicEffect(CONST_ME_FIREWORK_YELLOW)
	return false
end
