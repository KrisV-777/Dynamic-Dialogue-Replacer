---
title: Home
nav_order: 1
---

# Dynamic Dialogue Replacer

**Dynamic Dialogue Replacer (DDR)** is an SKSE64 plugin for Skyrim Special Edition that lets you override dialogue subtitles and voice files at runtime without editing the original plugin.

You provide YAML configuration files, and DDR applies them every time the matching dialogue is triggered in-game. No Creation Kit required. Changes take effect on load.

## What You Can Do

- **Replace NPC subtitles** - Change the text shown when an NPC speaks, on a per-line basis
- **Swap voice files** - Redirect a line to a different audio file at runtime
- **Edit player responses** - Change the text of a player dialogue option, hide it, swap it for another, or inject new options
- **Write Lua scripts** - Run code that transforms subtitle text dynamically based on game state
- **Use conditions** - Gate any replacement on arbitrary condition checks (faction, quest stage, actor value, and more)

## How It Works

DDR loads all `.yml` / `.yaml` files found in:

```
Data/SKSE/DynamicDialogueReplacer/
```

Each file can define any combination of three sections:

| Section | Purpose |
|---|---|
| `topicInfos` | Override NPC response subtitles and voice paths |
| `topics` | Edit player dialogue options |
| `scripts` | Run Lua scripts on dialogue lines |

Files are loaded on game start. No in-game command is needed.

## Pages

- [Topic Infos](topic-infos.md)
- [Topics](topics.md)
- [Conditions](conditions.md)
- [Scripting](scripting.md)
- [Text Replacement](text-replacement.md)

## Minimal Config File

A minimal config file can contain any subset of supported sections:

```yaml
topicInfos: []
topics: []
scripts: []
```

You can also define a `refMap` to create named shortcuts for frequently referenced actors, quests, or factions:

```yaml
refMap:
  player: Player
  whiterunGuard: 0x000D8D7A|Skyrim.esm
```

See [Conditions](conditions.md) for full `refMap` documentation.

## Notes

- Files must use `.yml` or `.yaml` extension.
- Unknown top-level keys are ignored by DDR.
- Use `DynamicDialogueReplacer.log` to troubleshoot load/parse errors.
