# Level 10: "The Eraser" - Detailed Design Document

> *"If this page cannot be perfect, it should not exist at all."*

---

## Overview

| Field | Value |
|---|---|
| Level Name | The Eraser |
| Current Code Name | Cruel World |
| Subtitle | "Everything. All at once." |
| Encounter Type | 4-phase mega-boss finale |
| Boss | The Eraser |
| Ability Earned | None - all 8 abilities are already unlocked; victory restores the notebook and triggers the ending |
| All Abilities Available | Yes |
| Narrative Metaphor | The Artist's self-doubt has taken form as the impulse to erase the whole notebook rather than finish it imperfectly. The Eraser is not a monster from a page - it is the force trying to remove every page. |

---

## Part 1: Encounter Structure

### Core Premise

This is not a normal stage followed by a boss. It is a single sustained final encounter against The Eraser, with four escalating phases. The player survives long enough to collect 20 memory anchors total, five per phase. Each set of five anchors tears away one layer of the Eraser's control over the notebook.

This preserves the current Level 10 backbone:

- 20 total apples to clear under the hood, reframed in the fiction as memory anchors
- Phase transitions at 5, 10, and 15 collected anchors
- Upside-down twist at anchor 19
- Final completion at anchor 20

### Arena Rules

- Fixed final-arena feel even if the world still uses current shrink systems under the hood
- The Eraser itself is a giant off-axis presence around and across the page, scraping lanes, smearing hazards, and redrawing rules between phases
- Apples are visually reframed as **memory anchors**: small stable points of color the snake eats to stop the page from being erased

### Tuning Parameters

| Parameter | Value | Notes |
|---|---|---|
| Total Anchors Required | 20 | 5 per phase |
| Base Speed by Phase | 12.0 / 13.0 / 14.0 / 15.0 | Keep current escalation |
| Timed Apple Window | 6.0 / 5.0 / 4.0 / 3.0 seconds | Preserve the current timer squeeze |
| Shrink Interval | 4 / 3 / 2 / 2 apples | Keep current pressure curve |
| Time-Based Shrink | Phase 4 only: every 5.0s | Existing final-phase panic timer |
| Screen Flip Trigger | Anchor 19 | Preserve `Plot Twist` moment |
| Star Threshold (2 stars) | 10 self-collisions | Current Level 10 target |
| Star Threshold (3 stars) | 5 self-collisions | Current Level 10 target |

### Boss Presentation

The Eraser is a massive half-materialized school eraser and smear of paper dust, always partly off-screen, as if it were too large for the page itself. Sometimes it manifests as a clean white block scraping across the notebook; sometimes only its pressure is visible as erased lines, dust plumes, and missing pieces of the world.

**Visual design:**

- Body: pale, chalky white with bruised gray edges and graphite-pink stains
- Motion language: scraping, rubbing, dragging, never elegant
- Attacks create erased paper patches, rubber crumbs, and jagged negative space rather than conventional projectiles
- As phases advance, The Eraser becomes less tool and more force of nature

### Player Experience Goals

- **Opening**: "This almost feels fair." The level begins as a callback to Level 1 before revealing that fairness was bait.
- **Mid-fight**: Each phase reminds the player of healed pages being threatened again, but now they have answers the early snake never had.
- **Final phase**: The game asks for true synthesis, not just survival. You are no longer escaping cruelty; you are disproving it.
- **Emotional note**: The final fight should feel like resisting deletion itself. Every anchor eaten is the snake insisting, "This world is worth keeping."

---

## Part 2: Phase 1 - "False Hope"

### Theme

The Eraser opens by pretending the notebook can still be simple. The page resembles the clean early-game notebook again, but The Eraser keeps rubbing away fresh anchors before they can last.

### Active Threats

- Timed memory anchors only
- No other hazards active yet
- Eraser swipe lines occasionally pass across the page, visually threatening anchor spawn lanes

### Gameplay Identity

