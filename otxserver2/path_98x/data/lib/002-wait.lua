wait = coroutine.yield

function runThread(co)
	if(coroutine.status(co) ~= 'dead') then
		local _, delay = coroutine.resume(co)
		addEvent(runThread, delay, co)
	end
end

function createThread(data)
	local dataType, fn = type(data), nil
	if(dataType == 'string') then
		fn = loadstring(data)
	elseif(dataType == 'function') then
		fn = data
	end

	if(fn ~= nil) then
		local co = coroutine.create(fn)
		runThread(co)
	else
		print("[createThread]", "Invalid data specified.")
	end
end
