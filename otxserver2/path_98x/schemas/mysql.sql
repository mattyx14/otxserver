DROP TRIGGER IF EXISTS `oncreate_players`;
DROP TRIGGER IF EXISTS `oncreate_guilds`;
DROP TRIGGER IF EXISTS `ondelete_players`;
DROP TRIGGER IF EXISTS `ondelete_guilds`;
DROP TRIGGER IF EXISTS `ondelete_accounts`;

DROP TABLE IF EXISTS `player_depotitems`;
DROP TABLE IF EXISTS `tile_items`;
DROP TABLE IF EXISTS `tile_store`;
DROP TABLE IF EXISTS `tiles`;
DROP TABLE IF EXISTS `bans`;
DROP TABLE IF EXISTS `house_lists`;
DROP TABLE IF EXISTS `houses`;
DROP TABLE IF EXISTS `player_items`;
DROP TABLE IF EXISTS `player_namelocks`;
DROP TABLE IF EXISTS `player_statements`;
DROP TABLE IF EXISTS `player_skills`;
DROP TABLE IF EXISTS `player_storage`;
DROP TABLE IF EXISTS `player_viplist`;
DROP TABLE IF EXISTS `player_spells`;
DROP TABLE IF EXISTS `player_deaths`;
DROP TABLE IF EXISTS `killers`;
DROP TABLE IF EXISTS `environment_killers`;
DROP TABLE IF EXISTS `player_killers`;
DROP TABLE IF EXISTS `guild_ranks`;
DROP TABLE IF EXISTS `guilds`;
DROP TABLE IF EXISTS `guild_invites`;
DROP TABLE IF EXISTS `global_storage`;
DROP TABLE IF EXISTS `players`;
DROP TABLE IF EXISTS `accounts`;
DROP TABLE IF EXISTS `server_record`;
DROP TABLE IF EXISTS `server_motd`;
DROP TABLE IF EXISTS `server_reports`;
DROP TABLE IF EXISTS `server_config`;
DROP TABLE IF EXISTS `account_viplist`;

