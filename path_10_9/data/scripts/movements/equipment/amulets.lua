-- default equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("necklace")
equip:id(2161, 2170, 2172, 2197, 2198, 2199, 2200, 2201, 2173, 8266, 11374)
equip:register()

-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("necklace")
deEquip:id(2161, 2170, 2172, 2197, 2198, 2199, 2200, 2201, 2173, 7887, 7888, 7889, 7890, 8266, 10218, 10219, 10220, 10221, 11374, 15403, 18402, 18407, 21691)
deEquip:register()

-- lvl 60 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("necklace")
equip:level(60)
equip:id(7887, 7888, 7889, 7890)
equip:register()

-- lvl 80 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("necklace")
equip:level(80)
equip:id(10218, 10219, 10220, 10221)
equip:register()

-- lvl 120 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("necklace")
equip:level(120)
equip:id(15403)
equip:register()

-- lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("necklace")
equip:level(150)
equip:id(18402, 18407, 21691)
equip:register()