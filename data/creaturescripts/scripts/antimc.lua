local AccForIp = 4
function onLogin(player)
	local mc = 0
	for _, check in ipairs(Game.getPlayers()) do
		if player:getIp() == check:getIp() then
			mc = mc + 1
			if mc > AccForIp then
				return false
			end
		end
	end

	return true
end
