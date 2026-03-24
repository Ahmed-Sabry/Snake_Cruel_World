# Cutscene Script

## Purpose

This document consolidates the narrative arc and cutscene beats for the redesign into one implementation-facing script source.

It is the bridge between:

- the story canon in `design/master-plan.md`
- the per-level reward and healing beats in `design/levels/`
- the current JSON cutscene runtime in `content/cutscenes/` and `src/Cutscene*.cpp`

## Canon Narrative Premise

- The Artist abandoned the notebook.
- The world became cruel because it was left unfinished, not because the Artist was dying.
- The snake is the one thing the Artist finished with love.
- Each boss defeat heals a page and proves the world is worth finishing.
- The Artist's hand progresses from hesitation to participation across the campaign.
- `L10` is the Artist's self-doubt made manifest as the Eraser.
- The ending is not "self-doubt is gone forever." It is "the Artist chooses to draw anyway."

## Tone Guardrails

- Melancholic, intimate, and restrained rather than melodramatic
- Brief and visual-first
- The snake never speaks
- The Artist never gives long exposition
- Healing should feel like beauty re-emerging from damage, not like corruption being merely deleted
- The cutscenes should reward victory without stalling pacing

## Runtime Alignment

The script must stay compatible with the current cutscene runtime:

- `intro` is the first-launch cutscene id
- cutscenes should be written as short, step-based beats that map cleanly to JSON timelines
- Escape skips the whole cutscene and immediately returns to `cutsceneReturnState`
- Enter or Space should only skip the current beat or text step
- scenes must still make narrative sense if fully skipped
- `CampaignHub` in this document means the redesigned post-level hub destination; during transition work it may temporarily map to the current `LevelSelect` enum until `StageSelectState` replaces it

## Recommended File And ID Map

Use these ids and JSON targets:

| Script Section | Cutscene ID | Target File | Return Target |
|---|---|---|---|
| Intro | `intro` | `content/cutscenes/intro.json` | `MainMenu` |
| L2 post-boss heal | `boss_blackout` | `content/cutscenes/boss_blackout.json` | `CampaignHub` |
| L3 post-boss heal | `boss_quicksand` | `content/cutscenes/boss_quicksand.json` | `CampaignHub` |
| L4 post-boss heal | `boss_mirror` | `content/cutscenes/boss_mirror.json` | `CampaignHub` |
| L5 post-boss heal | `boss_famine` | `content/cutscenes/boss_famine.json` | `CampaignHub` |
| L6 post-boss heal | `boss_poison` | `content/cutscenes/boss_poison.json` | `CampaignHub` |
| L7 post-boss heal | `boss_earthquake` | `content/cutscenes/boss_earthquake.json` | `CampaignHub` |
| L8 post-boss heal | `boss_watcher` | `content/cutscenes/boss_watcher.json` | `CampaignHub` |
| L9 post-boss heal | `boss_amnesia` | `content/cutscenes/boss_amnesia.json` | `CampaignHub` |
| Eraser intro | `eraser_intro` | `content/cutscenes/eraser_intro.json` | `Gameplay` |
| Eraser phase 2 | `eraser_phase_2` | `content/cutscenes/eraser_phase_2.json` | `Gameplay` |
| Eraser phase 3 | `eraser_phase_3` | `content/cutscenes/eraser_phase_3.json` | `Gameplay` |
| Eraser phase 4 | `eraser_phase_4` | `content/cutscenes/eraser_phase_4.json` | `Gameplay` |
| Ending | `ending` | `content/cutscenes/ending.json` | `MainMenu` or post-finale state |

`boss_*.json` should remain reserved for the post-boss healing scenes described in `design/master-plan.md`.

The ability-acquisition beats below are still canonical narrative content, but they should be treated as the reward moment immediately before or around cutscene handoff, not as a requirement to merge both moments into one `boss_*.json` file.

Each level section below therefore preserves an explicit boundary between:

- ability acquisition beat
- page-healing beat

## Global Artist-Hand Arc

Use this progression consistently:

| Level | Hand Beat |
|---|---|
| L2 | Visible in shadow only, hovers, pulls away |
| L3 | Slightly closer, interested |
| L4 | Reaches almost to the page, pulls back slower |
| L5 | Near the pen, rests beside it |
| L6 | Closest yet, steady, almost ready |
| L7 | Touching the page with fingertips, pen in other hand |
| L8 | Draws one final line on-page |
| L9 | Writes a short margin label, thinking with the page again |
| L10 | Picks up the pen and finishes the notebook |

## Intro

### Intent

The intro should replace the old simple origin beat with the new story:

- this world was loved once
- then abandoned
- the snake remains
- the notebook is unfinished and unstable
- the player is entering a world worth saving

### Script Beats

**Scene title**: `The Unfinished Page`

