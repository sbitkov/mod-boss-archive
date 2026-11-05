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


CREATE TABLE IF NOT EXISTS `character_archive_item` (
    `guid` INT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `source_type` TINYINT UNSIGNED NOT NULL,
    `source_entry` INT UNSIGNED NOT NULL,
    `source_context` INT UNSIGNED NOT NULL DEFAULT 0,
    `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (
        `guid`,
        `item_entry`,
        `source_type`,
        `source_entry`,
        `source_context`
    ),

    KEY `idx_archive_item_lookup` (`guid`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `character_boss_archive_copy` (
    `guid` INT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `item_guid` INT UNSIGNED NOT NULL,
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`item_guid`),
    KEY `idx_archive_owner_entry` (`guid`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_archive_variant` (
    `guid` INT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `random_property_id` INT NOT NULL,
    `suffix_factor` INT UNSIGNED NOT NULL DEFAULT 0,
    `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (
        `guid`,
        `item_entry`,
        `random_property_id`
    ),

    KEY `idx_archive_variant_item` (`guid`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;