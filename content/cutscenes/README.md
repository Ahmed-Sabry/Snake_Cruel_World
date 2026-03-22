# Cutscene JSON Quick Reference

## File Format

```json
{
  "id": "intro",
  "displayName": "The Cruel Beginning",
  "referenceResolution": [1366, 768],
  "timeline": [ ... ]
}
```

| Field | Type | Required | Notes |
|-------|------|----------|-------|
| `id` | string | yes | Cutscene identifier (matches filename stem) |
| `displayName` | string | no | UI display name |
| `referenceResolution` | [W, H] | no | Scale reference for `s` variable (default: [1366, 768]) |
| `timeline` | array | yes | Sequential list of action objects |

Use `{"_comment": "..."}` anywhere in the timeline for inline notes — entries without a `type` field are silently ignored.

---

## Expression Language

Any field that accepts a string (positions, dimensions, charSize) is evaluated as an arithmetic expression.

### Variables

| Var | Meaning | Computed as |
|-----|---------|-------------|
| `cx` | Screen center X | `windowWidth / 2` |
| `cy` | Screen center Y | `windowHeight / 2` |
| `w` | Screen width | `windowWidth` |
| `h` | Screen height | `windowHeight` |
| `s` | Scale factor | `min(w / refW, h / refH)` |

### Operators

`+` `-` `*` `/` — standard precedence (`*` `/` before `+` `-`). Tokens must be space-separated.

```
"cx - 100 * s"    → center X minus (100 × scale)
"w * 0.2"          → 20% of screen width
"h / 2 + 50"       → half height plus 50
```

---

## Action Types

### wait

Pause the timeline.

```json
{"type": "wait", "duration": 1.5}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `duration` | number | 1.0 | Seconds |

### waitForInput

Block until Enter or Space is pressed.

```json
{"type": "waitForInput"}
```

### spawn

Create a visual entity.

```json
{"type": "spawn", "name": "player", "shape": "rect",
  "position": ["cx", "cy"], "width": "40 * s", "height": "40 * s",
  "color": [60, 50, 45], "filled": true, "alpha": 0}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `name` | string | **required** | Entity identifier |
| `shape` | string | **required** | `rect` `circle` `line` `star` `text` `sprite` `none` |
| `position` | [X, Y] | [0, 0] | Expression support |
| `scale` | [X, Y] | [1, 1] | |
| `rotation` | number | 0 | Degrees |
| `alpha` | number | 255 | Opacity 0–255 |
| `color` | [R, G, B] | [60, 50, 45] | |
| `width` | number/expr | 0 | Rect/line |
| `height` | number/expr | 0 | Rect/line |
| `radius` | number/expr | 0 | Circle/star |
| `corruption` | number | 0.1 | Ink noise 0.0–1.0 |
| `seed` | uint | 0 | Noise seed |
| `zOrder` | int | 0 | Higher = on top |
| `filled` | bool | false | Filled vs outline |
| `hasEyes` | bool | false | Snake eyes |
| `isApple` | bool | false | Apple rendering |
| `text` | string | "" | For `text` shape |
| `charSize` | number/expr | 28 | Font size |
| `texturePath` | string | "" | For `sprite` shape |
| `flipX` | bool | false | Horizontal flip |
| `flipY` | bool | false | Vertical flip |
| `parent` | string | "" | Parent entity name |
| `visible` | bool | true | Render visibility |

### destroy

Remove an entity.

```json
{"type": "destroy", "name": "player"}
```

### animate

Smoothly interpolate a property over time.

```json
{"type": "animate", "entity": "player", "property": "alpha",
  "from": 0, "to": 255, "duration": 1.0, "easing": "EaseOutCubic"}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `entity` | string | "" | Entity name (empty for camera properties) |
| `property` | string | **required** | See Animation Properties table |
| `from` | number/[X,Y] | *current* | Omit to read current value at start |
| `to` | number/[X,Y] | 0 | Target value (expression support) |
| `duration` | number | 1.0 | Seconds |
| `easing` | string | "EaseOutQuad" | See Easing table |

For 2D properties (`position`, `scale`), use arrays: `"to": ["cx", "cy"]`.

### keyframe

Multi-keyframe animation with per-keyframe easing.

```json
{"type": "keyframe", "entity": "player", "property": "rotation",
  "keyframes": [
    {"time": 0, "value": 0},
    {"time": 0.5, "value": 45, "easing": "EaseOutQuad"},
    {"time": 1.0, "value": 0, "easing": "EaseOutElastic"}
  ]}