1. A blank notebook page fades into view.
2. Clean ruled lines and margin marks appear, suggesting order and care.
3. The snake appears in a calm, open page and eats an apple. The world feels simple, safe, and intentional.
4. The page begins to stutter, smear, and lose structure. The notebook is being left unfinished in front of us.
5. Areas of the page darken, buckle, or bleed as if no guiding hand remains.
6. Text establishes the emotional premise: this world was abandoned.
7. The snake remains centered and alive while the page around it decays.
8. The final beat frames the campaign question: will this world be erased, or finished?

### Recommended On-Screen Text

- `A world, drawn on notebook paper.`
- `And in this world, lived a small snake.`
- `It ate apples. It was happy.`
- `Then the Artist stopped.`
- `The page was left unfinished.`
- `But one thing remained.`

### Emotional Note

The intro should not explain everything outright. It should let the player feel the difference between a cared-for page and an abandoned one.

## L2: Blackout

### Ability Acquisition

1. The dissolved Blind Ink swirls toward the snake.
2. The snake absorbs it and glows bright white.
3. Text: `The darkness taught you to carry your own light.`
4. Ability icon appears: gold flare symbol.
5. Show tutorial prompt once only: `Press [ABILITY KEY] to use Ink Flare`.

### Post-Boss Healing

**Scene title**: `The Light Returns`

1. A single warm amber point appears where the boss died.
2. The point expands and pushes back darkness and stains.
3. The page resolves into warm parchment with faint ruled lines.
4. The Artist's hand appears in shadow, hovering and trembling.
5. The hand pulls away. Fade to Stage Select.

### Emotional Beat

This is the first proof that the notebook can heal.

## L3: Quicksand

### Ability Acquisition

1. Dry ink flakes lift from the page and swirl around the snake.
2. They form a cocoon, which the snake sheds.
3. Text: `What slows you down can become what shields you.`
4. Ability icon appears: sandy shed-skin outline.

### Post-Boss Healing

**Scene title**: `Solid Ground`

1. The dry page cracks like earth before rain.
2. From the cracks grow delicate plants, flowers, and small cacti.
3. The page becomes a sepia garden rather than a trap.
4. The Artist's hand appears slightly closer than before.
5. Fade to Stage Select. The page icon should now suggest garden growth instead of quicksand.

### Emotional Beat

Potential hidden in abandonment is given form.

## L4: Mirror

### Ability Acquisition

1. The snake's shadow detaches and stands beside it.
2. The shadow bows.
3. It dissolves back into the snake.
4. Text: `Your shadow no longer follows. Now it obeys.`
5. Ability icon appears: split silhouette.

### Post-Boss Healing

**Scene title**: `One Voice`

1. Two parallel ink lines appear, one confident and one shaky.
2. The shaky line is absorbed into the stronger one.
3. Color returns in subtle blueprint tones.
4. The Artist's hand reaches close, almost touching the page. The pen hovers.
5. The hand retreats slowly. Fade to Stage Select.

### Emotional Beat

The page heals by resolving indecision into intent.

## L5: Famine

### Ability Acquisition

1. Shattered golden sand swirls around the snake.
2. It crystallizes across the body, then sinks inward.
3. Everything freezes for one beat except the snake.
4. Text: `You broke the clock. Now time answers to you.`
5. Ability icon appears: cracked hourglass in gold and blue.

### Post-Boss Healing

**Scene title**: `Evergreen`

1. Broken hourglass pieces lie across a still page.
2. Golden sand sinks into the parchment and restores color.
3. Evergreen trees grow upward from the revived ink.
4. The Artist's hand appears closer than ever; the pen is in reach.
5. The hand does not yet pick it up, but rests beside it. Fade to Stage Select.

### Emotional Beat

The page heals by proving something can endure.

## L6: Poison

### Ability Acquisition

1. Poison vapor rises from the page.
2. It condenses onto the snake's tail in acid green.
3. The snake flicks the tail and leaves a glowing line behind.
4. Text: `What once consumed you is now yours to command.`
5. Ability icon appears: toxic green droplet.

### Post-Boss Healing

**Scene title**: `True Colors`

1. The muddied toxic page settles into stillness.
2. A single clear ink line draws itself across the page.
3. Honest greens form a meadow while false colors drain away.
4. The Artist's hand appears steady and very close, almost touching.
5. Fade to Stage Select. Caption flavor: `Five pages healed. Three remain.`

### Emotional Beat

Healing this page restores trust, clarity, and honest form.

## L7: Earthquake

### Ability Acquisition

1. Golden-veined ground hardens beneath the snake.
2. The snake becomes visibly grounded and heavy.
3. An anchor shape forms under the body and rises into it.
4. Text: `The ground learned to hold firm. So will you.`
5. Ability icon appears: golden anchor.

### Post-Boss Healing

**Scene title**: `Foundation`

