BIN = theotxserver

#-- If you installed the required libs on a different folder, change it here --#
INCLUDE = 	-I"/usr/include/libxml2"				-I"/usr/include/lua5.1"				-I"/usr/include/mysql" \
				-I"/usr/include/sqlite3" 				-I"/usr/include/boost"

#-- Type of the compiler, clang is also an option--#
CXX = g++

#-- Folder in which the compiled files '.o' will be stored --#
OBJDIR = objects


CXXOBJECTS = $(CXXSOURCES:%.cpp=$(OBJDIR)/%.o)

#-- FLAGS used in the source --#
FLAGS = -D_REENTRANT -DBOOST_DISABLE_ASSERTS -DNDEBUG -D__USE_MYSQL__ -D__ROOT_PERMISSION__


CXXFLAGS =	-pipe								-march=native				-mtune=native \
					$(INCLUDE) 					$(FLAGS)					-Wall \
					-Wno-maybe-uninitialized 	-Ofast 						-std=c++11 \
					-pthread 						-fno-strict-aliasing


#-- You may need to change the path of 'libtcmalloc_minimal' to the correct one for your distribution --#
#-- For Ubuntu 20 on WSL2 the path is /usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 --#
LDFLAGS =		/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 \
					-llua5.1				-lxml2						-lboost_thread \
					-lboost_system 	-lboost_filesystem		-lgmp \
					-lmysqlclient		-lboost_regex				-lcrypto \
					-lsqlite3				-g

# SQLITE: databasesqlite.cpp	MYSQL: databasemysql.cpp
# MYSQLPP: databasemysqlpp.cpp		PGSQL: databasepgsql.cpp
DATABASE = databasemysql.cpp


# For LOGIN_SERVER, add gameservers.cpp gameservers.h
# For OT_ADMIN, add admin.cpp admin.h
EXTRASOURCES = 

CXXSOURCES = 	actions.cpp				allocator.cpp			baseevents.cpp			beds.cpp \
						chat.cpp  				combat.cpp  			condition.cpp 				configmanager.cpp \
						connection.cpp		container.cpp			creature.cpp  				creatureevent.cpp\
						cylinder.cpp  			database.cpp 			databasemanager.cpp 	$(DATABASE) \
						depot.cpp 				dispatcher.cpp 		exception.cpp  			fileloader.cpp  \
						game.cpp  				$(EXTRASOURCES) 	globalevent.cpp  			group.cpp \
						house.cpp  			housetile.cpp  		ioban.cpp					ioguild.cpp	sha1.cpp \
						iologindata.cpp  		iomap.cpp 				iomapserialize.cpp 		item.cpp \
						itemattributes.cpp 	items.cpp  				luascript.cpp				mailbox.cpp \
						manager.cpp 			map.cpp					monster.cpp 				monsters.cpp \
						movement.cpp  		networkmessage.cpp npc.cpp  					otpch.cpp \
						otserv.cpp 				outfit.cpp 				outputmessage.cpp  	party.cpp \
						player.cpp 				position.cpp			protocol.cpp				protocolgame.cpp  \
						protocolhttp.cpp  	protocollogin.cpp		protocolold.cpp  			quests.cpp \
						raids.cpp  				rsa.cpp  				scheduler.cpp  			scriptmanager.cpp \
						server.cpp  			spawn.cpp 				spectators.cpp			spells.cpp \
						status.cpp 				talkaction.cpp			teleport.cpp 				textlogger.cpp \
						thing.cpp 				tile.cpp 					tools.cpp 					trashholder.cpp \
						waitlist.cpp 			weapons.cpp 			vocation.cpp


all: $(BIN)

clean:
	$(RM) $(CXXOBJECTS) $(BIN)

$(BIN): $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@echo [CC] $@
	@$(CXX) -c $(CXXFLAGS) -o $@ $<