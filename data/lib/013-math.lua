math.round = function(num, idp)
	return tonumber(string.format("%." .. (idp or 0) .. "f", num))
end
math.rand = std.random
