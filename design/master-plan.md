# Snake: Cruel World - Megaman X Redesign Plan

## Context

The game currently has 10 linear levels, each introducing one hazard mechanic, with no player abilities. The redesign transforms it into a Megaman X-style experience: non-linear stage select, boss fights that grant abilities, and a dark emotional narrative. The goal is to make the player feel they are **building up the snake's power** and experiencing the satisfaction of using earned abilities to overcome previously brutal challenges.

---

## Document Guide

### Status

- Master plan: complete
- Level design documents: complete for `L2` through `L10`
- Phase 1: complete (`feat/phase1-ability-core` merged, with follow-up fixes)
- Phase 2: complete (`feat/phase2-stage-select-progression` merged, with follow-up fixes)
- Remaining implementation roadmap: starts at Phase 3

### Recommended Reading Order

1. Read this file for the overall redesign vision, progression, and ability web.
2. Read the level docs in unlock order or counter-web order, depending on whether you want narrative flow or gameplay flow.
3. Read `Level 10: The Eraser` last, since it synthesizes the full ability set and campaign arc.

### Quick Links

- [Level 2: Lights Out / Ink Flare](levels/level-02-blackout.md)
- [Level 3: Quicksand / Shed Skin](levels/level-03-quicksand.md)
- [Level 4: Mirror Mirror / Shadow Decoy](levels/level-04-mirror.md)
- [Level 5: Famine / Time Freeze](levels/level-05-famine.md)
- [Level 6: Betrayal / Venom Trail](levels/level-06-poison.md)
- [Level 7: Earthquake / Ink Anchor](levels/level-07-earthquake.md)
- [Level 8: The Watcher / Hunter's Dash](levels/level-08-watcher.md)
- [Level 9: Amnesia / Ink Memory](levels/level-09-amnesia.md)
- [Level 10: The Eraser](levels/level-10-eraser.md)

---

## Narrative: "The Unfinished Page"

The Artist started drawing a world in their notebook but **abandoned it** - left it unfinished, imperfect, cruel. The ink on the unfinished pages has gone wild without the Artist's guiding hand. Each page is corrupted not because the Artist is dying, but because **they gave up on it**.

The snake is the one thing the Artist finished with love. And the snake refuses to let its world stay broken. Each boss defeated = a page HEALED. The corruption recedes, the ink calms, and the snake proves its world was worth finishing.

**Level metaphors (what went wrong on each page):**

- L2 Blackout = the Artist stopped drawing the light, so darkness filled the void
- L3 Quicksand = unfinished ink pooled and congealed without structure
- L4 Mirror Ghost = half-drawn ideas became echoes, stuck in loops
- L5 Timed Apples = without the Artist's hand, nothing on this page lasts
- L6 Poison = colors bled into each other, what looks right is wrong
- L7 Earthquake = the page itself is unstable, never given a foundation
- L8 The Watcher = a creature the Artist sketched but never tamed - it grew feral
- L9 Control Shuffle = the rules of this page were never defined, so they shift randomly

**Cutscene beats:** After each boss, a brief cutscene shows the page healing - corruption receding, ink settling into calm lines, beauty emerging from chaos. The Artist's hand appears in the background, hesitating. After L10: the Artist picks up the pen again. The world was worth finishing.

**Emotional arc matches gameplay arc:**

- Early: world is hostile, chaotic, cruel. Snake is small and alone.
- Mid: snake has abilities, pages are healing, momentum and hope building.
- Late: healed pages show glimpses of what the Artist originally intended - beauty from chaos.
- L10 "The Eraser": the Artist's self-doubt made manifest, trying to erase everything.
- Ending: notebook is complete. The Artist sees it, picks up the pen again.

---

## Progression Structure

### Stage Select (after L1 tutorial)

- All 8 levels (L2-L9) available immediately
- Stage select screen shows 8 corrupted notebook pages
- Each page displays: level name, corruption type icon, difficulty hint
- After beating: ability icon overlaid, page "calms" visually (corruption reduced)
- L10 unlocks after all 8 bosses defeated

### Level Structure (each of L2-L9)

1. **Stage phase**: Navigate the level, eat apples (current gameplay with hazard active)
2. **Boss phase**: Arena shrinks, unique boss entity spawns, hazard at maximum
3. **Ability earned**: Cutscene plays, snake absorbs the ink, ability unlocked

### Level 10: "The Eraser"

- 4-phase boss fight (keep existing structure but redesign as single mega-boss)
- Each phase requires creative use of a different ability to survive
- Final phase: all abilities available, all hazards active, the Eraser at full power

---

## The 8 Abilities

All abilities are **cooldown-based** (no resource management). Using an ability **visually transforms the snake** (color, body shape, eye glow) for the duration, like Megaman changing color with each weapon.

| # | Gained From | Ability | Cooldown | Duration | Effect | Strong Counter |
|---|---|---|---|---|---|---|
| 1 | L2: Blackout | **Ink Flare** | 10s | 3s | Pulse illuminates screen + reveals hidden elements (ghost pos, next apple spawn, boss weak points) | Mirror Ghost (L4) |
| 2 | L3: Quicksand | **Shed Skin** | 12s | Instant | Drop 3 body segments; shed segments become temporary walls (5s) blocking enemies/hazards | Earthquake (L7) |
| 3 | L4: Mirror Ghost | **Shadow Decoy** | 12s | 4s | Spawn fake snake head moving opposite direction; enemies/bosses target it | Predator (L8) |
| 4 | L5: Timed Apples | **Time Freeze** | 15s | 4s | Freeze ALL timers (apple expiry, shuffle countdown, predator movement, boss patterns) | Control Shuffle (L9) |
| 5 | L6: Poison | **Venom Trail** | 10s | 3s | Tail leaves poison trail; enemies crossing it are stunned 3s | Predator (L8, secondary) |
| 6 | L7: Earthquake | **Ink Anchor** | 12s | 5s | Lock walls in place + ignore terrain slowdowns | Quicksand (L3) |
| 7 | L8: The Watcher | **Hunter's Dash** | 8s | Instant | Dash 4 tiles forward, skip obstacles, auto-collect apples in path | Timed Apples (L5) |
| 8 | L9: Control Shuffle | **Ink Memory** | 10s | 6s | Lock current controls + purify poison effects instantly | Poison (L6) |

### Counter-Ability Web

```text
Blackout ──(Ink Flare)──► Mirror Ghost
Quicksand ──(Shed Skin)──► Earthquake
Mirror Ghost ──(Shadow Decoy)──► Predator
Timed Apples ──(Time Freeze)──► Control Shuffle
Poison ──(Venom Trail)──► Predator (secondary)
Earthquake ──(Ink Anchor)──► Quicksand
Predator ──(Hunter's Dash)──► Timed Apples
Control Shuffle ──(Ink Memory)──► Poison
```

### Canonical Ability Data

The `8 Abilities` table above and the `Counter-Ability Web` are the canonical source of truth for unlock source, cooldown, duration, core effect, and strong-counter relationships. Each level document should mirror these values exactly unless this master plan is updated first.

Validation step for doc maintenance:

- Add a lightweight doc check in CI or a pre-merge hook that compares every level doc against the master ability table and counter web.
- Validate at minimum: earned ability name, cooldown, duration, primary effect summary, and strong-counter target.
- Surface mismatches as warnings during active design iteration and promote them to failures once implementation begins, so level docs cannot silently drift away from the master spec.

### Visual Transform Per Ability

When activated, the snake's appearance changes for the ability's duration:

- **Ink Flare**: Snake glows bright white/gold, eyes become lanterns, illumination radiates from body
- **Shed Skin**: Snake turns pale/translucent momentarily, shed segments splatter as ink blots
- **Shadow Decoy**: Snake darkens to deep shadow, decoy is a ghostly outline version
- **Time Freeze**: Snake turns crystalline/frozen blue, everything else gets desaturated
- **Venom Trail**: Snake turns toxic green with dripping particles, trail is acid-green ink
- **Ink Anchor**: Snake turns rust-red/iron, body segments become rigid angular shapes
- **Hunter's Dash**: Snake turns cyan with motion blur trail, eyes sharpen to predator slits
- **Ink Memory**: Snake turns violet with stable clean lines (no corruption wobble)

---

## Boss Designs (Unique Ink Entities)

Each boss has 3 hit points (unless noted). Counter-ability deals 2 damage per hit (kills in 2 hits vs 3).

### L2 Boss: "The Blind Ink"

- Arena permanently dark. Giant ink blot pulses with faint glow
- "Light apples" spawn that illuminate area when eaten near the blot
- Eat 3 light apples near the blot to damage it (light burns it)
- Blot tries to absorb light apples first, getting faster each phase

### L3 Boss: "The Mire"

- Entire floor is quicksand except narrow shifting dry paths
- Slug-like creature oozes, leaving permanent quicksand trails
- Lead it into wall corners where it gets stuck, eat apple on its back
- 3 corner-traps to defeat

### L4 Boss: "The Doppelganger"

- Mirror ghost becomes solid aggressive enemy snake
- Copies inputs with decreasing delay (5-tick -> 3-tick -> 1-tick)
- Maneuver so ghost crashes into walls while you survive
- Puzzle boss: find moves where your mirror-image dies but you don't

### L5 Boss: "The Hourglass"

- Hourglass entity at center, apples spawn with 2-second timers
- Eating apple within 0.5s of spawn cracks the hourglass
- 5 cracks to shatter. Miss timing = apple just feeds you normally
- Hourglass speeds up apple decay each phase

### L6 Boss: "The Parasite"

- Poison creature attaches to tail, converts body segments to poison
- Eating real apples pushes parasite back (heals segments)
- Eating poison feeds it (converts more segments)
- 5 consecutive real apples purges it. Parasite can spit, converting nearby reals to poison

### L7 Boss: "The Fault Line"

- Centipede creature moves underground through a crack
- Bursts from ground at semi-random points (visual tells 1.5s before)
- Be near the apple when it surfaces - tremor knocks apple into creature, dealing damage
- 4 surface-hits. After each hit, creature avoids last eruption zone

### L8 Boss: "The Hunter"

- Massive 15-segment predator snake, faster than player
- Arena has "ink pits" (dark patches)
- Lead predator across 3 pits in sequence to trap it
- After each hit, predator avoids previously used pits. 3 trap-sequences to win

### L9 Boss: "The Scrambler"

- Octopus-like entity with 4 tentacles (one per direction)
- Glowing tentacle = that direction about to be remapped
- Eat apples to sever tentacles one at a time
- Each severed tentacle permanently locks that direction to random mapping
- By last tentacle, 3/4 controls are scrambled. 4 severs to win

---

## Hidden Ability Interactions (Replay Value)

Returning to completed levels with different abilities reveals secrets:

- **Quicksand + Hunter's Dash**: Dash over large quicksand clusters to reach hidden apple cluster (bonus score)
- **Blackout + Venom Trail**: Poison trail glows in the dark, creating your own light source
- **Earthquake + Time Freeze**: Freeze walls mid-shake to create shortcut through blocked areas
- **Predator + Shed Skin**: Drop shed skin as bait, predator eats it and gets stunned 3s
- **Timed + Ink Flare**: Flare reveals where the NEXT apple will spawn before current one expires
- **Poison + Shadow Decoy**: Decoy absorbs poison apples for you, converting them to real ones
- **Mirror + Ink Anchor**: Anchor makes you immovable, ghost passes through you harmlessly
- **Control Shuffle + Hunter's Dash**: Dash overrides shuffled controls (always goes the direction you're facing)

