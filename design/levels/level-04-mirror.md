# Level 4: "Mirror Mirror" - Detailed Design Document

> *"Half-drawn ideas became echoes, stuck in loops."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Mirror Mirror |
| Subtitle | "Trust nothing. Especially yourself." |
| Hazard | Mirror Ghost (delayed copy that mirrors your movement) |
| Boss | The Doppelganger |
| Ability Earned | Shadow Decoy |
| Strong Counter Against | Level 8: The Watcher |
| Narrative Metaphor | The Artist sketched ideas on this page but never finished them, so they became echoes: repeating endlessly, half-formed, and dangerous. |

---

## Part 1: Stage Phase

### Gameplay

A translucent ghost snake mirrors the player's head position with a 5-frame delay, reflected across a vertical or horizontal axis. Touching the ghost is death. The player must think spatially: every move they make creates a delayed hazard on the opposite side of the arena.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 11.0 | Slower pace - the player needs time to think spatially |
| Apples to Win | 12 | Moderate count - each apple requires careful ghost avoidance |
| Shrink Interval | 4 apples | Arena tightens, making ghost avoidance harder |
| Ghost Delay | 5 ticks | Existing `GHOST_DELAY = 5` constant |
| Initial Mirror Axis | Vertical | Ghost mirrors across the vertical center line |
| Axis Flip Interval | Every 3 apples | Axis switches between Vertical and Horizontal |
| Star Threshold (2 stars) | 3 self-collisions | Moderate - ghost deaths are separate from self-collisions |
| Star Threshold (3 stars) | 1 self-collision | Very tight |

### Mirror Ghost Behavior (existing mechanic - with modifications)

- **Existing**: `MirrorGhost` class with `GHOST_DELAY = 5`, vertical/horizontal axis, `FlipAxis()`, `CheckCollision()`
- **Modification for redesign**: Add a visual "prediction line" showing where the ghost WILL be in 2 ticks (faint dotted outline). This gives the player just enough information to plan without removing the challenge.
- **Ghost body**: The ghost mirrors the snake's head position history, creating a trailing body. It doesn't grow independently - its length matches the player's movements over the delay window.

### Difficulty Curve Within Stage

- **Apples 1-3**: Vertical axis only. Ghost is on the opposite side of the screen. Plenty of room. Player learns the mirroring concept.
- **Apples 4-6**: First axis flip to Horizontal. Suddenly the ghost is above/below instead of left/right. Player must re-adapt. Ghost prediction line helps.
- **Apples 7-9**: Back to Vertical, but arena has shrunk. Ghost is closer. Routes that were safe before now bring you near the ghost's path.
- **Apples 10-12**: Final stretch. Axis flips every 2 apples. Arena is tight. The ghost's delayed path constantly threatens. Every move must account for where the ghost will be 5 ticks later.

### Visual Theme (existing - keep as-is)

- Paper tone: pure white drafting paper `(242, 242, 240)`
- Ink tint: graphite black `(18, 18, 20)`
- Accent: silver-gray `(175, 175, 180)`
- Corruption: 0.30 (moderate - the page has an uneasy quality, like a pencil sketch that's been erased and redrawn too many times)
- Monochrome palette reinforces the "mirror/reflection" theme

### Player Experience Goals

- **First attempts**: Player doesn't think about the ghost, moves normally, and suddenly collides with it. "Where did that come from?" Then they realize: it came from them.
- **Learning curve**: Player starts thinking 5 moves ahead. "If I go left now, the ghost will be on the right in 5 ticks - is that safe?" This spatial-temporal thinking is the core skill.
- **Mastery**: Player develops an intuition for the mirror. They can navigate tight spaces while keeping the ghost in safe zones. The axis flips become puzzles, not surprises.
- **Emotional note**: Paranoia and self-awareness. Your worst enemy is literally yourself. The grayscale palette makes the ghost feel like a shadow you can't escape.

---

## Part 2: Boss Phase - "The Doppelganger"

### Transition

After the 12th apple, the ghost suddenly stops mirroring. It holds its position for 2 seconds, then turns to face the player. Its body solidifies from translucent to fully opaque. Text appears: *"It learned to think for itself."*

### Boss Entity: The Doppelganger

The mirror ghost becomes an independent, aggressive enemy snake. It matches the player's length and copies the player's inputs with decreasing delay, making it increasingly hard to outmaneuver.

**Visual design:**
- Identical to the player's snake but inverted colors (white body on the graphite-colored arena vs the player's dark body)
- Eyes glow with a cold silver light
- Slight after-image trail (each segment leaves a fading ghost for 2 frames)
- When damaged, it glitches (segments briefly scatter then reform)

### Boss Mechanics

**Arena**: Fixed size (no shrinking during boss). The mirror axis line is visible as a faint silver line bisecting the arena.

**The Doppelganger's movement:**
- Copies the player's directional inputs with a delay
- **Phase 1 (HP 3)**: 5-tick delay (same as stage ghost). Player already knows this pattern. The difference: the ghost is now solid and actively pursuing.
- **Phase 2 (HP 2)**: 3-tick delay. Much faster to react. Player has less time to create separation.
- **Phase 3 (HP 1)**: 1-tick delay. Nearly simultaneous. The ghost moves almost exactly when you do. Only asymmetric movements (using the mirror line) can create safe space.

