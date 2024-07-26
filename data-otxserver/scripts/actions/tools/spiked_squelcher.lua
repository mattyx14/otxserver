local spikedSquelcher = Action()

function spikedSquelcher.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	return onUseSpikedSquelcher(player, item, fromPosition, target, toPosition, isHotkey)
end

spikedSquelcher:id(7452)
spikedSquelcher:register()
