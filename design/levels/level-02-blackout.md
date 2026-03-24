# Level 2: "Lights Out" - Detailed Design Document

> *"The Artist stopped drawing the light, so darkness filled the void."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Lights Out |
| Subtitle | "Hope you memorized the layout." |
| Hazard | Blackout (cycling darkness) |
| Boss | The Blind Ink |
| Ability Earned | Ink Flare |
| Strong Counter Against | Level 4: Mirror Ghost |
| Narrative Metaphor | The Artist abandoned this page before drawing the light, so darkness crept in to fill the empty space. |

---

## Part 1: Stage Phase

### Gameplay

The player must eat apples while navigating periodic blackouts. During blackouts, the entire screen goes dark except for a faint glow around the snake's head.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 11.0 | Slightly slower than L1 to compensate for vision loss |
| Apples to Win | 12 | Fewer apples since each one is harder to find |
| Shrink Interval | 5 apples | Walls close in every 5 apples |
| Light Duration | 4.0s | Time with full visibility |
| Dark Duration | 3.0s | Time in total darkness |
| Star Threshold (2 stars) | 5 self-collisions | Generous - darkness makes collisions common |
| Star Threshold (3 stars) | 2 self-collisions | Tight - rewards memorization and caution |

### Blackout Behavior (existing mechanic - keep as-is)

- **Light phase**: Full visibility, normal gameplay. Player should use this time to plan routes.
- **Dark phase**: Screen covered by dark overlay. Only the snake's head is visible as a small glowing beacon (existing `m_headGlow` in `BlackoutEffect`).
- **Cycle**: Light -> Dark -> Light -> Dark (repeating)
- **Transition**: Instant toggle (no fade - the "lights cut out" feeling)

### Difficulty Curve Within Stage

- **Apples 1-4**: Light duration is generous (4s light / 3s dark). Player learns the rhythm.
- **Apples 5-8**: Light duration shortens to 3s. Dark extends to 4s. Pressure increases.
- **Apples 9-12**: Light 2.5s / Dark 4.5s. Player must memorize apple positions during light and navigate blind.

### Visual Theme (existing - keep as-is)

- Paper tone: warm parchment `(195, 185, 165)`
- Ink tint: dark navy `(25, 25, 55)`
- Accent: amber/candlelight `(220, 130, 65)`
- Corruption: 0.15 (subtle wobble)
- No post-processing shaders active (clean, calm page - the darkness IS the corruption)

### Player Experience Goals

- **First attempts**: Player dies during blackouts, crashing into walls they forgot were there. It feels harsh, but fair - the light phases give enough time to plan.
- **Learning curve**: Player starts memorizing the grid layout, counting tiles from walls.
- **Mastery**: Player navigates confidently during darkness, using the head glow as orientation.
- **Emotional note**: The level should feel lonely and tense - you're the only light left in a dark world.

---

## Part 2: Boss Phase - "The Blind Ink"

### Transition

After eating the 12th apple, the screen flashes white briefly, then plunges into **permanent darkness**. The borders pulse once to show the boss arena (slightly smaller than the stage arena). Text appears: *"Something lives in the dark."*

### Boss Entity: The Blind Ink

A giant ink blot - a sentient pool of darkness with no fixed shape. It appears as a pulsing, amorphous blob (3x3 to 5x5 tiles, fluctuating) that moves slowly around the arena.

**Visual design:**
- Rendered as overlapping semi-transparent dark circles with wobble (use existing corruption wobble system)
- Faint purple-black glow at edges (just barely visible in the darkness)
- When damaged, it shrieks (visual: rapid size fluctuation + ink splatter particles)

### Boss Mechanics

**Arena**: Permanently dark. No light/dark cycling. The only light sources are:
1. The snake's head glow (existing, small radius)
2. "Light apples" (new variant - golden apples that emit a visible glow radius of ~4 tiles)

**Light Apples:**
- Spawn one at a time in random valid positions
- Emit a soft golden glow visible even in darkness (rendered as a circular gradient)
- When the snake eats a light apple near the Blind Ink (within 3 tiles), the light burns the boss for 1 damage
- When the snake eats a light apple far from the Blind Ink (more than 3 tiles), it just feeds the snake normally (no damage)
- The Blind Ink tries to reach and absorb light apples before the player. If it absorbs one, it grows slightly larger and a new apple spawns elsewhere.