These unlock achievements and bonus star ratings.

---

## Implementation Strategy

**Approach**: Design all levels in detail FIRST, then implement. No new engine needed - build on existing SFML foundation.

### Pre-Implementation: Detailed Design Documents

Before any code, create detailed design docs for:

- Each of the 8 levels (stage + boss + ability + cutscene + hidden interactions)
- Core system architectures (Ability framework, Boss base class, Stage Select)
- Level 10 multi-phase boss design
- Narrative/cutscene script

During this phase, treat the master ability table and counter web in this document as the canonical reference, and resolve any per-level discrepancies here before updating the individual level docs.

### Code Implementation Phases (after design is complete)

Progress note: Phases 1 and 2 are already implemented in the main branch. The remaining implementation work below begins at Phase 3, and the Phase 1 / Phase 2 scope should continue to reflect what has already shipped.

#### Phase 1: Ability System Core

- `src/Snake.h/.cpp` - Add ability slot, visual transform state, cooldown tracking
- New: `src/Ability.h/.cpp` - Ability base class + 8 ability implementations
- New: `src/AbilityHUD.h` - Cooldown indicator, equipped ability display
- `src/PlayState.h/.cpp` - Integrate equip / activate ability flow into gameplay and existing hazard orchestration
- `src/LevelConfig.h` - Add `abilityReward` field per level
- `src/StateManager.h` - Track unlocked abilities, equipped ability
- `src/SaveManager.h/.cpp` - Persist unlocked abilities, equipped ability, and any required save-version migration

