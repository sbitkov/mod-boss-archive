CREATE TABLE IF NOT EXISTS `character_boss_archive` (
    `guid` INT UNSIGNED NOT NULL,
    `boss_entry` INT UNSIGNED NOT NULL,
    `loot_mode` SMALLINT UNSIGNED NOT NULL DEFAULT 1,
    `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`guid`, `boss_entry`, `loot_mode`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_boss_archive_item` (
    `guid` INT UNSIGNED NOT NULL,
    `boss_entry` INT UNSIGNED NOT NULL,
    `loot_mode` SMALLINT UNSIGNED NOT NULL DEFAULT 1,
    `item_entry` INT UNSIGNED NOT NULL,
    `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`guid`, `boss_entry`, `loot_mode`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_boss_archive_copy` (
    `guid` INT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `item_guid` INT UNSIGNED NOT NULL,
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`item_guid`),
    KEY `idx_archive_owner_entry` (`guid`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