This is the opening lie. The player is lulled into a familiar rhythm: route to apples, keep pace, stay clean. But every anchor is temporary, and The Eraser is already teaching the rule of the fight: if you hesitate, the page loses ground.

### Primary Ability Test

**Hunter's Dash** is the intended star here.

- Dash lets the player catch distant timed anchors before they vanish
- Dashing through an eraser swipe trail feels like "outrunning deletion"
- The phase teaches that the final boss is balanced around active ability use, not just passive endurance

### Secondary Helpful Abilities

- **Time Freeze**: stops anchor expiry and buys route-planning time
- **Ink Flare**: reveals the next anchor spawn outline through fading paper dust

### Phase Clear

After the 5th anchor:

1. The clean page tears at the edges
2. The Eraser drags a long scar through the notebook margin
3. The first callback illusion fails
4. Text: *"It gets worse."*

---

## Part 3: Phase 2 - "The Hunt"

### Theme

The Eraser stops pretending. The page darkens into the cold blue-gray tone of Level 8, and a predatory sketch-form begins stalking the snake through blackout pulses. It is The Eraser weaponizing the fear of being watched and chased.

### Active Threats

- Blackouts
- Predator behavior
- Timed anchors still active
- Frequent shrink cadence

### Gameplay Identity

The player must keep collecting anchors while competing with a hunter in partial darkness. This is the first phase where survival pressure and scoring pressure overlap hard: the player is racing for anchors, denying them to the pursuer, and keeping orientation during blackout windows.

### Primary Ability Test

**Shadow Decoy** is the intended answer.

- Decoy pulls the predator away from the player and away from critical anchor routes
- It creates short safe windows to collect anchors during blackout phases
- In fiction, the snake survives by proving it cannot be reduced to the target The Eraser thinks it is

### Secondary Helpful Abilities

- **Venom Trail**: stuns the hunter and creates safe routes
- **Ink Flare**: reveals predator position and anchor lane visibility in darkness
- **Time Freeze**: freezes the chase if the player needs a reset

### Phase Clear

After the 10th anchor:

1. The hunter sketch is partially rubbed out in a cloud of crumbs
2. The erased dust falls back to the page as toxic sludge
3. The page sags and loses structure
4. Text: *"It gets worse."*

---

## Part 4: Phase 3 - "Corrupted Revision"

### Theme

The Eraser stops chasing and starts grinding the page itself into something poisonous and unstable. The notebook becomes sickly green, sticky, and wrong - a mix of quicksand drag and poison deception. This phase is about surviving a page that has lost its integrity.

### Active Threats

- Quicksand
- Poison apples
- Timed anchors with shorter windows
- Aggressive shrink cadence

### Gameplay Identity

This is the nastiest terrain-management phase. The player is asked to maintain route discipline while the floor slows them and the page lies about which apples are safe. The Eraser is trying to make the snake distrust both the world and its own decisions.

### Primary Ability Test

**Ink Anchor** is the intended backbone ability.

- Anchor negates terrain slowdown and reintroduces stable movement to a collapsing page
- It creates the feeling of literally pinning the notebook in place against the Eraser's grinding pressure
- It is the most reliable way to preserve routing through quicksand-heavy lanes

### Secondary Helpful Abilities

- **Ink Memory**: instantly purifies poison mistakes and creates stable control windows
- **Hunter's Dash**: crosses toxic or slow lanes before they become fatal
- **Shed Skin**: drops emergency blockers or shortens body burden under pressure

### Phase Clear

After the 15th anchor:

1. The poisoned page splits open with a deep scrape
2. The Eraser finally manifests at full size, pressing into the notebook from above
3. The remaining healed-page fears return together
4. Text: *"Everything. All at once."*

---

## Part 5: Phase 4 - "The Eraser"

### Theme

This is the true boss. The Eraser stops using pieces of the notebook against you and instead attacks as total self-doubt: every old cruelty at once, every rule compromised, every page threatened. The world becomes scorched parchment and red-black ruin.

