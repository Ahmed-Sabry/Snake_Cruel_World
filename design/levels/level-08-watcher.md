# Level 8: "The Watcher" - Detailed Design Document

> *"A creature the Artist sketched but never tamed. It watched too long and learned hunger."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | The Watcher |
| Subtitle | "It learns." |
| Hazard | Predator (a rival snake that steals apples, grows stronger, and periodically hunts the player directly) |
| Boss | The Hunter |
| Ability Earned | Hunter's Dash |
| Strong Counter Against | Level 5: Famine |
| Narrative Metaphor | The Artist drew a living thing here, but never finished teaching it what it was for. Left alone, it learned only appetite. Observation became obsession. Hunger became instinct. |

---

## Part 1: Stage Phase

### Gameplay

The stage is a tense contest for survival against a rival snake. The Predator shares the arena, races the player to apples, and grows faster each time it wins the race. Every few apples, it stops competing for food and turns its full attention toward the player, creating short panic windows where survival matters more than scoring.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 13.0 | Fast, but not as extreme as Famine |
| Apples to Win | 10 | Existing level target stays low because the hazard itself creates the pressure |
| Shrink Interval | 3 apples | Frequent wall pressure to keep the contest tight |
| Predator Initial Length | 5 segments | Large enough to feel threatening immediately |
| Predator Initial Speed | 8.0 | Slower than player at the start |
| Predator Speed Gain | +1 per apple stolen | Existing escalation - every stolen apple matters |
| Hunt Trigger | Every 3 predator apples | Existing behavior - after 3 stolen apples it switches targets |
| Hunt Duration | 10.0s | Existing hunt window |
| Predator Loss Threshold | 5 stolen apples | If the predator eats 5 apples, the player loses |
| Star Threshold (2 stars) | 2 deaths | Moderate |
| Star Threshold (3 stars) | 0 deaths | Perfect evasive run |

### Predator Behavior (existing mechanic - with modifications)

- **Existing**: `Predator` class with `PredatorMode::HUNTING_APPLE` / `HUNTING_PLAYER`, path selection in `ChooseDirection()`, speed growth, 10-second hunt timer, and loss condition after 5 stolen apples.
- **Existing**: HUD already tracks stolen apples as `Predator: X/5`, and `PlayState` already triggers a world shrink when the predator steals an apple.
- **Modification for redesign**: Add "learning tells" as the stage progresses:
  - **Apples 1-3**: Predator takes direct, readable routes. Feels like a hungry animal.
  - **Apples 4-7**: Predator begins favoring cutoffs, not just shortest pathing. It tries to intercept likely player routes.
  - **Apples 8-10**: Predator aggressively uses arena shape, hovering near the center and forcing the player toward shrinking walls.
- **Modification for redesign**: Add "ink pit" tiles in late stage layouts. The predator dislikes crossing them during stage phase unless it is in direct hunt mode, making them temporary route-control tools for the player.

### Difficulty Curve Within Stage

- **Apples 1-3**: Player learns the Predator's two priorities: food first, player second. It feels manageable and almost fair.
- **Apples 4-6**: One stolen apple means a shrink and score loss. The pressure spikes. The player starts thinking about denial, baiting, and route timing.
- **Apples 7-10**: The arena is small, the Predator is faster, and hunt phases leave very little safe space. The feeling should be, "It knows what I'm about to do."

### Visual Theme (adapt current level identity)

- Paper tone: clinical blue-gray `(212, 218, 228)`
- Ink tint: cold indigo `(40, 45, 65)`
- Accent: pale paper-white `(220, 225, 235)` with danger rust `(200, 80, 60)` during hunt mode
- Corruption: 0.70 (one of the most corrupted pages - this thing has been left alone too long)
- Predator hunt transitions flash from steel-blue into rust-red, reinforcing the shift from watcher to killer

### Player Experience Goals

