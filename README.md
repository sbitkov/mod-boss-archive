# mod-boss-archive

Boss Archive is an AzerothCore module for World of Warcraft 3.3.5a that turns boss loot into permanent progression.

When a character kills a dungeon boss for the first time, the module records all eligible equipment that boss could have dropped for the active loot mode. The character can then create a recoverable archive copy of any unlocked item without repeatedly farming the boss for RNG drops.

## Current features

- Unlock boss loot on boss kill
- Resolve direct creature loot and referenced loot tables
- Respect the boss's active LootMode
- Ignore quest-required items
- Store unlocked bosses and items per character
- Create archive copies with `.archiveitem`
- Allow only one archive copy of a given item per character at a time
- Restore an archive copy after the previous copy has been destroyed
- Archive copies are automatically Soulbound
- Archive copies cannot be sold to vendors
- Archive copies cannot be disenchanted
- Normal world loot remains unchanged

## Commands

### `.archive`

Lists bosses currently unlocked in the Boss Archive for the character.

### `.archiveboss <bossEntry> [lootMode]`

Lists archived equipment unlocked from the specified boss.

### `.archiveitem <itemEntry>`

Creates an archive copy of an unlocked item.

If an archive copy of that item already exists on the character, another copy will not be created.

If the previous archive copy was destroyed, the module allows it to be restored.

### `.bossloot <bossEntry> [lootMode]`

Developer/debug command that resolves the equipment currently available from a boss's loot tables.

## Archive copy rules

Archive copies are tracked by individual item GUID rather than only by item entry.

This allows the module to distinguish an Archive-issued item from a normal copy of the same item obtained through regular gameplay.

Archive copies are:

- Soulbound
- Not sellable to vendors
- Not disenchantable
- Recoverable after destruction
- Limited to one active archive copy per item entry per character

Normal copies of the same item are not affected by these restrictions.

## Database

The module creates the following character database tables:

- `character_boss_archive`
- `character_boss_archive_item`
- `character_boss_archive_copy`

SQL files are located under:

`data/sql/db-characters/base/`

## Installation

Clone the module into the AzerothCore `modules` directory:

```bash
cd modules
git clone https://github.com/sbitkov/mod-boss-archive.git
```

Reconfigure and rebuild AzerothCore so the module is included in the generated project.

The module has currently been developed and tested against AzerothCore WotLK 3.3.5a.

## Project status

Early development.

The server-side Boss Archive backend is functional. A dedicated client addon/interface is planned for browsing unlocked bosses and items, creating archive copies, and marking archive items in tooltips.

## AI assistance

AI tools were used during development of this project, including assistance with code generation, code review, debugging, documentation, and exploration of AzerothCore APIs.

All generated or suggested code was manually reviewed, integrated, compiled, and tested as part of the development process.

## License

This project is licensed under the GNU General Public License version 2.

See `LICENSE` for the full license text.
