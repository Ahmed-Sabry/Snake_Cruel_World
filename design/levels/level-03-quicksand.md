# Level 3: "Quicksand" - Detailed Design Document

> *"Unfinished ink pooled and congealed without structure."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | Quicksand |
| Subtitle | "Some tiles are hungrier than others." |
| Hazard | Quicksand patches (3x3 slowdown zones, `0.5x`) |
| Boss | The Mire |
| Ability Earned | Shed Skin |
| Strong Counter Against | Level 7: Earthquake |
| Narrative Metaphor | The Artist left ink pooling on this page without giving it form, and it congealed into treacherous ground. |

---

## Part 1: Stage Phase

### Gameplay

The player navigates a grid littered with 3x3 quicksand patches. Touching quicksand slows the snake dramatically, and the patches periodically relocate, forcing constant re-routing.

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Base Speed | 13.0 | Faster than average - the "safe" ground feels fast, quicksand feels brutal by contrast |
| Apples to Win | 15 | Standard count - routing around patches adds time naturally |
| Shrink Interval | 0 (time-based) | Walls shrink every 8.0s instead of per-apple |
| Shrink Timer | 8.0s | Constant pressure - arena gets tighter, patches get harder to avoid |
| Patch Count (initial) | 4 | Manageable at start |
| Patch Relocate Interval | 6.0s → 4.0s (progressive) | Starts at 6.0s, then 5.0s, then 4.0s per milestone; see **Difficulty Curve Within Stage** (apples 1–5 / 6–10 / 11–15) |
| Star Threshold (2 stars) | 10 self-collisions | Very generous - quicksand causes many accidental crashes |
| Star Threshold (3 stars) | 0 self-collisions | Perfect run only - rewards flawless routing |

### Quicksand Behavior (existing mechanic - with modifications)

- **Existing**: 3x3 tile patches (`QuicksandPatch` struct), relocate on timer, `IsOnQuicksand()` check
- **Modification for redesign**: Touching regular quicksand should apply a **severe speed penalty** (`0.5x` movement speed while the snake is on the patch). This makes quicksand a hazard that punishes routing mistakes without abruptly ending the run, creating more interesting gameplay.
- **Visual**: Sepia-brown tiles with a subtle "sinking" animation (tiles pulse slightly darker in waves)

### Difficulty Curve Within Stage

- **Apples 1-5**: 4 patches, 6s relocate timer. Patches are spread out. Player learns to read the grid and route around them. Still comfortable.
- **Apples 6-10**: 6 patches, 5s relocate timer. Arena is also shrinking (time-based). Safe corridors get narrower. Player starts needing to plan 2-3 moves ahead.
- **Apples 11-15**: 8 patches, 4s relocate timer. Arena is noticeably smaller. Some apple spawns will be adjacent to quicksand, forcing risky approaches. The feeling: "the ground is closing in."

### Visual Theme (existing - keep as-is)

- Paper tone: sun-bleached journal `(215, 195, 155)`
- Ink tint: warm sepia brown `(110, 65, 30)`
- Accent: dusty gold `(190, 145, 45)`
- Corruption: 0.25 (moderate wobble - the ink on this page is unsettled)
- No post-processing shaders

### Player Experience Goals

- **First attempts**: Player runs straight into quicksand, gets slowed, crashes into walls or own body. Learns to visually scan the grid before committing to a direction.
- **Learning curve**: Player starts planning routes 3-4 tiles ahead, treating quicksand patches as walls to navigate around. Develops a "pathfinding instinct."
- **Mastery**: Player weaves through narrow gaps between patches at full speed, using the relocate timer to predict when gaps will open.
- **Emotional note**: It should feel like traversing unstable ground - the beautiful journal page is disintegrating beneath you. Finding a safe path should feel like relief.

---

## Part 2: Boss Phase - "The Mire"

### Transition

After the 15th apple, the remaining quicksand patches begin spreading. They grow, connecting to each other, until the ENTIRE floor is quicksand except for narrow dry paths. Text appears: *"The ink swallowed the ground."*

### Boss Entity: The Mire

A massive slug-like ink creature that oozes across the arena. It leaves a permanent trail of fresh quicksand wherever it moves, progressively eliminating safe ground.

