local focuses = {}
local function isFocused(cid)
	for i, v in pairs(focuses) do
		if(v == cid) then
			return true
		end
	end
	return false
end

local function addFocus(cid)
	if(not isFocused(cid)) then
		table.insert(focuses, cid)
	end
end
local function removeFocus(cid)
	for i, v in pairs(focuses) do
		if(v == cid) then
			table.remove(focuses, i)
			break
		end
	end
end
local function lookAtFocus()
	for i, v in pairs(focuses) do
		if(isPlayer(v)) then
			doNpcSetCreatureFocus(v)
			return
		end
	end
	doNpcSetCreatureFocus(0)
end

local itemWindow = {
	{id=2160, subType=0, buy=10000, sell=10000, name="Test1"},
	{id=2152, subType=0, buy=100, sell=100, name="Test2"},
	{id=2148, subType=0, buy=1, sell=1, name="Test3"},
	{id=2173, subType=0, buy=10000, sell=5000, name="Test4"}
}

local items = {}
for _, item in ipairs(itemWindow) do
	items[item.id] = {buyPrice = item.buy, sellPrice = item.sell, subType = item.subType, realName = item.name}
end

local onBuy = function(cid, item, subType, amount, ignoreCap, inBackpacks)
	if(items[item] == nil) then
		selfSay("Ehm.. sorry... this shouldn't be there, I'm not selling it.", cid)
		return
	end

	if(getPlayerMoney(cid) >= amount * items[item].buyPrice) then
		local itemz, i = doNpcSellItem(cid, item, amount, subType, ignoreCap, inBackpacks)
		if(i < amount) then
			if(i == 0) then
				selfSay("Sorry, but you don't have space to take it.", cid)
			else
				selfSay("I've sold some for you, but it seems you can't carry more than this. I won't take more money than necessary.", cid)
				doPlayerRemoveMoney(cid, i * items[item].buyPrice)
			end
		else
			selfSay("Thanks for the money!", cid)
			doPlayerRemoveMoney(cid, amount * items[item].buyPrice)
		end
	else
		selfSay("Stfu noob, you don't have money.", cid)
	end
end

local onSell = function(cid, item, subType, amount, ignoreCap, inBackpacks)
	if(items[item] == nil) then
		selfSay("Ehm.. sorry... this shouldn't be there, I'm not buying it.", cid)
	end

	if(subType < 1) then
		subType = -1
	end
	if(doPlayerRemoveItem(cid, item, amount, subType)) then
		doPlayerAddMoney(cid, items[item].sellPrice * amount)
		selfSay("Here you are.", cid)
	else
		selfSay("No item, no deal.", cid)
	end
end

function onCreatureAppear(cid)
	--
end

function onCreatureDisappear(cid)
	if(isFocused(cid)) then
		selfSay("Hmph!")
		removeFocus(cid)
		if(isPlayer(cid)) then --Be sure he's online
			closeShopWindow(cid)
		end
	end
end

function onCreatureSay(cid, type, msg)
	if((msg == "hi") and not (isFocused(cid))) then
		selfSay("Welcome, ".. getCreatureName(cid) ..".", cid, true)
		selfSay("Do you want to see my {wares}?", cid)
		addFocus(cid)
	elseif((isFocused(cid)) and (msg == "wares" or msg == "trade")) then
		selfSay("Pretty nice, right?", cid)
		openShopWindow(cid, itemWindow, onBuy, onSell)
	elseif((isFocused(cid)) and (msg == "bye" or msg == "goodbye" or msg == "cya")) then
		selfSay("Goodbye!", cid, true)
		closeShopWindow(cid)
		removeFocus(cid)
	end
end

function onPlayerCloseChannel(cid)
	if(isFocused(cid)) then
		selfSay("Hmph!")
		closeShopWindow(cid)
		removeFocus(cid)
	end
end

function onPlayerEndTrade(cid)
	selfSay("It was a pleasure doing business with you.", cid)
end

function onThink()
	for i, focus in pairs(focuses) do
		if(not isCreature(focus)) then
			removeFocus(focus)
		else
			local distance = getDistanceTo(focus) or -1
			if((distance > 4) or (distance == -1)) then
				selfSay("Hmph!")
				closeShopWindow(focus)
				removeFocus(focus)
			end
		end
	end
	lookAtFocus()
end