- **First attempts**: The player loses apples to the Predator and suddenly realizes the true failure condition is not just self-collision. "It can steal my win."
- **Learning curve**: The player starts reading the Predator's target choice, positioning near apples before it arrives, and surviving hunt windows without panicking.
- **Mastery**: The player uses the Predator's greed against it, deliberately baiting its route, stealing apples at the last second, and treating hunt mode as a predictable rhythm rather than chaos.
- **Emotional note**: This level should feel like being observed by something that is becoming more intelligent each minute. Not random danger - focused hunger.

---

## Part 2: Boss Phase - "The Hunter"

### Transition

After the 10th apple, the arena goes quiet. The Predator stops moving, lifts its head, and stares directly at the snake. Dark puddles spread outward from beneath its body, staining the paper into three connected ink pits. Text appears: *"It was never hunting the apples."*

### Boss Entity: The Hunter

The stage Predator evolves into its true form: a massive feral snake built for pursuit. It is longer, faster, and no longer distracted by food. The boss fight is a trap-setting duel: the player cannot outrun it forever and must turn the arena itself into a weapon.

**Visual design:**
- A 15-segment predator snake, much longer than the stage version
- Body ink shifts between cold blue-black and blood-rust depending on aggression phase
- Eyes are large, unblinking, and over-articulated - unsettlingly intelligent, too attentive
- When it crosses near an ink pit, its outline feathers into liquid ink, showing its connection to the corrupted page

### Boss Mechanics

**Arena:**
- Fixed-size arena with 3 permanent ink pits positioned so the player can route the boss across them in sequence
- Pits are safe for the player to cross normally, but they "mark" the Hunter when it passes over them
- Walls do not shrink further during boss phase; the pressure comes from pursuit speed and route planning

**The Hunter's behavior:**
- Constantly targets the player head. No apple distraction, no passive window.
- Moves faster than the player baseline and takes tighter lines through corners than the stage Predator.
- Each time it crosses an unused ink pit, the pit flares and brands the Hunter with a glowing seal.
- After 3 different pits are crossed in one uninterrupted chase sequence, the seals connect, binding the Hunter and dealing 1 damage.

**How to damage:**
- Lead the Hunter across **3 different pits in sequence** without letting it reset its pursuit pattern.
- Re-using the same pit in a sequence does not count.
- If the player stalls too long between pits, the seal chain fades and the sequence must start again.

### Boss Phases

- **Phase 1 (HP 3)**: Hunter is fast but direct. Pit order can be simple triangle routing. The player learns the trap-sequence rule.
- **Phase 2 (HP 2)**: After the first bind, the Hunter avoids the most recently used pit for 4 seconds and cuts across the arena more aggressively. The player must route around its avoidance pattern.
- **Phase 3 (HP 1)**: Hunter intermittently "bursts" for a half-second speed spike after every sharp turn. Pit seals decay faster, so the final trap requires clean, committed routing with no hesitation.

### Damage System

- **Hit condition**: Hunter crosses 3 unique ink pits before the seal chain expires
- **Hits to defeat**: 3
- **Counter-ability bonus (Shadow Decoy from L4)**: The decoy becomes the Hunter's target for 4 seconds. This gives the player time to reposition and line up pit routes safely. If the Hunter catches the decoy near a pit, the pit is counted as crossed immediately.
- **Secondary bonus (Venom Trail from L6)**: Trail tiles stun the Hunter for 3 seconds, letting the player repair a failed trap route or safely redirect it toward an unused pit.
- **Visual feedback on hit**: The three pit seals snap together as bright white lines, yanking the Hunter to the ground. Its body spasms and sheds corrupted ink, briefly exposing clean line art beneath.

### Boss Defeat

When HP reaches 0:
1. The final seal chain tightens and pins the Hunter in place
2. Its feral body dissolves into clean contour lines, as if the sketch is being redrawn correctly for the first time
3. The ink pits dry into dark, harmless shading rather than bottomless stains
4. The page reveals what it was meant to be: a naturalist study of a serpent in motion, elegant instead of monstrous
5. The stare is gone. The page is no longer watching the player back

---

## Part 3: Ability Earned - Hunter's Dash

### Ability Acquisition Cutscene

