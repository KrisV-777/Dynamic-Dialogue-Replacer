---
title: Scripting
nav_order: 5
---

# Scripting

`scripts` entries run Lua against dialogue text at runtime.

Scripts are loaded from:

- `Data/SKSE/DynamicDialogueReplacer/Scripts`

Each script file must define:

```lua
function replace(text)
  return text
end
```

If `replace` throws an error or returns a non-string value, DDR logs the failure and keeps the current text unchanged.

## YAML Shape

```yaml
scripts:
  - script: myScript.lua
    speaker: 0x000198BA|Skyrim.esm
    target: 0x00000014|Skyrim.esm
    type: 0
```

## Fields

### script

- Type: string
- Required: yes
- Description: filename/path relative to `Data/SKSE/DynamicDialogueReplacer/Scripts`.

### type

- Type: integer
- Required: yes
- Allowed values:
  - `0`: any dialogue
  - `1`: topic/player line context
  - `2`: topicInfo/NPC response context

If `type` is missing or outside `0-2`, loading fails for that script entry.

### speaker

- Type: form identifier
- Default: `0`
- Description: optional filter; only runs when speaker form ID matches.

Supported formats are numeric form IDs (`0xFORM|Plugin` or decimal). A value of `0` disables this filter.

### target

- Type: form identifier
- Default: `0`
- Description: optional filter; only runs when target form ID matches.

Same parsing rules as `speaker`.

## Script Lifecycle

1. DDR loads each YAML script entry.
2. DDR creates an isolated Lua environment for the script (state is preserved per script between calls).
3. DDR loads the script file and verifies `replace` exists.
4. On each dialogue line, DDR checks `type`/`speaker`/`target` filters.
5. DDR sets runtime globals (`context`, `speaker_id`, `target_id`) and calls `replace(text)`.
6. If a string is returned, it becomes the new text passed to the next script.

## Runtime Variables

DDR sets these globals in each script environment before `replace` runs:

- `context`: `1` for topic context, `2` for response context
- `speaker_id`: runtime speaker form ID, or `0`
- `target_id`: runtime target form ID, or `0`

`context` maps to the runtime call site, not the YAML `type` filter directly:

- `1`: topic text pass
- `2`: topic info response text pass

## Built-in Functions

### Function Reference

| Function | Signature | Return | Implementation Notes |
|---|---|---|---|
| `get_formid` | `get_formid(formId, pluginName)` | `number` | Returns full form ID resolved from local ID + plugin name. Returns `0` if unresolved. |
| `has_keyword` | `has_keyword(formId, keywordName, partialMatch)` | `1`, `0`, or `-1` | Checks whether a form has a keyword. `partialMatch=true` allows substring matching. |
| `is_in_faction` | `is_in_faction(actorFormId, factionFormId)` | `1`, `0`, or `-1` | Requires both actor and faction to resolve. |
| `get_faction_rank` | `get_faction_rank(actorFormId, factionFormId)` | rank or `-1` | Returns faction rank for the actor. |
| `has_magic_effect` | `has_magic_effect(actorFormId, effectFormId)` | `1`, `0`, or `-1` | Checks whether the actor currently has the effect. |
| `get_relationship_rank` | `get_relationship_rank(actorFormId, otherActorFormId)` | `string` | Returns relationship rank name, or empty string if none. |
| `get_sex` | `get_sex(formId)` | `0`, `1`, or `-1` | Supports actor or NPC forms. `0=male`, `1=female`. |
| `get_name` | `get_name(formId)` | `string` | Returns form name; for actors can fall back to actor base name; empty string if unresolved. |
| `read_text_file` | `read_text_file(relativePath)` | `string` | Reads UTF-8/binary-safe text from DDR data root; returns empty string on error and logs reason. |
| `read_json_file` | `read_json_file(relativePath)` | `table`/value/`nil` | Parses JSON and converts to Lua objects recursively; returns `nil` on error and logs reason. |
| `send_mod_event` | `send_mod_event(eventName, stringArg, numericArg, formId)` | none | Sends SKSE mod callback event; `formId=0` sends nil form pointer. |
| `log_info` | `log_info(message)` | none | Writes `Lua - <scriptName> - <message>` to DDR log at info level. |
| `log_error` | `log_error(message)` | none | Writes `Lua - <scriptName> - <message>` to DDR log at error level. |

### Return-Value Conventions

Most query-style helpers follow these conventions:

- `1` means true/present
- `0` means false/not present
- `-1` means invalid input or failed form lookup

String-returning helpers usually return an empty string on failure.

### JSON to Lua Mapping

`read_json_file` converts JSON values using this mapping:

| JSON | Lua |
|---|---|
| object | table with string keys |
| array | table with 1-based numeric keys |
| string | string |
| integer/unsigned | number |
| float | number |
| boolean | boolean |
| null | nil |

## Lua Standard Libraries

DDR opens these Lua libraries:

- `base`
- `package`
- `string`
- `table`
- `math`

No additional standard libs are enabled by default.

## Safety Notes

- File reads are sandboxed to `Data/SKSE/DynamicDialogueReplacer`.
- Absolute paths and path traversal outside this root are rejected.
- Scripts are skipped if the file does not exist or does not define `replace`.

## Examples

### Keyword-Gated Rewrite

```lua
function replace(text)
  if has_keyword(speaker_id, "ActorTypeNPC", false) == 1 then
    return "[NPC] " .. text
  end
  return text
end
```

### Load JSON Once and Reuse

```lua
local db = read_json_file("myData/dialogue_map.json") or {}

function replace(text)
  local key = tostring(speaker_id)
  if db[key] ~= nil then
    return db[key]
  end
  return text
end
```

### Emit Mod Event on Specific Context

```lua
function replace(text)
  if context == 2 then
    send_mod_event("DDR_ResponseSeen", text, 0, speaker_id)
  end
  return text
end
```
