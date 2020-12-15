local outfits = {
	[6096] = {6096, 'Pirate', 155, 151, 6096},
	[2662] = {2662, 'Conjurer', 635, 634, 2662},
	[10008] = {10008, 'Death Herald', 666, 667, 21252}, -- 10005, 10006 and 10007 is reserved storage for kill Dracula
	[9933] = {9933, 'Assassin', 156, 152, 9933},
	[8820] = {8820, 'Chaos Acolyte', 664, 665, 8820},
	[15545] = {15545, 'Deepling', 464, 463, 15408},
	[7431] = {7431, 'Demonhunter', 288, 289, 7431},
	[5807] = {5807, 'Brotherhood', 279, 278, 5807},
	[8267] = {8267, 'Oriental', 150, 146, 8267},
	[3697] = {3697, 'Marry', 329, 328, 2121},
	[1449] = {1449, 'Puppeteer', 696, 697, 3972},
	[14699] = {14699, 'Insectoid', 466, 465, 15643},
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local outfit = outfits[item.uid]
	if outfit then
		if player:getStorageValue(outfit[1]) == -1 then
			if player:getSex() == 0 then
				player:addOutfitAddon(outfit[3])
				player:addItem(outfit[5], 1)
			else
				player:addOutfitAddon(outfit[4])
				player:addItem(outfit[5], 1)
			end

			player:sendTextMessage(MESSAGE_INFO_DESCR, "You now have the " .. outfit[2] .. " outfit!")
			player:setStorageValue(outfit[1], 1)
			player:getPosition():sendMagicEffect(CONST_ME_ENERGYAREA)
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You already have the " .. outfit[2] .. " outfit.")
		end
	end

	return true
end