```

| Field | Type | Notes |
|-------|------|-------|
| `time` | number | Absolute seconds from action start |
| `value` | number/[X,Y] | Target at this keyframe |
| `easing` | string | Easing INTO this keyframe (default: "EaseOutQuad") |

Supports 2D properties — use `[X, Y]` arrays for `value`.

### bezier

Move along a bezier curve.

```json
{"type": "bezier", "entity": "player",
  "path": [
    {"p0": ["cx", "cy"], "c1": ["cx + 100", "cy - 50"],
     "c2": ["cx + 150", "cy - 100"], "p3": ["cx + 200", "cy"]}
  ],
  "duration": 2.0, "orientToPath": true, "easing": "EaseInOutCubic"}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `entity` | string | **required** | |
| `path` | array | **required** | Array of segments |
| `duration` | number | 1.0 | |
| `easing` | string | "EaseInOutCubic" | |
| `orientToPath` | bool | false | Rotate entity to face tangent |

**Cubic segment**: `{"p0": [x,y], "c1": [x,y], "c2": [x,y], "p3": [x,y]}`
**Quadratic segment**: `{"p0": [x,y], "control": [x,y], "p3": [x,y]}`

All coordinates support expressions.

### expression

Attach a continuous mathematical animation to an entity property. Runs every frame until the entity is destroyed.

```json
{"type": "expression", "entity": "player", "property": "scaleX",
  "expr": {"func": "breathing", "frequency": 0.5, "amplitude": 0.03, "offset": 1.0}}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `func` | string | "sin" | `sin` `cos` `triangle` `sawtooth` `breathing` |
| `frequency` | number | 1.0 | Oscillation frequency (Hz) |
| `amplitude` | number | 10.0 | |
| `phase` | number | 0.0 | Phase offset (sin/cos only) |
| `offset` | number/expr | 0 | Base value added to oscillation |

**Formulas:**
- `sin`: `sin(t × freq + phase) × amplitude + offset`
- `cos`: `cos(t × freq + phase) × amplitude + offset`
- `triangle`: triangle wave at freq/amplitude + offset
- `sawtooth`: sawtooth wave at freq/amplitude + offset
- `breathing`: `offset + sin(t × 2π / (1/freq)) × amplitude` — smooth breathing effect

### camera

Instant camera control. For smooth animations, use `animate` with `cameraX`/`cameraY`/`cameraZoom`/`cameraRotation`.

```json
{"type": "camera", "action": "follow", "entity": "player", "smoothing": 7.0}
{"type": "camera", "action": "stopFollow"}
{"type": "camera", "action": "pan", "to": ["cx", "cy"]}
{"type": "camera", "action": "zoom", "to": 1.5}
```

| Action | Fields | Notes |
|--------|--------|-------|
| `follow` | `entity`, `smoothing` (default: 5.0) | Smooth follow |
| `stopFollow` | — | Stop following |
| `pan` | `to` [X, Y] | Instant position set |
| `zoom` | `to` (number) | Instant zoom set |

### fade

Fade to/from a color overlay.

```json
{"type": "fade", "toBlack": true, "duration": 1.0, "color": [0, 0, 0]}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `toBlack` | bool | true | true = fade to color, false = fade from color |
| `duration` | number | 1.0 | |
| `color` | [R,G,B] or [R,G,B,A] | black | |

Fade-to-black is **persistent** — it stays on screen until a fade-out or `clearPersistent`.

### transition

Shader-based screen transitions.

```json
{"type": "transition", "kind": "irisClose", "duration": 1.5, "color": [60, 50, 45]}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `kind` | string | "fade" | See Transition Types table |
| `duration` | number | 1.0 | |
| `color` | [R,G,B] | black | |
| `easing` | string | "EaseInOutCubic" | |

### text

Typewriter text with character-by-character reveal.

```json
{"type": "text", "text": "The world begins...",
  "position": ["cx - 150", "cy + 100"], "charSize": "28 * s",
  "color": [60, 50, 45], "charsPerSec": 20, "waitForInput": true}
```

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `text` | string | **required** | |
| `position` | [X, Y] | [0, 0] | Expression support |
| `charSize` | number/expr | 28 | |
| `color` | [R,G,B] | [60, 50, 45] | |
| `charsPerSec` | number | 20 | |
| `waitForInput` | bool | true | Block until Enter/Space |

Text is **persistent** — stays until `clearPersistent`.

### parallel

Run multiple actions simultaneously. Completes when all actions finish.

```json
{"type": "parallel", "actions": [
  {"type": "animate", "entity": "a", "property": "alpha", "to": 255, "duration": 1.0},
  {"type": "animate", "entity": "b", "property": "position", "to": ["cx", "cy"], "duration": 1.5},
  {"type": "sound", "sound": "whoosh"}
]}
```

### sound

Play a sound effect.

```json
{"type": "sound", "sound": "phase_advance"}
```

### shake

Screen shake.

```json
{"type": "shake", "duration": 0.3, "intensity": 4.0}
```

### particles

Spawn particle burst.

```json
{"type": "particles", "kind": "inkSplat", "position": ["cx", "cy"],
  "color": [180, 55, 45], "count": 15}