**How to damage:**
- The Doppelganger dies when it collides with the arena walls. The player survives wall proximity because they can turn away - the ghost, copying with delay, crashes.
- **Strategy**: Move toward a wall, then turn at the last moment. The ghost, delayed, continues into the wall.
- Each wall-crash = 1 damage. The ghost reforms at the mirror position of the player after 2 seconds.
- **Key insight**: The shorter the delay, the more precise the turn timing must be. At 1-tick delay, only the mirror-line asymmetry creates the split-second difference needed.

**The mirror line mechanic:**
- The vertical/horizontal line remains visible during the boss fight
- The ghost always mirrors across this line when it respawns
- The line flips axis after each damage phase
- **Critical**: When the ghost is at 1-tick delay, the most reliable way to kill it is to use the mirror line - move along it, then sharply move away. Your position and the ghost's mirrored position diverge just enough.

### Damage System

- **Hit condition**: Doppelganger collides with a wall while copying the player's input
- **Hits to defeat**: 3 (one per phase)
- **Counter-ability bonus (Ink Flare from L2)**: Ink Flare reveals the ghost's predicted path for the next 5 ticks as a bright golden line. This makes the 1-tick delay phase significantly more manageable - you can see exactly where the ghost will be.
- **Visual feedback on hit**: Ghost's body shatters into fragments, scattered across the arena. Fragments slowly reassemble at the mirror position. Screen briefly flashes to negative colors (black/white inversion).

### Boss Defeat

When HP reaches 0:
1. The Doppelganger shatters one final time but doesn't reassemble
2. The fragments hover in the air, then slowly drift toward the player snake
3. The fragments merge into the snake - it absorbs its own reflection
4. The mirror line fades. The snake now casts no reflection.
5. The page's grayscale palette softens, gaining subtle warmth

---

## Part 3: Ability Earned - Shadow Decoy

### Ability Acquisition Cutscene

After absorbing the Doppelganger:
1. The snake's shadow stretches, detaches, and stands beside it briefly
2. The shadow bows to the snake (acknowledging the original as master)
3. The shadow dissolves back into the snake
4. Text: *"Your shadow no longer follows. Now it obeys."*
5. Ability icon appears: a split silhouette (half solid, half ghost outline) in silver-gray

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Shadow Decoy |
| Cooldown | 12 seconds |
| Duration | 4 seconds |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Doppelganger (L4 boss) |

### Effect Details

When activated:
1. **Immediate**: A ghostly copy of the snake's head spawns at the player's position
2. **For 4 seconds**: The decoy moves in the OPPOSITE direction of the player at the same speed
   - If player goes left, decoy goes right
   - If player turns up, decoy turns down
3. **Enemy AI targeting**: All enemies (Predator, boss entities) switch their target from the player to the decoy for the duration
4. **Collision**: Enemies that reach the decoy are "stunned" for 2 seconds (they freeze in place, confused)
5. **After 4s**: Decoy fades with a ghost-dissolve effect

### Visual Transform (Snake Appearance During Shadow Decoy)

- **Player snake**: Darkens to near-black shadow. Outline becomes indistinct/blurred. Hard to see.
- **Decoy**: Rendered as a translucent ghost version of the snake (same as the Mirror Ghost visual style). Glows faintly silver.
- **The contrast**: Real snake hides, fake snake attracts. The player becomes the shadow, the shadow becomes "real."

### When Shadow Decoy is Most Useful

| Level | How Shadow Decoy Helps |
|---|---|
| L4 Mirror Ghost (home level) | Decoy confuses the mirror ghost - it doesn't know which snake to mirror |
| **L8 The Watcher (COUNTER)** | **Predator chases the decoy instead of you. 4 seconds of free apple eating while predator is distracted. If predator catches decoy, it's stunned for 2s.** |
| L6 Poison | Decoy absorbs poison apples for you, converting them to real ones |
| L10 The Eraser | Decoy draws boss attacks away during critical phases |
| L3 Quicksand (boss) | Decoy can lure The Mire in a specific direction |

---

## Part 4: Hidden Ability Interactions

When replaying Level 4 with abilities from other levels:

### Ink Flare + Mirror Ghost (from L2) - PRIMARY COUNTER

- Ink Flare reveals the ghost's exact position AND its predicted path for the next 5 ticks
- During boss fight: reveals the Doppelganger's next moves as a golden trail
- Makes Phase 3 (1-tick delay) much more manageable
- **Achievement**: "Seen Through" - defeat The Doppelganger using Ink Flare in every phase

### Shed Skin + Mirror Ghost (from L3)

- Shed segments act as walls that the ghost CAN collide with
- Strategic: drop shed segments in the ghost's predicted path to force it into a death zone
- During boss fight: shed segments can block the Doppelganger's path, causing additional wall-crashes
- **Achievement**: "Self-Sabotage" - defeat The Doppelganger using only Shed Skin traps (no wall-kills)

### Ink Anchor + Mirror Ghost (from L7)

