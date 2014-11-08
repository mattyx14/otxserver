local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid) 			end
function onCreatureDisappear(cid) 			npcHandler:onCreatureDisappear(cid) 		end
function onCreatureSay(cid, type, msg) 		npcHandler:onCreatureSay(cid, type, msg) 	end
function onThink() 							npcHandler:onThink() 						end
function onPlayerEndTrade(cid)				npcHandler:onPlayerEndTrade(cid)			end
function onPlayerCloseChannel(cid)			npcHandler:onPlayerCloseChannel(cid)		end

-- Don't forget npcHandler = npcHandler in the parameters. It is required for all StdModule functions!
keywordHandler:addKeyword({'helmets'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy royal (40k), warrior (6k), crusader (9k), crown (5k), devil (4k), chain (35gp) and iron helmets (30gp), also mystic turbans (500gp).'})
keywordHandler:addKeyword({'boots'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy golden boots (100k), steel boots (40k) and boots of haste (40k).'})
keywordHandler:addKeyword({'armors'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy golden (30k), crown (20k), knight (5k), lady (7,5k), plate (400gp), brass (200gp) and chain armors (100gp), also mpa (100k), dsm (60k) and blue robes (15k).'})
keywordHandler:addKeyword({'legs'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy golden (80k), crown (15k), knight (6k), plate (500gp) and brass legs (100gp).'})
keywordHandler:addKeyword({'shields'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy blessed (150k), great (100k), demon (40k), vampire (25k), medusa (8k), amazon (4k), crown (5k), tower (4k), dragon (3k), guardian (2k), beholder (1k), and dwarven shields (100gp), also mms (80k).'})
keywordHandler:addKeyword({'swords'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy giant (10k), bright (6k), fire (3k) serpent (1.5k), spike (800gp) and two handed swords (400gp), also ice rapiers (4k), magic longswords (150k), magic swords (90k), warlord swords (100k) broad swords (70gp), short swords (30gp), sabres (25gp) and swords (25gp).'})
keywordHandler:addKeyword({'axes'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy fire (10k), guardian halberds (7,5k) knight (2k), double (200gp) and battle axes (100gp), also dragon lances (10k), stonecutters axes (90k), halberds (200gp) and hatchets (20gp).'})
keywordHandler:addKeyword({'clubs'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I buy thunder hammers (90k), war (6k), dragon (2k) and battle hammers (60gp), also skull staffs (10k) and clerical maces (200gp).'})

npcHandler:addModule(FocusModule:new())
