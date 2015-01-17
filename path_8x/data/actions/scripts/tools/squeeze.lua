local config = {
	functions = {
		[10511] = { -- sneaky stabber of eliteness
			TOOLS.ROPE,
			TOOLS.SHOVEL,
			TOOLS.PICK,
			TOOLS.MACHETE,
			TOOLS.KNIFE
		},
		[10513] = { -- squeezing gear of girlpower
			TOOLS.ROPE,
			TOOLS.SHOVEL,
			TOOLS.PICK,
			TOOLS.MACHETE,
			TOOLS.SCYTHE
		},
		[10515] = { -- whacking driller of fate
			TOOLS.ROPE,
			TOOLS.SHOVEL,
			TOOLS.PICK,
			TOOLS.MACHETE,
			TOOLS.KNIFE
		}
	},
	jamChance = 10
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local funcs = config.functions[item.itemid]
	if(funcs == nil) then
		return false
	end

	local result = false
	for _, func in ipairs(funcs) do
		if(func(cid, item, fromPosition, itemEx, toPosition)) then
			result = true
			break
		end
	end

	if(not result) then
		return false
	end

	if(math.random(1, 100) <= config.jamChance) then
		doTransformItem(item.uid, item.itemid + 1)
		doDecayItem(item.uid)
	end

	return true
end