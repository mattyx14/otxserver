local config = {
	storage = 40053,
	version = 2, -- Increase this value after adding new marks, so player can step again and receive new map marks
	marks = {
		{mark = 2, pos = {x = 941, y = 1002, z = 7}, desc = "Town Of Fynn City"},
		{mark = 3, pos = {x = 927, y = 1000, z = 7}, desc = "Depot of Fynn City"},
		{mark = 3, pos = {x = 914, y = 1003, z = 7}, desc = "Potions, trainer and loot"},
		{mark = 3, pos = {x = 910, y = 1004, z = 6}, desc = "Jozue's Potions"},
		{mark = 3, pos = {x = 910, y = 1004, z = 5}, desc = "Brintay's Loots"},
		{mark = 3, pos = {x = 906, y = 1010, z = 8}, desc = "PVP arena and Trainers"},
		{mark = 3, pos = {x = 897, y = 988, z = 9}, desc = "Trainers"},
		{mark = 3, pos = {x = 925, y = 1015, z = 7}, desc = "Riona's Tools"},
		{mark = 2, pos = {x = 897, y = 1012, z = 7}, desc = "Second Depot of Fynn City"},
		{mark = 2, pos = {x = 908, y = 1026, z = 7}, desc = "Milton's Tools"},
		{mark = 2, pos = {x = 979, y = 953, z = 7}, desc = "church of Fynn City"},
		{mark = 2, pos = {x = 970, y = 941, z = 7}, desc = "Rikon's Promotion"},
		{mark = 2, pos = {x = 964, y = 1027, z = 7}, desc = "Boat of Pirate Jack"},
		{mark = 2, pos = {x = 967, y = 1013, z = 7}, desc = "Ruby's Parcels"},
		{mark = 3, pos = {x = 886, y = 999, z = 7}, desc = "Lola's Furnitures"},
		{mark = 3, pos = {x = 900, y = 1000, z = 7}, desc = "Stevan's Food"},
							-- Ardgard --
		{mark = 2, pos = {x = 372, y = 1084, z = 6}, desc = "Boat of Pirate Jack"},
		{mark = 2, pos = {x = 406, y = 1109, z = 7}, desc = "Stevan Foods"},
		{mark = 2, pos = {x = 416, y = 1116, z = 7}, desc = "Lola's Furnitures"},
		{mark = 2, pos = {x = 430, y = 1116, z = 7}, desc = "The Ardgard Cruch"},
		{mark = 2, pos = {x = 422, y = 1102, z = 7}, desc = "Jozue's Potions"},
		{mark = 3, pos = {x = 410, y = 1087, z = 7}, desc = "Combat House"},
		{mark = 2, pos = {x = 400, y = 1080, z = 7}, desc = "Ardgard Depot and Ruby"},
							-- Jorvik --
		{mark = 2, pos = {x = 981, y = 754, z = 7}, desc = "Boat and Riona"},
		{mark = 2, pos = {x = 957, y = 717, z = 7}, desc = "Depot and Ruby"},
		{mark = 3, pos = {x = 1017, y = 722, z = 7}, desc = "Jozue's Pots"},
		{mark = 3, pos = {x = 1028, y = 715, z = 7}, desc = "Steven Food's"},
		{mark = 2, pos = {x = 1029, y = 662, z = 7}, desc = "Britany's Loot"},
							-- Varkhall --
		{mark = 3, pos = {x = 1520, y = 769, z = 10}, desc = "Riona's Tools"},
		{mark = 3, pos = {x = 1504, y = 704, z = 13}, desc = "Temple"},
		{mark = 2, pos = {x = 1516, y = 720, z = 12}, desc = "Jozue's Potions"},
		{mark = 2, pos = {x = 1541, y = 699, z = 12}, desc = "Britany'z Loot"},
		{mark = 2, pos = {x = 1535, y = 769, z = 10}, desc = "Pirate Jack"},
							-- Mer Jungle --
		{mark = 3, pos = {x = 1091, y = 1261, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 1086, y = 1248, z = 7}, desc = "Lola's Furnitures"},
		{mark = 2, pos = {x = 1084, y = 1232, z = 6}, desc = "Jozue, Britany and Milton"},
		{mark = 3, pos = {x = 1058, y = 1224, z = 7}, desc = "Cristina"},
		{mark = 2, pos = {x = 1072, y = 1224, z = 7}, desc = "Temple"},
							-- Vinor --
		{mark = 2, pos = {x = 420, y = 909, z = 6}, desc = "Jozue's Pots"},
		{mark = 3, pos = {x = 407, y = 893, z = 6}, desc = "Riona and Milton"},
		{mark = 3, pos = {x = 450, y = 895, z = 6}, desc =  "Pirate Jack"},
							-- Alefgard --
		{mark = 2, pos = {x = 779, y = 1234, z = 6}, desc = "Pirate Jack"},
		{mark = 5, pos = {x = 770, y = 1242, z = 7}, desc = "Stevan"},
		{mark = 2, pos = {x = 802, y = 1271, z = 7}, desc = "Depot, Ruby, Jozue And Lola"},
							-- Trekolt --
		{mark = 3, pos = {x = 1377, y = 653, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 1377, y = 605, z = 6}, desc = "Stevan Food"},
		{mark = 2, pos = {x = 1404, y = 600, z = 6}, desc = "Cristina Bless"},
		{mark = 3, pos = {x = 1377, y = 590, z = 6}, desc = "Milton"},
		{mark = 3, pos = {x = 1367, y = 582, z = 6}, desc = "Depot and Riona"},
		{mark = 2, pos = {x = 1349, y = 603, z = 6}, desc = "Riona's Tools"},
		{mark = 2, pos = {x = 1367, y = 595, z = 6}, desc = "Britany's Loot"},
							-- magical town --
		{mark = 2, pos = {x = 350, y = 1396, z = 6}, desc = "Pirate Jack"},
		{mark = 3, pos = {x = 367, y = 1401, z = 7}, desc = "Riona´s Tools"},
		{mark = 2, pos = {x = 381, y = 1390, z = 7}, desc = "Depot and Ruby"},
		{mark = 3, pos = {x = 377, y = 1404, z = 7}, desc = "Town And Cristina"},
		{mark = 2, pos = {x = 413, y = 1418, z = 7}, desc = "Milton"},
		{mark = 2, pos = {x = 426, y = 1433, z = 7}, desc = "Britany's Loot"},
							-- Deromin --
		{mark = 3, pos = {x = 657, y = 1215, z = 6}, desc = "Pirate Jack"},
		{mark = 3, pos = {x = 663, y = 1222, z = 8}, desc = "Britany's Loot"},
		{mark = 3, pos = {x = 633, y = 1222, z = 6}, desc = "Dp and Ruby"},
		{mark = 3, pos = {x = 661, y = 1205, z = 7}, desc = "Milton"},
							-- Anshara --
		{mark = 3, pos = {x = 682, y = 452, z = 6}, desc = "Pirate Jack"},
		{mark = 3, pos = {x = 683, y = 423, z = 7}, desc = "Dp and Ruby"},
		{mark = 3, pos = {x = 671, y = 422, z = 7}, desc = "Josue pots"},
		{mark = 3, pos = {x = 660, y = 411, z = 7}, desc = "Stevan's Food"},
		{mark = 3, pos = {x = 691, y = 408, z = 7}, desc = "Riona's Tools"},
		{mark = 3, pos = {x = 680, y = 396, z = 7}, desc = "Britany's Tools"},
		{mark = 3, pos = {x = 718, y = 406, z = 7}, desc = "Town and Cristina"},
		{mark = 3, pos = {x = 726, y = 409, z = 7}, desc = "Milton"},
		{mark = 3, pos = {x = 730, y = 433, z = 7}, desc = "Rikon's Promos"},
							-- Envior --
		{mark = 3, pos = {x = 144, y = 640, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 169, y = 627, z = 7}, desc = "Depot, Ruby and Temple"},
		{mark = 2, pos = {x = 190, y = 610, z = 7}, desc = "Lola's Furnitures"},
		{mark = 3, pos = {x = 194, y = 596, z = 7}, desc = "Britany, Josue, Stevan, Milton and Riona"},
							-- Karmia --
		{mark = 3, pos = {x = 128, y = 876, z = 6}, desc = "Pirate Jack"},	
		{mark = 3, pos = {x = 211, y = 855, z = 7}, desc = "Riona's Tools"},
		{mark = 3, pos = {x = 199, y = 895, z = 7}, desc = "Town And Soller"},
		{mark = 2, pos = {x = 220, y = 867, z = 7}, desc = "Depot, Jozue and Ruby"},
		{mark = 2, pos = {x = 231, y = 856, z = 7}, desc = "Milton"},
		{mark = 2, pos = {x = 253, y = 864, z = 7}, desc = "Britany's Loot"},
							-- Iquanus --	
		{mark = 2, pos = {x = 995, y = 1489, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 949, y = 1461, z = 6}, desc = "Depot And Ruby"},
							-- Samaransa Desdert --
		{mark = 2, pos = {x = 354, y = 1282, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 317, y = 1305, z = 7}, desc = "Riona's Tools"},
		{mark = 2, pos = {x = 300, y = 1310, z = 7}, desc = "Depot, Lola And Ruby"},
		{mark = 2, pos = {x = 312, y = 1292, z = 7}, desc = "Temple and Cristina"},
		{mark = 2, pos = {x = 323, y = 1286, z = 7}, desc = "Britany's Loot"},
							-- Misidia --
		{mark = 2, pos = {x = 1054, y = 326, z = 6}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 1074, y = 334, z = 7}, desc = "Ruby'Parcels "},
		{mark = 2, pos = {x = 1073, y = 342, z = 7}, desc = "Josue'Pots"},
		{mark = 2, pos = {x = 1082, y = 338, z = 7}, desc = "Depot And Riona"},
		{mark = 2, pos = {x = 1091, y = 340, z = 7}, desc = "Milton"},
		{mark = 2, pos = {x = 1075, y = 357, z = 7}, desc = "Britany's Loot"},
							-- Zao --
		{mark = 2, pos = {x = 1581, y = 941, z = 10}, desc = "Pirate Jack"},
		{mark = 2, pos = {x = 1607, y = 895, z = 10}, desc = "Depot"}
	}
}

local f_addMark = doPlayerAddMapMark
if(not f_addMark) then f_addMark = doAddMapMark end

function onStepIn(cid, item, position, fromPosition)
	if(isPlayer(cid) ~= true or getPlayerStorageValue(cid, config.storage) == config.version) then
		return
	end

	for _, m  in pairs(config.marks) do
		f_addMark(cid, m.pos, m.mark, m.desc ~= nil and m.desc or "")
	end
		setPlayerStorageValue(cid, config.storage, config.version)
	return true
end
