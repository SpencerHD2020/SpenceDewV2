# NPC & Dialogue Authoring Guide

This guide explains, step by step, how to **add NPCs** to the game and **write their
dialogue** — no C++ knowledge required beyond copy-pasting one line into a spawn
table. Everything here is data-driven: dialogue lives in JSON files under
`assets/dialogue/`, and social state (affection, flags) is tracked automatically.

---

## 1. The NPC class hierarchy (what each one is for)

```
NPC                     (base: position, sprite, movement — never used directly)
├── Enemy               combat AI (unchanged; not social)
└── Friend              a talk-to-able NPC with one dialogue
    └── PotentialPartner a Friend that can be romanced (affection tiers, gifts)
```

- Use a **`Friend`** for a villager/shopkeeper who just talks.
- Use a **`PotentialPartner`** for anyone romanceable — it adds affection **tiers**
  that change their greeting as your relationship grows.

You never edit the classes to add content. You only:
1. add a line to the **spawn table** (§2), and
2. create a **dialogue file** (§3).

---

## 2. Adding a new NPC instance

NPCs are created in [`src/main.cpp`](src/main.cpp) in the section marked:

```cpp
// >>> To add a new NPC: append one entry to the spawn table below ...
```

There are two helpers already defined there:

```cpp
// A romanceable NPC:
addPartner({worldX, worldY}, "id", "Display Name", "dialogueId");

// A plain, non-romanceable NPC:
addFriend({worldX, worldY}, "id", "Display Name", "dialogueId");
```

### The four arguments
| Argument | Meaning | Notes |
|---|---|---|
| `{worldX, worldY}` | Spawn position in world pixels | Same coordinate space as the player/enemies |
| `"id"` | **Unique** internal key | Used for save data & affection. Never reuse an id. Lowercase, no spaces. |
| `"Display Name"` | Name shown on the dialogue box | Free text |
| `"dialogueId"` | Which dialogue file to open | Loads `assets/dialogue/<dialogueId>.json` |

### Example — add a blacksmith named Bram
1. Open `src/main.cpp`, find the spawn table, and add:
   ```cpp
   addFriend({playerSpawn.x - 96.0f, playerSpawn.y}, "bram", "Bram", "bram");
   ```
2. Create `assets/dialogue/bram.json` (see §3).
3. Rebuild. Walk up to Bram and press **E**.

### Placing NPCs from LDtk instead of code (optional)
If you'd rather place NPCs visually in the LDtk editor, add entities named
**`NpcSpawn`** (→ becomes a `Friend`) or **`PartnerSpawn`** (→ becomes a
`PotentialPartner`) to the `Entities` layer. The **first** `PartnerSpawn` is used
as Willow's position automatically. Extra markers spawn generic NPCs you can then
rename in the spawn table. (Per-NPC id/name/dialogue from LDtk custom fields is not
parsed yet — code is still the source of truth for those.)

---

## 3. Writing a dialogue file

A dialogue is a JSON file at `assets/dialogue/<dialogueId>.json`. It's a bag of
named **nodes**. Each node shows **one line of text** and offers **choices**.

### Minimal example
```json
{
    "id": "bram",
    "start": "greet",
    "nodes": {
        "greet": {
            "speaker": "Bram",
            "text": "Need something forged? Speak up.",
            "choices": [
                { "text": "Just looking.", "next": "END" },
                { "text": "Teach me smithing?", "next": "lesson", "affection": 1 }
            ]
        },
        "lesson": {
            "speaker": "Bram",
            "text": "Hah! Come back when your hands are rough enough.",
            "choices": [
                { "text": "Fair enough.", "next": "END" }
            ]
        }
    }
}
```

### Node fields
| Field | Required | Meaning |
|---|---|---|
| `speaker` | optional | Name shown on the box. If omitted, the NPC's Display Name is used. |
| `text` | yes | The line of dialogue. Use `\n` for a manual line break; long lines wrap automatically. |
| `choices` | optional | List of options. **If omitted or empty, the node is a "press E to continue" line that ends the conversation.** |

