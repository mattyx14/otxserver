-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("hand")
deEquip:id(18390, 18409, 18411, 18412, 18452, 18451, 18465)
deEquip:register()

-- lvl 120 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("hand")
equip:level(120)
equip:id(18452, 18451, 18465)
equip:register()

-- sorcerer lvl 65 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("hand")
equip:level(65)
equip:vocation("sorcerer", true, true)
equip:vocation("master sorcerer")
equip:id(18390, 18409)
equip:register()

-- druid lvl 65 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("hand")
equip:level(65)
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(18411, 18412)
equip:register()