**Visual design:**
- Body: elongated blob, ~6 tiles long, 2 tiles wide. Dark brown-black with glistening wet texture.
- Trail: darker quicksand patches that are visually distinct from the floor quicksand. Touching regular quicksand slows the snake to `50%` of normal speed (`0.5x`), while touching The Mire's fresh trail slows the snake to `25%` of normal speed (`0.25x`).
- Head: blunt rounded front with two dim amber eyes.
- Movement: slow, deliberate oozing animation. Segments compress and expand like a real slug.

### Boss Mechanics

**Arena layout:**
- The entire floor starts as quicksand EXCEPT for 3-4 narrow "dry paths" (1-tile-wide corridors) that form a loose grid pattern.
- Dry paths shift position every 8 seconds (old paths sink into quicksand, new paths emerge elsewhere). A 1.5s warning glow shows where new paths will appear. When a warned segment actually sinks, any player standing on that segment is first moved to the nearest remaining dry-path tile within a 1-tile Manhattan radius if one exists (preserve facing and do not cancel the current input buffer); if no such tile exists, the player instead enters regular quicksand underfoot and immediately receives the regular quicksand speed penalty (`0.5x`, not instant death) plus a brief `0.5s` grace window before any additional stacking penalties from that sink event apply. While on The Mire's fresh trail, the snake uses the stricter `0.25x` modifier as elsewhere in this doc.
- The snake moves at full speed on dry paths, at `0.5x` speed on regular quicksand, and at `0.25x` speed on The Mire's fresh trail.

