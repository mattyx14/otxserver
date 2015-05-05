////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#ifndef __NPC__
#define __NPC__
#include "templates.h"
#include "const.h"

#include "creature.h"
#include "luascript.h"

class Npc;
struct NpcType
{
	std::string name, file, nameDescription, script;
	Outfit_t outfit;
};

class Npcs
{
	public:
		Npcs() {}
		virtual ~Npcs();
		void reload();

		bool loadFromXml(bool reloading = false);
		bool parseNpcNode(xmlNodePtr node, FileType_t path, bool reloading = false);

		NpcType* getType(const std::string& name) const;
		bool setType(std::string name, NpcType* nType);

	private:
		typedef std::map<std::string, NpcType*> DataMap;
		DataMap data;
};

struct NpcState;
class NpcScript : public LuaInterface
{
	public:
		NpcScript();
		virtual ~NpcScript() {}

		static void pushState(lua_State* L, NpcState* state);
		static void popState(lua_State* L, NpcState* &state);

	protected:
		virtual void registerFunctions();

		static int32_t luaActionFocus(lua_State* L);
		static int32_t luaActionSay(lua_State* L);
		static int32_t luaActionFollow(lua_State* L);

		static int32_t luaGetNpcId(lua_State* L);
		static int32_t luaGetNpcParameter(lua_State* L);

		static int32_t luaGetNpcState(lua_State* L);
		static int32_t luaSetNpcState(lua_State* L);

		static int32_t luaOpenShopWindow(lua_State* L);
		static int32_t luaCloseShopWindow(lua_State* L);
		static int32_t luaGetShopOwner(lua_State* L);
};

class Player;
class NpcEvents
{
	public:
		NpcEvents(std::string file, Npc* npc);
		virtual ~NpcEvents() {}

		virtual void onCreatureAppear(const Creature* creature);
		virtual void onCreatureDisappear(const Creature* creature);

		virtual void onCreatureMove(const Creature* creature, const Position& oldPos, const Position& newPos);
		virtual void onCreatureSay(const Creature* creature, MessageClasses, const std::string& text, Position* pos = NULL);

		virtual void onPlayerTrade(const Player* player, int32_t callback, uint16_t itemid,
			uint8_t count, uint8_t amount, bool ignore, bool inBackpacks);
		virtual void onPlayerEndTrade(const Player* player);
		virtual void onPlayerCloseChannel(const Player* player);

		virtual void onThink();
		bool isLoaded() const {return m_loaded;}

	private:
		Npc* m_npc;
		bool m_loaded;
		NpcScript* m_interface;
		int32_t m_onCreatureAppear, m_onCreatureDisappear, m_onCreatureMove, m_onCreatureSay,
			m_onPlayerCloseChannel, m_onPlayerEndTrade, m_onThink;
};

enum InteractType_t
{
	INTERACT_TEXT,
	INTERACT_EVENT
};

enum NpcEvent_t
{
	EVENT_NONE,
	EVENT_THINK,
	EVENT_BUSY,
	EVENT_IDLE,
	EVENT_PLAYER_ENTER,
	EVENT_PLAYER_MOVE,
	EVENT_PLAYER_LEAVE,
	EVENT_PLAYER_SHOPSELL,
	EVENT_PLAYER_SHOPBUY,
	EVENT_PLAYER_SHOPCLOSE,
	EVENT_PLAYER_CHATCLOSE
};

enum ResponseType_t
{
	RESPONSE_DEFAULT,
	RESPONSE_SCRIPT,
};

enum RespondParam_t
{
	RESPOND_DEFAULT = 0,
	RESPOND_MALE = 1 << 0,
	RESPOND_FEMALE = 1 << 1,
	RESPOND_PZBLOCK = 1 << 2,
	RESPOND_LOWMONEY = 1 << 3,
	RESPOND_NOAMOUNT = 1 << 4,
	RESPOND_LOWAMOUNT = 1 << 5,
	RESPOND_PREMIUM = 1 << 6,
	RESPOND_PROMOTED = 1 << 7,
	RESPOND_DRUID = 1 << 8,
	RESPOND_KNIGHT = 1 << 9,
	RESPOND_PALADIN = 1 << 10,
	RESPOND_SORCERER = 1 << 11,
	RESPOND_LOWLEVEL = 1 << 12
};

enum ReponseActionParam_t
{
	ACTION_NONE,
	ACTION_SETTOPIC,
	ACTION_SETLEVEL,
	ACTION_SETPRICE,
	ACTION_SETBUYPRICE,
	ACTION_SETSELLPRICE,
	ACTION_TAKEMONEY,
	ACTION_GIVEMONEY,
	ACTION_SELLITEM,
	ACTION_BUYITEM,
	ACTION_GIVEITEM,
	ACTION_TAKEITEM,
	ACTION_SETAMOUNT,
	ACTION_SETITEM,
	ACTION_SETSUBTYPE,
	ACTION_SETEFFECT,
	ACTION_SETSPELL,
	ACTION_SETLISTNAME,
	ACTION_SETLISTPNAME,
	ACTION_TEACHSPELL,
	ACTION_UNTEACHSPELL,
	ACTION_SETSTORAGE,
	ACTION_SETTELEPORT,
	ACTION_SCRIPT,
	ACTION_SCRIPTPARAM,
	ACTION_ADDQUEUE,
	ACTION_SETIDLE
};

