local config = {
	positions = {
		["Welcome"] = {
			x = 159, y = 383, z = 7
		},
		--[[
		["OTHER"] = {
			x = OTHER, y = OTHER, z = OTHER
		},
		]]
	},

	-- Find more on ../data/lib/000-constant.lua
	effects = {
		CONST_ME_TELEPORT,
		CONST_ME_MAGIC_BLUE,
		CONST_ME_MAGIC_GREEN,
		-- CONST_ME_MORTAREA
	}
}

function onThink(cid, interval, lastExecution)
	for text, pos in pairs(config.positions) do
		doSendMagicEffect(pos, config.effects[math.random(1, #config.effects)])
		doSendAnimatedText(pos, text, math.random(255))
	end

	return true
end