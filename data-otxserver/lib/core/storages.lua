--[[
Reserved player action storage key ranges (const.h)
	It is possible to place the storage in a quest door, so the player who has that storage will go through the door

	Reserved player action storage key ranges (const.h at the source)
	[10000000 - 20000000]
	[1000 - 1500]
	[2001 - 2011]

	Others reserved player action/storages
	[100] = unmoveable/untrade/unusable items
	[101] = use pick floor
	[102] = well down action
	[103-120] = others keys action
	[103] = key 0010
	[303] = key 0303
	[1000] = level door. Here 1 must be used followed by the level.
	Example: 1010 = level 10, 1100 = level 100

	[3001-3008] = key 3001/3008
	[3012] = key 3012
	[3033] = key 3033
	[3100] = key 3100
	[3142] = key 3142
	[3200] = key 3200
	[3301] = key 3301
	[3302] = key 3302
	[3303] = key 3303
	[3304] = key 3304
	[3350] = key 3350
	[3520] = key 3520
	[3600] = key 3600
	[3610] = key 3610
	[3620] = key 3620
	[3650] = key 3650
	[3666] = key 3666
	[3667] = key 3667
	[3700] = key 3700
	[3701/3703] = key 3701/3703
	[3800/3802] = key 3800/3802
	[3899] = key 3899
	[3900] = key 3900
	[3909/3917] = key 3909/3917
	[3923] = key 3923
	[3925] = key 3925
	[3930] = key 3930
	[3932] = key 3932
	[3934] = key 3934
	[3935] = key 3935
	[3936] = key 3936
	[3938] = key 3938
	[3940] = key 3940
	[3950] = key 3950
	[3960] = key 3960
	[3980] = key 3980
	[3988] = key 3988
	[4001] = key 4001
	[4009] = key 4009
	[4022] = key 4022
	[4023] = key 4023
	[4033] = key 4033
	[4037] = key 4037
	[4055] = key 4055
	[4210] = key 4210
	[4501] = key 4501
	[4502] = key 4502
	[4503] = key 4503
	[4600] = key 4600
	[4601] = key 4601
	[4603] = key 4603
	[5000] = key 5000
	[5002] = key 5002
	[5010] = key 5010
	[5050] = key 5050
	[6010] = key 6010

	Questline = Storage through the Quest
]]

