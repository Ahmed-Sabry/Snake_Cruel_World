# Level 7: "Earthquake" - Detailed Design Document

> *"The page itself is unstable, never given a foundation."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Earthquake |
| Subtitle | "The ground has opinions about your route." |
| Hazard | Earthquakes (walls shift unpredictably, crack effects) |
| Boss | The Fault Line |
| Ability Earned | Ink Anchor |
| Strong Counter Against | Level 3: Quicksand |
| Narrative Metaphor | The Artist never gave this page a foundation, so it folds, cracks, and shifts beneath everything drawn on it. |

---

## Part 1: Stage Phase

### Gameplay

Walls periodically shift position during earthquakes, changing the playable area unpredictably. Warning cracks appear before each quake, giving the player a brief window to reposition. The challenge: your safe route can become a death trap in an instant.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 12.0 | Moderate - reaction time matters more than speed |
| Apples to Win | 15 | Standard count |
| Shrink Interval | 3 apples | Fast shrinking - combined with quake shifts, arena gets volatile |
| Quake Interval | 8.0s (initial) | Time between earthquakes |
| Warning Duration | 1.5s | Crack lines appear before quake hits |
| Star Threshold (2 stars) | 5 self-collisions | Generous - quakes cause unavoidable repositioning deaths |
| Star Threshold (3 stars) | 2 self-collisions | Tight |

### Earthquake Behavior (existing mechanic - with modifications)

- **Existing**: `Earthquake` class with `QuakeState` (Idle/Warning/Quaking), per-side offsets `m_pendingOffset[4]`, crack line rendering, `ValidateOffsets()`
- **Modification for redesign**: Add "aftershock" - a smaller secondary quake 2 seconds after the main quake. Aftershock shifts walls by half the main quake amount. Creates a double-jeopardy pattern.
- **Visual**: Crack lines appear during warning phase (existing `m_cracks`). During quake, screen shakes. Ink bleed shader activates briefly (corruption = 0.60, `enableInkBleed = true`).

### Difficulty Curve Within Stage

- **Apples 1-5**: 8s quake interval. Single direction shifts (only 1-2 walls move per quake). Predictable. Player learns to read crack warnings.
- **Apples 6-10**: 6s quake interval. Multi-direction shifts (3-4 walls). Aftershocks begin. Arena has shrunk - less room to dodge.
- **Apples 11-15**: 5s quake interval. Maximum displacement per quake. Aftershocks are stronger. The arena is small and violently unstable. Every apple requires navigating through chaos.

### Visual Theme (existing - keep as-is)

- Paper tone: terracotta `(205, 175, 155)`
- Ink tint: rust-red `(130, 35, 20)`
- Accent: fiery orange `(220, 120, 30)`
- Corruption: 0.60 (high - the page is crumbling)
- Ink bleed shader enabled (walls visually wobble even between quakes)

### Player Experience Goals

- **First attempts**: Player gets crushed by walls they didn't see moving. "I was safe a second ago!" The warning cracks become critical reading.
- **Learning curve**: Player develops a habit of staying near the arena center (furthest from all walls). Learns to pre-position during warning phase.
- **Mastery**: Player reads crack patterns instantly, repositions proactively, and uses the wall shifts as opportunities (some shifts create shortcuts).
- **Emotional note**: Instability and chaos. Nothing is permanent. The ground beneath you can't be trusted, and the rust-red palette should feel volcanic and dangerous.

---

## Part 2: Boss Phase - "The Fault Line"

### Transition

After the 15th apple, a massive crack splits the arena floor from left to right. The crack widens, glowing orange-red. Text: *"The page is breaking apart."*

### Boss Entity: The Fault Line

A centipede-like creature that lives inside the cracks of the page. It moves underground, invisible, and bursts from the ground at semi-random locations, causing localized earthquakes on eruption.

**Visual design:**
- When underground: invisible, but cracks in the page follow its path (dark lines snaking across the floor)
- When surfacing: a segmented rust-red worm-like creature erupts from the ground, ~8 segments long
- Each segment is angular, rocky-looking, like cracked earth
- Eyes glow orange when surfacing. Sprays dirt/debris particles on eruption.

### Boss Mechanics

**The Fault Line's behavior:**
- Moves underground along crack paths. Cracks visibly creep across the arena floor showing its general location.
- Every 6 seconds, it surfaces at the end of a crack line. Eruption causes a localized earthquake (walls near eruption point shift 2-3 tiles).
- After surfacing, it remains above ground for 3 seconds, then burrows back down.
- An apple spawns at a random location when the creature surfaces.

