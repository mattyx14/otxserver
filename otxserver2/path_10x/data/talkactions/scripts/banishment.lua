-- How use:
-- /ban PLAYERNAME, NUMBERDAYS(?), actionId(?), reasonId(?), COMMENT

-- actionId
-- 0 -- Offensive Name
-- 1 -- Invalid Name Format
-- 2 -- Unsuitable Name
-- 3 -- Name Inciting Rule Violation
-- 4 -- Offensive Statement
-- 5 -- Spamming
-- 6 -- Illegal Advertising
-- 7 -- Off-Topic Public Statement
-- 8 -- Non-English Public Statement
-- 9 -- Inciting Rule Violation
-- 10 -- Bug Abuse
-- 11 -- Game Weakness Abuse
-- 12 -- Using Unofficial Software to Play
-- 13 -- Hacking
-- 14 -- Multi-Clienting
-- 15 -- Account Trading or Sharing
-- 16 -- Threatening Gamemaster
-- 17 -- Pretending to Have Influence on Rule Enforcement
-- 18 -- False Report to Gamemaster
-- 19 -- Destructive Behaviour
-- 20 -- Excessive Unjustified Player Killing

-- reasonId
-- 0 -- Notation
-- 1 -- Name Report
-- 2 -- Banishment
-- 3 -- Name Report + Banishment
-- 4 -- Banishment + Final Warning
-- 5 -- Name Report + Banishment + Final Warning
-- 6 -- Statement Report
-- 7 -- Deletion
-- 8 -- Name Look
-- 9 -- Name Lock + Banishment
-- 10 -- Name Lock + Banishment + Final Warning
-- 11 -- Name Lock + Banishment + Final Warning + IP Banishment

function onSay(cid, words, param)
	local parametres = string.explode(param, ",")
	if(parametres[1] ~= nil) then
		local accId = getAccountIdByName(parametres[1])
		if(accId > 0) then
			local reasonId = {
				[0] = {0}, [1] = {1}, [2] = {2}, [3] = {3}, [4] = {4}, [5] = {5},
				[6] = {6}, [7] = {7}, [8] = {8}, [9] = {9}, [10] = {10}, [11] = {11},
				[12] = {12}, [13] = {13}, [14] = {14}, [15] = {15}, [16] = {16}, [17] = {17},
				[18] = {18}, [19] = {19}, [20] = {20}
			}
			local actionId = {
				[0] = {0}, [1] = {1}, [2] = {2}, [3] = {3}, [4] = {4}, [5] = {5},
				[6] = {6}, [7] = {7}, [8] = {8}, [9] = {9}, [10] = {10}, [11] = {11}
			}
			local comment = ""

			if(parametres[2] == nil) then
				doPlayerSendCancel(cid, "You must enter ban days.")
				return true
			elseif(isNumber(parametres[2]) == false) then
				doPlayerSendCancel(cid, "Ban days use only numbers.")
				return true
			end
			if(parametres[3] ~= nil) then
				reasonId = parametres[3]
			end
			if(parametres[4] ~= nil) then
				actionId = parametres[4]
			end
			if(parametres[5] ~= nil) then
				comment = parametres[5]
			end

			doAddAccountBanishment(accId, getPlayerGUIDByName(parametres[1]), os.time() + (86400 * parametres[2]), reasonId, actionId, comment, getPlayerGUID(cid), '')
			local player = getPlayerByNameWildcard(parametres[1])
			if(isPlayer(player) == true) then
				doRemoveCreature(player)
			end
		else
			doPlayerSendCancel(cid, "Player with name " .. parametres[1] .. " doesn't exist.")
		end
	else
		doPlayerSendCancel(cid, "You must enter name.")
	end

	return true
end
