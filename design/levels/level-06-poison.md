# Level 6: "Betrayal" - Detailed Design Document

> *"Colors bled into each other. What looks right is wrong."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Betrayal |
| Subtitle | "Not everything green is good for you." |
| Hazard | Poison Apples (fake apples that cause control inversion, speed boost, extra growth) |
| Boss | The Parasite |
| Ability Earned | Venom Trail |
| Strong Counter Against | Level 8: The Watcher (secondary) |
| Narrative Metaphor | The Artist mixed colors carelessly before abandoning the page - the inks bled together, creating deceptive appearances. What looks nourishing is toxic. |

---

## Part 1: Stage Phase

### Gameplay

Poison apples spawn alongside real apples. They look similar but have subtle visual differences. Eating a poison apple inverts controls, boosts speed, and causes extra body growth - a triple punishment that makes the next few seconds chaotic. The player must learn to distinguish real from fake.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 12.0 | Moderate - speed boost from poison makes it feel much faster |
| Apples to Win | 12 | Standard count |
| Shrink Interval | 4 apples | Walls close every 4 real apples eaten |
| Poison Spawn Rate | 1 poison paired with 1 real apple | Exactly two apples are present per spawn cycle: one real and one poison. Eating either apple immediately restores the pair, and poison apples never accumulate beyond a single active poison apple. |
| Control Inversion Duration | 3.0s | How long controls stay inverted after eating poison |
| Speed Boost Amount | 1.5x | Speed multiplier during poison effect |
| Extra Growth | 2 segments | Eating poison grows you 2 extra segments (3 total instead of 1) |
| Star Threshold (2 stars) | 2 self-collisions | Tight - poison deaths count separately |
| Star Threshold (3 stars) | 0 self-collisions | Flawless identification |

### Poison Apple Behavior (existing mechanic - with modifications)

- **Existing**: `PoisonApple` class with control inversion, speed boost, consecutive tracking, `CheckCollision()`
- **Modification for redesign**: Make the visual difference between real and poison more subtle as the level progresses:
  - **Apples 1-4**: Poison apple is clearly different color (sickly yellow-green vs red)
  - **Apples 5-8**: Colors converge - poison becomes more red-tinted, harder to distinguish
  - **Apples 9-12**: Nearly identical colors. Player must rely on subtle cues: poison apple has a faint outline shimmer, or slightly different size (0.9x scale)
- **Recovery**: Eating a real apple while poisoned does **not** remove the active poison timers or the extra body length already gained; it only advances the spawn cycle and breaks the consecutive-poison chain, so any poison-added segments remain until lost through normal play.

### Difficulty Curve Within Stage

- **Apples 1-4**: Easy to tell apart. Player learns what poison does. "Oh, that's bad. Avoid the green one."
- **Apples 5-8**: Colors start blending. First accidental poison eats. Player learns to look more carefully. Arena is shrinking.
- **Apples 9-12**: Colors nearly identical. Player is relying on muscle memory and subtle visual tells. The paranoia is the gameplay: "Is this one safe? Is it?"

### Visual Theme (existing - keep as-is)

- Paper tone: sickly green-white `(205, 212, 195)`
- Ink tint: deep forest green `(25, 50, 25)`
- Accent: toxic bright green `(75, 160, 55)`
- Corruption: 0.50 (high - the page is deeply corrupted, colors are unreliable)
- No post-processing shaders (the corruption is in the content, not the rendering)

### Player Experience Goals

- **First attempts**: Player eats poison by accident, loses control, crashes. "I can't tell them apart!" Frustration that drives attention to detail.
- **Learning curve**: Player develops an eye for the subtle differences. Slows down, looks before committing. Paranoia becomes strategy.
- **Mastery**: Player can instantly distinguish real from poison at a glance. They can even eat poison strategically if they can handle the inverted controls.
- **Emotional note**: Betrayal and trust. The green palette is both nurturing (forest) and toxic (poison). The level asks: can you trust what you see?

