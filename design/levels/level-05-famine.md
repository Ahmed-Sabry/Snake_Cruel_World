# Level 5: "Famine" - Detailed Design Document

> *"Without the Artist's hand, nothing on this page lasts."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Famine |
| Subtitle | "Food is scarce. Patience is scarcer." |
| Hazard | Timed Apples (apples expire after a countdown) |
| Boss | The Hourglass |
| Ability Earned | Time Freeze |
| Strong Counter Against | Level 9: Control Shuffle |
| Narrative Metaphor | The Artist left this page mid-sentence; without their sustaining hand, nothing drawn here can persist. Everything fades. |

---

## Part 1: Stage Phase

### Gameplay

Every apple has a visible countdown timer. If the timer expires before the snake reaches it, the apple vanishes and a new one spawns elsewhere. Missing apples doesn't kill you, but it wastes time and builds pressure - the arena is shrinking, and every missed apple is a missed opportunity.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 14.0 | Fastest base speed of any level - urgency is the theme |
| Apples to Win | 20 | Highest apple count - tests endurance under pressure |
| Shrink Interval | 5 apples | Walls close every 5 apples |
| Apple Timer | 5.0s | Each apple exists for 5 seconds before vanishing |
| Star Threshold (2 stars) | 3 self-collisions | Moderate |
| Star Threshold (3 stars) | 0 self-collisions | Perfect run - speed without recklessness |

### Timed Apple Behavior (existing mechanic - with modifications)

- **Existing**: `TimedApple` class with `m_timerSec`, `m_timeRemaining`, `HasExpired()`, ring countdown visual
- **Modification for redesign**: Add a "desperation pulse" - when the apple has <1.5s remaining, it flashes rapidly and emits a faint sound cue. Creates urgency without being punitive.
- **Missed apple penalty**: No direct penalty, but the missed apple still counts toward the "apples needed" total. A new apple spawns immediately, resetting the timer. The pressure comes from the shrinking arena making each subsequent apple harder to reach in time.

### Difficulty Curve Within Stage

- **Apples 1-5**: 5.0s timer. Generous. Player learns the countdown rhythm and practices efficient routing.
- **Apples 6-10**: 4.5s timer. Slightly tighter. Player starts feeling the pressure of routing directly to apples rather than wandering.
- **Apples 11-15**: 4.0s timer. Arena has shrunk twice. Shorter distances help, but self-collision risk increases at high speed in tight spaces.
- **Apples 16-20**: 3.5s timer. Final stretch. Arena is small, speed is high, timers are tight. Every apple is a sprint. The feeling: "I'm starving and the food keeps disappearing."

### Visual Theme (existing - keep as-is)

- Paper tone: dried parchment `(225, 205, 170)`
- Ink tint: olive-green fading ink `(55, 65, 35)`
- Accent: dusty gold `(195, 170, 50)`
- Corruption: 0.40 (noticeable - the page feels dry, brittle, fading)
- The apple's ring countdown uses the accent gold color, creating a "sand timer" visual

### Player Experience Goals

- **First attempts**: Player moves at a normal pace and watches apples vanish before reaching them. "I need to be faster." The high base speed helps, but it also increases crash risk.
- **Learning curve**: Player develops route optimization - heading toward the apple the moment it spawns, cutting corners, using the growing body as a reference for safe paths.
- **Mastery**: Player enters a flow state: react, route, eat, repeat. The countdown becomes a rhythm, not a threat. The level feels like a speed run.
- **Emotional note**: Hunger and urgency. The fading olive-green palette reinforces the feeling of drought. Each eaten apple is a small victory against entropy.

---

## Part 2: Boss Phase - "The Hourglass"

### Transition

After the 20th apple, the countdown ring that was around the apple expands to fill the entire screen. It contracts into the center, forming a massive hourglass shape. Text appears: *"Time itself is hungry here."*

### Boss Entity: The Hourglass

A large ink-drawn hourglass entity at the center of the arena. It doesn't move. Instead, it drains time from everything around it - apples spawn with brutally short timers, and the hourglass itself is the source of the time drain.

