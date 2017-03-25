function Monster:onSpawn(position)
	return true
end

-- Example
--[[
	function Monster:onSpawn(position)
		local maxHealth = math.random(self:getHealth())
		self:addHealth(-maxHealth)
		return true
	end
]]
