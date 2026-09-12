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

INSERT IGNORE INTO `character_archive_item`
(
    `guid`,
    `item_entry`,
    `source_type`,
    `source_entry`,
    `source_context`,
    `unlocked_at`
)
SELECT
    `guid`,
    `item_entry`,
    1,
    `boss_entry`,
    `loot_mode`,
    `unlocked_at`
FROM `character_boss_archive_item`;