enum ShopEvent_t
{
	SHOPEVENT_SELL,
	SHOPEVENT_BUY,
	SHOPEVENT_CLOSE
};

enum StorageComparision_t
{
	STORAGE_LESS,
	STORAGE_LESSOREQUAL,
	STORAGE_EQUAL,
	STORAGE_NOTEQUAL,
	STORAGE_GREATEROREQUAL,
	STORAGE_GREATER
};

struct ResponseAction
{
	public:
		ResponseAction()
		{
			actionType = ACTION_NONE;
			intValue = 0;
			key = "";
			strValue = "";
			pos = Position();
		}

		ReponseActionParam_t actionType;
		int32_t intValue;
		std::string key, strValue;
		Position pos;
};

struct ListItem
{
	ListItem()
	{
		itemId = 0;
		subType = sellPrice = buyPrice = -1;
		keywords = name = pluralName = "";
	}

	int32_t sellPrice, buyPrice, itemId, subType;
	std::string keywords, name, pluralName;
};

struct ScriptVars
{
	ScriptVars()
	{
		b1 = b2 = b3 = false;
		n1 = n2 = n3 = -1;
		s1 = s2 = s3 = "";
	}

	bool b1, b2, b3;
	int32_t n1, n2, n3;
	std::string s1, s2, s3;
};

typedef std::list<ResponseAction> ActionList;

class NpcResponse;
typedef std::list<NpcResponse*> ResponseList;

typedef std::map<std::string, int32_t> ResponseScriptMap;
class NpcResponse
{
	public:
		struct ResponseProperties
		{
			ResponseProperties()
			{
				topic = amount = focusStatus = -1;
				interactType = INTERACT_TEXT;
				responseType = RESPONSE_DEFAULT;
				params = 0;
				storageComp = STORAGE_EQUAL;
				publicize = true;
			}

			bool publicize;
			InteractType_t interactType;
			ResponseType_t responseType;
			StorageComparision_t storageComp;
			int32_t topic, amount, focusStatus;
			uint32_t params;
			std::string output, knowSpell, storageId, storageValue;
			ActionList actionList;
			std::list<std::string> inputList;
			std::list<ListItem> itemList;
		};

		NpcResponse(const ResponseProperties& _prop,
			ResponseList _subResponseList,
			ScriptVars _scriptVars)
		{
			prop = _prop;
			subResponseList = _subResponseList;
			scriptVars = _scriptVars;
		}

		NpcResponse(NpcResponse& rhs)
		{
			prop = rhs.prop;
			scriptVars = rhs.scriptVars;
			for(ResponseList::iterator it = rhs.subResponseList.begin(); it != rhs.subResponseList.end(); ++it)
			{
				NpcResponse* response = new NpcResponse(*(*it));
				subResponseList.push_back(response);
			}
		}

		virtual ~NpcResponse()
		{
			for(ResponseList::iterator it = subResponseList.begin(); it != subResponseList.end(); ++it)
				delete *it;

			subResponseList.clear();
		}

		uint32_t getParams() const {return prop.params;}
		std::string getInputText() const {return (prop.inputList.empty() ? "" : *prop.inputList.begin());}
		int32_t getTopic() const {return prop.topic;}
		int32_t getFocusState() const {return prop.focusStatus;}
		std::string getStorageId() const {return prop.storageId;}
		std::string getStorage() const {return prop.storageValue;}
		ResponseType_t getResponseType() const {return prop.responseType;}
		InteractType_t getInteractType() const {return prop.interactType;}
		StorageComparision_t getStorageComp() const {return prop.storageComp;}
		const std::string& getKnowSpell() const {return prop.knowSpell;}
		const std::string& getText() const {return prop.output;}
		int32_t getAmount() const {return prop.amount;}
		void setAmount(int32_t _amount) {prop.amount = _amount;}
		bool publicize() const {return prop.publicize;}

		std::string formatResponseString(Creature* creature) const;
		void addAction(ResponseAction action) {prop.actionList.push_back(action);}
		const std::list<std::string>& getInputList() const {return prop.inputList;}

		void setResponseList(ResponseList _list) {subResponseList.insert(subResponseList.end(),_list.begin(),_list.end());}
		const ResponseList& getResponseList() const {return subResponseList;}

		ActionList::const_iterator getFirstAction() const {return prop.actionList.begin();}
		ActionList::const_iterator getEndAction() const {return prop.actionList.end();}

		ResponseProperties prop;
		ResponseList subResponseList;
		ScriptVars scriptVars;
};

struct NpcState
{
	bool isIdle, isQueued, ignore, inBackpacks;
	int32_t topic, price, sellPrice, buyPrice, amount, itemId, subType, level;
	uint32_t respondToCreature;
	uint64_t prevInteraction;
	std::string spellName, listName, listPluralName, respondToText, prevRespondToText;
	const NpcResponse* lastResponse;
	ScriptVars scriptVars;
	//Do not forget to update pushState/popState if you add more variables
};