1. The broken terracotta page remains cracked, but the cracks are now golden kintsugi.
2. Buildings, bridges, and towers rise from those healed lines.
3. A city spreads across the page on repaired foundations.
4. The Artist's fingertips touch the page. The pen is in the other hand.
5. Fade to Stage Select. Caption flavor: `Six pages healed. Two remain.`

### Emotional Beat

The healing does not erase brokenness; it turns it load-bearing.

## L8: Watcher

### Ability Acquisition

1. The seal lines that trapped the Hunter unwind and spiral around the snake.
2. They compress into a sleek, sharpened body form.
3. The snake blurs forward in an instant.
4. Text: `Its hunger became your momentum.`
5. Ability icon appears: cyan fang-arrow.

### Post-Boss Healing

**Scene title**: `Field Study`

1. The monstrous page becomes still, replaced by clean contour lines.
2. Those lines grow into notebook studies of serpents in motion.
3. Handwritten notes appear beside them in the Artist's hand.
4. The Artist finishes one final line on-screen, steadily.
5. Fade to Stage Select. Caption flavor: `Seven pages healed. Only the scrambled page and the final eraser remain ahead.`

### Emotional Beat

Attention becomes understanding instead of predation.

## L9: Amnesia

### Ability Acquisition

1. Restored direction labels orbit the snake in a clean square.
2. The labels dissolve into stable violet ink and settle into the body.
3. The snake's outline sharpens and loses its wobble.
4. Text: `You remembered what the page forgot.`
5. Ability icon appears: locked violet arrow-square.

### Post-Boss Healing

**Scene title**: `Remembered`

1. The scattered direction glyphs realign in calm formation.
2. Ruled lines, margins, labels, and diagram notes redraw themselves.
3. The page becomes a place of organized thought instead of panic.
4. The Artist's hand writes a short margin label, finishing a thought.
5. Fade to Stage Select. Caption flavor: `The page is calm. The notebook now feels almost complete.`

### Emotional Beat

Healing this page restores coherence and the ability to think clearly.

## L10: Eraser Phase Text

These beats are short gameplay-presentation moments rather than long standalone cutscenes, but they should be centralized here so the tone stays consistent.

### Phase 1 Clear

- Text: `It gets worse.`
- Meaning: the false simplicity is breaking.

### Phase 2 Clear

- Text: `It gets worse.`
- Meaning: predatory fear gives way to poisonous instability.

### Phase 3 Clear

- Text: `Everything. All at once.`
- Meaning: the Eraser stops fragmenting the notebook and attacks with total self-doubt.

### Anchor 19 Twist

- Text: `Oops.`
- Meaning: one last act of sabotage, petty and cruel.

### Final Defeat Beat

1. The 20th anchor flares gold.
2. Golden ink fills erased lanes and cracks.
3. The Eraser crumbles instead of exploding.
4. The page stops shaking.
5. Silence.

## Ending

### Scene Title

`The Hand Returns`

### Script Beats

1. The Eraser lies scattered as dust. No movement. No music.
2. Golden ink spreads through every healed page and connects them.
3. The notebook fills fully: forests, gardens, studies, city, light, structure. Not perfect. Finished.
4. The Artist's hand enters holding the pen and pauses only once.
5. The hand writes one final clean line across the last unfinished space.
6. The notebook closes gently. Fade out.

### Emotional Beat

The true victory is not destroying doubt. It is refusing to let doubt be the last author of the page.

## Stage Select Copy Notes

The following short lines appear in level docs and should be treated as centralized narrative copy, not throwaway local text:

- `Five pages healed. Three remain.`
- `Six pages healed. Two remain.`
- `Seven pages healed. Only the scrambled page and the final eraser remain ahead.`
- `The page is calm. The notebook now feels almost complete.`

If Stage Select later surfaces this progress copy in the hub, it should draw from the same narrative source so the campaign voice stays consistent.

## Skip And Routing Assumptions

- Intro skip should still mark the intro as seen and return to `MainMenu`.
- L2-L9 reward/heal cutscenes should assume Escape may skip the whole sequence and still leave the page healed and the ability unlocked.
- Eraser phase cutscenes should return to `Gameplay`.
- The ending should return to the post-finale destination chosen by game flow, likely `MainMenu` unless a dedicated credits or postgame state is added later.

## Authoring Guidance For JSON Implementation

- Prefer 5 to 8 short beats per cutscene.
- Keep text sparse; use one key line in ability scenes and minimal or no text in healing scenes.
- Use persistent fades and paper background changes intentionally, since the runtime keeps some visual state across steps.
- Use Enter or Space skippable sub-beats for text-heavy moments only.
- Do not rely on the player seeing every intermediate beat; the opening, reward, and final emotional image should survive full-skip behavior.

## Out Of Scope

This document does not define:

- boss gameplay logic
- Stage Select state architecture
- ability activation mechanics
- exact JSON animation coordinates
- sound implementation beyond broad intent

It only defines the narrative content and routing assumptions for cutscene authoring.