### Active Threats

- Earthquakes
- Control Shuffle
- Mirror Ghost
- Timed anchors at their shortest duration
- Apple-based shrink plus time-based shrink every 5 seconds
- Residual pressure from earlier phase pacing
- Screen flips upside-down at anchor 19

### Gameplay Identity

This phase is not about one clean counter. It is about composure and rotation through the full kit. The Eraser is trying to prove that when everything overlaps, the snake will break. The player's answer is mastery: they do not need one perfect tool, because they have learned from every page.

### Primary Ability Test

**Ink Memory** is the key stabilizer for the final phase.

- Locks controls during control-shuffle chaos
- Creates brief islands of certainty during the most cognitively hostile part of the game
- Becomes especially important after the upside-down flip, when disorientation becomes both physical and mental

### Required Ability Mindset

This phase expects chaining:

- **Ink Memory** to survive shuffle windows
- **Time Freeze** to pause overlapping timers and wall pressure
- **Hunter's Dash** to secure almost-expired anchors
- **Ink Flare** to read ghost position, quake tells, and disappearing safe lanes
- **Shadow Decoy** to buy breathing room against mirror pressure

No single ability should fully solve the phase. The design goal is that the player feels like all prior victories matter here.

### The Cruel Twist

At the 19th anchor:

1. The Eraser drags across the entire notebook spine
2. The page flips upside down
3. Text: *"Oops."*
4. One final anchor remains

This preserves the current signature twist and turns it into the Eraser's final act of sabotage: if it cannot kill the snake fairly, it will try to make the notebook unreadable one last time.

### Final Anchor / Defeat Condition

The 20th anchor should feel ceremonial, not random:

- It spawns in a dangerous but readable lane
- The player should need one last committed action to reach it
- Ideally, the final grab happens under intense pressure but with clear telegraphing, so victory feels earned rather than lucky

When the 20th anchor is eaten:

1. The anchor flares bright gold instead of the normal apple burst
2. Golden ink spreads through every erased lane and crack
3. The Eraser begins to crumble, not explode - it loses authority
4. The page stops shaking
5. Silence

---

## Part 6: Full Ability Integration

The final level should let every unlocked ability matter at least once, even if some are supportive rather than mandatory.

| Ability | Final Boss Role |
|---|---|
| Ink Flare | Reveals ghost position, quake warnings, weak visibility lanes, and final-anchor telegraphing |
| Shed Skin | Creates emergency blockers, shortens body burden, and buys route space in panic moments |
| Shadow Decoy | Diverts hunters/ghost pressure and creates precious decision time |
| Time Freeze | Pauses the most oppressive timer overlaps in any phase |
| Venom Trail | Stuns pursuit elements and carves temporary safe corridors |
| Ink Anchor | Restores movement stability on collapsing terrain and during quake-heavy windows |
| Hunter's Dash | Secures distant or nearly erased anchors instantly |
| Ink Memory | Locks certainty in place when the page tries to rewrite controls and identity |

### Design Rule

The Eraser should never feel like "use the one correct button four times." It should feel like the entire campaign has been secretly teaching the player how to answer this encounter.

---

## Part 7: Ending Cutscene

### "The Hand Returns"

**Duration**: ~12 seconds (longer than prior page-heal scenes)

1. **(0-2s)** The crumbled Eraser lies scattered as dust across the notebook. Nothing moves. No music.
2. **(2-4s)** Golden ink lines spread through every healed page at once, connecting them across the notebook like a finished circuit.
3. **(4-6s)** The notebook fills in fully: forests, cities, studies, creatures, light, structure. Not perfect. Finished.
4. **(6-8s)** The Artist's hand enters carrying the pen. It pauses over the notebook, trembling only once.
5. **(8-10s)** The hand writes a final clean line across the last unfinished space. The act is small, but it changes everything: the Artist is drawing again.
6. **(10-12s)** The notebook closes gently. Fade out.

