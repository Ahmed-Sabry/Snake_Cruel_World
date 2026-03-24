# Level 9: "Amnesia" - Detailed Design Document

> *"The rules of this page were never defined. So they keep changing."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Amnesia |
| Subtitle | "What controls? What controls." |
| Hazard | Control Shuffle (direction inputs remap on a timer, with warnings that become less informative as the level escalates) |
| Boss | The Scrambler |
| Ability Earned | Ink Memory |
| Strong Counter Against | Level 6: Poison |
| Narrative Metaphor | The Artist never decided what this page was supposed to obey. Directions, labels, and meaning were left unfinished, so the page keeps rewriting its own rules. |

---

## Part 1: Stage Phase

### Gameplay

This stage attacks the player's trust in their own hands. Controls remap at intervals, but the game warns the player before each shuffle. Early on, the warning is generous and fully readable. Later, it becomes brief, partial, and increasingly unreliable. The challenge is not pure reaction speed - it is keeping composure while your input language keeps changing underneath you.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 10.0 | Deliberately slower than most late-game stages so cognition, not raw reflex, is the main challenge |
| Apples to Win | 15 | Standard count - enough time for all three shuffle phases to matter |
| Shrink Interval | 5 apples | Moderate wall pressure |
| Shuffle Phase 1 | Apples 1-4 | "Learning" phase |
| Shuffle Phase 2 | Apples 5-9 | "Chaos" phase |
| Shuffle Phase 3 | Apples 10-15 | "Amnesia" phase |
| Base Shuffle Cycle | 15.0s / 12.0s / 9.0s | Existing per-phase cadence including warning time |
| Warning Duration | 2.0s / 2.0s / 1.0s | Existing per-phase countdown |
| Mapping Indicator | Full / Full / Partial | Existing system shows all 4 mappings early, then only 2 in phase 3 |
| Post-Shuffle Grace | 0.5s | Existing wall-death forgiveness window after shuffle |
| Star Threshold (2 stars) | 5 self-collisions | Slightly generous - mental overload is the point |
| Star Threshold (3 stars) | 2 self-collisions | Strong execution under confusion |

### Control Shuffle Behavior (existing mechanic - with modifications)

- **Existing**: `ControlShuffle` state machine with `Idle -> Warning -> Indicating`, timed remapping, warning bar, mapping overlay, and 0.5-second grace period after a shuffle.
- **Existing**: Three escalation phases based on apples eaten:
  - **Phase 1**: one axis swaps only
  - **Phase 2**: full 4-direction shuffle
  - **Phase 3**: full shuffle plus only 2 mapping hints shown
- **Existing**: `PlayState` remaps input before snake direction validation, which means the player must mentally translate controls before the normal 180-degree-reversal rule is applied.
- **Modification for redesign**: Add "memory smear" feedback after each shuffle. For about 0.75 seconds, directional UI text leaves ghost copies behind, making the screen feel like the page is rewriting itself.
- **Modification for redesign**: During the last 5 apples, subtle false labels briefly flicker near the indicator before fading, reinforcing the theme of unstable rules without changing the actual mapping beyond the existing system.

### Difficulty Curve Within Stage

- **Apples 1-4**: The player learns the warning rhythm. Only one axis changes, and the full mapping is shown clearly. It feels weird, but understandable.
- **Apples 5-9**: Full shuffles begin. The warning is still readable, but the player's habit memory starts betraying them. Mistakes now come from panic, not ignorance.
- **Apples 10-15**: The page stops helping. Only two mappings are shown. The player must infer the other two through movement, deduction, and composure under pressure.

### Visual Theme (existing - keep and emphasize)

- Paper tone: lavender-gray `(218, 205, 228)`
- Ink tint: dark violet `(50, 25, 60)`
- Accent: electric magenta-violet `(160, 90, 200)` with cyan confusion highlights `(100, 180, 210)`
- Corruption: 0.85 (extreme - nearly the final threshold before total collapse)
- Ink bleed, chromatic aberration, and psychedelic hue shift all stay active on this page, making it feel like wet ink and memory loss at the same time