---

## Part 2: Boss Phase - "The Parasite"

### Transition

After the 12th apple, all apples on screen transform into poison. They pulse, merge, and flow toward the snake's tail. Text appears: *"The poison learned to hunt."*

### Boss Entity: The Parasite

An ink creature that attaches to the snake's tail and slowly converts body segments to poison. It's not a separate entity you chase - it's on you, consuming you from within.

**Visual design:**
- A dark green-black blob attached to the snake's tail segment
- Infected segments change color from normal to sickly green, pulsing
- The parasite has small tendril-like extensions reaching toward the next healthy segment
- As it consumes more segments, it grows slightly larger and darker

### Boss Mechanics

**The infection:**
- The Parasite starts by converting the last body segment to "poisoned" (green, pulsing)
- Every 3 seconds, it converts the next segment toward the head
- If all segments are converted (the parasite reaches the head), the player dies
- Total infection time is dynamic: if the snake currently has `N` infectable body segments, full infection takes `N x 3 seconds` unless the player pushes the parasite back by eating real apples

**How to fight back:**
- Eating a real apple pushes the parasite back 2 segments (heals 2 segments)
- Eating a poison apple feeds the parasite - it converts 3 more segments instantly
- The arena has both real and poison apples, with the same visual deception from the stage phase
- **Win condition**: Eat 5 consecutive real apples without eating any poison. This purges the parasite completely.

**The Parasite fights back:**
- **Spit attack**: Every 8 seconds, the parasite samples from the pool of currently spawned real apples on the field and converts the nearest eligible real apple to poison. It never spawns extra apples just to satisfy a spit conversion; if no real apples are currently available, the spit simply fails to convert anything. Visual: green projectile arcs from the tail to the target apple, and the apple changes color.
- **Phase 2 (after 2 real apples)**: Spit interval decreases to 5 seconds. Each spit converts up to 2 real apples at once using `min(2, currently spawned real apples)`, choosing the nearest eligible targets first.
- **Phase 3 (after 4 real apples)**: Spit interval = 3 seconds. It keeps the same `min(2, currently spawned real apples)` conversion rule, and on each spit cadence all currently infected segments pulse to `1.2x` their normal radius with a smooth `0.15s` expand, `0.4s` hold, and `0.15s` shrink. The expanded state is a real head-collision hazard for that `0.7s` window: if the snake's head overlaps an expanded infected segment, the player dies immediately. The pulse does not push or solid-block the snake's existing body; it is a timed head-avoidance threat, not a full-body physics obstacle.

### Damage System

- **Win condition**: 5 consecutive real apples eaten (no poison between them)
- **Counter-ability bonus (Ink Memory from L9)**: Ink Memory locks controls for 6 seconds and purifies poison instantly. Using it during the boss fight immediately clears all current poison effects and pushes the parasite back 3 segments; the purification itself directly damages it.
- **Visual feedback on real apple**: Healthy green pulse travels from head to tail, pushing the infection back. Satisfying "cleanse" effect. Each consecutive real apple makes the pulse brighter.
- **Visual feedback on poison apple**: Infection surge - segments rapidly convert with a sickening green wave.

### Boss Defeat

When the 5th consecutive real apple is eaten:
1. A massive golden purge wave travels from head to tail
2. The Parasite shrieks (visual: violent shaking, ink splatter)
3. It detaches from the tail and writhes on the ground
4. The snake's healthy segments stomp it flat (automatic animation)
5. The squished parasite dissolves into the page, becoming fertilizer
6. The toxic green palette transforms into lush, healthy green

---

## Part 3: Ability Earned - Venom Trail

### Ability Acquisition Cutscene

