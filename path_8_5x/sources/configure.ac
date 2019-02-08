AC_PREREQ([2.50])
AC_INIT([OTXServer], [2.X.Series])
AM_INIT_AUTOMAKE([1.10 foreign silent-rules])
AC_CONFIG_SRCDIR([account.h])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CXX
AC_LANG(C++)

# Config for 32Bits
#OPTIONAL_FLAGS="-Ofast -march=native -DNDEBUG"

# Config for 64Bits
#OPTIONAL_FLAGS="-Ofast -march=core2 -DNDEBUG"

# Config for 32Bits
DEBUG_FLAGS="-Ofast -march=native -fomit-frame-pointer -DNDEBUG"

# Config for 64Bits
#DEBUG_FLAGS="-Ofast -march=core2 -fomit-frame-pointer -DNDEBUG"

# check if we want server diag enabled
serverDiag=no
AC_ARG_ENABLE(server-diag, [  --enable-server-diag		enable server diagnostic],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__ENABLE_SERVER_DIAGNOSTIC__"]
	serverDiag=yes
)

# check if we want login server mode
AM_CONDITIONAL(LOGIN_SERVER, false)
loginServer=no
AC_ARG_ENABLE(login-server, [  --enable-login-server		enable login server mode],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__LOGIN_SERVER__"]
	AM_CONDITIONAL(LOGIN_SERVER, true)
	loginServer=yes
)

# check if we want OTAdmin protocol
AM_CONDITIONAL(OT_ADMIN, false)
otAdmin=no
AC_ARG_ENABLE(ot-admin, [  --enable-ot-admin		enable OTAdmin protocol],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__OTADMIN__"]
	AM_CONDITIONAL(OT_ADMIN, true)
	otAdmin=yes
)

# check if we want otserv allocator enabled
otservAllocator=no
AC_ARG_ENABLE(otserv-allocator, [  --enable-otserv-allocator	enable otserv custom allocator],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__OTSERV_ALLOCATOR__"]
	otservAllocator=yes
)

# check if we want homedir conf enabled
homedirConf=no
AC_ARG_ENABLE(homedir-conf, [  --enable-homedir-conf		enable home directory configuration],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__HOMEDIR_CONF__"]
	homedirConf=yes
)

# check do we want to allow running on root user
rootPermission=no
AC_ARG_ENABLE(root-permission, [  --enable-root-permission	enable running on root user],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__ROOT_PERMISSION__"]
	rootPermission=yes
)

# check if we want to use luajit instead of lua
luaJIT=no
AC_ARG_ENABLE(luajit, [  --enable-luajit		enable luajit instead of lua],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__LUAJIT__"]
	luaJIT=yes
)

# check if we want to cache ground items
groundCache=no
AC_ARG_ENABLE(groundcache, [  --enable-groundcache		enable caching of ground items],
	[OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__GROUND_CACHE__"]
	groundCache=yes
)

# check if we want a debug build
debugBuild=no
AC_ARG_ENABLE(debug, [  --enable-debug		enable debuging],
	[DEBUG_FLAGS="-D__DEBUG__ -D__DEBUG_MOVESYS__ -D__DEBUG_CHAT__ -D__DEBUG_HOUSES__ -D__DEBUG_LUASCRIPTS__ -D__DEBUG_MAILBOX__ -D__DEBUG_NET__ -D__DEBUG_NET_DETAIL__ -D__DEBUG_RAID__ -D__DEBUG_SCHEDULER__ -D__DEBUG_SPAWN__ -D__SQL_QUERY_DEBUG__ -O0 -g3"]
	debugBuild=yes
)
AC_SUBST(DEBUG_FLAGS)

AM_CONDITIONAL(USE_MYSQL, false)
useMySQL=no
AM_CONDITIONAL(USE_MYSQLPP, false)
useMySQLpp=no
AM_CONDITIONAL(USE_PGSQL, false)
usePostgreSQL=no
AM_CONDITIONAL(USE_SQLITE, false)
useSQLite=no

# check if we want mysql enabled
AC_ARG_ENABLE(mysql, [  --enable-mysql		enable MySQL support], [
	AM_CONDITIONAL(USE_MYSQL, true)
	MYSQL_FLAGS=-D__USE_MYSQL__
	AC_SUBST(MYSQL_FLAGS)
	useMySQL=yes
])

# check if we want mysql++ enabled
AC_ARG_ENABLE(mysqlpp, [  --enable-mysqlpp		enable MySQL++Connector support], [
	AM_CONDITIONAL(USE_MYSQLPP, true)
	MYSQLPP_FLAGS=-D__USE_MYSQLPP__
	AC_SUBST(MYSQLPP_FLAGS)
	useMySQLpp=yes
])

# check if we want postgresql enabled
AC_ARG_ENABLE(pgsql, [  --enable-pgsql		enable PostgreSQL support], [
	AM_CONDITIONAL(USE_PGSQL, true)
	PGSQL_FLAGS="-D__USE_PGSQL__ -I`pg_config --includedir`"
	AC_SUBST(PGSQL_FLAGS)
	usePostgreSQL=yes
])

# check if we want sqlite enabled
AC_ARG_ENABLE(sqlite, [  --enable-sqlite		enable SQLite support], [
	AM_CONDITIONAL(USE_SQLITE, true)
	SQLITE_FLAGS=-D__USE_SQLITE__
	AC_SUBST(SQLITE_FLAGS)
	useSQLite=yes
])

# check if we want the profiler
AC_ARG_ENABLE(profiler, [  --enable-profiler		enable profiler support], [PROFILER_FLAGS=-pg])
AC_SUBST(PROFILER_FLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h sys/socket.h sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ceil floor ftime gethostbyname gethostname memset pow sqrt strcasecmp strncasecmp strstr strtol])

