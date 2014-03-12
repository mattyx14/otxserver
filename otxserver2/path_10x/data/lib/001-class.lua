function createClass(parent)
	local newClass = {}
	function newClass:new(instance)
		local instance = instance or {}
		setmetatable(instance, {__index = newClass})
		return instance
	end

	if(parent ~= nil) then
		setmetatable(newClass, {__index = parent})
	end

	function newClass:getSelf()
		return newClass
	end

	function newClass:getParent()
		return baseClass
	end

	function newClass:isa(class)
		local tmp = newClass
		while(tmp ~= nil) do
			if(tmp == class) then
				return true
			end

			tmp = tmp:getParent()
		end

		return false
	end

	function newClass:setAttributes(attributes)
		for k, v in pairs(attributes) do
			newClass[k] = v
		end
	end

	return newClass
end