**How to damage:**
- When the creature surfaces near an apple (within 3 tiles), the eruption tremor knocks the apple into the creature's mouth, dealing 1 damage.
- Player strategy: predict where the creature will surface by reading the crack paths, then be positioned near the apple when it erupts. The tremor does the rest.
- If the player eats the apple before the eruption, no damage to boss (but apple feeds the snake).

**Boss phases:**
- **Phase 1 (HP 4)**: Crack paths are slow and clearly visible. 6s between eruptions. 1.5s warning before surface (cracks widen). Easy to predict.
- **Phase 2 (HP 3)**: Crack paths move faster. 5s between eruptions. Creature starts leaving "fault lines" - permanent cracks that cause mini-quakes when the player crosses them.
- **Phase 3 (HP 2)**: 4s eruptions. Multiple crack paths simultaneously (creature splits into 2 segments underground). Only one is real - the other is a decoy crack.
- **Phase 4 (HP 1)**: 3s eruptions. Creature is aggressive - crack paths chase the player. Arena walls shift on every eruption. Total chaos. The final hit requires perfect positioning amidst constant tremors.

### Damage System

- **Hit condition**: Apple is within 3 tiles of eruption point when creature surfaces
- **Hits to defeat**: 4
- **Counter-ability bonus (Shed Skin from L3)**: Shed segments anchor to the grid and don't move during earthquakes. They create stable "islands" the snake can navigate to. Placing them on crack paths also blocks the creature's underground movement, forcing it to surface earlier and more predictably.
- **Visual feedback on hit**: Creature recoils underground with a scream. The crack it emerged from seals shut permanently (golden ink fills it). One less crack to worry about.

### Boss Defeat

When HP reaches 0:
1. All cracks in the arena glow golden simultaneously
2. The Fault Line is forced to surface one final time - fully exposed, writhing
3. Golden ink pours into every crack, filling them like molten gold in kintsugi pottery
4. The cracks become beautiful golden veins across the terracotta page
5. The ground stabilizes. Solid. Permanent. The foundation the Artist never provided.

---

## Part 3: Ability Earned - Ink Anchor

### Ability Acquisition Cutscene

After the Fault Line is sealed:
1. The golden-veined ground hardens beneath the snake
2. The snake's body becomes heavy, grounded - it sinks slightly into the page
3. An anchor shape forms in golden ink beneath the snake, then absorbs upward
4. Text: *"The ground learned to hold firm. So will you."*
5. Ability icon appears: a golden anchor with ink dripping from it

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Ink Anchor |
| Cooldown | 12 seconds |
| Duration | 5 seconds |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Fault Line (L7 boss) |

### Effect Details

When activated:
1. **For 5 seconds**: Walls are locked in place (no earthquakes, no wall shifts)
2. **Terrain immunity**: Snake ignores all terrain slowdowns (quicksand, etc.)
3. **Stability aura**: Nearby entities are also stabilized (Mirror Ghost within 3 tiles freezes briefly)
4. **Visual**: Golden anchor symbol appears beneath the snake. Ground tiles near the snake gain golden vein cracks (stabilized ground).

### Visual Transform (Snake Appearance During Ink Anchor)

- **Snake body**: Turns rust-red/iron color. Segments become rigid, angular (rectangular instead of smooth).
- **Eyes**: Glow deep orange-red, steady and unblinking.
- **Movement**: Snake moves slightly slower visually (heavier feel) but actual speed is unchanged.
- **Ground effect**: Each tile the snake crosses gains a brief golden shimmer (anchored ground).

### When Ink Anchor is Most Useful

| Level | How Ink Anchor Helps |
|---|---|
| L7 Earthquake (home level) | Locks walls in place, prevents quake disruption |
| **L3 Quicksand (COUNTER)** | **Ignores quicksand slowdown entirely. Glide over patches at full speed. Prevents patch relocation during duration.** |
| L4 Mirror Ghost | Stability aura briefly freezes the ghost if it's nearby |
| L9 Control Shuffle | Doesn't directly counter, but the stability provides psychological comfort during shuffle chaos |

---

## Part 4: Hidden Ability Interactions

### Shed Skin + Earthquake (from L3) - PRIMARY COUNTER

