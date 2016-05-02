CREATE TABLE "server_config" (
	"config" VARCHAR(35) NOT NULL DEFAULT '',
	"value" VARCHAR(255) NOT NULL DEFAULT '',
	UNIQUE ("config")
);

INSERT INTO "server_config" VALUES ('db_version', 31);

CREATE TABLE "server_motd" (
	"id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"text" TEXT NOT NULL DEFAULT '',
	UNIQUE ("id", "world_id")
);

INSERT INTO "server_motd" VALUES (1, 0, 'Welcome to The Forgotten Server!');

CREATE TABLE "server_record" (
	"record" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"timestamp" INTEGER NOT NULL,
	UNIQUE ("record", "world_id", "timestamp")
);

INSERT INTO "server_record" VALUES (0, 0, 0);

CREATE TABLE "server_reports" (
	"id" INTEGER PRIMARY KEY,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"player_id" INTEGER NOT NULL DEFAULT 0,
	"posx" INTEGER NOT NULL DEFAULT 0,
	"posy" INTEGER NOT NULL DEFAULT 0,
	"posz" INTEGER NOT NULL DEFAULT 0,
	"timestamp" INTEGER NOT NULL DEFAULT 0,
	"report" TEXT NOT NULL DEFAULT '',
	"reads" INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE "accounts" (
	"id" INTEGER PRIMARY KEY NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	"password" VARCHAR(255) NOT NULL,
	"salt" VARCHAR(40) NOT NULL DEFAULT '',
	"premdays" INTEGER NOT NULL DEFAULT 0,
	"lastday" INTEGER NOT NULL DEFAULT 0,
	"email" VARCHAR(255) NOT NULL DEFAULT '',
	"key" VARCHAR(32) NOT NULL DEFAULT '0',
	"blocked" BOOLEAN NOT NULL DEFAULT FALSE,
	"warnings" INTEGER NOT NULL DEFAULT 0,
	"group_id" INTEGER NOT NULL DEFAULT 1,
	UNIQUE ("name")
);

INSERT INTO "accounts" VALUES (1, '1', '356a192b7913b04c54574d18c28d46e6395428ab', '', 65535, 0, '', '0', 0, 0, 1);

CREATE TABLE "players" (
	"id" INTEGER PRIMARY KEY NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"group_id" INTEGER NOT NULL,
	"account_id" INTEGER NOT NULL,
	"level" INTEGER NOT NULL DEFAULT 1,
	"vocation" INTEGER NOT NULL DEFAULT 0,
	"health" INTEGER NOT NULL DEFAULT 100,
	"healthmax" INTEGER NOT NULL DEFAULT 100,
	"experience" INTEGER NOT NULL DEFAULT 0,
	"lookbody" INTEGER NOT NULL DEFAULT 10,
	"lookfeet" INTEGER NOT NULL DEFAULT 10,
	"lookhead" INTEGER NOT NULL DEFAULT 10,
	"looklegs" INTEGER NOT NULL DEFAULT 10,
	"looktype" INTEGER NOT NULL DEFAULT 136,
	"lookaddons" INTEGER NOT NULL DEFAULT 0,
	"lookmount" INTEGER NOT NULL DEFAULT 0,
	"maglevel" INTEGER NOT NULL DEFAULT 0,
	"mana" INTEGER NOT NULL DEFAULT 100,
	"manamax" INTEGER NOT NULL DEFAULT 100,
	"manaspent" INTEGER NOT NULL DEFAULT 0,
	"soul" INTEGER NOT NULL DEFAULT 0,
	"town_id" INTEGER NOT NULL,
	"posx" INTEGER NOT NULL DEFAULT 0,
	"posy" INTEGER NOT NULL DEFAULT 0,
	"posz" INTEGER NOT NULL DEFAULT 0,
	"conditions" BLOB NOT NULL,
	"cap" INTEGER NOT NULL DEFAULT 0,
	"sex" INTEGER NOT NULL DEFAULT 0,
	"lastlogin" INTEGER NOT NULL DEFAULT 0,
	"lastip" INTEGER NOT NULL DEFAULT 0,
	"save" BOOLEAN NOT NULL DEFAULT TRUE,
	"skull" INTEGER NOT NULL DEFAULT 0,
	"skulltime" INTEGER NOT NULL DEFAULT 0,
	"rank_id" INTEGER NOT NULL,
	"guildnick" VARCHAR(255) NOT NULL DEFAULT '',
	"lastlogout" INTEGER NOT NULL DEFAULT 0,
	"blessings" INTEGER NOT NULL DEFAULT 0,
	"pvp_blessing" BOOLEAN NOT NULL DEFAULT FALSE,
	"balance" INTEGER NOT NULL DEFAULT 0,
	"stamina" INTEGER NOT NULL DEFAULT 151200000,
	"direction" INTEGER NOT NULL DEFAULT 2,
	"loss_experience" INTEGER NOT NULL DEFAULT 100,
	"loss_mana" INTEGER NOT NULL DEFAULT 100,
	"loss_skills" INTEGER NOT NULL DEFAULT 100,
	"loss_containers" INTEGER NOT NULL DEFAULT 100,
	"loss_items" INTEGER NOT NULL DEFAULT 100,
	"premend" INTEGER NOT NULL DEFAULT 0,
	"online" INTEGER NOT NULL DEFAULT 0,
	"marriage" INTEGER NOT NULL DEFAULT 0,
	"promotion" INTEGER NOT NULL DEFAULT 0,
	"deleted" INTEGER NOT NULL DEFAULT 0,
	"description" VARCHAR(255) NOT NULL DEFAULT '',
	UNIQUE ("name", "deleted"),
	FOREIGN KEY ("account_id") REFERENCES "accounts" ("id")
);

INSERT INTO "players" VALUES (1, 'Account Manager', 0, 1, 1, 1, 0, 150, 150, 0, 0, 0, 0, 0, 110, 0, 0, 0, 0, 0, 0, 0, 0, 853, 921, 7, '', 400, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 201660000, 0, 100, 100, 100, 100, 100, 0, 0, 0, 0, 0, '');

CREATE TABLE "account_viplist" (
	"account_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"player_id" INTEGER NOT NULL,
	UNIQUE ("account_id", "player_id"),
	FOREIGN KEY ("account_id") REFERENCES "accounts" ("id"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "global_storage" (
	"key" VARCHAR(32) NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"value" VARCHAR(255) NOT NULL DEFAULT '0',
	UNIQUE ("key", "world_id")
);

CREATE TABLE "guilds" (
	"id" INTEGER PRIMARY KEY,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"name" VARCHAR(255) NOT NULL,
	"ownerid" INTEGER NOT NULL,
	"creationdata" INTEGER NOT NULL,
	"checkdata" INTEGER NOT NULL,
	"motd" VARCHAR(255) NOT NULL DEFAULT '',
	UNIQUE ("name", "world_id"),
	FOREIGN KEY ("ownerid") REFERENCES "players" ("id")
);

CREATE TABLE "guild_invites" (
	"player_id" INTEGER NOT NULL,
	"guild_id" INTEGER NOT NULL,
	UNIQUE ("player_id", "guild_id"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id"),
	FOREIGN KEY ("guild_id") REFERENCES "guilds" ("id")
);

CREATE TABLE "guild_ranks" (
	"id" INTEGER PRIMARY KEY,
	"guild_id" INTEGER NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	"level" INTEGER NOT NULL,
	FOREIGN KEY ("guild_id") REFERENCES "guilds" ("id")
);

CREATE TABLE "houses" (
	"id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"owner" INTEGER NOT NULL,
	"paid" INTEGER NOT NULL DEFAULT 0,
	"warnings" INTEGER NOT NULL DEFAULT 0,
	"lastwarning" INTEGER NOT NULL DEFAULT 0,
	"name" VARCHAR(255) NOT NULL,
	"town" INTEGER NOT NULL DEFAULT 0,
	"size" INTEGER NOT NULL DEFAULT 0,
	"price" INTEGER NOT NULL DEFAULT 0,
	"rent" INTEGER NOT NULL DEFAULT 0,
	"doors" INTEGER NOT NULL DEFAULT 0,
	"beds" INTEGER NOT NULL DEFAULT 0,
	"tiles" INTEGER NOT NULL DEFAULT 0,
	"guild" BOOLEAN NOT NULL DEFAULT FALSE,
	"clear" BOOLEAN NOT NULL DEFAULT FALSE,
	UNIQUE ("id", "world_id")
);

CREATE TABLE "house_lists" (
	"house_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"listid" INTEGER NOT NULL,
	"list" TEXT NOT NULL,
	UNIQUE ("house_id", "world_id", "listid"),
	FOREIGN KEY ("house_id", "world_id") REFERENCES "houses" ("id", "world_id")
);

CREATE TABLE "house_data" (
	"house_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"data" LONGBLOB NOT NULL,
	UNIQUE ("house_id", "world_id"),
	FOREIGN KEY ("house_id", "world_id") REFERENCES "houses" ("id", "world_id")
);

CREATE TABLE "house_auctions" (
	"house_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"player_id" INTEGER NOT NULL,
	"bid" INTEGER NOT NULL DEFAULT 0,
	"limit" INTEGER NOT NULL DEFAULT 0,
	"endtime" INTEGER NOT NULL DEFAULT 0,
	UNIQUE ("house_id", "world_id"),
	FOREIGN KEY ("house_id", "world_id") REFERENCES "houses" ("id", "world_id")
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_deaths" (
	"id" INTEGER PRIMARY KEY,
	"player_id" INTEGER NOT NULL,
	"date" INTEGER NOT NULL,
	"level" INTEGER NOT NULL,
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "killers" (
	"id" INTEGER PRIMARY KEY,
	"death_id" INTEGER NOT NULL,
	"final_hit" BOOLEAN NOT NULL DEFAULT FALSE,
	"unjustified" BOOLEAN NOT NULL DEFAULT FALSE,
	FOREIGN KEY ("death_id") REFERENCES "player_deaths" ("id")
);

CREATE TABLE "player_killers" (
	"kill_id" INTEGER NOT NULL,
	"player_id" INTEGER NOT NULL,
	FOREIGN KEY ("kill_id") REFERENCES "killers" ("id"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "environment_killers" (
	"kill_id" INTEGER NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	FOREIGN KEY ("kill_id") REFERENCES "killers" ("id")
);

CREATE TABLE "player_depotitems" (
	"player_id" INTEGER NOT NULL,
	"sid" INTEGER NOT NULL,
	"pid" INTEGER NOT NULL DEFAULT 0,
	"itemtype" INTEGER NOT NULL,
	"count" INTEGER NOT NULL DEFAULT 0,
	"attributes" BLOB NOT NULL,
	UNIQUE ("player_id", "sid"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_namelocks" (
	"player_id" INTEGER NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	"new_name" VARCHAR(255) NOT NULL,
	"date" INTEGER NOT NULL DEFAULT 0,
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_statements" (
	"id" INTEGER PRIMARY KEY,
	"player_id" INTEGER NOT NULL,
	"channel_id" INTEGER NOT NULL DEFAULT 0,
	"text" VARCHAR (255) NOT NULL,
	"date" INTEGER NOT NULL DEFAULT 0,
	FOREIGN KEY ("player_id") REFERENCES "players"("id")
);

CREATE TABLE "player_skills" (
	"player_id" INTEGER NOT NULL,
	"skillid" INTEGER NOT NULL,
	"value" INTEGER NOT NULL DEFAULT 0,
	"count" INTEGER NOT NULL DEFAULT 0,
	UNIQUE ("player_id", "skillid"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_storage" (
	"player_id" INTEGER NOT NULL,
	"key" VARCHAR(32) NOT NULL,
	"value" VARCHAR(255) NOT NULL DEFAULT '0',
	UNIQUE ("player_id", "key"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_viplist" (
	"player_id" INTEGER NOT NULL,
	"vip_id" INTEGER NOT NULL,
	UNIQUE ("player_id", "vip_id"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id"),
	FOREIGN KEY ("vip_id") REFERENCES "players" ("id")
);

CREATE TABLE "tiles" (
	"id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"house_id" INTEGER NOT NULL,
	"x" INTEGER NOT NULL,
	"y" INTEGER NOT NULL,
	"z" INTEGER NOT NULL,
	UNIQUE ("id", "world_id"),
	FOREIGN KEY ("house_id", "world_id") REFERENCES "houses" ("id", "world_id")
);

CREATE TABLE "tile_items" (
	"tile_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"sid" INTEGER NOT NULL,
	"pid" INTEGER NOT NULL DEFAULT 0,
	"itemtype" INTEGER NOT NULL,
	"count" INTEGER NOT NULL DEFAULT 0,
	"attributes" BLOB NOT NULL,
	UNIQUE ("tile_id", "world_id", "sid"),
	FOREIGN KEY ("tile_id") REFERENCES "tiles" ("id")
);

CREATE TABLE "player_items" (
	"player_id" INTEGER NOT NULL,
	"sid" INTEGER NOT NULL,
	"pid" INTEGER NOT NULL DEFAULT 0,
	"itemtype" INTEGER NOT NULL,
	"count" INTEGER NOT NULL DEFAULT 0,
	"attributes" BLOB NOT NULL,
	UNIQUE ("player_id", "sid"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_spells" (
	"player_id" INTEGER NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	UNIQUE ("player_id", "name"),
	FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "bans" (
	"id" INTEGER PRIMARY KEY NOT NULL,
	"type" INTEGER NOT NULL,
	"value" INTEGER NOT NULL,
	"param" INTEGER NOT NULL,
	"active" BOOLEAN NOT NULL DEFAULT TRUE,
	"expires" INTEGER NOT NULL DEFAULT -1,
	"added" INTEGER NOT NULL,
	"admin_id" INTEGER NOT NULL DEFAULT 0,
	"comment" TEXT NOT NULL
);

CREATE TABLE "tile_store"
(
	"house_id" INTEGER NOT NULL,
	"world_id" INTEGER NOT NULL DEFAULT 0,
	"data" LONGBLOB NOT NULL,
	FOREIGN KEY ("house_id") REFERENCES "houses" ("id")
);

CREATE TRIGGER "oncreate_guilds"
AFTER INSERT ON "guilds"
BEGIN
	INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Leader", 3, NEW."id");
	INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Vice-Leader", 2, NEW."id");
	INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Member", 1, NEW."id");
END;

CREATE TRIGGER "oncreate_players"
AFTER INSERT
ON "players"
BEGIN
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 0, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 1, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 2, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 3, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 4, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 5, 10);
	INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 6, 10);
END;

CREATE TRIGGER "ondelete_accounts"
BEFORE DELETE
ON "accounts"
FOR EACH ROW
BEGIN
	DELETE FROM "players" WHERE "account_id" = OLD."id";
	DELETE FROM "account_viplist" WHERE "account_id" = OLD."id";
	DELETE FROM "bans" WHERE "type" IN (3, 4) AND "value" = OLD."id";
END;

CREATE TRIGGER "ondelete_players"
BEFORE DELETE
ON "players"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'DELETE on table "players" violates foreign: "ownerid" from table "guilds"')
	WHERE (SELECT "id" FROM "guilds" WHERE "ownerid" = OLD."id") IS NOT NULL;

	DELETE FROM "account_viplist" WHERE "player_id" = OLD."id";
	DELETE FROM "player_viplist" WHERE "player_id" = OLD."id" OR "vip_id" = OLD."id";
	DELETE FROM "player_storage" WHERE "player_id" = OLD."id";
	DELETE FROM "player_skills" WHERE "player_id" = OLD."id";
	DELETE FROM "player_items" WHERE "player_id" = OLD."id";
	DELETE FROM "player_depotitems" WHERE "player_id" = OLD."id";
	DELETE FROM "player_spells" WHERE "player_id" = OLD."id";
	DELETE FROM "player_killers" WHERE "player_id" = OLD."id";
	DELETE FROM "player_deaths" WHERE "player_id" = OLD."id";
	DELETE FROM "guild_invites" WHERE "player_id" = OLD."id";
	DELETE FROM "player_namelocks" WHERE "player_id" = OLD."id";
	DELETE FROM "player_statements" WHERE "player_id" = OLD."id";
	DELETE FROM "bans" WHERE "type" IN (2, 5) AND "value" = OLD."id";
	UPDATE "houses" SET "owner" = 0 WHERE "owner" = OLD."id";
END;

CREATE TRIGGER "ondelete_guilds"
BEFORE DELETE
ON "guilds"
FOR EACH ROW
BEGIN
	UPDATE "players" SET "guildnick" = '', "rank_id" = 0 WHERE "rank_id" IN (SELECT "id" FROM "guild_ranks" WHERE "guild_id" = OLD."id");
	DELETE FROM "guild_ranks" WHERE "guild_id" = OLD."id";
	DELETE FROM "guild_invites" WHERE "guild_id" = OLD."id";
END;

CREATE TRIGGER "oninsert_players"
BEFORE INSERT
ON "players"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "players" violates foreign: "account_id"')
	WHERE NEW."account_id" IS NULL
		OR (SELECT "id" FROM "accounts" WHERE "id" = NEW."account_id") IS NULL;
END;

CREATE TRIGGER "onupdate_players"
BEFORE UPDATE
ON "players"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "players" violates foreign: "account_id"')
	WHERE NEW."account_id" IS NULL
		OR (SELECT "id" FROM "accounts" WHERE "id" = NEW."account_id") IS NULL;
END;

CREATE TRIGGER "oninsert_guilds"
BEFORE INSERT
ON "guilds"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "guilds" violates foreign: "ownerid"')
	WHERE NEW."ownerid" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."ownerid") IS NULL;
END;

CREATE TRIGGER "onupdate_guilds"
BEFORE UPDATE
ON "guilds"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "guilds" violates foreign: "ownerid"')
	WHERE NEW."ownerid" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."ownerid") IS NULL;
END;

CREATE TRIGGER "ondelete_houses"
BEFORE DELETE
ON "houses"
FOR EACH ROW
BEGIN
	DELETE FROM "house_lists" WHERE "house_id" = OLD."id";
END;

CREATE TRIGGER "ondelete_tiles"
BEFORE DELETE
ON "tiles"
FOR EACH ROW
BEGIN
	DELETE FROM "tile_items" WHERE "tile_id" = OLD."id";
END;

CREATE TRIGGER "oninsert_guild_ranks"
BEFORE INSERT
ON "guild_ranks"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "guild_ranks" violates foreign: "guild_id"')
	WHERE NEW."guild_id" IS NULL
		OR (SELECT "id" FROM "guilds" WHERE "id" = NEW."guild_id") IS NULL;
END;

CREATE TRIGGER "onupdate_guild_ranks"
BEFORE UPDATE
ON "guild_ranks"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "guild_ranks" violates foreign: "guild_id"')
	WHERE NEW."guild_id" IS NULL
		OR (SELECT "id" FROM "guilds" WHERE "id" = NEW."guild_id") IS NULL;
END;

CREATE TRIGGER "oninsert_house_lists"
BEFORE INSERT
ON "house_lists"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "house_lists" violates foreign: "house_id"')
	WHERE NEW."house_id" IS NULL
		OR (SELECT "id" FROM "houses" WHERE "id" = NEW."house_id") IS NULL;
END;

CREATE TRIGGER "onupdate_house_lists"
BEFORE UPDATE
ON "house_lists"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "house_lists" violates foreign: "house_id"')
	WHERE NEW."house_id" IS NULL
		OR (SELECT "id" FROM "houses" WHERE "id" = NEW."house_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_depotitems"
BEFORE INSERT
ON "player_depotitems"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_depotitems" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_depotitems"
BEFORE UPDATE
ON "player_depotitems"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_depotitems" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_skills"
BEFORE INSERT
ON "player_skills"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_skills" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_skills"
BEFORE UPDATE
ON "player_skills"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_skills" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_storage"
BEFORE INSERT
ON "player_storage"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_storage" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_storage"
BEFORE UPDATE
ON "player_storage"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_storage" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_viplist"
BEFORE INSERT
ON "player_viplist"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_viplist" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;

	SELECT RAISE(ROLLBACK, 'INSERT on table "player_viplist" violates foreign: "vip_id"')
	WHERE NEW."vip_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."vip_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_viplist"
BEFORE UPDATE
ON "player_viplist"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_viplist" violates foreign: "vip_id"')
	WHERE NEW."vip_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."vip_id") IS NULL;
END;

CREATE TRIGGER "oninsert_account_viplist"
BEFORE INSERT
ON "account_viplist"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "account_viplist" violates foreign: "account_id"')
	WHERE NEW."account_id" IS NULL
		OR (SELECT "id" FROM "accounts" WHERE "id" = NEW."account_id") IS NULL;

	SELECT RAISE(ROLLBACK, 'INSERT on table "account_viplist" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_account_viplist"
BEFORE UPDATE
ON "account_viplist"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "account_viplist" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_tile_items"
BEFORE INSERT
ON "tile_items"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "tile_items" violates foreign: "tile_id"')
	WHERE NEW."tile_id" IS NULL
		OR (SELECT "id" FROM "tiles" WHERE "id" = NEW."tile_id") IS NULL;
END;

CREATE TRIGGER "onupdate_tile_items"
BEFORE UPDATE
ON "tile_items"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "tile_items" violates foreign: "tile_id"')
	WHERE NEW."tile_id" IS NULL
		OR (SELECT "id" FROM "tiles" WHERE "id" = NEW."tile_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_spells"
BEFORE INSERT
ON "player_spells"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_spells" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_spells"
BEFORE UPDATE
ON "player_spells"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_spells" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_deaths"
BEFORE INSERT
ON "player_deaths"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_deaths" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_deaths"
BEFORE UPDATE
ON "player_deaths"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_deaths" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_killers"
BEFORE INSERT
ON "killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "killers" violates foreign: "death_id"')
	WHERE NEW."death_id" IS NULL
		OR (SELECT "id" FROM "player_deaths" WHERE "id" = NEW."death_id") IS NULL;
END;

CREATE TRIGGER "onupdate_killers"
BEFORE UPDATE
ON "killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "killers" violates foreign: "death_id"')
	WHERE NEW."death_id" IS NULL
		OR (SELECT "id" FROM "player_deaths" WHERE "id" = NEW."death_id") IS NULL;
END;

CREATE TRIGGER "oninsert_environment_killers"
BEFORE INSERT
ON "environment_killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "enviroment_killers" violates foreign: "kill_id"')
	WHERE NEW."kill_id" IS NULL
		OR (SELECT "id" FROM "killers" WHERE "id" = NEW."kill_id") IS NULL;
END;

CREATE TRIGGER "onupdate_environment_killers"
BEFORE UPDATE
ON "environment_killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "enviroment_killers" violates foreign: "kill_id"')
	WHERE NEW."kill_id" IS NULL
		OR (SELECT "id" FROM "killers" WHERE "id" = NEW."kill_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_killers"
BEFORE INSERT
ON "player_killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_killers" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
	
	SELECT RAISE(ROLLBACK, 'INSERT on table "player_killers" violates foreign: "kill_id"')
	WHERE NEW."kill_id" IS NULL
		OR (SELECT "id" FROM "killers" WHERE "id" = NEW."kill_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_killers"
BEFORE UPDATE
ON "player_killers"
FOR EACH ROW
BEGIN
	SELECT RAISE(ROLLBACK, 'UPDATE on table "player_killers" violates foreign: "player_id"')
	WHERE NEW."player_id" IS NULL
		OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
		
	SELECT RAISE(ROLLBACK, 'UPDATE on table "killers" violates foreign: "kill_id"')
	WHERE NEW."kill_id" IS NULL
		OR (SELECT "id" FROM "killers" WHERE "id" = NEW."kill_id") IS NULL;
END;
