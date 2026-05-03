---
title: Topic Infos
nav_order: 2
---

# Topic Infos

`topicInfos` replacements modify individual NPC-spoken lines at runtime — replacing the subtitle text, the voice file, or both.

A **Topic Info** is a single NPC dialogue entry that can contain one or more sequential response lines (each one a separate spoken sentence). DDR lets you override these responses without touching the original plugin.

## YAML Shape

```yaml
topicInfos:
  - id: 0x2ABB7|Skyrim.esm
    priority: 100
    responses:
      - keep: true
      - subtitle: "A new subtitle for response 2"
        path: "$voices/[VOICE_TYPE]/myline.fuz"
    voices:
      - MaleNord
      - FemaleNord
    conditions:
      - GetIsReference Player == 0 AND
    random: false
    cut: true
```

## Fields

### id

- Type: form identifier
- Required: yes
- Description: Topic Info form to override.

### responses

- Type: list
- Required: yes (must contain at least one item)
- Description: List of response replacements. The first entry applies to the first spoken line, the second to the second, and so on.

Each response item supports:

- `keep` (bool, default `false`): if `true`, this response line is left unchanged.
- `subtitle` (string, default empty): the new subtitle text for this response.
- `sub` (string): shorthand alias for `subtitle`.
- `path` (string, default empty): path to a replacement voice file (`.fuz`).

### priority

- Type: integer
- Default: `0`
- Description: higher values win over lower values.

### voices

- Type: list of voice type editor IDs
- Default: empty (applies to all voice types)
- Description: restricts this replacement to actors using one of the listed voice types.

Values must be voice type editor IDs (e.g. `MaleNord`, `FemaleEvenToned`), not form ID strings.

### conditions

- Type: list of condition strings
- Default: empty
- Description: additional runtime checks that must pass.

See [Conditions](conditions.md).

### random

- Type: bool
- Default: `false`
- Description: if `true`, this replacement enters a random pool with other equally-prioritised `random` entries for the same Topic Info, and one is picked at runtime. If `false`, this replacement is applied immediately when it matches.

### cut

- Type: bool
- Default: `true`
- Description: if your replacement has fewer response entries than the original, controls whether extra original responses are removed.

- `true`: extra original responses are cut.
- `false`: extra original responses are kept.

## Voice Path Variables

If `path` starts with `$`, DDR expands placeholders in each path segment:

- `[VOICE_TYPE]`
- `[TOPIC_MOD_FILE]`
- `[TOPIC_INFO_MOD_FILE]`
- `[VOICE_MOD_FILE]`

Example:

```yaml
path: "$my_pack/[VOICE_TYPE]/[TOPIC_INFO_MOD_FILE]/line_01.fuz"
```

If `path` does not start with `$`, it is used as-is.

## Selection Behavior

When DDR finds multiple entries targeting the same Topic Info:

1. All valid entries (conditions met, voice type matched) are considered.
2. The entry with the highest `priority` wins.
3. If the winning entry has `random: false`, it is applied immediately.
4. If there are multiple winning entries all marked `random: true` at the same priority, one is chosen at random.

## Examples

### Replace a subtitle only

Change the subtitle text on a single-response NPC line, for all voice types:

```yaml
topicInfos:
  - id: 0x1234|MyMod.esp
    responses:
      - subtitle: "Greetings, traveler. What brings you here?"
```

---

### Replace only the second response, leave the first untouched

When a Topic Info has multiple sequential lines, use `keep: true` to skip the ones you don't need to change:

```yaml
topicInfos:
  - id: 0x5678|MyMod.esp
    responses:
      - keep: true
      - subtitle: "Actually, I changed my mind."
```

---

### Swap a voice file using voice-type variables

Use a `$`-prefixed path with `[VOICE_TYPE]` so DDR picks the right voice folder automatically:

```yaml
topicInfos:
  - id: 0xABCD|MyMod.esp
    responses:
      - subtitle: "I have seen things you would not believe."
        path: "$MyVoicePack/[VOICE_TYPE]/MyMod.esp/0xABCD_00.fuz"
```

---

### Restrict to specific voice types

Only apply when the speaker uses one of the listed voice types:

```yaml
topicInfos:
  - id: 0x2ABB7|Skyrim.esm
    voices:
      - MaleNord
      - MaleEvenToned
    responses:
      - subtitle: "Nord-only subtitle override."
```

---

### Random pool — one of several alternatives chosen at runtime

Create multiple entries at the same priority with `random: true` to let DDR pick one each time the line plays:

```yaml
topicInfos:
  - id: 0x9999|MyMod.esp
    priority: 10
    random: true
    responses:
      - subtitle: "Variant A."

  - id: 0x9999|MyMod.esp
    priority: 10
    random: true
    responses:
      - subtitle: "Variant B."

  - id: 0x9999|MyMod.esp
    priority: 10
    random: true
    responses:
      - subtitle: "Variant C."
```

---

### Conditional override (higher priority when condition passes, fallback otherwise)

Apply a special subtitle only when the player is not detected, fall back to a generic one otherwise:

```yaml
topicInfos:
  - id: 0x1111|MyMod.esp
    priority: 100
    conditions:
      - GetDetected Player == 0 AND
    responses:
      - subtitle: "Ah, how dare you sneak up on me!?"

  - id: 0x1111|MyMod.esp
    priority: 0
    responses:
      - subtitle: "Greetings stranger."
```