After the Hunter is pinned:
1. The bright seal lines that trapped it unwind and spiral around the snake
2. The lines compress into the snake's body, sharpening every segment into a sleek, aerodynamic form
3. For a single instant, the snake blurs forward several tiles in a straight line
4. Text: *"Its hunger became your momentum."*
5. Ability icon appears: a cyan fang-shaped arrow with trailing ink

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Hunter's Dash |
| Cooldown | 8 seconds |
| Duration | Instant |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Hunter (L8 boss) |

### Effect Details

When activated:
1. **Immediate dash**: Snake lunges 4 tiles straight forward in its current facing direction
2. **Obstacle skip**: Dash passes through temporary hazards and narrow danger windows that would normally require pathing around
3. **Apple sweep**: Any apples on the dashed path are auto-collected
4. **Commitment rule**: Dash cannot turn mid-use - it is pure forward commitment
5. **Safety rule**: If the dash would end fully inside a wall, it stops at the last safe tile instead

### Visual Transform (Snake Appearance During Hunter's Dash)

- **Snake body**: Turns bright cyan with white edge highlights, body stretched into sleek spear-like shapes
- **Eyes**: Narrow to sharp predator slits, glowing cold white
- **Motion effect**: Strong trailing smear of cyan ink and speed lines
- **Impact effect**: Apples collected during the dash burst into small white sparks rather than normal particles

### When Hunter's Dash is Most Useful

| Level | How Hunter's Dash Helps |
|---|---|
| L8 The Watcher (home level) | Burst through pursuit lines, deny apples, and cut across the arena before the Predator can intercept |
| **L5 Famine (COUNTER)** | **Reach expiring apples instantly and guarantee the Hourglass boss timing window when a fresh apple spawns at distance.** |
| L9 Control Shuffle | Dash ignores momentary confusion because it commits to current facing direction |
| L3 Quicksand | Cross wide quicksand patches before slow terrain can trap your route |
| L7 Earthquake | Escape collapsing routes or jump into newly opened lanes before aftershocks punish you |

---

## Part 4: Hidden Ability Interactions

When replaying Level 8 with abilities from other levels:

### Shadow Decoy + The Watcher (from L4) - PRIMARY COUNTER

- Predator and Hunter retarget to the decoy for 4 seconds
- Lets the player steal apples uncontested during stage phase
- During boss: if the Hunter catches the decoy on top of a pit, that pit counts as a marked pit without risking the player
- **Achievement**: "False Prey" - defeat The Hunter with every trap sequence started by a decoy bait

### Venom Trail + The Watcher (from L6) - SECONDARY COUNTER

- Trail tiles stun the Predator/Hunter for 3 seconds when crossed
- Stage phase: place trail across the most efficient apple route to deny steals
- Boss phase: use trail to salvage a broken pit sequence or force the Hunter to take the long way around
- **Achievement**: "Apex Antidote" - stun the Predator three times in one run without taking damage

### Shed Skin + The Watcher (from L3)

- Shed segments act as bait. The stage Predator briefly prioritizes the dropped mass if it is closer than the apple
- Boss phase: if the Hunter collides with a shed segment, it spends 1 second crushing through it, creating a tiny timing window
- Creates emergency blockers in tight arenas

### Time Freeze + The Watcher (from L5)

- Freezes Predator/Hunter movement entirely for 4 seconds
- Stage phase: lets the player recover a contested apple or survive a hunt window
- Boss phase: freezes pit-seal decay timer, making the 3-pit sequence easier to complete cleanly

### Ink Flare + The Watcher (from L2)

- Flare reveals the Predator's exact route intent for a brief moment as a faint line to its current target
- Off-screen predator position becomes visible through darkness or obstruction
- Boss phase: unused pits glow brighter during Flare, helping the player read the correct route under pressure

---

## Part 5: Post-Boss Cutscene

