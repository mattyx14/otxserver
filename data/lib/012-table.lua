table.append = table.insert
table.empty = function (t)
	return next(t) == nil
end

table.size = function (t)
	local size = 0
	for k, v in pairs(t) do
		size = size + 1
	end

	return size
end

table.find = function (t, value, caseSensitive)
	if((caseSensitive == nil or caseSensitive == false) and type(value) == "string") then
		local lowerValue = value:lower()
		for k, v in pairs(t) do
			if(type(v) == "string" and lowerValue == v:lower()) then
				return k
			end
		end
	else
		for k, v in pairs(t) do
			if(value == v) then
				return k
			end
		end
	end

	return nil
end
table.getPos = table.find

table.contains = function (t, value, caseSensitive)
	if((caseSensitive == nil or caseSensitive == false) and type(value) == "string") then
		local lowerValue = value:lower()
		for k, v in pairs(t) do
			if(type(v) == "string" and lowerValue == v:lower()) then
				return true
			end
		end
	else
		for k, v in pairs(t) do
			if(value == v) then
				return true
			end
		end
	end

	return false
end
table.isInArray = table.contains

table.count = function (t, item)
	local count = 0
	for k, v in pairs(t) do
		if(item == v) then
			count = count + 1
		end
	end

	return count
end
table.countElements = table.count

table.isStrIn = function (txt, str)
	for k, v in pairs(str) do
		if(txt:find(v) and not txt:find('(%w+)' .. v) and not txt:find(v .. '(%w+)')) then
			return true
		end
	end

	return false
end

table.getCombinations = function (t, num)
	local a, number, select, newList = {}, table.size(t), num, {}
	for i = 1, select do
		table.insert(a, i)
	end

	local newThing = {}
	while(true) do
		local newRow = {}
		for i = 1, select do
			table.insert(newRow, t[a[i]])
		end

		t.insert(newList, newRow)
		i = select
		while(a[i] == (number - select + i)) do
			i = i - 1
		end

		if(i < 1) then
			break
		end

		a[i] = a[i] + 1
		for j = i, select do
			a[j] = a[i] + j - i
		end
	end

	return newList
end

function table.serialize(x, recur)
	local t = type(x)
	recur = recur or {}

	if(t == nil) then
		return "nil"
	elseif(t == "string") then
		return string.format("%q", x)
	elseif(t == "number") then
		return tostring(x)
	elseif(t == "boolean") then
		return x and "true" or "false"
	elseif(getmetatable(x)) then
		error("Can not serialize a table that has a metatable associated with it.")
	elseif(t == "table") then
		if(table.find(recur, x)) then
			error("Can not serialize recursive tables.")
		end
		table.append(recur, x)

		local s = "{"
		for k, v in pairs(x) do
			s = s .. "[" .. table.serialize(k, recur) .. "]" .. " = " .. table.serialize(v, recur) .. ", "
		end

		return s:sub(0, s:len() - 2) .. "}"
	end

	error("Can not serialize value of type '" .. t .. "'.")
end

function table.unserialize(str)
	if(type(str) ~= 'string' or str:len() == 0) then
		return {}
	end

	return loadstring("return " .. str)()
end

function table.clone(src)
	if(type(src) ~= 'table') then
		return src
	end

	local clone = {}
	for key, value in pairs(src) do
		clone[key] = table.clone(value)
	end

	return clone
end

function table.merge(t1, t2, override)
	if(override) then
		for k, v in pairs(t2) do
			t1[k] = v
		end
	else
		for _, v in ipairs(t2) do
			table.insert(t1, v)
		end
	end

	return t1
end