```

| Kind | Description |
|------|-------------|
| `inkDust` | Small floating dust |
| `inkSplat` | Splatter burst |
| `inkDrips` | Dripping particles |

### background

Set paper tone and ink tint.

```json
{"type": "background", "paperTone": [240, 230, 210], "inkTint": [40, 30, 20], "corruption": 0.15}
```

### postProcess

Apply or animate post-processing effects.

```json
{"type": "postProcess", "corruption": 0.3, "inkBleed": true}
```

Animated variant (add `duration`):

```json
{"type": "postProcess", "duration": 1.4,
  "fromCorruption": 0.0, "toCorruption": 0.35, "easing": "EaseInQuad",
  "chromatic": true}
```

| Field | Type | Default |
|-------|------|---------|
| `corruption` | number | 0.0 |
| `inkBleed` | bool | false |
| `chromatic` | bool | false |
| `psychedelic` | bool | false |
| `duration` | number | — (omit for instant) |
| `fromCorruption` | number | 0.0 |
| `toCorruption` | number | `corruption` |
| `easing` | string | "EaseOutQuad" |

### clearPersistent

Clear all persistent overlays (fades, text).

```json
{"type": "clearPersistent"}
```

---

## Reference Tables

### Animation Properties

| Property | Dims | Range | Notes |
|----------|------|-------|-------|
| `positionX` | 1D | — | |
| `positionY` | 1D | — | |
| `position` | 2D | — | `[X, Y]` array |
| `scaleX` | 1D | 0+ | |
| `scaleY` | 1D | 0+ | |
| `scale` | 2D | — | `[X, Y]` array |
| `rotation` | 1D | degrees | |
| `alpha` | 1D | 0–255 | |
| `colorR` | 1D | 0–255 | |
| `colorG` | 1D | 0–255 | |
| `colorB` | 1D | 0–255 | |
| `corruption` | 1D | 0.0–1.0 | |
| `cameraX` | 1D | — | Use with `entity: ""` |
| `cameraY` | 1D | — | Use with `entity: ""` |
| `cameraZoom` | 1D | 0.001+ | |
| `cameraRotation` | 1D | degrees | |

### Easing Functions

| Family | Functions |
|--------|-----------|
| Linear | `Linear` |
| Quadratic | `EaseInQuad` `EaseOutQuad` `EaseInOutQuad` |
| Cubic | `EaseInCubic` `EaseOutCubic` `EaseInOutCubic` |
| Quartic | `EaseInQuart` `EaseOutQuart` `EaseInOutQuart` |
| Quintic | `EaseInQuint` `EaseOutQuint` `EaseInOutQuint` |
| Exponential | `EaseInExpo` `EaseOutExpo` `EaseInOutExpo` |
| Back | `EaseInBack` `EaseOutBack` `EaseInOutBack` |
| Elastic | `EaseInElastic` `EaseOutElastic` `EaseInOutElastic` |
| Bounce | `BounceIn` `BounceOut` `BounceInOut` |
| Spring | `Spring` |

Default when unspecified: `EaseOutQuad` (animate/keyframe) or `EaseInOutCubic` (bezier/transition).

### Entity Shapes

| Shape | Key Properties |
|-------|---------------|
| `rect` | `width`, `height`, `filled` |
| `circle` | `radius`, `filled` |
| `line` | `width` (x offset), `height` (y offset) |
| `star` | `radius` |
| `text` | `text`, `charSize` |
| `sprite` | `texturePath`, `flipX`, `flipY` |
| `none` | Invisible grouping container |

### Transition Types

`fade` `wipeLeft` `wipeRight` `wipeUp` `wipeDown` `irisOpen` `irisClose` `dissolve`

---

## Tips

- **Comments**: Use `{"_comment": "Act 1 — Setup"}` anywhere in the timeline.
- **Choreography**: Wrap related animations in `parallel` to synchronize them.
- **Persistent actions**: `fade` (to-black) and `text` stay on screen. Use `clearPersistent` to remove them.
- **Responsive layout**: Use `s` for dimensions and `cx`/`cy` for centering. Set `referenceResolution` to your authoring resolution.
- **Parent-child**: Set `"parent": "groupName"` on spawn. Child positions are relative to parent's transform (position + rotation + scale).
- **Camera animations**: `camera` action is instant. For smooth pans/zooms, use `animate` with `cameraX`/`cameraY`/`cameraZoom`.
- **Deferred from**: Omit `"from"` in `animate` to automatically read the current value when the action starts — useful for chaining animations.