# check for xml2
AM_PATH_XML2(2.6.5, , AC_MSG_ERROR([You need libxml2 >= 2.6.5 to compile otxserver!]))
LIBS="$LIBS $XML_LIBS"

# check for zlib
AC_CHECK_HEADERS([zlib.h], ,[AC_MSG_ERROR("zlib header not found.")])
AC_CHECK_LIB(z, main, ,[AC_MSG_ERROR("Linking against zlib library failed.")])

# check for pthread
AC_CHECK_HEADERS([pthread.h], ,[AC_MSG_ERROR("pthread header not found.")])
AC_CHECK_LIB(pthread, main, ,[AC_MSG_ERROR("Linking against pthread library failed)])

# check for OpenSSL
AC_CHECK_HEADERS([openssl/rsa.h openssl/bn.h openssl/err.h openssl/sha.h openssl/md5.h], , [AC_MSG_ERROR("Required OpenSSL headers not found.")])
AC_CHECK_LIB(crypto, main, , [AC_MSG_ERROR("Linking against OpenSSL library failed")])

# check for boost
AC_CHECK_LIB(boost_thread-gcc-mt, main, , [
	AC_CHECK_LIB(boost_thread-mt, main, , [
		AC_CHECK_LIB(boost_thread, main, , [
			AC_MSG_ERROR("Linking against boost::thread library failed.")
		])
	])
])

AC_CHECK_LIB(boost_regex-gcc-mt, main, , [
	AC_CHECK_LIB(boost_regex-mt, main, , [
		AC_CHECK_LIB(boost_regex, main, , [
			AC_MSG_ERROR("Linking against boost::regex library failed.")
		])
	])
])

AC_CHECK_LIB(boost_system-gcc-mt, main, , [
	AC_CHECK_LIB(boost_system-mt, main, , [
		AC_CHECK_LIB(boost_system, main, , [
			AC_MSG_ERROR("Linking against boost::system library failed.")
		])
	])
])

AC_CHECK_LIB(boost_date_time-gcc-mt, main, , [
	AC_CHECK_LIB(boost_date_time-mt, main, , [
		AC_CHECK_LIB(boost_date_time, main, , [
			AC_MSG_ERROR("Linking against boost::date-time library failed.")
		])
	])
])

AC_CHECK_LIB(boost_filesystem-gcc-mt, main, , [
	AC_CHECK_LIB(boost_filesystem-mt, main, , [
		AC_CHECK_LIB(boost_filesystem, main, , [
			AC_MSG_ERROR("Linking against boost::filesystem library failed.")
		])
	])
])

# check for boost::unordered_set (should only check if gnu compiler version is below 4)
# AC_CHECK_HEADERS([boost/tr1/unordered_set.hpp], , [AC_MSG_ERROR("boost::unordered_set header not found. Please update your boost to at least 1.40.")])

# check for Lua
if test "$luaJIT" = "yes" ; then
	PKG_CHECK_MODULES(LUA, luajit, , [
		AC_CHECK_HEADERS([luajit-2.0/lua.hpp], ,[AC_MSG_ERROR("LuaJIT header not found.")])
		AC_CHECK_LIB(luajit, main, , [
			AC_CHECK_LIB(luajit-5.1, main, , [AC_MSG_ERROR("Linking against LuaJIT library failed.")])
		])
	])
else
	PKG_CHECK_MODULES(LUA, lua5.1 >= 5.1, , [
		AC_CHECK_HEADERS([lua.h], , [
			OPTIONAL_FLAGS="$OPTIONAL_FLAGS -D__ALT_LUA_PATH__"
			AC_CHECK_HEADERS([lua5.1/lua.h], , [AC_MSG_ERROR("Lua header not found.")])
		])
		AC_CHECK_LIB(lua, main, , [
			AC_CHECK_LIB(lua5.1, main, , [AC_MSG_ERROR("Linking against Lua library failed.")])
		])
	])
fi

AC_SUBST(OPTIONAL_FLAGS)
AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)

if test "$useMySQL" = "no" ; then
	if test "$useMySQLpp" = "no" ; then
		if test "$useSQLite" = "no" ; then
			if test "$usePostgreSQL" = "no" ; then
				AM_CONDITIONAL(USE_SQLITE, true)
				SQLITE_FLAGS=-D__USE_SQLITE__
				AC_SUBST(SQLITE_FLAGS)
				useSQLite=yes

				echo WARNING: Any of available database drivers was enabled, using SQLite as default. To enable a database driver, configure with --enable-\<driver\> \(example: --enable-mysqlpp\)
				echo
			fi
		fi
	fi
fi

# check for mysql if it is enabled
if test -n "$MYSQL_FLAGS"; then
	AC_CHECK_HEADERS([mysql/mysql.h],[MYSQL_LIBS=-lmysqlclient],[AC_MSG_ERROR("MySQL headers missing.")])
	AC_CHECK_LIB(mysqlclient, main,[],[
		if test -d /usr/lib64/mysql ; then
			echo "NOTICE -- Copying mysqlclient files into the /usr/lib64 directory."
			echo "NOTICE -- If mysqlclient is not found, please re-run the configure script."
			cp -Rsf /usr/lib64/mysql/libmysqlclient* /usr/lib64
			AC_CHECK_LIB(mysqlclient, main,[], [AC_MSG_ERROR("Linking against mysqlclient failed.")])
		elif test -d /usr/lib/mysql ; then
			echo "NOTICE -- Copying mysqlclient files into the /usr/lib directory."
			echo "NOTICE -- If mysqlclient is not found, please re-run the configure script."
			cp -Rsf /usr/lib/mysql/libmysqlclient* /usr/lib
			AC_CHECK_LIB(mysqlclient, main,[], [AC_MSG_ERROR("Linking against mysqlclient failed.")])
		else
			AC_MSG_ERROR("Linking against mysql-client failed.")
		fi
	])

	AC_SUBST(MYSQL_LIBS)
fi

# check for mysqlcppconn if it is enabled
if test -n "$MYSQLPP_FLAGS"; then
	AC_CHECK_HEADERS([cppconn/config.h],[MYSQLPP_LIBS=-lmysqlcppconn],[AC_MSG_ERROR("MySQL++Connector headers missing.")])
	AC_CHECK_LIB(mysqlcppconn, main,[],[AC_MSG_ERROR("Linking against libmysqlcppconn failed.")])
	AC_SUBST(MYSQLPP_LIBS)
fi

# check for postgresql if it is enabled
if test -n "$PGSQL_FLAGS"; then
	AC_CHECK_HEADERS([`pg_config --includedir`/libpq-fe.h],[PGSQL_LIBS=-lpg],[AC_MSG_ERROR("PostgreSQL headers missing.")])
	AC_CHECK_LIB(pq, main,[],[AC_MSG_ERROR("Linking against libpq failed.")])
	AC_SUBST(PGSQL_LIBS)
fi

# check for sqlite if it is enabled
if test -n "$SQLITE_FLAGS"; then
	AC_CHECK_HEADERS([sqlite3.h],[SQLITE_LIBS=-lsqlite3],[AC_MSG_ERROR("SQLite3 headers missing.")])
	AC_CHECK_LIB(sqlite3, main,[],[AC_MSG_ERROR("Linking against sqlite3 failed.")])
	AC_SUBST(SQLITE_LIBS)
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
#echo The OTX Server 2.X.Series
echo $PACKAGE $VERSION
echo
echo Using LuaJIT................ : $luaJIT
echo Using ground cache.......... : $groundCache
echo Server diagnostics.......... : $serverDiag
echo Home-directory configuration : $homedirConf
echo Root run permission......... : $rootPermission
echo Login server mode........... : $loginServer
echo OTAdmin protocol............ : $otAdmin
echo OTServ custom allocator..... : $otservAllocator
echo Debug build................. : $debugBuild
echo
echo Build with MySQL............ : $useMySQL
echo Build with MySQL++Connector. : $useMySQLpp
echo Build with SQLite........... : $useSQLite
echo Build with PostgreSQL....... : $usePostgreSQL
echo

echo Configure complete, now you may type \'./build.sh\'.