**Visual design:**
- Classic hourglass shape, ~4 tiles tall, 2 tiles wide at the bulbs, 1 tile at the neck
- Drawn in dark olive-green ink with golden sand particles flowing through it
- The sand particles accelerate as the boss takes damage (time is running out for it too)
- Cracks appear on the glass after each hit, glowing golden through the fractures

### Boss Mechanics

**Arena**: Fixed size. The Hourglass sits at dead center, immovable. It's not a physical obstacle - the snake can pass through its position. But being near it drains apple timers faster (proximity penalty).

**Apple spawning:**
- Apples spawn with a 2.0-second timer (very short)
- The Hourglass has a "time drain" aura: apples within 4 tiles of it lose time 2x faster (effectively 1.0s timer)
- Apples far from the Hourglass (>4 tiles) keep their normal 2.0s timer

**How to damage:**
- Eating an apple within 0.5 seconds of it spawning (catching it almost instantly) sends a "time shockwave" that cracks the Hourglass
- The timing window is tight: the apple must be eaten within 0.5s of spawn, not within 0.5s of expiry
- Visual tell: when an apple first spawns, it has a bright golden flash for 0.5s. Eating during the flash = crack
- Missing the timing window just feeds the snake normally (no damage to boss)

**Boss behavior:**
- The Hourglass periodically "pulses" (every 8 seconds), instantly expiring all currently active apples and spawning a new one. This resets the board and forces reactive play.
- After each crack, the pulse interval shortens (8s -> 6s -> 5s -> 4s -> 3s)
- The Hourglass also rotates slowly. As it rotates, the time drain aura shifts direction, creating a moving "danger zone" around it.

### Boss Phases

- **Phase 1 (Cracks 0-1)**: 2.0s apple timers. 8s pulse interval. Drain aura is small (3 tiles). Player learns the "catch on spawn" mechanic.
- **Phase 2 (Cracks 2-3)**: 1.8s apple timers. 6s pulse interval. Drain aura expands to 4 tiles. Hourglass starts rotating (drain zone moves).
- **Phase 3 (Cracks 4)**: 1.5s apple timers. 4s pulse interval. Drain aura = 5 tiles (covers most of the arena). Only apples near the walls have full timers. The final crack requires pixel-perfect reaction.

### Damage System