**Boss AI - The Blind Ink Movement:**
- Phase 1 (HP 3): Moves slowly toward the nearest light apple. Speed = 3 tiles/sec. Gives player time to reach apple first.
- Phase 2 (HP 2): Speed increases to 5 tiles/sec. Player must be more strategic about approach angles.
- Phase 3 (HP 1): Speed = 7 tiles/sec. Boss also starts spawning "ink puddles" (1-tile dark patches that kill the snake on contact) along its movement path. These fade after 3 seconds.

**Collision:**
- Snake touching the Blind Ink body = death (restart boss phase)
- Snake touching ink puddles (Phase 3) = death
- Boss cannot leave the arena boundaries

### Damage System

- **Hit condition**: Eat a light apple while within 3 tiles of any part of the Blind Ink's body
- **Hits to defeat**: 3
- **Counter-ability bonus (Ink Flare)**: If player replays this level with Ink Flare, using the ability reveals the exact position and shape of the Blind Ink for 3 seconds. It also counts as a "light burst" that deals 1 damage if the boss is within 5 tiles. Effectively 2 damage per use (the flare itself + easier apple positioning).
- **Visual feedback on hit**: Boss recoils (shrinks rapidly for 0.5s), bright flash in the darkness, ink splatter particles radiate outward. Screen briefly illuminates to show the boss wounded.

### Boss Defeat

When HP reaches 0:
1. The Blind Ink lets out a final pulse (expanding ring of light)
2. It collapses inward, dissolving into small ink droplets
3. The arena gradually fills with warm amber light (the candlelight returns)
4. The page is healed - light was restored

---

## Part 3: Ability Earned - Ink Flare

### Ability Acquisition Cutscene

After the Blind Ink dissolves:
1. The dissolved ink swirls and flows toward the snake
2. The snake absorbs it - body briefly glows bright white
3. Text: *"The darkness taught you to carry your own light."*
4. Ability icon appears: a radiating star/flare symbol in gold
5. Brief tutorial prompt: "Press [ABILITY KEY] to use Ink Flare" (shown once)

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Ink Flare |
| Cooldown | 10 seconds |
| Duration | 3 seconds |
| Activation Key | Spacebar (or configurable ability key) |
| Available After | Defeating The Blind Ink (L2 boss) |

### Effect Details

When activated:
1. **Immediate**: Bright flash pulse radiates from snake (visual: expanding golden circle, 0.3s)
2. **For 3 seconds**: Entire screen is illuminated. All hidden elements become visible:
   - Mirror Ghost position and trajectory (L4)
   - Next apple spawn location (shown as faint outline)
   - Boss weak points glow (in any boss fight)
   - Predator position through walls (L8)
   - Quicksand patches highlighted with warning color (L3)
3. **End**: Light fades back to normal over 0.5s

### Visual Transform (Snake Appearance During Ink Flare)

- Snake body: bright white with golden edges
- Snake eyes: become large glowing lanterns (bright circles instead of normal eye dots)
- Head: radiates light (circular gradient emanating from head, radius ~6 tiles)
- Body segments: each segment has a small light glow (diminishing from head to tail)
- Particle effect: tiny golden sparks drift upward from the snake's body

### When Ink Flare is Most Useful

| Level | How Ink Flare Helps |
|---|---|
| L2 Blackout (home level) | Illuminates during blackout, reveals apple positions |
| **L4 Mirror Ghost (COUNTER)** | **Reveals exact ghost position and predicted path - the key advantage** |
| L5 Timed Apples | Shows where next apple will spawn before current expires |
| L8 The Watcher | Reveals predator position even when off-screen |
| L10 The Eraser | Reveals boss weak points during dark phases |

---

## Part 4: Hidden Ability Interactions

When replaying Level 2 with abilities from other levels:

### Venom Trail + Blackout (from L6)

- The poison trail left by Venom Trail **glows in the dark** with an acid-green light
- This creates a player-made light source: your own path illuminates behind you
- Useful for navigation AND for trapping the Blind Ink boss (the trail damages it too)
- **Achievement**: "Neon Breadcrumbs" - complete L2 boss using only Venom Trail for light

### Hunter's Dash + Blackout (from L8)