### Player Experience Goals

- **First attempts**: The player knows the game warned them, but still crashes. "I saw it, I just couldn't think fast enough."
- **Learning curve**: The player stops relying on raw muscle memory and starts reading the warning bar, mapping text, and their own current heading before committing.
- **Mastery**: The player treats each shuffle as a quick puzzle, adapts instantly, and uses the grace window to test a safe move rather than panic-turning.
- **Emotional note**: This level should feel like cognitive erosion. Not cruelty through speed or pursuit, but cruelty through destabilized certainty.

---

## Part 2: Boss Phase - "The Scrambler"

### Transition

After the 15th apple, the control indicator tears itself off the HUD and splashes into the center of the arena. Four ink limbs grow from it, each clutching a direction label. Text appears: *"The page can't remember what 'up' means."*

### Boss Entity: The Scrambler

The Scrambler is a central octopus-like knot of living ink with four tentacles, each representing one direction. It does not chase the player directly. Instead, it corrupts the control scheme by physically tugging the page's directions out of place. The fight is about targeted severing under worsening cognitive load.

**Visual design:**
- Central bulb of dark violet ink, like a spilled thought pooling on wet paper
- Four tentacles radiate outward, each ending in a hand-drawn direction glyph: `Up`, `Down`, `Left`, `Right`
- Before a tentacle acts, its glyph glows cyan-white, clearly telegraphing which direction is about to be affected
- Severed tentacles don't vanish cleanly; they smear into the page and leave that direction permanently corrupted

### Boss Mechanics

**Arena:**
- Fixed-size arena with the Scrambler anchored in the middle
- The boss body is a hazard zone the player should route around, but the tentacles extend outward into reachable lanes
- Apples spawn near the currently active tentacle, baiting the player toward the next target

**The Scrambler's behavior:**
- One tentacle becomes the "active" tentacle at a time, glowing to signal its direction is about to be remapped
- A shuffle occurs on that tentacle's timer, distorting that direction first or most visibly
- Eating the apple near the active tentacle severs it, dealing 1 damage and removing that tentacle from the boss
- After a tentacle is severed, that direction becomes **permanently locked to a random mapping** for the rest of the fight

**How to damage:**
- Read which tentacle is glowing
- Route to the apple positioned near that tentacle
- Eat it before the next scramble resolves to sever the limb
- Repeat until all four tentacles are cut

### Boss Phases

- **Phase 1 (4 tentacles)**: One glowing tentacle at a time. Long telegraph. Only the active direction is emphasized. Player learns the sever rule.
- **Phase 2 (3 tentacles)**: Telegraphs shorten. Two tentacles may twitch at once, but only one glows fully. One severed direction is now permanently random, so movement planning requires real adaptation.
- **Phase 3 (2 tentacles)**: Two directions are permanently scrambled. The active apple spawns farther from the player's likely route, forcing confident commitment.
- **Phase 4 (1 tentacle)**: Three directions are permanently scrambled. The final intact tentacle is the only trustworthy directional relationship left. The player must use that last island of certainty to land the finishing sever.

### Damage System

