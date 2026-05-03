---
title: Topics
nav_order: 3
---

# Topics

`topics` replacements modify player dialogue options and response-topic flow.

## YAML Shape

```yaml
topics:
  - id: 0x267DE|Skyrim.esm
    affects: 0x267DC|Skyrim.esm
    replace: 0x123|Example.esp
    text: "Updated player response text"
    inject:
      - 0x456|Example.esp
    conditions:
      - GetIsReference Player == 0 AND
    priority: 100
    proceed: true
    check: false
    hide: false
```

## Fields

### id

- Type: form identifier
- Default: `0`
- Description: parent topic where this replacement is evaluated.

If omitted (`0`), the entry is treated as an orphan replacement and keyed by `affects`.

### affects

- Type: form identifier
- Default: `0`
- Description: response topic inside the parent topic that this entry targets.

### replace

- Type: form identifier
- Default: `0`
- Description: replacement topic to use instead of `affects`.


### text

- Type: string
- Default: empty
- Description: replacement text for the player option represented by `id`.

### inject

- Type: list of topic form identifiers
- Default: empty
- Description: inject additional response topics into the parent (`id`) topic.

### conditions

- Type: list of condition strings
- Default: empty
- Description: runtime checks required for this entry.

See [Conditions](conditions.md).

### priority

- Type: integer
- Default: `0`
- Description: higher values are evaluated first.

### proceed

- Type: bool
- Default: `true`
- Description: whether processing continues to later matching entries.

### check

- Type: bool
- Default: `false`
- Description: requires at least one originally valid response topic before applying this edit.

### hide

- Type: bool
- Default: `false`
- Description: hide the `affects` topic.

## Validation Rules

A topic entry must produce at least one action:

- non-empty `text`, or
- valid `replace`, or
- non-empty `inject`, or
- `hide: true`

Additional enforced constraints:

- `replace` requires `affects`.
- `hide` requires `affects`.
- `hide` cannot be combined with `replace`.
- `inject` requires `id`.
- If `id` is non-zero, it must resolve to a valid topic form.

## Examples

### Replace player-choice text

```yaml
topics:
  - id: 0x267DE|Skyrim.esm
    text: "Tell me more about the Companions."
```

### Swap the response topic for a different one

```yaml
topics:
  - id: 0x267DE|Skyrim.esm
    affects: 0x267DC|Skyrim.esm
    replace: 0x123|MyMod.esp
```

### Inject an extra topic conditionally

```yaml
topics:
  - id: 0x267DE|Skyrim.esm
    inject:
      - 0x456|MyMod.esp
    conditions:
      - GetLevel >= 20
```

### Layered priority: strict override with a fallback

Use a high-priority entry for specific cases and a lower-priority entry as the general fallback:

```yaml
topics:
  - id: 0x267DE|Skyrim.esm
    text: "(Companion member line)"
    priority: 100
    conditions:
      - GetInFaction CompanionsFaction == 1

  - id: 0x267DE|Skyrim.esm
    text: "Tell me about the Companions."
    priority: 0
```
