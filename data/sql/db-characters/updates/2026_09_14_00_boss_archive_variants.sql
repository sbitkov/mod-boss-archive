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