- **Hit condition**: Eat apple within 0.5s of its spawn
- **Hits to defeat**: 5 (more hits than other bosses - but each requires precision, not endurance)
- **Counter-ability bonus (Hunter's Dash from L8)**: Dash auto-collects apples in path. If you dash toward a just-spawned apple, you can cover the distance instantly and guarantee the timing window. Reduces the precision requirement significantly.
- **Visual feedback on hit**: Golden crack appears on the hourglass with a satisfying glass-breaking sound. Sand particles spray outward. Time briefly slows for 0.5s (ironic - the time boss gives you a moment of slow-motion on hit).

### Boss Defeat

When the 5th crack lands:
1. The Hourglass shatters - glass fragments fly outward in slow motion
2. Golden sand pours out, pooling on the arena floor
3. The sand transforms into golden ink, spreading across the page
4. Time resumes at normal pace. The golden ink settles into warm, permanent drawings - things that will LAST now
5. The page's fading olive tones become rich, warm gold

---

## Part 3: Ability Earned - Time Freeze

### Ability Acquisition Cutscene

After the Hourglass shatters:
1. The scattered sand swirls around the snake like a golden tornado
2. The sand crystallizes on the snake's body, then absorbs inward
3. The world briefly freezes - everything stops except the snake for 1 second (preview of the ability)
4. Text: *"You broke the clock. Now time answers to you."*
5. Ability icon appears: a cracked hourglass with frozen sand in gold/blue

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Time Freeze |
| Cooldown | 15 seconds (longest cooldown - most powerful ability) |
| Duration | 4 seconds |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Hourglass (L5 boss) |

### Effect Details

When activated:
1. **Immediate**: Screen desaturates slightly. Chromatic aberration pulse outward from snake.
2. **For 4 seconds**: ALL timers in the game are frozen:
   - Apple expiry timers stop (timed apples don't vanish)
   - Control shuffle countdown stops (no shuffle during freeze)
   - Predator movement freezes (frozen in place)
   - Boss attack patterns pause (boss timers stop)
   - Earthquake timers stop (no quakes during freeze)
   - Mirror ghost freezes in current position
3. **The snake still moves normally** - full speed, full control
4. **End**: Color returns over 0.3s. All timers resume from where they were.

### Visual Transform (Snake Appearance During Time Freeze)

- **Snake body**: Turns crystalline ice-blue. Segments have a faceted, frozen appearance (angular instead of smooth).
- **Eyes**: Glow bright blue-white, like frozen stars.
- **Environment**: Everything except the snake gets a blue-gray desaturation filter. Particles freeze mid-air. The visual effect makes it clear that time has stopped for everything except you.
- **Particle effect**: Tiny crystalline ice particles drift slowly around the snake (frozen in mid-float, not falling).

### When Time Freeze is Most Useful

| Level | How Time Freeze Helps |
|---|---|
| L5 Famine (home level) | Freezes apple timers, giving unlimited time to reach them |
| **L9 Control Shuffle (COUNTER)** | **Freezes the shuffle timer, preventing control remapping for 4 seconds. During boss fight: freezes The Scrambler's tentacle attacks.** |
| L2 Blackout | Freezing during light phase prevents blackout from starting (extended visibility) |
| L8 The Watcher | Predator freezes in place - 4 seconds of free movement |
| L7 Earthquake | Earthquake timers stop - no wall shifts for 4 seconds |
| L4 Mirror Ghost | Ghost freezes in current position - safe to navigate near it |

---

## Part 4: Hidden Ability Interactions

When replaying Level 5 with abilities from other levels:

### Hunter's Dash + Timed Apples (from L8) - PRIMARY COUNTER

- Dash covers 4 tiles instantly, auto-collecting apples in path
- Reach expiring apples before their timer runs out, even from across the arena
- During boss fight: dash to just-spawned apples guarantees the 0.5s timing window
- **Achievement**: "Fast Food" - complete L5 boss using only dash-catches (never walk to an apple)

### Ink Flare + Timed Apples (from L2)

- Flare reveals where the NEXT apple will spawn before the current one expires
- Shows a faint golden outline at the next spawn position for 3 seconds
- Allows pre-positioning: start moving toward the next apple before the current one vanishes
- **Achievement**: "Prescient Hunger" - eat 10 apples in a row without any expiring (using Flare to pre-position)

### Shadow Decoy + Timed Apples (from L4)

- The decoy can "eat" apples too (but doesn't count toward score)
- Useful in boss fight: decoy eats apples near the Hourglass's drain aura, preventing them from being wasted
- The decoy's apple eat doesn't crack the Hourglass (only player eats count)

### Ink Memory + Timed Apples (from L9)

- Ink Memory locks controls for 6 seconds - at high speed, stable controls prevent routing mistakes
- Indirect help: the main challenge of L5 is routing efficiently at high speed, and locked controls ensure no accidental direction changes

---

## Part 5: Post-Boss Cutscene

### "Evergreen"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The shattered hourglass lies in pieces on the parchment. Golden sand is scattered everywhere. Everything is still.
2. **(2-4s)** The sand begins to sink into the page. Where it absorbs, the faded olive-green ink darkens and deepens. The dried parchment gains moisture, color, and life.
3. **(4-6s)** From the enriched ink, evergreen trees grow upward - tall, permanent, enduring. The page was meant to be a forest. The Artist's vision: things that grow and stay.
4. **(6-7s)** The Artist's hand appears. It's closer than ever before. The fingers twitch slightly - almost reaching for the pen. The pen lies at the bottom of the frame, within reach.
5. **(7-8s)** The hand doesn't pick up the pen. But it rests beside it now, no longer pulling away. Progress. Fade to stage select.

### Emotional Beat

Time was the enemy because without the Artist, nothing endured. But the snake proved that persistence can outlast decay. The evergreen forest - things that grow and stay - is the opposite of the famine. The Artist's hand resting beside the pen, no longer pulling away, shows growing resolve. Halfway there.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Medium-Hard**. High speed + timed apples = constant pressure. No time to think, only react. Players who are good at snake routing will thrive; players who explore will struggle. Deaths: 3-5 attempts average.
- Boss phase: **Hard**. The 0.5s timing window for cracks is demanding. The pulse mechanic that expires all apples forces reactive play. The drain aura means positioning matters. Deaths: 5-8 attempts average. Key learning: position near apple spawn locations, not near the boss.

### With Abilities

- **Hunter's Dash (COUNTER)**: Major help. Dash-to-apple eliminates the routing challenge. Boss becomes much easier (guaranteed timing window on dash).
- **Time Freeze (own ability, replay)**: Trivializes stage (freeze apple timers). During boss fight, freezing the pulse timer gives extra time.
- **Ink Flare**: Strong help. Pre-positioning from knowing next apple location is a significant advantage.
- **Shadow Decoy**: Moderate help. Decoy eating drain-zone apples prevents waste.
- **Shed Skin**: Minimal help. Shorter body = faster routing is marginal.

### Recommended First-Time Experience

This is a **mid-to-late game level** (4th-6th attempt recommended). The high speed and tight timers demand good fundamental snake skills. Players who have already beaten 2-3 other levels will have the routing instincts needed. Time Freeze is the game's most universally powerful ability because it freezes all timers, so the boss should feel like a worthy challenge to earn it.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `TimedApple` class - timer management, `HasExpired()`, `OnAppleEaten()`, ring countdown rendering
- `TimedApple::m_ring` - visual countdown ring (can be extended for boss proximity effects)
- `TimedApple::GetTimeRemaining()` / `GetTimerDuration()` - already provide the data needed for desperation pulse logic
- High base speed (14.0) already set in LevelConfig

### New Code Required

- **Desperation pulse**: Add flash/sound when `GetTimeRemaining() < 1.5f` in `TimedApple::Render()`
- **Progressive timer shortening**: Stage phase apple timer decreases at apple milestones (5.0 -> 4.5 -> 4.0 -> 3.5)
- **The Hourglass Boss**: New class `src/bosses/HourglassBoss.h/.cpp`
  - Static entity at arena center (no movement, but rotates)
  - Time drain aura (proximity check, apple timer multiplier)
  - Pulse mechanic (expire all apples on timer, timer shortens per crack)
  - Crack tracking (5 hits to defeat)
  - Spawn-timing detection (check if apple was eaten within 0.5s of spawn)
  - Rotation system for drain aura direction
- **Time Freeze ability**: New class in ability system
  - Global timer pause (all game timers multiply by 0 for duration)
  - Affects: `TimedApple`, `ControlShuffle`, `Predator`, `Earthquake`, `MirrorGhost`, boss timers
  - Visual: desaturation shader + chromatic aberration (use existing `PostProcessor` shaders)
  - Snake remains unaffected (normal speed, normal input)
- **Boss transition**: Countdown ring expands to form hourglass shape (animation sequence)

### Existing LevelConfig Values (reference)

```cpp
// Level 5: "Famine" -- Fading olive-green ink on dried parchment
l[4] = {
    5, "Famine", "Food is scarce. Patience is scarcer.",
    14.0f, 20, 5, 0.0f,
    sf::Color(25, 28, 18), sf::Color(85, 85, 45),
    sf::Color(65, 75, 30), sf::Color(90, 100, 50), sf::Color(210, 170, 40),
    false, false, false, false, false, false, true, false,
    5.0f, 3, 0,
    sf::Color(225, 205, 170), sf::Color(55, 65, 35), sf::Color(195, 170, 50),
    0.40f, false, false, false
};
```
