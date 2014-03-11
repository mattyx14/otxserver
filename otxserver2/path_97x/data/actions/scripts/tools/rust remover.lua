function onUse(cid, item, frompos, item2, topos)
	local effect_broke = 3
	local effect_renew = 28
	local developed = 0
	local const = item2.itemid
	local pos = getCreaturePosition(cid)
	local item = 
	{
		[9808] = {
			[1] = {id = 2464, name = "Chain Armor", opportunity = 33},
			[2] = {id = 2483, name = "Scale Armor", opportunity = 25},
			[3] = {id = 2465, name = "Brass Armor", opportunity = 10},
			[4] = {id = 2463, name = "Plate Armor", opportunity = 2}
		},
		[9809] = {
			[1] = {id = 2464, name = "Chain Armor", opportunity = 16},
			[2] = {id = 2465, name = "Brass Armor", opportunity = 14},
			[3] = {id = 2483, name = "Scale Armor", opportunity = 13},
			[4] = {id = 2463, name = "Plate Armor", opportunity = 10},
			[5] = {id = 2476, name = "Knight Armor", opportunity = 6},
			[6] = {id = 8891, name = "Paladin Armor", opportunity = 3},
			[7] = {id = 2487, name = "Crown Armor", opportunity = 1}
		},
		[9810] = {
			[1] = {id = 2464, name = "Chain Armor", opportunity = 20},
			[2] = {id = 2465, name = "Brass Armor", opportunity = 17},
			[3] = {id = 2483, name = "Scale Armor", opportunity = 15},
			[4] = {id = 2463, name = "Plate Armor", opportunity = 12},
			[5] = {id = 2476, name = "Knight Armor", opportunity = 10},
			[6] = {id = 8891, name = "Paladin Armor", opportunity = 5},
			[7] = {id = 2487, name = "Crown Armor", opportunity = 4},
			[8] = {id = 2466, name = "Golden Armor", opportunity = 2},
			[9] = {id = 2472, name = "Magic Plate Armor", opportunity = 1}
		},
		[9811] = {
			[1] = {id = 2468, name = "Studded Legs", opportunity = 33},
			[2] = {id = 2648, name = "Chain Legs", opportunity = 25},
			[3] = {id = 2478, name = "Brass Legs", opportunity = 10},
			[4] = {id = 2647, name = "Plate Legs", opportunity = 2}
		},
		[9812] = {
			[1] = {id = 2468, name = "Studded Legs", opportunity = 16},
			[2] = {id = 2648, name = "Chain Legs", opportunity = 14},
			[3] = {id = 2478, name = "Brass Legs", opportunity = 13},
			[4] = {id = 2647, name = "Plate Legs", opportunity = 10},
			[5] = {id = 2477, name = "Knight Legs", opportunity = 6},
			[7] = {id = 2488, name = "Crown Legs", opportunity = 1}
		},
		[9813] = {
			[2] = {id = 2478, name = "Brass Legs", opportunity = 17},
			[4] = {id = 2647, name = "Plate Legs", opportunity = 12},
			[5] = {id = 2477, name = "Knight Legs", opportunity = 10},
			[7] = {id = 2488, name = "Crown Legs", opportunity = 4},
			[8] = {id = 2470, name = "Golden Legs", opportunity = 2}
		}
	}

	if item[const] then
		local random_item = math.random(1, 100)

		for i = 1, #item[const] do
			if random_item <= item[const][i].opportunity then
				developed = i
			end
		end

		if developed > 0 then
			doSendMagicEffect(topos, effect_renew)
			doTransformItem(item2.uid, item[const][developed].id)
			doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "You have renewed the ".. item[const][developed].name .." !")
			doPlayerRemoveItem(item.uid, 1)
		else
			doSendMagicEffect(topos, effect_broke)
			doRemoveItem(item2.uid, 1)
			doRemoveItem(item.uid, 1)
			doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "Your Rusty Remover has broken.")
			return false
		end
	else
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "Use it on Rusty Items (Common, Semi-Rare or Rare: Armors or Legs).")
		return false
	end

	return true
end