#### Phase 2: Stage Select & Progression

- New: `src/StageSelectState.h/.cpp` - 8-page stage select screen (replaces linear LevelSelect)
- `src/LevelConfig.h` - Replace linear unlock assumptions with Stage Select presentation metadata
- `src/StateManager.h` - Track per-page campaign progression (`stageSeen`, `stageCompleted`, `bossDefeated`, `pageHealed`)
- `src/SaveManager.h/.cpp` - Migrate old linear saves into the new campaign progression model

#### Phase 3: Boss System Foundation

- New: `src/Boss.h/.cpp` - Boss base class with health, hit patterns, phases, and shared runtime hooks
- New: `src/bosses/` - Initial boss implementations and reusable support code for level-specific boss behavior
- `src/PlayState.h` - Add boss phase transition (stage -> boss arena)
- `src/World.h` - Boss arena mode (fixed size, special spawns)
- `src/LevelConfig.h` - Add `bossConfig` per level on top of the earlier `abilityReward` / Stage Select metadata expansion

#### Phase 4: `L2` Blackout Delivery

- Deliver the full `L2` content slice: blackout stage flow, Blind Ink boss, Ink Flare reward, post-boss cutscene, and replay hooks
- Use `levels/level-02-blackout.md` as the implementation and tuning spec

