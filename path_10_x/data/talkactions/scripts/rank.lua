modaldialog2 = {
	title = "Rank",
	message = "Select the rank you want to view:",
	buttons = {
		{ id = 1, value = "Ver" },
		{ id = 2, value = "Cancelar" },
	},
	buttonEnter = 1,
	buttonEscape = 2,
	choices = {
		{ id = 1, value = "Fist" },
		{ id = 2, value = "Club" },
		{ id = 3, value = "Sword" },
		{ id = 4, value = "Axe" },
		{ id = 5, value = "Distance" },
		{ id = 6, value = "Shield" },
		{ id = 7, value = "Fish" },
		{ id = 8, value = "Magic" },
		{ id = 9, value = "Level" }
	},
	popup = false
}

function callback2(cid, button, choice)
	if (button == 1) then
		local str = ""
		if (choice == 1) then
			str = "--[ Fist Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 0 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 2) then
			str = "--[ Club Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 1 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 3) then
			str = "--[ Sword Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 2 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 4) then
			str = "--[ Axe Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 3 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 5) then
			str = "--[ Distance Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 4 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 6) then
			str = "--[ Shield Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 5 ORDER BY `value` DESC;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 7) then
			str = "--[ Fish Rank ]--\n"
			query = db.getResult("SELECT `player_id`, `value` FROM `player_skills` WHERE `skillid` = 6 ORDER BY `value` DESC LIMIT 20;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(getPlayerNameByGUID(query:getDataString("player_id"))).." - [" .. query:getDataInt("value") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 8) then
			str = "--[ Magic Rank ]--\n"
			query = db.getResult("SELECT `maglevel`, `name` FROM `players` WHERE `group_id` < 3 ORDER BY `maglevel` DESC LIMIT 20;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(query:getDataString("name")).." - [" .. query:getDataInt("maglevel") .. "]"
				k = k + 1 until not query:next()
			end
		elseif (choice == 9) then
			str = "--[ Level Rank ]--\n"
			query = db.getResult("SELECT `name`, `level`, `experience` FROM `players` WHERE `group_id` < 3 ORDER BY `experience` DESC LIMIT 20;")
			if (query:getID() ~= -1) then k = 1 repeat if k > 20 then break end
				str = str .. "\n " .. k .. ". "..(query:getDataString("name")).." - [" .. query:getDataInt("level") .. "]"
				k = k + 1 until not query:next()
			end
		end
		doShowTextDialog(cid, 2529, str)
	end
end
	
function onSay(cid, words, param)
	addDialog(modaldialog2, 1002, cid, callback2);
	return true
end