- Dashing during blackout leaves a brief light trail along the dash path (cyan flash)
- The dash auto-collects apples, so you can dash toward a remembered apple position in the dark
- Adds a high-skill speedrun option for the level

### Time Freeze + Blackout (from L5)

- Freezing time during a light phase prevents the blackout from starting for 4 seconds
- Extends your visibility window - useful during boss fight to have more time to reach light apples

### Shed Skin + Blackout (from L3)

- Shed segments glow faintly for 5 seconds before disappearing
- Creates temporary light landmarks in the arena
- Strategic: drop shed segments at key navigation points to remember the layout

---

## Part 5: Post-Boss Cutscene

### "The Light Returns"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The arena is dark. A single warm amber dot appears at center (where the boss died).
2. **(2-4s)** The dot expands slowly into a circle of warm light. Ink stains on the page begin to recede, pulling back like water evaporating.
3. **(4-6s)** The full page is revealed - clean, warm parchment. Faint ruled notebook lines appear. The page looks how the Artist originally intended it.
4. **(6-7s)** In the background, barely visible: the shadow of the Artist's hand, hovering above the page. It hesitates, trembles slightly.
5. **(7-8s)** The hand pulls away. Not yet. But it noticed. Fade to stage select.

### Emotional Beat

This is the first page the snake heals. The player should feel: "I did that. I brought the light back." The Artist's hesitating hand is a subtle tease - the narrative seed that pays off 7 bosses later.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Medium difficulty**. Blackouts are disorienting but the snake head glow provides enough to avoid walls if you're careful. The shortening light phases create tension.
- Boss phase: **Hard**. Permanent darkness + mobile boss + light apple racing = intense. Players will die 3-5 times on average. The key learning moment is realizing you need to eat apples near the boss, not just anywhere.

### With Counter-Ability (Ink Flare from this level - replaying)

- Not applicable (you earn Ink Flare FROM this level). But on replay, Ink Flare makes the boss significantly easier (reveals boss position + deals direct damage).

### With Other Abilities

- **Hunter's Dash**: Makes boss easier (dash to light apples faster in the dark)
- **Time Freeze**: Moderate help (freeze boss movement to eat light apple safely)
- **Shed Skin**: Creative help (glow landmarks)
- **Shadow Decoy**: Minimal help (boss targets apples, not snake)

### Recommended First-Time Experience

This should be one of the **first 2-3 levels** a player attempts from the stage select. The blackout mechanic is simple to understand ("it gets dark"), the boss has clear rules, and the earned ability (Ink Flare) is universally useful. It's a good "gateway" level.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `BlackoutEffect` class (cycling dark/light, head glow rendering) - use as-is for stage phase
- `BlackoutEffect::RenderTo()` for the dark overlay
- `World::SpawnApple()` logic for light apple spawning (extend with glow rendering)
- `ParticleSystem` InkSplat effect for boss damage feedback

### New Code Required

- **Light Apple variant**: Extend apple rendering to support a glow radius (circular gradient shader or soft circle overlay)
- **Blind Ink Boss entity**: New class in `src/bosses/BlindInkBoss.h/.cpp`
  - Amorphous shape rendering (overlapping circles with wobble)
  - Simple pathfinding toward nearest light apple
  - Phase transitions (speed increase, ink puddle spawning)
  - Hit detection (proximity check when snake eats light apple)
- **Ink Flare ability**: New class in ability system
  - Screen illumination effect (temporarily disable blackout overlay + add golden tint)
  - Element reveal rendering (ghost outlines, apple spawn preview, etc.)
  - Snake visual transform (white/gold body with glow particles)
- **Boss transition**: PlayState logic to switch from stage phase to boss phase after apple count reached

### Existing LevelConfig Values (reference)

```cpp
// Level 2: "Lights Out" -- Dark India ink with amber candlelight
l[1] = {
    2, "Lights Out", "Hope you memorized the layout.",
    11.0f, 12, 5, 0.0f,
    sf::Color(8, 8, 18), sf::Color(45, 45, 75),
    sf::Color(210, 120, 50), sf::Color(170, 95, 40), sf::Color(230, 175, 55),
    true, false, false, false, false, false, false, false,
    0.0f, 5, 2,
    sf::Color(195, 185, 165), sf::Color(25, 25, 55), sf::Color(220, 130, 65),
    0.15f, false, false, false
};
```