#### Phase 5: `L3` Quicksand Delivery

- Deliver the full `L3` content slice: quicksand stage flow, Mire boss, Shed Skin reward, post-boss cutscene, and replay hooks
- Use `levels/level-03-quicksand.md` as the implementation and tuning spec

#### Phase 6: `L4` Mirror Delivery

- Deliver the full `L4` content slice: mirror stage flow, Doppelganger boss, Shadow Decoy reward, post-boss cutscene, and replay hooks
- Use `levels/level-04-mirror.md` as the implementation and tuning spec

#### Phase 7: `L5` Famine Delivery

- Deliver the full `L5` content slice: timed-apples stage flow, Hourglass boss, Time Freeze reward, post-boss cutscene, and replay hooks
- Use `levels/level-05-famine.md` as the implementation and tuning spec

#### Phase 8: `L6` Poison Delivery

- Deliver the full `L6` content slice: poison stage flow, Parasite boss, Venom Trail reward, post-boss cutscene, and replay hooks
- Use `levels/level-06-poison.md` as the implementation and tuning spec

#### Phase 9: `L7` Earthquake Delivery

- Deliver the full `L7` content slice: earthquake stage flow, Fault Line boss, Ink Anchor reward, post-boss cutscene, and replay hooks
- Use `levels/level-07-earthquake.md` as the implementation and tuning spec

#### Phase 10: `L8` Watcher Delivery

- Deliver the full `L8` content slice: predator stage flow, Hunter boss, Hunter's Dash reward, post-boss cutscene, and replay hooks
- Use `levels/level-08-watcher.md` as the implementation and tuning spec

#### Phase 11: `L9` Amnesia Delivery

- Deliver the full `L9` content slice: control-shuffle stage flow, Scrambler boss, Ink Memory reward, post-boss cutscene, and replay hooks
- Use `levels/level-09-amnesia.md` as the implementation and tuning spec

#### Phase 12: `L10` Eraser Final Integration

- Redesign `L10` as the Eraser mega-boss finale with multi-phase ability-driven encounter flow
- Implement finale cutscenes, ending integration, and final campaign validation using `levels/level-10-eraser.md`

### Shared Content Requirements For Every Level Phase

- Each level phase must include its stage mechanic, boss implementation, ability reward integration, cutscene integration, secret/replay hooks, balance pass, and regression testing
- Implement ability-specific visual transforms, particles, and screen effects as part of the relevant level phase rather than postponing all polish to a separate late phase
- Add achievements / bonus ratings when the level doc specifies them
- Use the corresponding level design doc as the acceptance spec for final behavior

#### Per-level completion checklist

- Stage phase mechanic implemented and tuned to match the level doc
- Boss phase transition implemented with arena rules, fail states, and victory conditions
- Boss behavior implemented and balanced against intended death count / learning curve
- Ability reward granted correctly, persisted correctly, and surfaced correctly in HUD / stage select
- Post-boss cutscene implemented and hooked into campaign progression
- Hidden interactions and replay hooks implemented where specified
- Ability visuals / particles / screen effects implemented where specified
- Achievements / bonus ratings connected where specified
- Regression test pass completed for save/load, retry flow, replay flow, and campaign unlock state
- Final doc-to-game validation completed against: mechanic rules, apple counts, pacing targets, counter-ability behavior, and narrative beats

#### Exit criteria for the redesign

- All branch levels (`L2-L9`) meet their per-level completion checklist
- `L10` is fully playable as the final multi-phase boss encounter
- Stage select, save data, ability unlocks, cutscenes, achievements, replay interactions, and finale progression function correctly across a full campaign playthrough

---

## Detailed Level Design Documents

- [Level 2: Lights Out / Ink Flare](levels/level-02-blackout.md)
- [Level 3: Quicksand / Shed Skin](levels/level-03-quicksand.md)
- [Level 4: Mirror Mirror / Shadow Decoy](levels/level-04-mirror.md)
- [Level 5: Famine / Time Freeze](levels/level-05-famine.md)
- [Level 6: Betrayal / Venom Trail](levels/level-06-poison.md)
- [Level 7: Earthquake / Ink Anchor](levels/level-07-earthquake.md)
- [Level 8: The Watcher / Hunter's Dash](levels/level-08-watcher.md)
- [Level 9: Amnesia / Ink Memory](levels/level-09-amnesia.md)
- [Level 10: The Eraser](levels/level-10-eraser.md)