- **Hit condition**: Eat the apple attached to the currently active tentacle before its scramble window resolves
- **Hits to defeat**: 4
- **Counter-ability bonus (Time Freeze from L5)**: Freezes the shuffle timer and tentacle telegraphs for 4 seconds, preserving the player's understanding of the current mapping long enough to route calmly. It is the cleanest answer to the fight's cognitive pressure.
- **Secondary bonus (Hunter's Dash from L8)**: Dash follows current facing direction, not remapped input intent, so it can cut through confusion and reach the active tentacle's apple before the next scramble.
- **Visual feedback on hit**: The severed tentacle recoils and sprays violet ink across the page. The corresponding direction label on-screen fractures and reappears in a wrong place, showing that the page has "remembered it wrong" permanently.

### Boss Defeat

When the fourth tentacle is severed:
1. The central knot collapses inward like a crumpled note
2. All floating direction labels snap back into clean alignment
3. The psychedelic shimmer drains out of the page, leaving precise ruled lines
4. The wet, bleeding paper resolves into a neat diagram page - margins, arrows, annotations, meaning restored
5. For the first time, the page obeys a single coherent logic

---

## Part 3: Ability Earned - Ink Memory

### Ability Acquisition Cutscene

After the Scrambler collapses:
1. The snapped-back direction labels orbit the snake in a clean square
2. Each label dissolves into stable violet ink and sinks into a different segment of the snake's body
3. The snake's outline sharpens - no wobble, no corruption drift
4. Text: *"You remembered what the page forgot."*
5. Ability icon appears: a violet square of four arrows locked into place

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Ink Memory |
| Cooldown | 10 seconds |
| Duration | 6 seconds |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Scrambler (L9 boss) |

### Effect Details

When activated:
1. **For 6 seconds**: Current controls are locked and cannot be remapped by control shuffle effects
2. **Purification effect**: Any active poison/control corruption is cleansed immediately
3. **Stability effect**: Visual wobble on the snake reduces sharply, communicating restored certainty
4. **Memory window**: If a shuffle would occur during the duration, the player keeps the locked mapping until Ink Memory ends

### Visual Transform (Snake Appearance During Ink Memory)

- **Snake body**: Turns deep violet with crisp, clean edges instead of sketchy wobble
- **Eyes**: Glow steady lavender-white, calm rather than aggressive
- **Body pattern**: Each segment carries a faint arrow glyph, reinforcing directional certainty
- **Screen effect**: Nearby chromatic fringing collapses inward around the snake, creating a pocket of visual clarity

### When Ink Memory is Most Useful

| Level | How Ink Memory Helps |
|---|---|
| L9 Amnesia (home level) | Locks current controls and ignores shuffles during the duration |
| **L6 Poison (COUNTER)** | **Purifies poison instantly, cancels control inversion, and creates a stable input window during the Parasite fight.** |
| L5 Famine | Removes routing mistakes caused by panic and keeps movement stable during fast apple runs |
| L10 The Eraser | Protects one phase from input corruption when multiple hazards overlap |
| L8 The Watcher | Lets the player keep exact control while escaping hunt lines or lining up a dash route |

---

## Part 4: Hidden Ability Interactions

When replaying Level 9 with abilities from other levels:

### Time Freeze + Amnesia (from L5) - PRIMARY COUNTER

- Freezes shuffle timers completely
- Preserves the currently known mapping for 4 seconds
- During boss: freezes the active tentacle's countdown, giving the player time to reach the sever apple
- **Achievement**: "Lucid Interval" - defeat The Scrambler without being hit by a single unexpected shuffle after phase 1

### Hunter's Dash + Amnesia (from L8)

- Dash always goes in the direction the snake is currently facing, overriding the practical pain of remapped moment-to-moment steering
- Lets the player bypass late-phase confusion and reach safe lanes or active tentacle apples instantly
- **Achievement**: "Muscle Memory" - finish the boss with the last two tentacles severed via dash-assisted apples

### Ink Anchor + Amnesia (from L7)

- Anchor creates a pocket of stability that visually clarifies directional labels near the snake
- During boss: tentacle telegraphs remain crisp and readable while the anchor is active
- Doesn't stop remapping, but lowers the perception load by reducing screen chaos

### Ink Flare + Amnesia (from L2)

- Flare reveals all four mappings clearly for its duration, even during phase 3 partial indicators
- Boss phase: makes false/faded labels disappear, briefly exposing the page's "true" direction layout

### Shadow Decoy + Amnesia (from L4)

- Decoy obeys the current real mapping perfectly, making it a living demonstration of which direction inputs actually do what
- During boss: the decoy can show the player the safe route to the active tentacle before they commit

---

## Part 5: Post-Boss Cutscene

### "Remembered"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The wet violet page lies still. Direction glyphs that were scattered and wrong now hover in clean alignment.
2. **(2-4s)** Ruled notebook lines draw themselves back across the page. Margins reappear. Arrows turn into careful notes, labels, and diagram callouts.
3. **(4-6s)** The page reveals what it was meant to be: a study page full of organized thoughts, not chaos - a place where the Artist was trying to think clearly.
4. **(6-7s)** The Artist's hand enters and writes a short label in the margin, finishing a thought that had been abandoned.
5. **(7-8s)** Fade to stage select. The page is calm. The notebook now feels almost complete.

### Emotional Beat

This page heals by recovering coherence. The tragedy was not that the page was empty, but that it could not hold onto meaning. Restoring it turns panic into understanding. The Artist writing in the margin is small, intimate, and important: it means they are no longer just touching the work, but thinking with it again.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Hard**. The lower speed keeps it fair, but the mental tax is severe. Deaths come from hesitation and habit conflict more than from impossible layouts. Deaths: 4-7 attempts average.
- Boss phase: **Very Hard**. The fight becomes progressively less legible as more directions are permanently scrambled. The final tentacle should feel like solving a puzzle while already under stress. Deaths: 5-9 attempts average.

### With Abilities

- **Time Freeze (COUNTER)**: Major help. Simplest answer to both stage and boss.
- **Hunter's Dash**: Strong help. Bypasses control confusion at clutch moments.
- **Ink Flare**: Strong informational help. Restores clarity to unreadable mappings.
- **Ink Anchor**: Moderate help. Reduces sensory overload, especially in the boss.
- **Ink Memory (own ability, replay)**: Nearly trivializes the stage for short windows and makes the boss dramatically more manageable.

### Recommended First-Time Experience

This is the **final pre-Eraser level** and should feel like the most psychologically disorienting page in the set. Players should ideally arrive with several other abilities already unlocked, especially Time Freeze or Hunter's Dash. Ink Memory is a late reward because stable control is one of the most valuable kinds of power in a game built on exact movement.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `ControlShuffle` class - phase-based state machine, shuffle logic, warning/indicator rendering, partial info in phase 3, and grace timer
- `PlayState` input remap hook - current input already passes through `m_controlShuffle.MapDirection()` before snake validation
- Existing audio cues - `shuffle_warning` and `control_shuffle` already support the stage rhythm
- Existing Level 9 visual config - `enableInkBleed`, `enableChromatic`, and `enablePsychedelic` are all already active and fit the redesign perfectly

### New Code Required

- **Memory smear VFX**:
  - Brief afterimage copies of mapping text after each shuffle
  - Optional false-label flicker during late stage without changing actual mapping logic
- **The Scrambler Boss**: New class `src/bosses/ScramblerBoss.h/.cpp`
  - Four tentacle entities with per-direction identity
  - Active-tentacle telegraph and apple spawn assignment
  - Permanent per-direction corruption state after sever
  - Boss-specific shuffle timing and phase transitions as tentacles are removed
- **Ink Memory ability**: New class in ability system
  - Lock current mapping for 6 seconds
  - Immunity hook so control shuffle cannot overwrite active mapping during duration
  - Purify poison/control corruption on activation
  - Visual clarity pocket / reduced wobble rendering
- **Cross-system hooks**:
  - Time Freeze interaction with control-shuffle timers and boss telegraphs
  - Hunter's Dash override behavior under scrambled controls
  - HUD support for ability-driven mapping lock state

### Existing LevelConfig Values (reference)

```cpp
// Level 9: "Amnesia" -- Purple-violet ink bleeding on wet paper
l[8] = {
    9, "Amnesia", "What controls? What controls.",
    10.0f, 15, 5, 0.0f,
    sf::Color(38, 12, 48), sf::Color(210, 200, 230),
    sf::Color(80, 35, 100), sf::Color(110, 55, 130), sf::Color(100, 180, 210),
    false, false, false, false, true, false, false, false,
    0.0f, 5, 2,
    sf::Color(218, 205, 228), sf::Color(50, 25, 60), sf::Color(160, 90, 200),
    0.85f, true, true, true
};
```