After the Parasite dissolves:
1. The dissolved poison rises as vapor from the page
2. The vapor condenses onto the snake's tail, coating it in bright acid-green
3. The snake flicks its tail - a line of glowing poison splatters on the page
4. Text: *"What once consumed you is now yours to command."*
5. Ability icon appears: a dripping acid-green droplet

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Venom Trail |
| Cooldown | 10 seconds |
| Duration | 3 seconds |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Parasite (L6 boss) |

### Effect Details

When activated:
1. **For 3 seconds**: The snake's tail leaves a glowing acid-green poison trail on every tile it passes through
2. **Trail tiles**: Remain for 5 seconds before fading
3. **Enemy effect**: Any enemy (Predator, boss entity) that crosses a trail tile is stunned for 3 seconds (frozen in place, can't move or attack)
4. **Trail does not hurt the player** - the snake can cross its own trail safely
5. **Trail interaction with apples**: Trail tiles that overlap with a poison apple neutralize it (convert to real apple)

### Visual Transform (Snake Appearance During Venom Trail)

- **Snake body**: Turns toxic neon green. Body segments drip with acid-green particles.
- **Eyes**: Glow bright green with a slit pupil (venomous snake look)
- **Trail**: Bright acid-green tiles with a bubbling/sizzling animation. Faint green glow around each tile.
- **Particle effect**: Green droplets fall from the snake's body, splattering on contact with the ground.

### When Venom Trail is Most Useful

| Level | How Venom Trail Helps |
|---|---|
| L6 Poison (home level) | Trail neutralizes poison apples, converting them to real ones |
| **L8 The Watcher (COUNTER)** | **Lay trail in Predator's path - it gets stunned for 3s. Create safe zones. Combined with Shadow Decoy, completely controls the Predator.** |
| L2 Blackout | Trail glows in the dark, creating a visible path during blackouts |
| L4 Mirror Ghost | Trail stuns the ghost for 3s if it crosses it |
| L3 Quicksand (boss) | Venom Trail stuns The Mire for 3 seconds if it crosses the trail, briefly interrupting its path control without dealing damage |

---

## Part 4: Hidden Ability Interactions

### Ink Memory + Poison (from L9) - PRIMARY COUNTER

- Ink Memory locks controls AND purifies poison instantly
- During boss fight: purifies all current poison effects AND pushes parasite back 3 segments
- Locked controls during poison = no accidental direction changes from inverted controls
- **Achievement**: "Immune System" - complete L6 without ever being affected by poison (using Ink Memory to pre-emptively purify)

### Shadow Decoy + Poison (from L4)

- Decoy can "eat" poison apples for you, removing them from the field
- When decoy eats poison, it converts to a real apple at the decoy's position
- During boss fight: decoy draws the Parasite's spit attacks (spit targets decoy instead of real apples)
- **Achievement**: "Taste Tester" - have your decoy eat 5 poison apples in a single run

### Time Freeze + Poison (from L5)

- Freezing time stops poison effect timers (control inversion doesn't count down during freeze)
- It also freezes the Parasite, so it can't convert segments during the effect
- Gives breathing room to plan next move while poisoned

### Hunter's Dash + Poison (from L8)

- Dashing auto-collects apples - but doesn't distinguish real from poison!
- Risky: if you dash through a poison apple, you get poisoned
- If you know which apple is real, dashing to it is the fastest way to keep the consecutive counter going during the boss fight

---

## Part 5: Post-Boss Cutscene

### "True Colors"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The toxic green page. Colors are muddled, unclear. Everything bleeds into everything else.
2. **(2-4s)** A single clear ink line appears - definitive, true, unmistakable. It draws itself across the page like a border between truth and deception.
3. **(4-6s)** On one side of the line, the toxic colors settle into rich, honest greens - a meadow. On the other side, the deceptive colors drain away, leaving clean white paper. The page is divided: truth and clarity.
4. **(6-7s)** The Artist's hand appears. It's the closest yet - fingertips almost touching the page. The hand is steady now. Not trembling, not pulling away. Ready.
5. **(7-8s)** Fade to stage select. Six pages healed. Two remain.

### Emotional Beat

Betrayal was the deepest wound - the page where things looked right but weren't. Healing it means establishing truth: clear lines, honest colors, reliable appearances. The meadow represents genuine nourishment, not the poison's false promise. The Artist's hand being steady and close signals that the climax is near. The Artist is almost ready to create again.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Medium**. Poison is punishing but survivable. The visual deception creates tension. Players who rush will eat more poison; players who observe will thrive. Deaths: 2-4 attempts average.
- Boss phase: **Hard**. The 5-consecutive-real requirement is demanding when the Parasite is converting apples. The pressure of the infection timer (`current infectable segments x 3 seconds` before death, before any healing) creates urgency. Players must balance speed with caution. Deaths: 4-7 attempts average.

### With Abilities

- **Ink Memory (COUNTER)**: Major help. Purifies poison instantly. During boss: direct damage to Parasite AND control lock prevents poison chaos.
- **Shadow Decoy**: Strong help. Decoy eating poison apples clears the field. Boss fight: draws spit attacks.
- **Time Freeze**: Moderate help. Freezes Parasite progression. Gives breathing room.
- **Ink Flare**: Mild help. Flare could highlight which apple is real vs poison (reveals "true color" briefly).
- **Venom Trail (own ability, replay)**: Neutralizes poison apples by crossing them. Trivializes the identification challenge.

### Recommended First-Time Experience

This is a **mid-to-late game level** (5th-7th attempt). The deception mechanic is psychologically different from other levels - it tests observation, not reflexes or routing. Players who have beaten the more action-focused levels (Blackout, Famine, The Watcher) will find this a refreshing change of pace. The Parasite boss's "infection" mechanic is unique and memorable.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `PoisonApple` class - spawn logic, collision, `IsControlInverted()`, `GetSpeedMultiplier()`, `GetGrowAmount()`
- `PoisonApple::m_controlInverted` / `m_invertTimer` - existing control inversion system
- `PoisonApple::m_speedBoostTimer` / `m_speedBoostAmount` - existing speed boost
- `PoisonApple::m_consecutivePoisons` - tracks consecutive poison count
- `PoisonApple::OnRealAppleEaten()` - existing recovery mechanic

### New Code Required

- **Progressive visual deception**: Modify poison apple rendering to converge with real apple color over time (parameterized by `m_realApplesEaten`)
- **The Parasite Boss**: New class `src/bosses/ParasiteBoss.h/.cpp`
  - Attaches to snake tail (reference to snake body segment list)
  - Segment infection system (convert segments on timer)
  - Infection visual (color change per segment, pulsing)
  - Spit attack (projectile toward nearest real apple, converts to poison)
  - Consecutive real apple counter for win condition
  - Phase transitions (spit interval changes)
- **Venom Trail ability**: New class in ability system
  - Trail tile spawning on snake movement path
  - Trail tile lifetime (5 seconds with fade-out)
  - Enemy stun on trail crossing (modify `Predator` and `MirrorGhost` to check trail tiles)
  - Poison apple neutralization (trail tile overlapping poison apple converts it)
  - Glow rendering for trail tiles (acid-green gradient)

### Existing LevelConfig Values (reference)

```cpp
// Level 6: "Betrayal" -- Forest-green ink on poisoned page
l[5] = {
    6, "Betrayal", "Not everything green is good for you.",
    12.0f, 12, 4, 0.0f,
    sf::Color(12, 28, 12), sf::Color(35, 75, 35),
    sf::Color(55, 100, 50), sf::Color(75, 120, 65), sf::Color(185, 150, 40),
    false, false, false, false, false, true, false, false,
    0.0f, 2, 0,
    sf::Color(205, 212, 195), sf::Color(25, 50, 25), sf::Color(75, 160, 55),
    0.50f, false, false, false
};
```
