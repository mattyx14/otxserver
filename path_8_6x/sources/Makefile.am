noinst_PROGRAMS = theotxserver

CXXFLAGS = -pipe -Ofast -march=native -DNDEBUG
AM_CXXFLAGS = $(XML_CPPFLAGS) $(OTSERV_FLAGS) $(LUA_CFLAGS) $(DEBUG_FLAGS)\
$(MYSQL_FLAGS) $(MYSQLPP_FLAGS) $(SQLITE_FLAGS) $(PGSQL_FLAGS) $(PROFILER_FLAGS)\
$(OPTIONAL_FLAGS) -D_THREAD_SAFE -D_REENTRANT -Wall -Wextra -Wno-strict-aliasing -Wno-unused-parameter -Wno-array-bounds -pthread

theotxserver_LDADD = $(LUA_LIBS)

if USE_MYSQL
MAYBE_MYSQL = databasemysql.cpp databasemysql.h
endif
if USE_MYSQLPP
MAYBE_MYSQLPP = databasemysqlpp.cpp databasemysqlpp.h
endif
if USE_SQLITE
MAYBE_SQLITE = databasesqlite.cpp databasesqlite.h
endif
if USE_PGSQL
MAYBE_PGSQL = databasepgsql.cpp databasepgsql.h
endif
if LOGIN_SERVER
MAYBE_LOGIN = gameservers.cpp gameservers.h
endif
if OT_ADMIN
MAYBE_OTADMIN = admin.cpp admin.h
endif

theotxserver_SOURCES = account.h actions.cpp actions.h $(MAYBE_OTADMIN) \
	allocator.cpp allocator.h baseevents.cpp baseevents.h beds.cpp \
	beds.h chat.cpp chat.h combat.cpp combat.h condition.cpp condition.h \
	config.h configmanager.cpp configmanager.h connection.cpp connection.h \
	const.h container.cpp container.h creature.cpp creature.h \
	creatureevent.cpp creatureevent.h cylinder.cpp cylinder.h database.cpp \
	database.h databasemanager.cpp databasemanager.h $(MAYBE_MYSQL) \
	$(MAYBE_MYSQLPP) $(MAYBE_SQLITE) $(MAYBE_PGSQL) depot.cpp depot.h dispatcher.cpp \
	dispatcher.h exception.cpp exception.h fileloader.cpp fileloader.h \
	game.cpp game.h $(MAYBE_LOGIN) globalevent.cpp globalevent.h group.cpp \
	group.h gui.cpp gui.h house.cpp house.h housetile.cpp housetile.h inputbox.cpp inputbox.h ioban.cpp ioban.h \
	ioguild.cpp ioguild.h iologindata.cpp iologindata.h iomap.cpp iomap.h \
	iomapserialize.cpp iomapserialize.h item.cpp item.h itemattributes.cpp \
	itemattributes.h items.cpp items.h luascript.cpp luascript.h \
	mailbox.cpp mailbox.h manager.cpp manager.h map.cpp map.h monster.cpp \
	monster.h monsters.cpp monsters.h movement.cpp movement.h \
	networkmessage.cpp networkmessage.h npc.cpp npc.h otpch.cpp otpch.h otserv.cpp \
	otsystem.h outfit.cpp outfit.h outputmessage.cpp outputmessage.h \
	party.cpp party.h playerbox.cpp playerbox.h player.cpp player.h position.cpp position.h \
	protocol.cpp protocol.h protocolgame.cpp protocolgame.h \
	protocolhttp.cpp protocolhttp.h protocollogin.cpp protocollogin.h \
	protocolold.cpp protocolold.h quests.cpp quests.h raids.cpp raids.h \
	resources.h scheduler.cpp scheduler.h scriptmanager.cpp \
	scriptmanager.h server.cpp server.h spawn.cpp spawn.h spectators.cpp spectators.h \
	spells.cpp spells.h status.cpp status.h talkaction.cpp talkaction.h teleport.cpp \
	teleport.h templates.h textlogger.cpp textlogger.h thing.cpp thing.h \
	tile.cpp tile.h tools.cpp tools.h town.h trashholder.cpp trashholder.h \
	waitlist.cpp waitlist.h waypoints.h weapons.cpp weapons.h vocation.cpp \
	vocation.h
