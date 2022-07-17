Storage = {
	-- General storages
	isTraining = 30000,
	NpcExhaust = 30001,
	RentedHorseTimer = 30015,
	-- Promotion Storage cannot be changed, it is set in source code
	Promotion = 30018,
	combatProtectionStorage = 30023,
	Factions = 30024,
	blockMovementStorage = 30025,
	FamiliarSummon = 30026,
	TrainerRoom = 30027,
	NpcSpawn = 30028,
	ExerciseDummyExhaust = 30029,
	StrawberryCupcake = 30032,
	StoreExaust = 30051,
	LemonCupcake = 30052,
	BlueberryCupcake = 30053,
	FamiliarSummonEvent10 = 30054,
	FamiliarSummonEvent60 = 30055,
	FreeQuests = 30057,
	PremiumAccount = 30058,

	--[[
	Old storages
	Over time, this will be dropped and replaced by the table above
	]]
	Grimvale = {
		-- Reserved storage from 50380 - 50399
		SilverVein = 50380,
		WereHelmetEnchant = 50381
	},
	OutfitQuest = {
		-- Reserved storage from 50960 - 51039
		-- Until all outfit quests are completed
		DefaultStart = 50960,
		Ref = 50961,
		-- Golden Outfit
		GoldenOutfit = 51015,
	},
	AdventurersGuild = {
		-- Reserved storage from 52130 - 52159
		Stone = 52130,
	},
}

GlobalStorage = {
	ExpBoost = 65004,
	XpDisplayMode = 65006,
	OberonEventTime = 65009,
	ScarlettEtzelEventTime = 65011,
	CobraBastionFlask = 65012
}

-- Values extraction function
local function extractValues(tab, ret)
	if type(tab) == "number" then
		table.insert(ret, tab)
	else
		for _, v in pairs(tab) do
			extractValues(v, ret)
		end
	end
end

local extraction = {}
extractValues(Storage, extraction) -- Call function
table.sort(extraction) -- Sort the table
-- The choice of sorting is due to the fact that sorting is very cheap O (n log2 (n))
-- And then we can simply compare one by one the elements finding duplicates in O(n)

-- Scroll through the extracted table for duplicates
if #extraction > 1 then
	for i = 1, #extraction - 1 do
		if extraction[i] == extraction[i+1] then
			Spdlog.warn(string.format("Duplicate storage value found: %d",
				extraction[i]))
		end
	end
end
