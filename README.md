### What is OTX Server 3 Series
We are trying to create the perfect custom open tibia server.

### Script Contribution
**Important:**
- Scripts should follow this [lua style guide](https://github.com/Olivine-Labs/lua-style-guide) and be as efficient as possible.
- Use tabs as indentation.
- Use new metatable functions or you have to revise your pull request.

**Example script (using metatables):**
```lua
local player = Player(cid)
if not player then
	return true
end

player:addItem(2160, 5)
player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, 'Here is some cash.')
```
**Example script (not using metatables):**
```lua
if not isPlayer(cid) then
	return true
end

doPlayerAddItem(cid, 2160, 5)
doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, 'Here is some cash.')
```

Current map (missing areas are marked):
![minimap_floor](--)


**Remember to have fun and say thanks!**