- Ink Anchor makes the snake immovable for 5 seconds
- While anchored, the ghost passes through the snake harmlessly (no collision)
- Emergency panic button when the ghost is about to collide with you
- During boss fight: Doppelganger copies your "stop" and also freezes, resetting the chase

### Venom Trail + Mirror Ghost (from L6)

- Poison trail can stun the ghost for 3 seconds if it crosses it
- Creates safe zones: lay venom trail in the ghost's path, it gets stunned, you have breathing room
- During boss fight: venom trail stuns the Doppelganger, making wall-kills easier to set up

---

## Part 5: Post-Boss Cutscene

### "One Voice"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The grayscale page. Two identical ink lines run parallel across it - the snake and its ghost, drawn by the Artist long ago. One line is confident, the other shaky.
2. **(2-4s)** The shaky line slowly fades, absorbed into the confident one. The remaining line becomes bolder, cleaner. It was always meant to be one drawing, not two.
3. **(4-6s)** Color bleeds in from the edges. The white drafting paper gains subtle blue tones (like a blueprint). The page's purpose becomes clear: it was a design sketch. The Artist was planning something.
4. **(6-7s)** The Artist's hand appears. Closer than before. It reaches toward the page, almost touches it. The pen hovers.
5. **(7-8s)** The hand pulls back again. But slower this time. It's considering. Fade to stage select.

### Emotional Beat

The echo was a first draft that never got erased. By absorbing it, the snake completed what the Artist started - resolved the indecision. The hand reaching closer shows the Artist is paying attention now. The blueprint colors hint that this page was the PLANNING page - the Artist's vision for the whole notebook started here.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Medium-Hard**. The mirror mechanic is mentally taxing. Axis flips are disorienting. Players who think linearly will struggle; players who think spatially will thrive. Death count: 3-6 attempts average.
- Boss phase: **Hard**. The decreasing delay is the core challenge. Phase 1 is manageable (familiar from stage). Phase 2 is tricky. Phase 3 (1-tick delay) is the hardest boss phase in the game so far - requires precise timing and understanding of the mirror line. Deaths: 5-8 attempts average.

### With Abilities

- **Ink Flare (COUNTER)**: Major help. Seeing the ghost's path removes the mental overhead of tracking it. Boss Phase 3 goes from "nearly impossible" to "challenging but fair."
- **Shed Skin**: Creative help. Dropping obstacles in the ghost's path is satisfying and opens alternate strategies for the boss.
- **Time Freeze**: Moderate help. Freezing the ghost gives 4 seconds of free movement. Useful but doesn't teach the core mechanic.
- **Ink Anchor**: Emergency help. Passing through the ghost is a safety net, not a strategy. Good for survival, not for dealing damage.

### Recommended First-Time Experience

This is a **mid-game level** (3rd-5th attempt recommended). The spatial thinking required is more complex than Blackout or Quicksand. Players who have earned Ink Flare first will have a smoother experience. The boss's Phase 3 is designed to be the first real "wall" - a boss that makes you think "I need a specific ability to beat this comfortably."

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `MirrorGhost` class - full mirroring system with `GHOST_DELAY`, axis management, `MirrorPosition()`, collision detection
- `MirrorGhost::FlipAxis()` - axis switching already implemented
- `MirrorGhost::CheckCollision()` - ghost body collision already works
- `MirrorGhost::m_history` deque - position history tracking for delayed copy
- Graphite pencil visual theme colors already in LevelConfig

### New Code Required

- **Prediction line**: Add a `GetPredictedPositions(int ticks)` method to `MirrorGhost` that returns where the ghost WILL be in N ticks (extrapolate from current snake trajectory)
- **The Doppelganger Boss**: New class `src/bosses/DoppelgangerBoss.h/.cpp`
  - Input copy system (reads player's last N inputs, replays with delay)
  - Variable delay per phase (5 -> 3 -> 1)
  - Wall collision detection for damage
  - Respawn at mirror position after damage
  - Solid rendering (not translucent like stage ghost)
  - Shatter/reform animation
- **Shadow Decoy ability**: New class in ability system
  - Spawn ghost entity at player position, move in opposite direction
  - Enemy AI target override (redirect `Predator::m_targetMode` to decoy)
  - Stun effect on enemies that catch the decoy
  - 4-second lifetime with fade-out
- **Boss transition**: The stage ghost "solidifies" - transition from `MirrorGhost` rendering to `DoppelgangerBoss` rendering

### Existing LevelConfig Values (reference)

```cpp
// Level 4: "Mirror Mirror" -- Graphite pencil on white drafting paper
l[3] = {
    4, "Mirror Mirror", "Trust nothing. Especially yourself.",
    11.0f, 12, 4, 0.0f,
    sf::Color(18, 18, 22), sf::Color(115, 115, 120),
    sf::Color(35, 35, 40), sf::Color(70, 70, 75), sf::Color(230, 230, 225),
    false, false, false, false, false, false, false, true,
    0.0f, 3, 1,
    sf::Color(242, 242, 240), sf::Color(18, 18, 20), sf::Color(175, 175, 180),
    0.30f, false, false, false
};
```