Storage = {
	-- General storages
	IsTraining = 30000,
	EmoteSpells = 30008,
	RentedHorseTimer = 30015,
	Promotion = 30018,
	Factions = 30024,
	ExerciseDummyExhaust = 30029,
	SwampDiggingTimeout = 30034, -- check
	LemonCupcake = 30052,
	BlueberryCupcake = 30053,
	FreeQuests = 30057,
	PremiumAccount = 30058,

	Kilmaresh = {
		-- Reserved storage from 50015 - 50049
		Questline = 50015,
		First = {
			Title = 50016
		},
		Second = {
			Investigating = 50017
		},
		Third = {
			Recovering = 50018
		},
		Fourth = {
			Moe = 50019,
			MoeTimer = 50020
		},
		Fifth = {
			Memories = 50021,
			MemoriesShards = 50022
		},
		Sixth = {
			Favor = 50023,
			FourMasks = 50024,
			BlessedStatues = 50025
		},
		Set = {
			Ritual = 50026
		},
		Eighth = {
			Yonan = 50027,
			Narsai = 50028,
			Shimun = 50029,
			Tefrit = 50030
		},
		Nine = {
			Owl = 50031
		},
		Tem = {
			Bleeds = 50032
		},
		Eleven = {
			Basin = 50033
		},
		Twelve = {
			Boss = 50034,
			Bragrumol = 50035,
			Mozradek = 50036,
			Xogixath = 50037
		},
		Thirteen = {
			Fafnar = 50038,
			Lyre = 50039,
			Presente = 50040
		},
		Fourteen = {
			Remains = 50041
		},
		UrmahlulluTimer = 50042,
		AccessDoor = 50043,
		NeferiTheSpyTimer = 50044,
		SisterHetaiTimer = 55045,
		AmenefTimer = 55046,
		CatacombDoors = 55047
	},
	TheSecretLibrary = {
		-- Reserved storage from 50050 - 50074
		TheOrderOfTheFalcon = {
			OberonTimer = 50050
		},
	},
	CultsOfTibia = {
		-- Reserved storage from 50200 - 50269
		Questline = 50200,
		Minotaurs = {
			EntranceAccessDoor = 50201,
			JamesfrancisTask = 50202,
			Mission = 50203,
			BossTimer = 50204,
			AccessDoor = 50205
		},
		MotA = {
			Mission = 50210,
			Stone1 = 50211,
			Stone2 = 50212,
			Stone3 = 50213,
			Answer = 50214,
			QuestionId = 50215,
			AccessDoorInvestigation = 50216,
			AccessDoorGareth = 50217,
			AccessDoorDenominator = 50218
		},
		Barkless = {
			Mission = 50225,
			sulphur = 50226,
			Tar = 50227,
			Ice = 50228,
			Death = 50229,
			Objects = 50230,
			Temp = 50231,
			BossTimer = 50232,
			TrialAccessDoor = 50243, -- 50233 is used by an ore wagon
			TarAccessDoor = 50234,
			AccessDoor = 50235,
			BossAccessDoor = 50236
		},
		Orcs = {
			Mission = 50240,
			LookType = 50241,
			BossTimer = 50242
		},
		Life = {
			Mission = 50245,
			BossTimer = 50246,
			AccessDoor = 50264
		},
		Humans = {
			Mission = 50250,
			Vaporized = 50251,
			Decaying = 50252,
			BossTimer = 50253
		},
		Misguided = {
			Mission = 50255,
			Monsters = 50256,
			Exorcisms = 50257,
			Time = 50258,
			BossTimer = 50259,
			AccessDoor = 50260
		},
		FinalBoss = {
			Mission = 50261,
			BossTimer = 50262,
			AccessDoor = 50263
		}
	},
	FirstDragon = {
		-- Reserved storage from 50350 - 50379
		Questline = 50350,
		DragonCounter = 50351,
		ChestCounter = 50352,
		TazhadurTimer = 50353,
		KalyassaTimer = 50354,
		SecretsCounter = 50355,
		GelidrazahAccess = 50356,
		GelidrazahTimer = 50357,
		DesertTile = 50358,
		StoneSculptureTile = 50359,
		SuntowerTile = 50360,
		ZorvoraxTimer = 50361,
		Horn = 50362,
		Scale = 50363,
		Bones = 50364,
		Tooth = 50365,
		AccessCave = 50366,
		SomewhatBeatable = 50367,
		FirstDragonTimer = 50368,
		RewardFeather = 50369,
		RewardMask = 50370,
		RewardBackpack = 50371
	},
	Grimvale = {
		-- Reserved storage from 50380 - 50399
		WereHelmetEnchant = 50381
	},
	FerumbrasAscension = {
		-- Reserved storage from 50420 - 50469
		RiftRunner = 50420, -- Scroll
		TheShattererTimer = 50421,
		TheLordOfTheLiceTimer = 50422,
		TarbazTimer = 50423,
		RazzagornTimer = 50424,
		RagiazTimer = 50425,
		ZamuloshTimer = 50426,
		ShulgraxTimer = 50427,
		MazoranTimer = 50428,
		PlagirathTimer = 50429,
		FerumbrasTimer = 50430,
		Tarbaz = 50431,
		Razzagorn = 50432,
		Ragiaz = 50433,
		Zamulosh = 50434,
		Shulgrax = 50435,
		Mazoran = 50436,
		Plagirath = 50437,
		Access = 50438,
		TheShatterer = 50439,
		ZamuloshTeleports = 50440,
		BasinCounter = 50441,
		TheLordOfTheLiceAccess = 50442,
		FirstDoor = 50443,
		MonsterDoor = 50444,
		TarbazDoor = 50445,
		HabitatsAccess = 50446,
		HabitatsTimer = 50447,
		TarbazNotes = 50448,
		ColorLever = 50449,
		BoneFluteWall = 50450,
		BoneFlute = 50451,
		Ring = 50452,
		Statue = 50453,
		Fount = 50454,
		Vampire = 50455,
		Flower = 50456,
		Ring2 = 50457,
		Bone = 50458,
		Reward = 50459,
		TheShattererLever = 50460
	},
	ForgottenKnowledge = {
		-- Reserved storage from 50470 - 50519
		AccessDeath = 50470,
		AccessViolet = 50471,
		AccessEarth = 50472,
		AccessFire = 50473,
		AccessIce = 50474,
		AccessGolden = 50475,
		AccessLast = 50476,
		OldDesk = 50477,
		GirlPicture = 50478,
		SilverKey = 50479,
		Phial = 50480,
		BirdCounter = 50481,
		PlantCounter = 50482,
		GoldenServantCounter = 50483,
		DiamondServantCounter = 50484,
		AccessPortals = 50485,
		AccessMachine = 50486,
		LadyTenebrisTimer = 50487,
		LadyTenebrisKilled = 50488,
		LloydTimer = 50489,
		LloydKilled = 50490,
		ThornKnightTimer = 50491,
		ThornKnightKilled = 50492,
		DragonkingTimer = 50493,
		DragonkingKilled = 50494,
		HorrorTimer = 50495,
		HorrorKilled = 50496,
		TimeGuardianTimer = 50497,
		TimeGuardianKilled = 50498,
		LastLoreTimer = 50499,
		LastLoreKilled = 50501,
		BirdCage = 50502,
		AccessLavaTeleport = 50503,
		Ivalisse = 50504,
		Chalice = 50505,
		Tomes = 50506,
		BabyDragon = 50507,
		SpiderWeb = 50508,
		LloydEvent = 50509
	},
	BigfootBurden = {
		-- Reserved storage from 50660 - 50719
		QuestLine = 50660,
		Test = 50661,
		Shooting = 50662,
		QuestLineComplete = 50663,
		MelodyTone1 = 50664,
		MelodyTone2 = 50665,
		MelodyTone3 = 50666,
		MelodyTone4 = 50667,
		MelodyTone5 = 50668,
		MelodyTone6 = 50669,
		MelodyTone7 = 50670,
		MelodyStatus = 50671,
		Rank = 50672,
		MissionCrystalKeeper = 50673,
		CrystalKeeperTimout = 50674,
		RepairedCrystalCount = 50675,
		MissionRaidersOfTheLostSpark = 50676,
		ExtractedCount = 50677,
		RaidersOfTheLostSparkTimeout = 50678,
		MissionExterminators = 50679,
		ExterminatedCount = 50680,
		ExterminatorsTimeout = 50681,
		MissionMushroomDigger = 50682,
		MushroomCount = 50683,
		MushroomDiggerTimeout = 50684,
		MissionMatchmaker = 50685,
		MatchmakerStatus = 50686,
		MatchmakerIdNeeded = 50687,
		MatchmakerTimeout = 50688,
		MissionTinkersBell = 50689,
		GolemCount = 50690,
		TinkerBellTimeout = 50691,
		MissionSporeGathering = 50692,
		SporeCount = 50693,
		SporeGatheringTimeout = 50694,
		MissionGrindstoneHunt = 50695,
		GrindstoneStatus = 50696,
		GrindstoneTimeout = 50697,
		WarzoneStatus = 50698,
		Warzone1Access = 50699,
		Warzone2Access = 50700,
		Warzone3Access = 50701,
		Warzone1Reward = 50702,
		Warzone2Reward = 50703,
		Warzone3Reward = 50704,
		BossKills = 50705,
		DoorGoldenFruits = 50706,
		BossWarzone1 = 50707,
		BossWarzone2 = 50708,
		BossWarzone3 = 50709,
		GnomedixMsg = 50710
	},
	OutfitQuest = {
		-- Reserved storage from 50960 - 51039
		-- Until all outfit quests are completed
		DefaultStart = 50960,
		Ref = 50961,
		-- Golden Outfit
		GoldenOutfit = 51015
	},
	TheRookieGuard = {
		--Reserved storage 52360 - 52395
		Mission05 = 52365
	},
	AdventurersGuild = {
		-- Reserved storage from 52130 - 52159
		Stone = 52130
	},
	Dawnport = {
		-- Reserved storage from 52250 - 52289
		-- Reward items storages
		SorcererHealthPotion = 52251,
		SorcererManaPotion = 52252,
		SorcererLightestMissile = 52253,
		SorcererLightStoneShower = 52254,
		SorcererMeat = 52255,
		DruidHealthPotion = 52256,
		DruidManaPotion = 52257,
		DruidLightestMissile = 52258,
		DruidLightStoneShower = 52259,
		DruidMeat = 52260,
		PaladinHealthPotion = 52261,
		PaladinManaPotion = 52262,
		PaladinLightestMissile = 52263,
		PaladinLightStoneShower = 52264,
		PaladinMeat = 52265,
		KnightHealthPotion = 52266,
		KnightManaPotion = 52267,
		KnightMeat = 52268,
		Sorcerer = 52269,
		Druid = 52270,
		Paladin = 52271,
		Knight = 52272,
		DoorVocation = 52273,
		DoorVocationFinish = 52274,
		ChestRoomFinish = 52275,
		Tutorial = 52276,
		MessageStair = 52277,
		Lever = 52278,
		Mainland = 52279
	},
	GraveDanger = {
		-- Reserved storage from 52310 - 52339
		Questline = 52310,
		CobraBastion = {
			Questline = 52311,
			ScarlettTimer = 52312
		}
	},
	TheOrderOfTheLion = {
		-- Reserved storage 52360-52395 (TheRookieGuard)
		-- Reserved storage 52396-52410 (TheOrderOfTheLion)
		Drume = {
			Commander = 52396, -- Global
			TotalLionCommanders = 52397, -- Global
			TotalUsurperCommanders = 52398, -- Global
			Timer = 52399
		},
	},
	-- News quest development
	-- These new structure will reserve ranges for each version. Please use accordingly.
	-- New storages
	Quest = {
		-- Start of quests per version
		-- Use the reserved storage keys accordingly
		KeysUpdate = 40000,
		U8_5 = { -- update 8.5 - Reserved Storages 42146 - 42550
			KillingInTheNameOf = {
				MonsterKillCount = {
					KillCount = 42150,
					-- Grizzly Adams
					CrocodileCount = 42151,
					BadgerCount = 42152,
					TarantulaCount = 42153,
					CarniphilasCount = 42154,
					StoneGolemCount = 42155,
					MammothCount = 42156,
					GnarlhoundCount = 42157,
					TerramiteCount = 42158,
					ApesCount = 42159,
					ThornbackTortoiseCount = 42160,
					GargoyleCount = 42161,
					IceGolemCount = 42162,
					QuaraScoutsCount = 42163,
					MutatedRatCount = 42164,
					AncientScarabCount = 42165,
					WyvernCount = 42166,
					LancerBeetleCount = 42167,
					WailingWidowCount = 42168,
					KillerCaimanCount = 42169,
					BonebeastCount = 42170,
					CrystalSpiderCount = 42171,
					MutatedTigerCount = 42172,
					UnderwaterQuarasCount = 42173,
					GiantSpiderCount = 42174,
					WerewolveCount = 42175,
					NightmareCount = 42176,
					HellspawnCount = 42177,
					HighClassLizardCount = 42178,
					StamporCount = 42179,
					BrimstoneBugCount = 42180,
					MutatedBatCount = 42181,
					HydraCount = 42182,
					SerpentSpawnCount = 42183,
					MedusaCount = 42184,
					BehemothCount = 42185,
					SeaSerpentsCount = 42186,
					HellhoundCount = 42187,
					GhastlyDragonCount = 42188,
					DrakenCount = 42189,
					DestroyerCount = 42190,
					UndeadDragonCount = 42191,
					DemonCount = 42192,
					-- Others
					GreenDjinnCount = 42193,
					BlueDjinnCount = 42194,
					PirateCount = 42195,
					MinotaurCount = 42196,
					NecromancerCount = 42197,
					TrollCount = 42198,
					GoblinCount = 42199,
					RotwormCount = 42200,
					CyclopsCount = 42201,
				},
				BossKillCount = {
					-- Grizzly Adams
					SnapperCount = 42350,
					HideCount = 42351,
					DeathbineCount = 42352,
					BloodtuskCount = 42353,
					ShardheadCount = 42354,
					EsmeraldaCount = 42355,
					FleshcrawlerCount = 42356,
					RibstrideCount = 42357,
					BloodwebCount = 42358,
					ThulCount = 42359,
					WidowCount = 42360,
					HemmingCount = 42361,
					TormentorCount = 42362,
					FlamebornCount = 42363,
					FazzrahCount = 42364,
					TromphonyteCount = 42365,
					ScuttlerCount = 42366,
					PayneCount = 42367,
					ManyCount = 42368,
					NoxiousCount = 42369,
					GorgoCount = 42370,
					StonecrackerCount = 42371,
					LeviathanCount = 42372,
					KerberosCount = 42373,
					EthershreckCount = 42374,
					PauperizerCount = 42375,
					BretzecutionerCount = 42376,
					ZanakephCount = 42377,
					TiquandasCount = 42378,
					DemodrasCount = 42379,
					-- Others
					NecropharusCount = 42380,
					FoxCount = 42381,
					PiratesCount = 42382,
					MerikhCount = 42383,
					FahimCount = 42384,
				},
				AltKillCount = {
					-- Grizzly Adams
					-- Apes
					KongraCount = 42450,
					MerlkinCount = 42451,
					SibangCount = 42452,
					-- Quara Scouts
					QuaraConstrictorScoutCount = 42453,
					QuaraHydromancerScoutCount = 42454,
					QuaramMntassinScoutCount = 42455,
					QuaraPincherScoutCount = 42456,
					QuaraPredatorScoutCount = 42457,
					-- Underwater Quara
					QuaraConstrictorCount = 42458,
					QuaraHydromancerCount = 42459,
					QuaraMantassinCount = 42460,
					QuaraPincherCount = 42461,
					QuaraPredatorCount = 42462,
					-- Nightmares
					NightmareCount = 42463,
					NightmareScionCount = 42464,
					-- High Class Lizards
					LizardChosenCount = 42465,
					LizardDragonPriestCount = 42466,
					LizardHighGuardCount = 42467,
					LizardLegionnaireCount = 42468,
					LizardZaogunCount = 42469,
					-- Sea Serpents
					SeaSerpentCount = 42470,
					YoungSeaSerpentCount = 42471,
					-- Drakens
					DrakenAbominationCount = 42472,
					DrakenEliteCount = 42473,
					DrakenSpellweaverCount = 42474,
					DrakenWarmasterCount = 42475,
					-- Others
					-- Minotaurs
					MinotaurCount = 42476,
					MinotaurGuardCount = 42477,
					MinotaurMageCount = 42478,
					MinotaurArcherCount = 42479,
					-- Necromancers and Priestesses
					NecromancerCount = 42480,
					PriestessCount = 42481,
					BloodPriestCount = 42482,
					BloodHandCount = 42483,
					ShadowPupilCount = 42484,
					-- Green Djinns or Efreets
					GreenDjinnCount = 42485,
					EfreetCount = 42486,
					-- Blue Djinns or Marids
					BlueDjinnCount = 42487,
					MaridCount = 42488,
					-- Pirates
					PirateMarauderCount = 42489,
					PirateCutthroadCount = 42490,
					PirateBuccaneerCount = 42491,
					PirateCorsairCount = 42492,
					-- Trolls
					TrollCount = 42493,
					TrollChampionCount = 42494,
					-- Goblins
					GoblinCount = 42495,
					GoblinScavengerCount = 42496,
					GoblinAssassinCount = 42497,
					-- Rotworms
					RotwormCount = 42498,
					CarrionWormnCount = 42499,
					-- Cyclops
					CyclopsCount = 42500,
					CyclopsDroneCount = 42501,
					CyclopsSmithCount = 42502
				}
			}
		},
		U10_55 = { -- update 10.55 - Reserved Storages 44751 - 44800
			Dawnport = {
				VocationReward = 20000,
				Questline = 20001,
				GoMain = 20002,
				TheLostAmulet = 20003,
				TheStolenLogBook = 20004,
				TheRareHerb = 20005,
				TheDormKey = 20006,
				StrangeAmulet = 20007,
				TornLogBook = 20008,
				HerbFlower = 20009,
				MorriskTroll = 20010,
				MorrisTrollCount = 20011,
				MorrisGoblin = 20012,
				MorrisGoblinCount = 20013,
				MorrisMinos = 20014,
				MorrisMinosCount = 20015
			}
		},
		U10_80 = { -- update 10.80 - Reserved Storages 44951 - 45200
			AsuraPalace = {},
			Cartography101 = {},
			ChakoyaIcebergMiniWorldChange = {},
			GrimvaleMineWorldChange = {},
			Grimvale = {
				BloodbackTimer = 44951,
				DarkfangTimer = 44952,
				SharpclawTimer = 44953,
				ShadowpeltTimer = 44954,
				BlackVixenTimer = 44955,
				AncientFeudDoors = 44956,
				AncientFeudShortcut = 44957,
				YirkasTimer = 44958,
				SrezzTimer = 44959,
				UtuaTimer = 44960,
				KatexTimer = 44961,
			},
			HiveOutpostMiniWorldChange = {},
			JungleCampMiniWorldChange = {},
			NightmareIslesMiniWorldChange = {},
			NightmareTeddy = {},
			PoacherCavesMiniWorldChange = {},
			TheGreatDragonHunt = {},
			TheLostBrother = {},
			TheTaintedSouls = {},
		},
		U12_00 = { -- update 12.00 - Reserved Storages 46301 - 46600
			TheDreamCourts = {
				FacelessBaneTime = 50283
			},
		},
		U12_20 = { -- update 12.20 - Reserved Storages 46851 - 47000
			GraveDanger = {
				QuestLine = 46851,
				Graves = {
					Edron = 46852,
					DarkCathedral = 46853,
					Ghostlands = 46854,
					Cormaya = 46855,
					FemorHills = 46856,
					Ankrahmun = 46857,
					Kilmaresh = 46858,
					Vengoth = 46859,
					Darashia = 46860,
					Thais = 46861,
					Orclands = 46862,
					IceIslands = 46863
				},
				Bosses = {
					BaelocNictrosTimer = 46865,
					BaelocNictrosKilled = 46866,
					CountVlarkorthTimer = 46867,
					CountVlarkorthKilled = 46868,
					DukeKruleTimer = 46869,
					DukeKruleKilled = 46870,
					EarlOsamTimer = 46871,
					EarlOsamKilled = 46872,
					LordAzaramTimer = 46873,
					LordAzaramKilled = 46874,
					KingZelosDoor = 46875,
					KingZelosTimer = 46876,
					KingZelosKilled = 46877,
					InquisitionOutfitReceived = 46878,
				},
				Cobra = 46864
			},
			HandOfTheInquisitionOutfits = {},
			-- Kilmaresh = {}, done earlier in the file
		},
		U12_30 = { -- update 12.30 - Reserved Storages 47001 - 47200
			FalconerOutfits = {},
			FeasterOfSouls = {
				IrgixTimer = 47005,
				IrgixKilled = 47006,
				UnazTimer = 47007,
				UnazKilled = 47008,
				VokTimer = 47009,
				VokKilled = 47010,
				FearFeasterTimer = 47011,
				FearFeasterKilled = 47012,
				DreadMaidenTimer = 47013,
				DreadMaidenKilled = 47014,
				UnwelcomeTimer = 47015,
				UnwelcomeKilled = 47016,
				PaleWormEntrance = 47017,
				PaleWormTimer = 47018,
				PaleWormKilled = 47019,
			},
			PoltergeistOutfits = {
				Received = 47020,
			}
		},
		U12_40 = { -- update 12.40 - Reserved Storages 47201 - 47500
			TheOrderOfTheLion = {
				QuestLine = 47401,
				AccessEastSide = 47402,
				AccessSouthernSide = 47403
			},
			SoulWar = {
				GoshnarMaliceTimer = 47210,
				GoshnarMaliceKilled = 47211,
				GoshnarHatredTimer = 47212,
				GoshnarHatredKilled = 47213,
				GoshnarSpiteTimer = 47214,
				GoshnarSpiteKilled = 47215,
				GoshnarCrueltyTimer = 47216,
				GoshnarCrueltyKilled = 47217,
				GoshnarGreedTimer = 47218,
				GoshnarGreedKilled = 47219,
				GoshnarMegalomaniaAccess = 47220,
				GoshnarMegalomaniaTimer = 47221,
				GoshnarMegalomaniaKilled = 47222,
				QuestReward = 47223,
			},
		},
		U12_60 = { -- update 12.60 - Reserved Storages 47501 - 47600
			APiratesTail = {
				QuestLine = 47501,
				RascacoonShortcut = 47512,
				TentuglyKilled = 47513,
				TentuglyDoor = 47514,
				TentuglyTimer = 47515,
				RatmiralTimer = 47516,
			},
		},
		U12_70 = { -- update 12.70 - Reserved Storages 47601 - 47800
			AdventuresOfGalthen = {
				AccessDoor = 47601,
				MegasylvanYseldaTimer = 47602,
			},
		},
		U12_90 = { -- update 12.90 - Reserved Storages 47851 - 47900
			PrimalOrdeal = {
				QuestLine = 47851,
				Hazard = {
					Current = 47856,
					Max = 47857,
				},
				Bosses = {
					MagmaBubbleTimer = 47852,
					MagmaBubbleKilled = 47853,
					ThePrimalMenaceTimer = 47854,
					ThePrimalMenaceKilled = 47855,
				},
			},
		},
	},

	-- Reserved storage from 64000 - 64099
	TibiaDrome = {
		-- General Upgrades
		BestiaryBetterment = {
			TimeLeft = 64000,
			LastActivatedAt = 64001,
		},
		CharmUpgrade = {
			TimeLeft = 64002,
			LastActivatedAt = 64003,
		},
		KooldownAid = {
			LastActivatedAt = 64005,
		},
		StaminaExtension = {
			LastActivatedAt = 64007,
		},
		StrikeEnhancement = {
			TimeLeft = 64008,
			LastActivatedAt = 64009,
		},
		WealthDuplex = {
			TimeLeft = 64010,
			LastActivatedAt = 64011,
		},
		-- Resilience
		FireResilience = {
			TimeLeft = 64012,
			LastActivatedAt = 64013,
		},
		IceResilience = {
			TimeLeft = 64014,
			LastActivatedAt = 64015,
		},
		EarthResilience = {
			TimeLeft = 64016,
			LastActivatedAt = 64017,
		},
		EnergyResilience = {
			TimeLeft = 64018,
			LastActivatedAt = 64019,
		},
		HolyResilience = {
			TimeLeft = 64020,
			LastActivatedAt = 64021,
		},
		DeathResilience = {
			TimeLeft = 64022,
			LastActivatedAt = 64023,
		},
		PhysicalResilience = {
			TimeLeft = 64024,
			LastActivatedAt = 64025,
		},
		-- Amplifications
		FireAmplification = {
			TimeLeft = 64026,
			LastActivatedAt = 64027,
		},
		IceAmplification = {
			TimeLeft = 64028,
			LastActivatedAt = 64029,
		},
		EarthAmplification = {
			TimeLeft = 64030,
			LastActivatedAt = 64031,
		},
		EnergyAmplification = {
			TimeLeft = 64032,
			LastActivatedAt = 64033,
		},
		HolyAmplification = {
			TimeLeft = 64034,
			LastActivatedAt = 64035,
		},
		DeathAmplification = {
			TimeLeft = 64036,
			LastActivatedAt = 64037,
		},
		PhysicalAmplification = {
			TimeLeft = 64038,
			LastActivatedAt = 64039,
		},
	},

	VipSystem = {
		IsVip = 150001,
		OnlineCoinsGain = 150002,
		OnlineTokensGain = 150003,
	},
}

GlobalStorage = {
	ExpBoost = 65004,
	CobraBastionFlask = 65012,
	UglyMonster = 65017,
	KeysUpdate = 40000, -- Reserved storage from 40000 - 40000
}

startupGlobalStorages = {
	-- 
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
		if extraction[i] == extraction[i + 1] then
			Spdlog.warn(string.format("Duplicate storage value found: %d",
				extraction[i]))
		end
	end
end