### "Field Study"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The clinical blue-gray page sits still. The monstrous Hunter is gone, replaced by clean contour lines and soft shading.
2. **(2-4s)** Those contour lines expand into a full notebook study page: sketches of snakes coiled, resting, striking, gliding. Observation without fear.
3. **(4-6s)** Small handwritten notes appear beside the studies in the Artist's hand - arrows, labels, little marks of curiosity. This page was meant to be about attention and care, not predation.
4. **(6-7s)** The Artist's hand enters and finishes one final line on the page, steady and deliberate.
5. **(7-8s)** Fade to stage select. Eight pages healed. Only the scrambled page and the final eraser remain ahead.

### Emotional Beat

This page heals by transforming surveillance into understanding. The unfinished creature was terrifying because it was observed without being understood or guided. Once healed, the same attention becomes study, curiosity, even tenderness. The Artist finally draws again on-screen, which signals that the paralysis is breaking.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Hard**. The player is solving a competitive routing problem under constant threat. The predator stealing apples creates a very different kind of pressure than pure survival hazards. Deaths: 4-7 attempts average.
- Boss phase: **Hard**. The 3-pit trap rule is readable, but executing it cleanly against a faster pursuer takes strong route planning. Deaths: 4-8 attempts average.

### With Abilities

- **Shadow Decoy (COUNTER)**: Major help. Safest way to set up routes and trap sequences.
- **Venom Trail (secondary counter)**: Strong help. Turns pursuit into a controllable rhythm.
- **Time Freeze**: Strong help. Pauses the chase and preserves pit-seal progress.
- **Shed Skin**: Moderate help. Emergency blockers and bait points.
- **Hunter's Dash (own ability, replay)**: Makes the stage dramatically easier because it can steal apples back on reaction and break away from hunt lines.

### Recommended First-Time Experience

This is a **late-game level** (6th-8th clear recommended). The player should ideally arrive with at least one pursuit-control tool already unlocked, especially Shadow Decoy or Venom Trail. The level's identity is not raw speed alone; it is the psychological pressure of being hunted by something that learns. Earning Hunter's Dash this late feels right because it is a confident movement tool that opens up strong replay routes in earlier levels.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `Predator` class - movement, target switching, speed growth, spawn logic, body rendering, and hunt timer
- `PredatorMode` - existing apple-hunt / player-hunt split maps cleanly to the stage redesign
- `PlayState` predator integration - player death on contact, apple-steal handling, score penalty, world shrink, hunt audio/shake cues
- `HUD` predator counter - existing `Predator: X/5` display already communicates the stage loss condition well
- Existing Level 8 `LevelConfig` values - speed, shrink cadence, palette, and predator flag already establish the base identity

### New Code Required

- **Predator learning layer**: extend targeting heuristics so late-stage Predator prefers interception lanes instead of only shortest-path routing
- **Ink pit system**:
  - Stage phase optional pit tiles that Predator avoids unless in hunt mode
  - Boss phase permanent 3-pit arena layout with per-pit activation state
- **The Hunter Boss**: New class `src/bosses/HunterBoss.h/.cpp`
  - Direct pursuit AI based on Predator movement foundation
  - Pit crossing detection and unique-sequence validation
  - Seal-chain timer and phase-specific avoidance behavior
  - Burst acceleration for final phase
- **Hunter's Dash ability**: New class in ability system
  - Instant 4-tile forward traversal
  - Safe-stop validation against walls
  - Apple auto-collection along dash path
  - Distinct dash trail / speed-line rendering
- **Counter hooks**:
  - Decoy retargeting for Predator/Hunter AI
  - Venom trail stun support in Predator/Hunter update logic
  - Pit-seal timer freeze interaction with `Time Freeze`

### Existing LevelConfig Values (reference)

```cpp
// Level 8: "The Watcher" -- Cold steel-blue ink on clinical gray paper
l[7] = {
    8, "The Watcher", "It learns.",
    13.0f, 10, 3, 0.0f,
    sf::Color(14, 16, 26), sf::Color(55, 65, 95),
    sf::Color(50, 55, 90), sf::Color(70, 75, 110), sf::Color(200, 80, 60),
    false, false, true, false, false, false, false, false,
    0.0f, 5, 2,
    sf::Color(212, 218, 228), sf::Color(40, 45, 65), sf::Color(220, 225, 235),
    0.70f, true, false, false
};
```