CREATE TABLE `accounts`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`name` VARCHAR(32) NOT NULL DEFAULT '',
	`password` VARCHAR(255) NOT NULL/* VARCHAR(32) NOT NULL COMMENT 'MD5'*//* VARCHAR(40) NOT NULL COMMENT 'SHA1'*/,
	`salt` VARCHAR(40) NOT NULL DEFAULT '',
	`premdays` INT NOT NULL DEFAULT 0,
	`lastday` INT UNSIGNED NOT NULL DEFAULT 0,
	`email` VARCHAR(255) NOT NULL DEFAULT '',
	`key` VARCHAR(32) NOT NULL DEFAULT '0',
	`blocked` TINYINT(1) NOT NULL DEFAULT FALSE COMMENT 'internal usage',
	`warnings` INT NOT NULL DEFAULT 0,
	`group_id` INT NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`), UNIQUE (`name`)
) ENGINE = InnoDB;

INSERT INTO `accounts` VALUES (1, '1', '356a192b7913b04c54574d18c28d46e6395428ab', '', 65535, 0, '', '0', 0, 0, 1);

CREATE TABLE `players`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`name` VARCHAR(255) NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`group_id` INT NOT NULL DEFAULT 1,
	`account_id` INT NOT NULL DEFAULT 0,
	`level` INT NOT NULL DEFAULT 1,
	`vocation` INT NOT NULL DEFAULT 0,
	`health` INT NOT NULL DEFAULT 150,
	`healthmax` INT NOT NULL DEFAULT 150,
	`experience` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	`lookbody` INT NOT NULL DEFAULT 0,
	`lookfeet` INT NOT NULL DEFAULT 0,
	`lookhead` INT NOT NULL DEFAULT 0,
	`looklegs` INT NOT NULL DEFAULT 0,
	`looktype` INT NOT NULL DEFAULT 136,
	`lookaddons` INT NOT NULL DEFAULT 0,
	`lookmount` INT NOT NULL DEFAULT 0, 
	`maglevel` INT NOT NULL DEFAULT 0,
	`mana` INT NOT NULL DEFAULT 0,
	`manamax` INT NOT NULL DEFAULT 0,
	`manaspent` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	`soul` INT UNSIGNED NOT NULL DEFAULT 0,
	`town_id` INT NOT NULL DEFAULT 0,
	`posx` INT NOT NULL DEFAULT 0,
	`posy` INT NOT NULL DEFAULT 0,
	`posz` INT NOT NULL DEFAULT 0,
	`conditions` BLOB NOT NULL,
	`cap` INT NOT NULL DEFAULT 0,
	`sex` INT NOT NULL DEFAULT 0,
	`lastlogin` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	`lastip` INT UNSIGNED NOT NULL DEFAULT 0,
	`save` TINYINT(1) NOT NULL DEFAULT 1,
	`skull` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`skulltime` INT NOT NULL DEFAULT 0,
	`rank_id` INT NOT NULL DEFAULT 0,
	`guildnick` VARCHAR(255) NOT NULL DEFAULT '',
	`lastlogout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	`blessings` TINYINT(2) NOT NULL DEFAULT 0,
	`pvp_blessing` TINYINT(1) NOT NULL DEFAULT 0,
	`balance` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	`stamina` BIGINT UNSIGNED NOT NULL DEFAULT 151200000 COMMENT 'stored in miliseconds',
	`direction` INT NOT NULL DEFAULT 2,
	`loss_experience` INT NOT NULL DEFAULT 100,
	`loss_mana` INT NOT NULL DEFAULT 100,
	`loss_skills` INT NOT NULL DEFAULT 100,
	`loss_containers` INT NOT NULL DEFAULT 100,
	`loss_items` INT NOT NULL DEFAULT 100,
	`premend` INT NOT NULL DEFAULT 0 COMMENT 'NOT IN USE BY THE SERVER',
	`online` TINYINT(1) NOT NULL DEFAULT 0,
	`marriage` INT UNSIGNED NOT NULL DEFAULT 0,
	`promotion` INT NOT NULL DEFAULT 0,
	`deleted` INT NOT NULL DEFAULT 0,
	`description` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`), UNIQUE (`name`, `deleted`),
	KEY (`account_id`), KEY (`group_id`),
	KEY (`online`), KEY (`deleted`),
	FOREIGN KEY (`account_id`) REFERENCES `accounts`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

INSERT INTO `players` VALUES (1, 'Account Manager', 0, 1, 1, 1, 0, 150,  150, 0, 0, 0, 0, 0, 110, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 7, '', 400, 0,  0, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 201660000, 0, 100, 100, 100, 100, 100, 0,  0, 0, 0, 0, '');

CREATE TABLE `account_viplist`
(
	`account_id` INT NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`player_id` INT NOT NULL,
	KEY (`account_id`), KEY (`player_id`), KEY (`world_id`), UNIQUE (`account_id`, `player_id`),
	FOREIGN KEY (`account_id`) REFERENCES `accounts`(`id`) ON DELETE CASCADE,
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_deaths`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`player_id` INT NOT NULL,
	`date` BIGINT UNSIGNED NOT NULL,
	`level` INT UNSIGNED NOT NULL,
	PRIMARY KEY (`id`), INDEX (`date`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_depotitems`
(
	`player_id` INT NOT NULL,
	`sid` INT NOT NULL COMMENT 'any given range, eg. 0-100 is reserved for depot lockers and all above 100 will be normal items inside depots',
	`pid` INT NOT NULL DEFAULT 0,
	`itemtype` INT NOT NULL,
	`count` INT NOT NULL DEFAULT 0,
	`attributes` BLOB NOT NULL,
	KEY (`player_id`), UNIQUE (`player_id`, `sid`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_items`
(
	`player_id` INT NOT NULL,
	`pid` INT NOT NULL DEFAULT 0,
	`sid` INT NOT NULL DEFAULT 0,
	`itemtype` INT NOT NULL DEFAULT 0,
	`count` INT NOT NULL DEFAULT 0,
	`attributes` BLOB NOT NULL,
	KEY (`player_id`), UNIQUE (`player_id`, `sid`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_namelocks`
(
	`player_id` INT NOT NULL,
	`name` VARCHAR(255) NOT NULL,
	`new_name` VARCHAR(255) NOT NULL,
	`date` BIGINT NOT NULL DEFAULT 0,
	KEY (`player_id`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_statements`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`player_id` INT NOT NULL,
	`channel_id` INT NOT NULL DEFAULT 0,
	`text` VARCHAR (255) NOT NULL,
	`date` BIGINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`), KEY (`player_id`), KEY (`channel_id`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_skills`
(
	`player_id` INT NOT NULL,
	`skillid` TINYINT(2) NOT NULL DEFAULT 0,
	`value` INT UNSIGNED NOT NULL DEFAULT 0,
	`count` INT UNSIGNED NOT NULL DEFAULT 0,
	KEY (`player_id`), UNIQUE (`player_id`, `skillid`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_spells`
(
	`player_id` INT NOT NULL,
	`name` VARCHAR(255) NOT NULL,
	KEY (`player_id`), UNIQUE (`player_id`, `name`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_storage`
(
	`player_id` INT NOT NULL,
	`key` VARCHAR(32) NOT NULL DEFAULT '0',
	`value` TEXT NOT NULL,
	KEY (`player_id`), UNIQUE (`player_id`, `key`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_viplist`
(
	`player_id` INT NOT NULL,
	`vip_id` INT NOT NULL,
	KEY (`player_id`), KEY (`vip_id`), UNIQUE (`player_id`, `vip_id`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE,
	FOREIGN KEY (`vip_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `killers`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`death_id` INT NOT NULL,
	`final_hit` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE,
	`unjustified` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE,
	PRIMARY KEY (`id`),
	FOREIGN KEY (`death_id`) REFERENCES `player_deaths`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `player_killers`
(
	`kill_id` INT NOT NULL,
	`player_id` INT NOT NULL,
	FOREIGN KEY (`kill_id`) REFERENCES `killers`(`id`) ON DELETE CASCADE,
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `environment_killers`
(
	`kill_id` INT NOT NULL,
	`name` VARCHAR(255) NOT NULL,
	FOREIGN KEY (`kill_id`) REFERENCES `killers`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `houses`
(
	`id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`owner` INT NOT NULL,
	`paid` INT UNSIGNED NOT NULL DEFAULT 0,
	`warnings` INT NOT NULL DEFAULT 0,
	`lastwarning` INT UNSIGNED NOT NULL DEFAULT 0,
	`name` VARCHAR(255) NOT NULL,
	`town` INT UNSIGNED NOT NULL DEFAULT 0,
	`size` INT UNSIGNED NOT NULL DEFAULT 0,
	`price` INT UNSIGNED NOT NULL DEFAULT 0,
	`rent` INT UNSIGNED NOT NULL DEFAULT 0,
	`doors` INT UNSIGNED NOT NULL DEFAULT 0,
	`beds` INT UNSIGNED NOT NULL DEFAULT 0,
	`tiles` INT UNSIGNED NOT NULL DEFAULT 0,
	`guild` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE,
	`clear` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE,
	UNIQUE (`id`, `world_id`)
) ENGINE = InnoDB;

CREATE TABLE `tile_store`
(
	`house_id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`data` LONGBLOB NOT NULL,
	FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `house_auctions`
(
	`house_id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`player_id` INT NOT NULL,
	`bid` INT UNSIGNED NOT NULL DEFAULT 0,
	`limit` INT UNSIGNED NOT NULL DEFAULT 0,
	`endtime` BIGINT UNSIGNED NOT NULL DEFAULT 0,
	UNIQUE (`house_id`, `world_id`),
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE,
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `house_lists`
(
	`house_id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`listid` INT NOT NULL,
	`list` TEXT NOT NULL,
	UNIQUE (`house_id`, `world_id`, `listid`),
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `house_data`
(
	`house_id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`data` LONGBLOB NOT NULL,
	UNIQUE (`house_id`, `world_id`),
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `tiles`
(
	`id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`house_id` INT UNSIGNED NOT NULL,
	`x` INT(5) UNSIGNED NOT NULL,
	`y` INT(5) UNSIGNED NOT NULL,
	`z` TINYINT(2) UNSIGNED NOT NULL,
	UNIQUE (`id`, `world_id`),
	KEY (`x`, `y`, `z`),
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `tile_items`
(
	`tile_id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`sid` INT NOT NULL,
	`pid` INT NOT NULL DEFAULT 0,
	`itemtype` INT NOT NULL,
	`count` INT NOT NULL DEFAULT 0,
	`attributes` BLOB NOT NULL,
	UNIQUE (`tile_id`, `world_id`, `sid`), KEY (`sid`),
	FOREIGN KEY (`tile_id`) REFERENCES `tiles`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `guilds`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`name` VARCHAR(255) NOT NULL,
	`ownerid` INT NOT NULL,
	`creationdata` INT NOT NULL,
	`checkdata` INT NOT NULL,
	`motd` VARCHAR(255) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE (`name`, `world_id`)
) ENGINE = InnoDB;

CREATE TABLE `guild_invites`
(
	`player_id` INT NOT NULL DEFAULT 0,
	`guild_id` INT NOT NULL DEFAULT 0,
	UNIQUE (`player_id`, `guild_id`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE,
	FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `guild_ranks`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`guild_id` INT NOT NULL,
	`name` VARCHAR(255) NOT NULL,
	`level` INT NOT NULL COMMENT '1 - leader, 2 - vice leader, 3 - member',
	PRIMARY KEY (`id`),
	FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

CREATE TABLE `bans`
(
	`id` INT UNSIGNED NOT NULL auto_increment,
	`type` TINYINT(1) NOT NULL COMMENT '1 - ip, 2 - player, 3 - account, 4 - notation',
	`value` INT UNSIGNED NOT NULL COMMENT 'ip - ip address, player - player_id, account - account_id, notation - account_id',
	`param` INT UNSIGNED NOT NULL COMMENT 'ip - mask, player - type (1 - report, 2 - lock, 3 - ban), account - player, notation - player',
	`active` TINYINT(1) NOT NULL DEFAULT TRUE,
	`expires` INT NOT NULL DEFAULT -1,
	`added` INT UNSIGNED NOT NULL,
	`admin_id` INT UNSIGNED NOT NULL DEFAULT 0,
	`comment` TEXT NOT NULL,
	`reason` INT UNSIGNED NOT NULL DEFAULT 0,
	`action` INT UNSIGNED NOT NULL DEFAULT 0,
	`statement` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`),
	KEY `type` (`type`, `value`),
	KEY `active` (`active`)
) ENGINE = InnoDB;

CREATE TABLE `global_storage`
(
	`key` VARCHAR(32) NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`value` TEXT NOT NULL,
	UNIQUE  (`key`, `world_id`)
) ENGINE = InnoDB;

CREATE TABLE `server_config`
(
	`config` VARCHAR(35) NOT NULL DEFAULT '',
	`value` VARCHAR(255) NOT NULL DEFAULT '',
	UNIQUE (`config`)
) ENGINE = InnoDB;

INSERT INTO `server_config` VALUES ('db_version', 31);

CREATE TABLE `server_motd`
(
	`id` INT UNSIGNED NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`text` TEXT NOT NULL,
	UNIQUE (`id`, `world_id`)
) ENGINE = InnoDB;

INSERT INTO `server_motd` VALUES (1, 0, 'Welcome to The OTX Server!');

CREATE TABLE `server_record`
(
	`record` INT NOT NULL,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`timestamp` BIGINT NOT NULL,
	UNIQUE (`record`, `world_id`, `timestamp`)
) ENGINE = InnoDB;

INSERT INTO `server_record` VALUES (0, 0, 0);

CREATE TABLE `server_reports`
(
	`id` INT NOT NULL AUTO_INCREMENT,
	`world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	`player_id` INT NOT NULL DEFAULT 1,
	`posx` INT NOT NULL DEFAULT 0,
	`posy` INT NOT NULL DEFAULT 0,
	`posz` INT NOT NULL DEFAULT 0,
	`timestamp` BIGINT NOT NULL DEFAULT 0,
	`report` TEXT NOT NULL,
	`reads` INT NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	KEY (`world_id`), KEY (`reads`),
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
) ENGINE = InnoDB;

DELIMITER |

CREATE TRIGGER `ondelete_accounts`
BEFORE DELETE
ON `accounts`
FOR EACH ROW
BEGIN
	DELETE FROM `bans` WHERE `type` IN (3, 4) AND `value` = OLD.`id`;
END|

CREATE TRIGGER `oncreate_guilds`
AFTER INSERT
ON `guilds`
FOR EACH ROW
BEGIN
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Leader', 3, NEW.`id`);
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Vice-Leader', 2, NEW.`id`);
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Member', 1, NEW.`id`);
END|

CREATE TRIGGER `ondelete_guilds`
BEFORE DELETE
ON `guilds`
FOR EACH ROW
BEGIN
	UPDATE `players` SET `guildnick` = '', `rank_id` = 0 WHERE `rank_id` IN (SELECT `id` FROM `guild_ranks` WHERE `guild_id` = OLD.`id`);
END|

CREATE TRIGGER `oncreate_players`
AFTER INSERT
ON `players`
FOR EACH ROW
BEGIN
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 0, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 1, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 2, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 3, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 4, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 5, 10);
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 6, 10);
END|

CREATE TRIGGER `ondelete_players`
BEFORE DELETE
ON `players`
FOR EACH ROW
BEGIN
	DELETE FROM `bans` WHERE `type` IN (2, 5) AND `value` = OLD.`id`;
	UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`;
END|

DELIMITER ;
