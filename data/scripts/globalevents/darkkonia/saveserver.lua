local autoSave = GlobalEvent("AutoSave")
function autoSave.onThink(interval)
	saveServer()
	return true
end

autoSave:interval(900000)
autoSave:register()