- Shed segments anchor to grid permanently (don't move during quakes)
- Create stable navigation landmarks in the shifting arena
- During boss: block crack paths, forcing The Fault Line to surface predictably
- **Achievement**: "Earthquake-Proof" - defeat The Fault Line using only Shed Skin (block all crack paths)

### Time Freeze + Earthquake (from L5)

- Freezing time stops earthquake timers completely
- Walls stay in current position for 4 seconds
- During boss: creature freezes underground mid-movement

### Venom Trail + Earthquake (from L6)

- Trail tiles remain fixed during earthquakes (anchored like shed segments)
- Creates stable pathways through the shifting arena
- Creature takes stun damage if it surfaces on a trail tile

### Hunter's Dash + Earthquake (from L8)

- Dash is unaffected by wall shifts mid-dash (you teleport through)
- Can dash to safety when walls are about to crush you
- **Achievement**: "Seismograph" - dash through 3 quakes in a row without dying

---

## Part 5: Post-Boss Cutscene

### "Foundation"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The cracked terracotta page with golden kintsugi veins. Beautiful in its brokenness - the cracks are now decoration, not damage.
2. **(2-4s)** From the solid ground, structures rise: ink-drawn buildings, bridges, towers. The Artist's vision for this page was an architecture sketch - a city built on strong foundations.
3. **(4-6s)** The city sprawls across the page. Every building's foundation touches the golden veins - built on healed cracks. What was broken now supports something greater.
4. **(6-7s)** The Artist's hand appears. It's touching the page now. Fingertips resting on the golden veins. Feeling the strength. The pen is in the other hand.
5. **(7-8s)** Fade to stage select. Seven pages healed. One remains.

### Emotional Beat

The earthquake page was about lacking foundation. The healing doesn't erase the cracks - it makes them beautiful and load-bearing (kintsugi). The city built on healed foundations is the message: strength comes from repaired broken things, not from things that were never broken. The Artist touching the page is the most intimate connection yet - almost ready.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Hard**. Earthquakes are the most disorienting hazard. Walls moving changes your mental model of the arena. Combined with fast shrinking, it's chaotic. Deaths: 4-7 attempts.
- Boss phase: **Hard**. The prediction-based damage mechanic - being near the apple before the eruption - requires reading crack paths well. Phase 4 chaos is intense. Deaths: 5-9 attempts.

### With Abilities

- **Shed Skin (COUNTER)**: Major help. Stable anchors + blocking crack paths = significant control over chaos.
- **Time Freeze**: Strong help. Freezing quake timers gives breathing room.
- **Ink Anchor (own ability, replay)**: Trivializes stage phase. Boss fight still requires positioning skill.
- **Hunter's Dash**: Moderate help. Dashing through quakes is a safety net.

### Recommended First-Time Experience

This is a **late game level** (6th-8th attempt). Earthquakes are the most mechanically complex hazard. Players need strong fundamental skills and ideally 3-4 abilities already earned. The boss's 4-hit requirement and multi-phase escalation make it one of the harder boss fights. Ink Anchor is a strong utility ability that complements Shed Skin well.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `Earthquake` class - `QuakeState` management, warning/quake cycle, `m_pendingOffset[4]`, `ValidateOffsets()`
- `Earthquake::GenerateCrackLines()` - crack rendering
- `Earthquake::ApplyQuake()` - wall offset application
- Ink bleed shader already enabled for this level

### New Code Required

- **Aftershock system**: Secondary quake 2s after main quake (half displacement)
- **The Fault Line Boss**: New class `src/bosses/FaultLineBoss.h/.cpp`
  - Underground movement along crack paths (crack generation system)
  - Surface/burrow cycle with eruption mechanics
  - Apple proximity check on eruption for damage
  - Phase transitions (speed, decoy cracks, aggression)
  - Permanent crack sealing on hit (golden ink fill)
- **Ink Anchor ability**: New class in ability system
  - Wall lock (prevent all earthquake offsets for duration)
  - Terrain immunity flag (snake ignores `IsOnQuicksand()` etc.)
  - Visual: golden anchor overlay, ground shimmer effect
- **Crack path system** (boss arena): Animated cracks that show creature's underground path
  - Crack widening animation as creature approaches surface
  - Permanent fault lines (hazard tiles from Phase 2+)

### Existing LevelConfig Values (reference)

```cpp
// Level 7: "Earthquake" -- Rust-red ink on terracotta paper
l[6] = {
    7, "Earthquake", "The ground has opinions about your route.",
    12.0f, 15, 3, 0.0f,
    sf::Color(32, 12, 8), sf::Color(150, 40, 25),
    sf::Color(140, 45, 20), sf::Color(170, 70, 35), sf::Color(215, 180, 50),
    false, false, false, true, false, false, false, false,
    0.0f, 5, 2,
    sf::Color(205, 175, 155), sf::Color(130, 35, 20), sf::Color(220, 120, 30),
    0.60f, true, false, false
};
```
