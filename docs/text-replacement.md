---
title: Text Replacement
nav_order: 6
---

# Text Replacement

Dynamic Dialogue Replacer supports inline text replacement in dialogue strings using tags. These tags are resolved at runtime against live game data, letting you embed actor names, relationship labels, global variable values, and more directly in your replacement text. This system is a direct alternative to the default [text replacement](https://ck.uesp.net/wiki/Text_Replacement).

Tags use angle bracket syntax: `<Root=Argument>` or `<Root.Subtag=Argument>`.

If a tag cannot be resolved (e.g. the form does not exist or the actor has no name), it is replaced with `[...]`.
Unknown or malformed tags are left unchanged.

---

## Form ID Format

Arguments that identify a form accept either of two formats:

| Format | Example | Notes |
|--------|---------|-------|
| `FormID\|Plugin` | `0x14\|Skyrim.esm` | Hex form ID with plugin name |
| Editor ID | `PlayerRef` | The form's editor ID string |
| `Player` | (special keyword) | Always resolves to the player character |

---

## Ref Tags

Ref tags resolve a reference (actor or object) and insert information about it.

**Syntax:** `<Ref=FormID>` or `<Ref.Subtag=FormID>`

### Subtags

| Tag | Output | Example |
|-----|--------|---------|
| `<Ref=FormID>` | Full display name of the reference | `Lydia` |
| `<Ref.ShortName=FormID>` | Short name from actor base; falls back to full name | `Lydia` |
| `<Ref.Race=FormID>` | Race name of the actor | `Nord` |
| `<Ref.Pronoun=FormID>` | Subject pronoun based on sex | `he` / `she` |
| `<Ref.PronounObj=FormID>` | Object pronoun | `him` / `her` |
| `<Ref.PronounPos=FormID>` | Possessive pronoun (independent) | `his` / `hers` |
| `<Ref.PronounPosObj=FormID>` | Possessive pronoun (attributive) | `his` / `her` |
| `<Ref.PronounRef=FormID>` | Reflexive pronoun | `himself` / `herself` |
| `<Ref.PronounInt=FormID>` | Intensive pronoun (same as reflexive) | `himself` / `herself` |

### Player Shorthand

`<Ref=Player>` always resolves to the player character, regardless of the current playthrough's name or race.

### Examples

```
You fight well, <Ref=Player>. Come, let me show you to <Ref.PronounPos=0x000A2C8E|Skyrim.esm> quarters.
```
→ `You fight well, Dovahkiin. Come, let me show you to his quarters.`

```
<Ref=Player> is a <Ref.Race=Player>.
```
→ `Dovahkiin is a Nord.`

---

## Cap Suffix

Any subtag name can be suffixed with `Cap` to capitalize the first letter of the output. This is useful at the start of a sentence.

| Tag | Output |
|-----|--------|
| `<Ref.PronounCap=FormID>` | `He` / `She` |
| `<Ref.PronounObjCap=FormID>` | `Him` / `Her` |
| `<Ref.RaceCap=FormID>` | `Nord`, `Bosmer`, etc. |

`Cap` works on all tag families: `Ref`, `Relationship`, and `Global`.

### Example

```
<Ref.PronounCap=Player> doesn't look like much of a hero to me.
```
→ `He doesn't look like much of a hero to me.`

---

## Relationship Tags

Relationship tags look up the relationship between two actors using the game's association type system (e.g. Spouse, Parent, Friend) and insert the appropriate label for the first actor.

**Syntax:** `<Relationship.LeftRef=RightRef>`

- `LeftRef` — the actor whose label you want (e.g. "she is the _wife_ of RightRef")
- `RightRef` — the other actor in the relationship

The label is gendered based on `LeftRef`'s sex.

### Example

```
Did you know that <Ref=0x0001A67C|Skyrim.esm> is <Ref.PronounPos=0x0001A67C|Skyrim.esm> <Relationship.0x0001A67C|Skyrim.esm=Player>?
```
→ `Did you know that Ysolda is her spouse?`

> **Note:** The relationship must exist in the game's BGSRelationship records with a valid association type. If it does not, the tag resolves to `[...]`.

---

## Global Tags

Global tags read the current value of a `TESGlobal` variable. They support formatting subtags for displaying in-game date and time values stored as game-days (a float where 1.0 = one full day).

**Syntax:** `<Global=FormID>` or `<Global.Subtag=FormID>`

### Default (no subtag)

| Tag | Output |
|-----|--------|
| `<Global=FormID>` | Float globals: value formatted to 2 decimal places. Integer globals: rounded whole number. |

### Date and Time Subtags

These subtags interpret the global's value as a game-days float (like `GameDaysPassed`):

| Subtag | Output | Example |
|--------|--------|---------|
| `Hour12` | Current hour in 12-hour format | `3` |
| `Minutes` | Current minutes | `45` |
| `Meridiem` | AM or PM | `PM` |
| `Time` | Elapsed time (hours or minutes) | `87 Hours` / `30 Minutes` |
| `TimeSpan` | Time of day name | `Morning`, `Afternoon`, `Evening`, `Night` |
| `Day` | Day of the month (1–30) | `15` |
| `WeekDay` | Name of the day | `Fredas` |
| `Month` | Month number (1–12) | `9` |
| `MonthWord` | Name of the month | `Hearthfire` |
| `Year` | In-game year (approximate) | `201` |

#### TimeSpan ranges

| Hour | Label |
|------|-------|
| 0–5 | Night |
| 6–11 | Morning |
| 12–17 | Afternoon |
| 18–23 | Evening |

#### WeekDay names

Morndas, Tirdas, Middas, Turdas, Fredas, Loredas, Sundas

#### MonthWord names

Morning Star, Sun's Dawn, First Seed, Rain's Hand, Second Seed, Midyear, Sun's Height, Last Seed, Hearthfire, Frostfall, Sun's Dusk, Evening Star

### Examples

```
The time is <Global.Hour12=GameDaysPassed>:<Global.Minutes=GameDaysPassed> <Global.Meridiem=GameDaysPassed>.
```
→ `The time is 3:45 PM.`

```
Today is <Global.WeekDay=GameDaysPassed>, the <Global.Day=GameDaysPassed>th of <Global.MonthWord=GameDaysPassed>.
```
→ `Today is Fredas, the 15th of Hearthfire.`

```
Current gold: <Global=Gold001>
```
→ `Current gold: 500`

---

## Full Syntax Reference

```
<Root=Argument>
<Root.Subtag=Argument>
<Root.SubtagCap=Argument>
```

| Root | Argument | Subtag |
|------|----------|--------|
| `Ref` | Form ID or `Player` | _(empty)_, `ShortName`, `Race`, `Pronoun`, `PronounObj`, `PronounPos`, `PronounPosObj`, `PronounRef`, `PronounInt` |
| `Relationship` | Right ref Form ID | Left ref Form ID (as subtag) |
| `Global` | Form ID | _(empty)_, `Hour12`, `Minutes`, `Meridiem`, `Time`, `TimeSpan`, `Day`, `WeekDay`, `Month`, `MonthWord`, `Year` |

Tags are case-insensitive. The `Cap` suffix can be appended to any subtag name.