### Choice fields
| Field | Required | Default | Meaning |
|---|---|---|---|
| `text` | yes | — | The option label the player sees |
| `next` | yes | `"END"` | Node id to jump to. Use `"END"` to close the conversation. |
| `affection` | optional | `0` | Added to this NPC's affection when chosen (can be negative) |
| `requiresAffection` | optional | `0` | Choice is **hidden** unless affection ≥ this value (gate special options) |
| `setFlag` | optional | — | Sets a named boolean flag on this NPC (e.g. `"confessed"`) |

> **Rule of thumb:** every path should eventually reach a choice whose `next` is
> `"END"`, or a node with no choices. Otherwise the player can't leave.

---

## 4. Romance tiers (PotentialPartner only)

As affection rises, a partner can greet you differently. The runner picks the
**entry node** based on the current tier:

| Tier | Reached when affection ≥ | Entry node it looks for |
|---|---|---|
| 0 | (default) | `start` (or the file's `"start"` value) |
| 1 | `tier1Threshold` (default **10**) | `start_tier1` |
| 2 | `tier2Threshold` (default **25**) | `start_tier2` |

If a tier's entry node doesn't exist, it **falls back** to the next lower tier,
down to `start`. So you can add tiers gradually — a file with only `start` still
works at every affection level.

To change the thresholds for a specific partner, set them right after spawning in
`main.cpp` (the spawn helper returns nothing, so add a dedicated block if needed),
or adjust the defaults in
[`src/core/entities/friends/PotentialPartner.h`](src/core/entities/friends/PotentialPartner.h).

See [`assets/dialogue/willow.json`](assets/dialogue/willow.json) for a complete
three-tier example.

### Authoring pattern for tiers
```jsonc
{
    "id": "willow",
    "start": "greet",             // tier 0 — first meetings
    "nodes": {
        "greet":       { "...": "..." },
        "start_tier1": { "...": "..." },   // shown once affection ≥ 10
        "start_tier2": { "...": "..." }    // shown once affection ≥ 25
    }
}
```

---

## 5. How affection & flags persist

- Affection and flags are stored per-NPC (keyed by the `"id"` you gave) in the
  **`RelationshipManager`**.
- They are **saved automatically** to `relationships.sav` (next to the executable)
  when the game closes, and loaded on startup.
- The flag `"met"` is set automatically the first time you talk to anyone.
- To reset all relationships during testing, delete `relationships.sav`.

---

## 6. Controls

| Key | Action |
|---|---|
| **E** | Talk to the nearest NPC in range / advance dialogue / confirm choice |
| **W / S** or **↑ / ↓** | Move the choice selection |
| **Enter / Space** | Also confirm |

While a conversation is open the world is **paused** (no combat), and it resumes
when the dialogue ends.

---

## 7. Quick checklist for a new romanceable NPC

1. [ ] Add `addPartner({x, y}, "id", "Name", "id");` to the spawn table in `main.cpp`.
2. [ ] Create `assets/dialogue/id.json` with at least a `start` node.
3. [ ] (Optional) Add `start_tier1` / `start_tier2` nodes for higher affection.
4. [ ] Use `"affection"` on choices so the player can build the relationship.
5. [ ] Rebuild, walk up, press **E**, and test each branch.

---

## 8. Common mistakes

| Symptom | Cause / Fix |
|---|---|
| "Cannot open" in console | Dialogue file name doesn't match `dialogueId` (case-sensitive) or isn't in `assets/dialogue/`. |
| Conversation won't close | A node's `next` points to a node with no `"END"` path. Add an exit choice. |
| A choice never appears | Its `requiresAffection` is higher than the current affection. |
| Tier greeting never changes | Affection hasn't crossed the threshold, or the `start_tierN` node is misspelled. |
| Two NPCs share progress | They were given the **same `"id"`**. Ids must be unique. |