**The Mire's behavior:**
- Moves along the dry paths (it's fast on dry ground too).
- Leaves a permanent quicksand trail behind it, destroying the dry paths it crosses.
- Periodically, The Mire pauses and "spreads" - extending its body to cover a wider area, blocking entire path intersections.

**How to damage:**
- An apple spawns on The Mire's back (riding on it) **immediately when the boss phase begins** (first apple is present from frame one of the fight).
- The player must lure The Mire into a wall corner. When it hits a corner, it gets stuck for 3 seconds (visually: it compresses against the walls, struggling).
- While stuck, the player can approach and eat the apple off its back = 1 damage.
- After taking damage, The Mire thrashes (ink splatter particles), frees itself, and resumes. A new apple appears on its back after 2 seconds.

**How to lure The Mire into corners:**
- Movement priority (highest first): **Priority 1** — if the player is on the same dry path as The Mire and within 5 tiles along that path, The Mire **chases the player**. **Priority 2** — otherwise, The Mire moves along the dry paths toward the **nearest path intersection** (its default routing when no valid chase applies).
- Strategy: position yourself so the Mire chases you down a path that leads to a corner.

### Boss Phases

- **Phase 1 (HP 3)**: 4 dry paths. Mire speed = 4 tiles/sec. Generous path network. Learning the lure mechanic.
- **Phase 2 (HP 2)**: 3 dry paths remaining (Mire's trail has destroyed one). Speed = 5 tiles/sec. Mire also occasionally "lunges" (2-tile burst forward). Less safe ground.
- **Phase 3 (HP 1)**: 2 dry paths only. Speed = 6 tiles/sec. Mire lunges frequently. Very little safe ground. The arena feels claustrophobic. Player must be perfect with the corner trap.

### Damage System

- **Hit condition**: Eat apple off The Mire's back while it's stuck in a corner
- **Hits to defeat**: 3
- **Counter-ability bonus (Shed Skin)**: On replay runs after the player has already earned Shed Skin, activating it drops body segments that create temporary dry patches (5 seconds). These are new safe ground the player can walk on, and they also block The Mire's trail from spreading. Effectively creates escape routes and shortens the distance to corners. Additionally, shed segments placed in The Mire's path slow it down for 2 seconds (it has to "consume" them).
- **Visual feedback on hit**: Mire convulses, chunks of its body splatter off as ink particles. It shrinks slightly. Fresh quicksand it left behind briefly solidifies into dry ground (3 seconds of relief).

### Boss Defeat

When HP reaches 0:
1. The Mire collapses flat, spreading into a thin ink film
2. The film rapidly evaporates - quicksand dries up across the entire arena
3. Clean, dry parchment is revealed beneath. Notebook ruled lines appear.
4. Small golden flowers (ink drawings) bloom from where the quicksand was - the page is healed

---

## Part 3: Ability Earned - Shed Skin

### Ability Acquisition Cutscene

After The Mire dissolves:
1. The dried ink residue lifts off the page in flakes
2. The flakes swirl around the snake, briefly encasing it in a cocoon
3. The snake sheds the cocoon - emerging cleaner, lighter
4. Text: *"What slows you down can become what shields you."*
5. Ability icon appears: a curved snake skin outline in sandy gold

### Ability Specification

| Parameter | Value |
|---|---|
| Name | Shed Skin |
| Cooldown | 12 seconds |
| Duration | Instant (shed segments last 5s) |
| Activation Key | Spacebar (ability key) |
| Available After | Defeating The Mire (L3 boss) |
| Minimum Length | Snake must have at least 5 total segments to use |

### Effect Details

When activated:
1. **Immediate**: Snake drops its last 3 body segments in place. Snake length decreases by 3, and activation is only allowed when the snake has at least 5 total segments before the shed so the post-effect snake is still 2 total segments long and playable.
2. **Shed segments**: At the instant of activation, snapshot the grid coordinates of the snake's last three body segments (e.g. the tail end of `snake.body` — **not** stale positions from a previous tick and not inferred from the head alone). Those exact cells spawn solid shed obstacles for 5 seconds. Activation still requires `>= 5` total segments so the post-shed snake remains playable.
   - Block enemy movement (Predator, Mirror Ghost must path around them)
   - Block quicksand spreading (The Mire's trail cannot cross them)
   - Create stable ground during earthquakes (anchor points)
   - Other entities collide with them as if they were walls
3. **After 5s**: Shed segments dissolve into ink splatter particles and disappear.

### Visual Transform (Snake Appearance During Shed Skin)

- **Trigger moment**: Snake body flashes pale/translucent for 0.3s as segments detach
- **Shed segments on ground**: Rendered as the snake's body color but faded, with a cracking/drying texture. They visually "harden" (become angular rather than smooth).
- **Snake after shedding**: Brief shimmer effect on remaining body. No persistent visual change (instant ability, not duration-based).

### When Shed Skin is Most Useful

| Level | How Shed Skin Helps |
|---|---|
| L3 Quicksand (home level) | Creates temporary dry patches on quicksand, slows The Mire |
| **L7 Earthquake (COUNTER)** | **Shed segments anchor to the grid and don't move when walls shake. Creates stable "islands" the snake can navigate to. Also blocks shifted wall positions from crushing you.** |
| L8 The Watcher | Drop shed segments as bait/walls - Predator must path around them, buying time |
| L4 Mirror Ghost | Shed segments act as walls the ghost might collide with |
| L2 Blackout | Shed segments glow faintly in the dark, creating navigation landmarks |

---

## Part 4: Hidden Ability Interactions

When replaying Level 3 with abilities from other levels:

### Ink Anchor + Quicksand (from L7)

- Using Ink Anchor makes the snake **ignore quicksand slowdown** for 5 seconds
- Snake glides over quicksand at full speed as if it were dry ground
- Also prevents quicksand patches from relocating during the duration
- **Achievement**: "Solid Ground" - complete L3 stage phase without touching any quicksand (using Ink Anchor to traverse)

### Hunter's Dash + Quicksand (from L8)

- Dashing skips over quicksand patches entirely (snake teleports across them)
- Can dash to reach apple clusters that are surrounded by quicksand
- During boss fight: dash to The Mire's back directly without needing to lure it to a corner (high-skill shortcut, tight timing required)
- **Achievement**: "Sandskipper" - defeat The Mire using only dash-attacks (no corner traps)

### Time Freeze + Quicksand (from L5)

- Freezing time prevents quicksand patches from relocating
- Also freezes The Mire in place during boss fight (3 seconds to freely eat the apple off its back)
- Doesn't freeze the quicksand slowdown effect itself - just the timers

### Ink Flare + Quicksand (from L2)

- Flare highlights all quicksand patches with a bright warning overlay (even ones about to appear from relocation)
- Shows the safe route to the apple as a faint golden line
- During boss fight: reveals where new dry paths will emerge before the warning glow

---

## Part 5: Post-Boss Cutscene

### "Solid Ground"

**Duration**: ~8 seconds (short, wordless)

1. **(0-2s)** The dried arena floor. Cracks appear in the parchment - but not destruction cracks. Growth cracks. Like dried earth before rain.
2. **(2-4s)** From the cracks, thin ink lines grow upward. They curl and branch - becoming delicate drawings of desert plants, small flowers, tiny cacti. The Artist's original vision for this page was a garden.
3. **(4-6s)** The garden fills the page. Warm sepia tones shift to softer golden-greens. The page breathes.
4. **(6-7s)** The Artist's hand appears in the background. It moves slightly closer than in L2's cutscene. Still hesitating, but interested.
5. **(7-8s)** Fade to stage select. The L3 page icon now shows the garden instead of quicksand.

### Emotional Beat

The quicksand wasn't just corruption - it was potential. The congealed ink, given form, becomes a garden. The message: what the Artist abandoned still had beauty waiting inside it.

---

## Part 6: Difficulty Balancing Notes

### Without Any Abilities (First Playthrough)

- Stage phase: **Medium difficulty**. Quicksand is visible and avoidable with good routing. The real challenge is the shrinking arena + relocating patches creating tight corridors. Players who rush will get caught; players who plan will thrive.
- Boss phase: **Hard**. Limited safe ground, a chasing boss that destroys paths, and the lure-to-corner mechanic requires spatial thinking. Players will die 4-6 times learning the patterns. The "aha moment" is realizing the Mire follows paths and can be led.

### With Abilities

- **Ink Anchor (COUNTER)**: Significantly easier. Ignoring quicksand slowdown removes the core threat of the stage phase. Boss fight becomes much more manageable with full-speed movement everywhere.
- **Hunter's Dash**: Strong. Dashing over quicksand and directly to the boss is a power move. Reduces boss fight to a timing challenge.
- **Time Freeze**: Moderate help. Freezing the Mire in place gives free hits but doesn't solve the limited safe ground problem.
- **Ink Flare**: Mild help. Seeing patch positions in advance is useful but doesn't change the physical challenge.

### Recommended First-Time Experience

This is a good **2nd or 3rd level** for players. The quicksand mechanic is more spatially complex than blackout but still intuitive. The Mire boss teaches "environmental manipulation" (using the arena against the boss), which is a skill that pays off in later bosses. Shed Skin is a versatile, creative ability that rewards experimentation.

---

## Part 7: Technical Notes

### Existing Code to Reuse

- `Quicksand` class - patch generation, relocation timer, `IsOnQuicksand()` collision check
- `QuicksandPatch` struct - 3x3 grid coordinate patches
- `Quicksand::RenderTo()` - patch rendering with sepia colors
- `ParticleSystem` InkDrip effect for Mire trail particles
- `World::m_shrinkTimerSec` - existing time-based shrink system (L3 uses timer not apple-count)

### New Code Required

- **Quicksand speed penalty**: Modify snake movement to apply speed debuff when `IsOnQuicksand()` returns true (currently may be collision-based - needs verification in `PlayState.cpp`)
- **Progressive patch count**: Extend `Quicksand` to support increasing `m_patchCount` at apple milestones
- **The Mire Boss entity**: New class `src/bosses/MireBoss.h/.cpp`
  - Slug movement along dry paths (path-following AI)
  - Permanent quicksand trail behind movement
  - Corner-stuck detection (adjacent to 2+ walls)
  - Apple-on-back mechanic (apple rides with entity)
  - Lunge attack (burst forward 2 tiles)
- **Dry path system** (boss arena): Grid overlay marking 1-tile-wide safe corridors
  - Path shifting on timer with visual warning
  - Path destruction from Mire trail
- **Shed Skin ability**: New class in ability system
  - Detach last 3 body segments from snake
  - Spawn them as temporary solid obstacles
  - 5-second lifetime with dissolve animation
  - Collision registration (enemies, quicksand blocking)
  - Validation case: with a 5-total-segment snake, activate Shed Skin and verify the snake survives at 2 total segments, can still move on the next input, and can still eat the next apple normally

### Existing LevelConfig Values (reference)

```cpp
// Level 3: "Quicksand" -- Sepia fountain pen on sun-bleached journal
l[2] = {
    3, "Quicksand", "Some tiles are hungrier than others.",
    13.0f, 15, 0, 8.0f,
    sf::Color(55, 42, 25), sf::Color(170, 115, 45),
    sf::Color(90, 50, 25), sf::Color(130, 80, 40), sf::Color(165, 40, 35),
    false, true, false, false, false, false, false, false,
    0.0f, 10, 0,
    sf::Color(215, 195, 155), sf::Color(110, 65, 30), sf::Color(190, 145, 45),
    0.25f, false, false, false
};
```
