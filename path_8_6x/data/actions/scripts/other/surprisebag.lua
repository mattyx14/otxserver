local config = {
	-- NOTE: data from tibia.wikia.com
	
	--[xxxx] = {
		--{ rate = 1, item = 2160, count = { min = 1, max = 2 }},
		--{ rate = 2, item = 2400, count = { const = 1 }}
	--}
	
	-- blue
	[6570] = {		
		{ rate = 1132, item = 6280, count = { const = 1 }},
		{ rate = 1132, item = 2114, count = { const = 1 }},
		{ rate = 991, item = 6575, count = { const = 1 }},
		{ rate = 943, item = 6576, count = { const = 1 }},
		{ rate = 991, item = 6578, count = { const = 1 }},
		{ rate = 991, item = 6572, count = { const = 1 }},
		{ rate = 802, item = 6577, count = { const = 1 }},
		{ rate = 802, item = 2687, count = { const = 10 }},
		{ rate = 849, item = 6569, count = { const = 3 }},
		{ rate = 755, item = 6574, count = { const = 1 }},
		{ rate = 613, item = 6394, count = { const = 1 }}
	},
	-- red
	[6571] = {
		{ rate = 4745, item = 6576, count = { const = 1 }},
		{ rate = 2072, item = 6394, count = { const = 1 }},
		{ rate = 1152, item = 6574, count = { const = 1 }},
		{ rate = 1017, item = 6578, count = { const = 1 }},
		{ rate = 538, item = 2114, count = { const = 1 }},
		{ rate = 20, item = 2112, count = { const = 1 }},
		{ rate = 17, item = 2195, count = { const = 1 }},
		{ rate = 17, item = 5080, count = { const = 1 }},
		{ rate = 15, item = 2520, count = { const = 1 }},
		{ rate = 7, item = 2173, count = { const = 1 }},
		{ rate = 17, item = 2492, count = { const = 1 }},
		{ rate = 167, item = 2153, count = { const = 1 }},
		{ rate = 37, item = 2156, count = { const = 1 }},
		{ rate = 159, item = 5944, count = { const = 1 }},
		{ rate = 7, item = 6566, count = { const = 1 }},
		{ rate = 13, item = 2498, count = { const = 1 }}
	},
}

local sumOfRate = {}

for k,v in pairs(config) do
	sumOfRate[k] = 0
	for k2,v2 in pairs(v) do
		sumOfRate[k] = sumOfRate[k] + v2.rate
	end
end

function randomGift(itemid)
	local rand = math.random(1, sumOfRate[itemid])

	local subSum = 0
	local exactItem = nil
	for i,v in ipairs(config[itemid]) do
		if (subSum > rand) then
			if (i > 1) then
				exactItem = i-1
			end
			break
		end
		subSum = subSum + v.rate
		if (subSum >= rand) then
			exactItem = i
			break
		end
	end
	
	return exactItem
end

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local run = false
	
	for k,v in pairs(config) do
		if (k == item.itemid) then
			run = true
			break
		end
	end
	
	if (not run) then
		return false
	end
	
	local gift = randomGift(item.itemid)

	if (config[item.itemid][gift].count.const ~= nil) then
		doPlayerAddItem(cid, config[item.itemid][gift].item, config[item.itemid][gift].count.const)
	else
		doPlayerAddItem(cid, config[item.itemid][gift].item, math.random(config[item.itemid][gift].count.min, config[item.itemid][gift].count.max))
	end
	
	doSendMagicEffect(fromPosition, CONST_ME_GIFT_WRAPS)
	doRemoveItem(item.uid, 1)
	return true
end