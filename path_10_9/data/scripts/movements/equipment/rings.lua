-- default equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("ring")
equip:id(2164, 2165, 2202, 2166, 2203, 2167, 2204, 2168, 2205, 2169, 2206, 2207, 2210, 2208, 2211, 2209, 2212, 2213, 2215, 2214, 2216, 6300, 6301)
equip:register()

-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("ring")
deEquip:id(2164, 2202, 2203, 2204, 2205, 2206, 2210, 2211, 2212, 2215, 2216, 6301, 18528, 22516)
deEquip:register()

-- lvl 120 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("ring")
equip:level(120)
equip:id(18408, 18528)
equip:register()

-- lvl 200 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("ring")
equip:level(200)
equip:id(22516)
equip:register()
