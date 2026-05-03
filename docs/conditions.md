---
title: Conditions
nav_order: 4
---

# Conditions

Conditions control whether a `topicInfos`, `topics`, or `scripts` entry is active.

## YAML Shape

```yaml
conditions:
  - GetIsReference Player == 0 AND
  - IsInSameCurrentLocAsRef Player 0x198BA|Skyrim.esm == 1
```

If `conditions` is empty or omitted, the entry is treated as always valid.

## Grammar

DDR parses each line using this form:

```text
[Subject <>] Function [Arg1] [Arg2] Operator Comparand [AND|OR]
```

Examples:

```yaml
conditions:
  - GetDead == 0 AND
  - GetInFaction 0x0005C84D|Skyrim.esm == 1
  - 0x000198BA|Skyrim.esm <> GetIsReference Player == 0
```

## Token Reference

| Token | Required | Meaning |
|---|---|---|
| `Subject <>` | no | Optional object to run condition on (must resolve to a reference). |
| `Function` | yes | Creation Kit condition function name. (See the [CK Wiki](https://ck.uesp.net/wiki/Condition_Functions)) |
| `Arg1` | no | First function argument. |
| `Arg2` | no | Second function parameter. |
| `Operator` | yes | One of `==`, `!=`, `>`, `>=`, `<`, `<=`. |
| `Comparand` | yes | Numeric value or global variable reference. |
| `AND`/`OR` | no | Connective to next condition item. (Default: `AND`) |

## refMap

You can define reusable aliases at file scope:

```yaml
refMap:
  player: Player
  guard: 0x000D8D7A|Skyrim.esm
  whiterunFaction: GuardFactionWhiterun
```

Then use aliases in conditions and form fields:

```yaml
conditions:
  - GetIsReference player == 0 AND
  - IsInFaction whiterunFaction == 1
```

`refMap` keys are case-insensitive. DDR also predefines `player` automatically.

### Ref Resolution Order

When resolving condition refs/params, DDR uses:

1. `refMap` alias lookup (case-insensitive)
2. Form string parsing (`0xFORM|Plugin`, decimal)
3. Editor ID lookup (for supported form types)

## Parameter Notes

- Arguments are parsed according to the selected condition function.
- Numeric arguments should be plain numbers.
- Form arguments can be form IDs, editor IDs, or `refMap` aliases.
- Extra arguments beyond what a function supports are ignored.

## Comparand Rules

`Comparand` is interpreted as:

- a `TESGlobal` reference if lookup succeeds, otherwise
- a numeric literal

So these are valid:

```yaml
conditions:
  - GetLevel == 10
  - GetStage MyQuest == MyGlobalVariable
```

## Evaluation Semantics

Conditions are compiled into a linked list and evaluated in order.

- Non-`OR` items are evaluated directly.
- `OR` marks start/continuation of an OR-group.
- An OR-group passes if any item in that contiguous group is true.
- Overall evaluation short-circuits on first failing segment.

### Important OR Behavior

`OR` belongs to the current line and links it to the following condition line(s).

Example:

```yaml
conditions:
  - GetDead == 0 OR
  - GetInFaction GuardFactionWhiterun == 1 AND
  - GetLevel > 5
```

The first two lines are grouped as OR alternatives, then combined with the third via AND-style progression.

## Subject (`<>`) Behavior

If `Subject <>` is provided, that condition is evaluated on the specified reference.

If `Subject <>` is omitted, DDR uses the default subject for the current dialogue check. This is usually the currently speaking/active dialogue actor.

If subject resolution fails, parsing throws and the owning entry is rejected.

## GetVMQuestVariable Format

For `GetVMQuestVariable`, use this argument format:

- `<script>::<property>`

Example:

```yaml
conditions:
  - GetVMQuestVariable MQ101 ScriptName::StageValue >= 20
```

Using the wrong variable format will fail the condition.

## Failure Modes

- Unknown/invalid condition function: parse failure
- Regex mismatch / malformed line: parse failure
- Invalid subject ref (`<>`): parse failure
- Invalid argument/comparison values: parse failure

A parse failure in any condition line rejects that entire replacement entry during load.

## Formatting Guidelines

- Keep one condition expression per YAML list item.
- Use consistent spacing between tokens.
- Use uppercase `AND`/`OR` for readability.
- Prefer `refMap` aliases for maintainability in large packs.

## Diagnostic Tips

- Check `DynamicDialogueReplacer.log` for parse errors with YAML line numbers.
- Validate function names against CK condition function names.
- If a global comparand is intended, make sure it resolves to a valid `TESGlobal`.

## Quick Examples

### Basic AND Chain

```yaml
conditions:
  - GetDead == 0 AND
  - GetInFaction GuardFactionWhiterun == 1
```

### Subjected Condition

```yaml
conditions:
  - player <> GetDistance guard < 200
```

### Quest Variable Condition

```yaml
conditions:
  - GetVMQuestVariable MQ101 ScriptName::StageValue >= 20
```
