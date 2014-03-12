local config = {
	waters = {4614, 4615, 4616, 4617, 4618, 4619, 4620, 4621, 4622, 4623, 4624, 4625, 4665, 4666, 4820, 4821, 4822, 4823, 4824, 4825},
	fishable = {4608, 4609, 4610, 4611, 4612, 4613},
	spawning = {4614, 4615, 4616, 4617, 4618, 4619},
	holes = {7236},

	corpses = {
		-- [corpse] = {[aid] = { {itemid, countmax, chance} }}
	},
	rateLoot = getConfigValue("rateLoot"),
	summons = {
		-- {skill, name, chance, bossName, bossChance}
	},
	rateSpawn = getConfigValue("rateSpawn"),
	baitFailRemoveChance = 10,
	allowFromPz = false,
	useBait = true,
	baitCount = 1,
	fishes = 1
}


function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(isInArray(config.waters, itemEx.itemid)) then
		if(isInArray(config.spawning, itemEx.itemid)) then
			doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		end

		doSendMagicEffect(toPosition, CONST_ME_LOSEENERGY)
		return true
	end

	local corpse = config.corpses[itemEx.itemid]
	if(corpse ~= nil and corpse ~= 0) then
		corpse = corpse[itemEx.actionid]
		if(corpse ~= nil and corpse ~= 0) then
			local chance, items, default, max = math.random(0, 100000) / config.rateLoot, {}, {}, 0
			for _, data in ipairs(corpse) do
				if(data[3] >= chance) then
					local tmp = {data[1], math.random(1, data[2])}
					table.insert(items, tmp)
				end
				if(data[3] > max) then
					default = data
					max = data[3]
				end
			end

			local itemCount = table.maxn(items)
			if(itemCount > 0) then
				local loot = items[math.random(1, itemCount)]
				doPlayerAddItem(cid, loot[1], loot[2])
			else
				doPlayerAddItem(cid, default[1], default[2])
			end

			doTransformItem(itemEx.uid, getItemInfo(itemEx.itemid).decayTo)
			doSendMagicEffect(toPosition, CONST_ME_WATERSPLASH)
			doDecayItem(itemEx.uid)
			return true
		end
	end

	if(not isInArray(config.fishable, itemEx.itemid)) then
		return false
	end

	local position, formula, tries = getThingPosition(cid), getPlayerSkill(cid, SKILL_FISHING) / 200 + 0.85 * math.random(), 0
	config.allowFromPz = config.allowFromPz or not getTileInfo(position).protection
	if(item.itemid ~= ITEM_MECHANICAL_FISHING_ROD) then
		if(config.allowFromPz and (not config.useBait or getPlayerItemCount(cid, ITEM_WORM) >= config.baitCount)) then
			tries = 1
			if(isInArray(config.holes, itemEx.itemid)) then
				if(doPlayerRemoveItem(cid, ITEM_WORM, config.baitCount)) then
					tries = 2
					if(formula > 0.83) then
						doPlayerAddItem(cid, ITEM_RAINBOW_TROUT, config.fishes)
					elseif(formula > 0.7) then
						doPlayerAddItem(cid, ITEM_NORTHERN_PIKE, config.fishes)
					elseif(formula > 0.5) then
						doPlayerAddItem(cid, ITEM_GREEN_PERCH, config.fishes)
					else
						doPlayerAddItem(cid, ITEM_FISH, config.fishes)
					end
				end
			elseif(formula > 0.7 and doPlayerRemoveItem(cid, ITEM_WORM, config.baitCount)) then
				tries = 2
				if(table.maxn(config.summons) > 0 and getDistanceBetween(position, toPosition) < 2) then
					local skill, summon = getPlayerSkill(cid, SKILL_FISHING), {name = "", chance = 0, bossName = "", bossChance = 0}
					for _, data in pairs(config.summons) do
						if(skill >= data[1]) then
							summon.name = data[2]
							summon.chance = data[3]
							summon.bossName = data[4]
							summon.bossChance = data[5]
						end
					end

					local random = math.random(1, 100000) / config.rateSpawn
					if(summon.bossName ~= "" and summon.bossChance >= random) then
						doCreateMonster(summon.bossName, position)
						tries = 4
					elseif(summon.name ~= "" and summon.chance >= random) then
						doCreateMonster(summon.name, position)
						tries = 3
					else
						doPlayerAddItem(cid, ITEM_FISH, config.fishes)
					end
				else
					doPlayerAddItem(cid, ITEM_FISH, config.fishes)
				end
			end
		end
	elseif(config.allowFromPz and (not config.useBait or getPlayerItemCount(cid, ITEM_NAIL) >= config.baitCount)) then
		if(formula > 0.7 and doPlayerRemoveItem(cid, ITEM_NAIL, config.baitCount)) then
			doPlayerAddItem(cid, ITEM_MECHANICAL_FISH, config.fishes)
			tries = 2
		else
			tries = 1
		end
	end

	if(tries > 1) then
		doPlayerAddSkillTry(cid, SKILL_FISHING, tries)
		if(not isInArray(config.holes, itemEx.itemid)) then
			doTransformItem(itemEx.uid, itemEx.itemid + 6)
		else
			doTransformItem(itemEx.uid, itemEx.itemid + 1)
		end

		doDecayItem(itemEx.uid)
	elseif(tries > 0) then
		doPlayerAddSkillTry(cid, SKILL_FISHING, 1)
		if(config.baitFailRemoveChance >= math.random(1, 100)) then
			if(item.itemid == ITEM_MECHANICAL_FISHING_ROD) then
				doPlayerRemoveItem(cid, ITEM_NAIL, config.baitCount)
			else
				doPlayerRemoveItem(cid, ITEM_FISH, config.baitCount)
			end
		end
	end

	doSendMagicEffect(toPosition, CONST_ME_LOSEENERGY)
	return true
end