struct Voice
{
	bool randomSpectator;
	MessageClasses type;
	uint32_t interval, margin;
	std::string text;
};

class Npc : public Creature
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t npcCount;
#endif
		virtual ~Npc();

		static Npc* createNpc(NpcType* nType);
		static Npc* createNpc(const std::string& name);

		virtual Npc* getNpc() {return this;}
		virtual const Npc* getNpc() const {return this;}
		virtual CreatureType_t getType() const {return CREATURETYPE_NPC;}

		virtual uint32_t rangeId() {return NPC_ID_RANGE;}
		static AutoList<Npc> autoList;

		void addList() {autoList[id] = this;}
		void removeList() {autoList.erase(id);}

		virtual bool isPushable() const {return false;}
		virtual bool isAttackable() const {return attackable;}
		virtual bool isWalkable() const {return walkable;}

		virtual bool canSee(const Position& pos) const;
		virtual bool canSeeInvisibility() const {return true;}

		bool isLoaded() {return loaded;}
		bool load();
		void reload();

		void setNpcPath(const std::string& _name, bool fromXmlFile = false);

		virtual const std::string& getName() const {return nType->name;}
		virtual const std::string& getNameDescription() const {return nType->nameDescription;}

		void doSay(const std::string& text, MessageClasses type, Player* player);

		void onPlayerTrade(Player* player, ShopEvent_t type, int32_t callback, uint16_t itemId, uint8_t count,
			uint8_t amount, bool ignore = false, bool inBackpacks = false);
		void onPlayerEndTrade(Player* player, int32_t buyCallback,
			int32_t sellCallback);
		void onPlayerCloseChannel(const Player* player);

		void setCreatureFocus(Creature* creature);
		NpcScript* getInterface();

	protected:
		Npc(NpcType* _nType);
		NpcType* nType;
		bool loaded;

		void reset();
		bool loadFromXml();

		virtual void onCreatureAppear(const Creature* creature);
		virtual void onCreatureDisappear(const Creature* creature, bool isLogout);
		virtual void onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
			const Tile* oldTile, const Position& oldPos, bool teleport);
		virtual void onCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos = NULL);
		virtual void onThink(uint32_t interval);

		bool isImmune(CombatType_t) const {return true;}
		bool isImmune(ConditionType_t) const {return true;}

		virtual std::string getDescription(int32_t) const {return nType->nameDescription + ".";}
		virtual bool getNextStep(Direction& dir, uint32_t& flags);
		bool getRandomStep(Direction& dir);
		bool canWalkTo(const Position& fromPos, Direction dir);

		const NpcResponse* getResponse(const ResponseList& list, const Player* player,
			NpcState* npcState, const std::string& text, bool exactMatch = false);
		const NpcResponse* getResponse(const Player* player, NpcState* npcState, const std::string& text);
		const NpcResponse* getResponse(const Player* player, NpcEvent_t eventType);
		const NpcResponse* getResponse(const Player* player, NpcState* npcState, NpcEvent_t eventType);
		std::string getEventResponseName(NpcEvent_t eventType);

		NpcState* getState(const Player* player, bool makeNew = true);
		uint32_t getMatchCount(NpcResponse* response, StringVec wordList,
			bool exactMatch, int32_t& matchAllCount, int32_t& totalKeywordCount);
		uint32_t getListItemPrice(uint16_t itemId, ShopEvent_t type);

		std::string formatResponse(Creature* creature, const NpcState* npcState, const NpcResponse* response) const;
		void executeResponse(Player* player, NpcState* npcState, const NpcResponse* response);

		uint32_t parseParamsNode(xmlNodePtr node);
		ResponseList parseInteractionNode(xmlNodePtr node);

		void onPlayerEnter(Player* player, NpcState* state);
		void onPlayerLeave(Player* player, NpcState* state);

		void addShopPlayer(Player* player);
		void removeShopPlayer(const Player* player);
		void closeAllShopWindows();

		bool floorChange, attackable, walkable, isIdle, hasBusyReply, hasScriptedFocus, defaultPublic;
		Direction baseDirection;

		int32_t talkRadius, idleTime, idleInterval, focusCreature;
		uint32_t walkTicks;
		int64_t lastVoice;

		typedef std::map<std::string, std::list<ListItem> > ItemListMap;
		ItemListMap itemListMap;

		typedef std::map<std::string, std::string> ParametersMap;
		ParametersMap m_parameters;

		typedef std::list<Player*> ShopPlayerList;
		ShopPlayerList shopPlayerList;

		typedef std::list<NpcState*> StateList;
		StateList stateList;

		typedef std::list<uint32_t> QueueList;
		QueueList queueList;

		typedef std::list<Voice> VoiceList;
		VoiceList voiceList;

		ResponseScriptMap responseScriptMap;
		ResponseList responseList;

		NpcEvents* m_npcEventHandler;
		static NpcScript* m_interface;

		friend class Npcs;
		friend class NpcScript;
};
#endif