### Emotional Beat

The ending is not about destroying self-doubt forever. It is about refusing to obey it. The snake wins by proving the unfinished world deserved completion, not erasure. The Artist picking up the pen again is the true victory.

---

## Part 8: Difficulty Balancing Notes

### Without Ability Mastery

- **Very Hard**. Even with all abilities unlocked, players who haven't internalized when to use them will collapse under the phase overlap.
- The fight should still be learnable because each phase spotlights a primary answer before the final synthesis test.

### With Ability Mastery

- The fight becomes readable rather than easy
- Good players feel increasingly empowered, not merely overwhelmed
- The final phase should still be punishing, but deaths should feel like execution or sequencing mistakes, not pure chaos

### Recommended Feel Per Phase

- **Phase 1**: Controlled anxiety
- **Phase 2**: Predatory pressure
- **Phase 3**: Corrupted footing
- **Phase 4**: Total war

### Achievement Intent

- Preserve `Iron Snake` as the clean-execution challenge
- Preserve `Plot Twist` by making the upside-down section intentional and memorable
- Preserve `Against All Odds` as a badge of impossible-seeming first-clear composure

---

## Part 9: Technical Notes

### Existing Code to Reuse

- Current Level 10 phase backbone in `PlayState`:
  - Phase changes at 5, 10, and 15 apples
  - Upside-down screen flip at anchor 19
  - Final completion at anchor 20
- Existing mechanic integrations:
  - Timed apples
  - Predator
  - Blackouts
  - Quicksand
  - Poison apples
  - Earthquakes
  - Control shuffle
  - Mirror ghost
- Existing announcement system:
  - `"It gets worse."`
  - `"Everything. All at once."`
  - `"Oops."`
- Existing Level 10 achievements and victory handling

### Redesign Goal in Code Terms

Refactor Level 10 from a mechanic pile-up into a boss-state machine that still uses the same milestone rhythm.

### New Code Required

- **EraserBoss**: `src/bosses/EraserBoss.h/.cpp`
  - Phase controller for presentation, attack scripting, and transition logic
  - Giant off-screen boss rendering / scrape effects
  - Final-anchor presentation logic
- **Phase scripting layer**
  - Map each 5-apple block to a named Eraser phase
  - Attach phase-specific telegraphs, narration, and visuals
  - Preserve current milestone timings while changing meaning
- **Ability integration hooks**
  - Final-boss-safe versions of all eight abilities
  - Optional boss responses to ability use (for example: freeze pauses scrape timer, flare reveals safe lane, memory locks control state)
- **Ending content**
  - `content/cutscenes/eraser_intro.json`
  - `content/cutscenes/eraser_phase_2.json`
  - `content/cutscenes/eraser_phase_3.json`
  - `content/cutscenes/eraser_phase_4.json`
  - `content/cutscenes/ending.json`
- **Visual polish**
  - Paper erasure dust particles
  - Negative-space scrape trails
  - Strong upside-down transition treatment at anchor 19
  - Boss-specific silence / audio drop after final hit

### Current LevelConfig Values (reference)

```cpp
// Level 10: "Cruel World" -- 4-phase escalation, starts as L1 callback
l[9] = {
    10, "Cruel World", "Everything. All at once.",
    12.0f, 20, 4, 0.0f,
    sf::Color(28, 22, 30), sf::Color(175, 120, 75),
    sf::Color(55, 45, 65), sf::Color(80, 70, 95), sf::Color(180, 55, 45),
    true, true, true, true, true, true, true, true,
    6.0f, 10, 5,
    sf::Color(248, 242, 228), sf::Color(45, 40, 55), sf::Color(170, 65, 55),
    1.00f, true, true, true
};
```

### Migration Note

`Level 10` should be renamed in design-facing materials from `Cruel World` to `The Eraser`, while the code may temporarily retain `Cruel World` as the internal config/state name until the boss refactor is implemented.
