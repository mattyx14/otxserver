local items = {101, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382}
 
function onMoveItem(moveItem, frompos, position, cid)
    local item = getTileThingByPos(position)
    if (item.itemid > 0 and isInArray(items, item.itemid)) then
        return doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE) and false
    end
    return true
end