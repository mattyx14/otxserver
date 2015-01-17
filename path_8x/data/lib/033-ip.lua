function doConvertIntegerToIp(int, mask)
	local b = {
		bit.urshift(bit.uband(int, 4278190080), 24),
		bit.urshift(bit.uband(int, 16711680), 16),
		bit.urshift(bit.uband(int, 65280), 8),
		bit.urshift(bit.uband(int, 255), 0)
	}
	if(mask ~= nil) then
		local m = {
			bit.urshift(bit.uband(mask,  4278190080), 24),
			bit.urshift(bit.uband(mask,  16711680), 16),
			bit.urshift(bit.uband(mask,  65280), 8),
			bit.urshift(bit.uband(mask,  255), 0)
		}
		if((m[1] == 255 or m[1] == 0) and (m[2] == 255 or m[2] == 0) and (m[3] == 255 or m[3] == 0) and (m[4] == 255 or m[4] == 0)) then
			for i = 1, 4 do
				if(m[i] == 0) then
					b[i] = "*"
				end
			end
		elseif(m[1] ~= 255 or m[2] ~= 255 or m[3] ~= 255 or m[4] ~= 255) then
			return b[4] .. "." .. b[3] .. "." .. b[2] .. "." .. b[1] .. ":" .. m[4] .. "." .. m[3] .. "." .. m[2] .. "." .. m[1]
		end
	end

	return b[4] .. "." .. b[3] .. "." .. b[2] .. "." .. b[1]
end

function doConvertIpToInteger(str)
	local maskIndex = str:find(":")
	if(maskIndex == nil) then
		local ip, mask, index = 0, 0, 24
		for b in str:gmatch("([x%d]+)%.?") do
			if(b ~= "x") then
				if(b:find("x") ~= nil) then
					return 0, 0
				end

				local t = tonumber(b)
				if(t == nil or t > 255 or t < 0) then
					return 0, 0
				end

				mask = bit.ubor(mask, bit.ulshift(255, index))
				ip = bit.ubor(ip, bit.ulshift(b, index))
			end

			index = index - 8
			if(index < 0) then
				break
			end
		end

		if(index ~= -8) then
			return 0, 0
		end

		return ip, mask
	end

	if(maskIndex <= 1) then
		return 0, 0
	end

	local ipString, maskString, ip, mask, index = str:sub(1, maskIndex - 1), str:sub(maskIndex), 0, 0, 0
	for b in ipString:gmatch("(%d+).?") do
		local t = tonumber(b)
		if(t == nil or t > 255 or t < 0) then
			return 0, 0
		end

		ip = bit.ubor(ip, bit.ulshift(b, index))
		index = index + 8
		if(index > 24) then
			break
		end
	end

	if(index ~= 32) then
		return 0, 0
	end

	index = 0
	for b in maskString:gmatch("(%d+)%.?") do
		local t = tonumber(b)
		if(t == nil or t > 255 or t < 0) then
			return 0, 0
		end

		mask = bit.ubor(mask, bit.ulshift(b, index))
		index = index + 8
		if(index > 24) then
			break
		end
	end

	if(index ~= 32) then
		return 0, 0
	end

	return ip, mask
end

function doRevertIp(str)
	local i, ip = 4, {}
	for b in str:gmatch("(%d+).?") do
		ip[i] = b
		i = i - 1
	end

	if(not ip[1] or not ip[2] or not ip[3] or not ip[4]) then
		return nil
	end

	return table.concat(ip, ".")
end
