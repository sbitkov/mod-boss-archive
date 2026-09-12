-- Boss Archive 0.2
-- Allow multiple archive copies of the same unlocked item.
-- item_guid identifies each physical archive copy.

SET @schema_name = DATABASE();

SET @has_target_pk = (
    SELECT COUNT(*)
    FROM information_schema.statistics
    WHERE table_schema = @schema_name
      AND table_name = 'character_boss_archive_copy'
      AND index_name = 'PRIMARY'
      AND column_name = 'item_guid'
      AND seq_in_index = 1
);

SET @sql = IF(
    @has_target_pk = 0,
    'ALTER TABLE `character_boss_archive_copy`
        DROP PRIMARY KEY,
        ADD PRIMARY KEY (`item_guid`)',
    'SELECT 1'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_old_unique = (
    SELECT COUNT(*)
    FROM information_schema.statistics
    WHERE table_schema = @schema_name
      AND table_name = 'character_boss_archive_copy'
      AND index_name = 'uk_archive_item_guid'
);

SET @sql = IF(
    @has_old_unique > 0,
    'ALTER TABLE `character_boss_archive_copy`
        DROP INDEX `uk_archive_item_guid`',
    'SELECT 1'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_owner_entry_index = (
    SELECT COUNT(*)
    FROM information_schema.statistics
    WHERE table_schema = @schema_name
      AND table_name = 'character_boss_archive_copy'
      AND index_name = 'idx_archive_owner_entry'
);

SET @sql = IF(
    @has_owner_entry_index = 0,
    'ALTER TABLE `character_boss_archive_copy`
        ADD KEY `idx_archive_owner_entry` (`guid`, `item_entry`)',
    'SELECT 1'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;