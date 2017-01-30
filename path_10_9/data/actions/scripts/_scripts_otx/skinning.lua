local config = {
	[5908] = {
		-- Minotaurs
		[2830] = {value = 25000, newItem = 5878},
		[2871] = {value = 25000, newItem = 5878},
		[2866] = {value = 25000, newItem = 5878},
		[2876] = {value = 25000, newItem = 5878},
		[3090] = {value = 25000, newItem = 5878},
		[5969] = {value = 25000, newItem = 5878, after = 3091},
		[5981] = {value = 25000, newItem = 5878, after = 2867},
		[5982] = {value = 25000, newItem = 5878, after = 2872},
		[5983] = {value = 25000, newItem = 5878, after = 2877},
		[23367] = {value = 25000, newItem = 5878, after = 23369},
		[23368] = {value = 25000, newItem = 5878},
		[23371] = {value = 25000, newItem = 5878, after = 23373},
		[23372] = {value = 25000, newItem = 5878},
		[23375] = {value = 25000, newItem = 5878, after = 23377},
		[23376] = {value = 25000, newItem = 5878},
		[23462] = {value = 25000, newItem = 5878, after = 23464},
		[23463] = {value = 25000, newItem = 5878},
		[23467] = {value = 25000, newItem = 5878},
		[23466] = {value = 25000, newItem = 5878, after = 23468},
		[23470] = {value = 25000, newItem = 5878, after = 23472},
		[23471] = {value = 25000, newItem = 5878},
		[23472] = {value = 25000, newItem = 5878},

		-- Low Class Lizards
		[4256] = {value = 25000, newItem = 5876},
		[4259] = {value = 25000, newItem = 5876},
		[4262] = {value = 25000, newItem = 5876},
		[4251] = {value = 25000, newItem = 5876, after = 4257},
		[6040] = {value = 25000, newItem = 5876, after = 4260},
		[6041] = {value = 25000, newItem = 5876, after = 4263},

		-- High Class Lizards
		[11285] = {value = 25000, newItem = 5876},
		[11288] = {value = 25000, newItem = 5876, after = 11286},
		[11277] = {value = 25000, newItem = 5876},
		[11280] = {value = 25000, newItem = 5876, after = 11278},
		[11269] = {value = 25000, newItem = 5876},
		[11272] = {value = 25000, newItem = 5876, after = 11270},
		[11281] = {value = 25000, newItem = 5876},
		[11284] = {value = 25000, newItem = 5876, after = 11282},

		-- Dragons
		[3104] = {value = 25000, newItem = 5877},
		[2844] = {value = 25000, newItem = 5877},
		[5973] = {value = 25000, newItem = 5877, after = 3105},
		[7621] = {value = 25000, newItem = 5877, after = 7626},
		[7623] = {value = 25000, newItem = 5877, after = 7626},

		-- Dragon Lords
		[2881] = {value = 25000, newItem = 5948},
		[5984] = {value = 25000, newItem = 5948, after = 2882},
		[7624] = {value = 25000, newItem = 5948},
		[7622] = {value = 25000, newItem = 5948, after = 7625},

		-- Behemoths
		[2931] = {value = 35000, newItem = 5893},
		[5999] = {value = 25000, newItem = 5893, after = 2932},

		-- Bone Beasts
		[3031] = {value = 25000, newItem = 5925},
		[6030] = {value = 25000, newItem = 5925, after = 3032},

		-- The Mutated Pumpkin
		[8961] = { { value = 5000, newItem = 7487 }, { value = 10000, newItem = 7737 }, { value = 20000, 6492 }, { value = 30000, newItem = 8860 }, { value = 45000, newItem = 2683 }, { value = 60000, newItem = 2096 }, { value = 90000, newItem = 9005, amount = 50 } },

		-- Marble
		[11343] = { {value = 10000, newItem = 11345, desc = "This shoddy work was made by |PLAYERNAME|." }, {value = 35000, newItem = 11345, desc = "This little figurine made by |PLAYERNAME| has some room for improvement." }, { value = 60000, newItem = 11346, desc = "This little figurine of God was masterfully sculpted by |PLAYERNAME|." } },

		-- Ice Cube
		[7441] = {value = 25000, newItem = 7442},
		[7442] = {value = 25000, newItem = 7444},
		[7444] = {value = 25000, newItem = 7445},
		[7445] = {value = 25000, newItem = 7446},
	},
	[5942] = {
		-- Vampires
		[2956] = {value = 25000, newItem = 5905},
		[6006] = {value = 25000, newItem = 5905, after = 2957},
		[8938] = {value = 25000, newItem = 5905},
		[9654] = {value = 25000, newItem = 5905, after = 9658},
		[21275] = {value = 25000, newItem= 5905},
		[21278] = {value = 25000, newItem = 5905, after = 21276}
	}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local skin = config[item.itemid][target.itemid]

	-- Wrath of the emperor quest
	if item.itemid == 5908 and target.itemid == 12295 then
		target:transform(12287)
		player:say("You carve a solid bowl of the chunk of wood.", TALKTYPE_MONSTER_SAY)
	end

	if not skin then
		player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	local random, effect, transform = math.random(1, 100000), CONST_ME_MAGIC_GREEN, true
	if type(skin[1]) == 'table' then
		local added = false
		local _skin
		for i = 1, #skin do
			_skin = skin[i]
			if random <= _skin.value then
				if target.itemid == 11343 then
					effect = CONST_ME_ICEAREA
					local gobletItem = player:addItem(_skin.newItem, _skin.amount or 1)
					if gobletItem then
						gobletItem:setDescription(_skin.desc:gsub('|PLAYERNAME|', player:getName()))
					end
					added = true
				elseif table.contains({7441, 7442, 7444, 7445}, target.itemid) then
					player:addItem(_skin.newItem, _skin.amount or 1)
					effect = CONST_ME_HITAREA
					added = true
				else
					player:addItem(_skin.newItem, _skin.amount or 1)
					added = true
				end
				break
			end
		end

		if not added and target.itemid == 8961 then
			effect = CONST_ME_POFF
			transform = false
		end
	elseif random <= skin.value then
		if target.itemid == 11343 then
			effect = CONST_ME_ICEAREA
			local gobletItem = player:addItem(skin.newItem, skin.amount or 1)
			if gobletItem then
				gobletItem:setDescription(skin.desc:gsub('|PLAYERNAME|', player:getName()))
			end
		elseif table.contains({7441, 7442, 7444, 7445}, target.itemid) then
			if skin.newItem == 7446 then
				player:addAchievement('Ice Sculptor')
			end
			player:addItem(skin.newItem, skin.amount or 1)
			effect = CONST_ME_HITAREA
		else
			player:addItem(skin.newItem, skin.amount or 1)
		end
	else
		if table.contains({7441, 7442, 7444, 7445}, target.itemid) then
			player:say('The attempt of sculpting failed miserably.', TALKTYPE_MONSTER_SAY)
			effect = CONST_ME_HITAREA
		else
			effect = CONST_ME_POFF
		end
	end

	toPosition:sendMagicEffect(effect)
	if transform then
		target:transform(skin.after or target.itemid + 1)
	end

	return true
end
