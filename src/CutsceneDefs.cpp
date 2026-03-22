#include "CutsceneDefs.h"
#include "CutsceneActions.h"
#include "CutsceneState.h"
#include "StateManager.h"
#include "LevelConfig.h"

static CutsceneTimeline BuildIntroCutscene(StateManager& l_sm)
{
	CutsceneTimeline tl;
	sf::Vector2u winSize = l_sm.GetWindow().GetWindowSize();
	float cx = (float)winSize.x / 2.f;
	float cy = (float)winSize.y / 2.f;

	// Scale all offsets and font sizes proportionally to the design resolution
	static constexpr float REF_W = 1366.f;
	static constexpr float REF_H = 768.f;
	float sx = (float)winSize.x / REF_W;
	float sy = (float)winSize.y / REF_H;
	float s = std::min(sx, sy);

	// Color palette
	sf::Color inkColor(60, 50, 45);
	sf::Color darkInk(45, 35, 30);
	sf::Color redInk(180, 50, 40);
	sf::Color crimson(160, 30, 30);
	sf::Color appleRed(180, 55, 45); // matches Level 1 apple color

	// Entity sizes
	float headSz = 24.f * s;
	float bodySz = 20.f * s;
	float borderW = 500.f * s;
	float borderH = 350.f * s;

	// ===================================================================
	// ACT 1 — THE BLANK PAGE
	// ===================================================================

	// 1. Fade from black
	tl.Add(std::make_unique<FadeAction>(false, 1.5f));

	// 2. Ambient ink dust + pause — the world doesn't exist yet
	tl.AddParallel(Actions(
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx, cy), inkColor, 6),
		std::make_unique<WaitAction>(1.5f)
	));

	// ===================================================================
	// ACT 2 — THE WORLD IS DRAWN
	// ===================================================================

	// 3. Spawn border at center, scale {0,0} — invisible until animated
	tl.Add(std::make_unique<SpawnEntityAction>("border", EntityShape::Rect,
		[cx, cy, inkColor, borderW, borderH, s](CutsceneEntity& e)
		{
			e.position = {cx - borderW / 2.f, cy - borderH / 2.f + 20.f * s};
			e.width = borderW;
			e.height = borderH;
			e.color = inkColor;
			e.corruption = 0.15f;
			e.seed = 42;
			e.alpha = 255.f;
			e.scale = {0.f, 0.f};
			e.zOrder = -1;
		}));

	// 4. Border pops into existence + dramatic sound + ink splat
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {1.f, 1.f}, 1.0f, Easing::EaseOutBack),
		std::make_unique<SoundAction>("phase_advance"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(cx, cy + 20.f * s), inkColor, 15)
	));

	// 4b. Border settle oscillation — dampened bounce after overshoot
	tl.Add(ScaleToAction::Create("border", {0.98f, 0.98f}, 0.12f, Easing::EaseInQuad));
	tl.Add(ScaleToAction::Create("border", {1.0f, 1.0f}, 0.15f, Easing::EaseOutQuad));

	// 5. Ink dust at all 4 corners (decorative flourish)
	tl.AddParallel(Actions(
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx - 240.f * s, cy - 145.f * s), inkColor, 4),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx + 240.f * s, cy - 145.f * s), inkColor, 4),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx - 240.f * s, cy + 195.f * s), inkColor, 4),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx + 240.f * s, cy + 195.f * s), inkColor, 4),
		std::make_unique<WaitAction>(0.6f)
	));

	// 6. Narration
	tl.Add(std::make_unique<TypewriterTextAction>(
		"A world, drawn on notebook paper.",
		sf::Vector2f(cx - 230.f * s, cy - 260.f * s),
		(unsigned int)(28 * s), inkColor, 24.f, false));

	// 7. Brief pause
	tl.Add(std::make_unique<WaitAction>(0.8f));

	// ===================================================================
	// ACT 3 — THE SNAKE IS BORN
	// ===================================================================

	// Snake segment positions (head at center, body trailing left)
	float headX = cx - headSz / 2.f;
	float headY = cy + 8.f * s;
	float b1X   = cx - headSz / 2.f - bodySz - 2.f * s;
	float b1Y   = cy + 10.f * s;
	float b2X   = b1X - bodySz - 2.f * s;
	float b2Y   = b1Y;

	// 8. Ink splat — the snake is being drawn into existence
	tl.Add(std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
		sf::Vector2f(cx, cy + 20.f * s), inkColor, 8));

	// 9-10. Spawn + fade in head
	tl.Add(std::make_unique<SpawnEntityAction>("sHead", EntityShape::Rect,
		[headX, headY, headSz, inkColor](CutsceneEntity& e)
		{
			e.position = {headX, headY};
			e.width = headSz;
			e.height = headSz;
			e.color = inkColor;
			e.corruption = 0.08f;
			e.seed = 7;
			e.alpha = 0.f;
			e.filled = true;
			e.hasEyes = true;
		}));
	tl.Add(FadeEntityAction::Create("sHead", 255.f, 0.4f, Easing::EaseOutCubic));

	// 11-12. Spawn + fade in body1
	tl.Add(std::make_unique<SpawnEntityAction>("sB1", EntityShape::Rect,
		[b1X, b1Y, bodySz, inkColor](CutsceneEntity& e)
		{
			e.position = {b1X, b1Y};
			e.width = bodySz;
			e.height = bodySz;
			e.color = inkColor;
			e.corruption = 0.08f;
			e.seed = 11;
			e.alpha = 0.f;
			e.filled = true;
		}));
	tl.Add(FadeEntityAction::Create("sB1", 255.f, 0.3f, Easing::EaseOutCubic));

	// 13. Spawn body2
	tl.Add(std::make_unique<SpawnEntityAction>("sB2", EntityShape::Rect,
		[b2X, b2Y, bodySz, inkColor](CutsceneEntity& e)
		{
			e.position = {b2X, b2Y};
			e.width = bodySz;
			e.height = bodySz;
			e.color = inkColor;
			e.corruption = 0.08f;
			e.seed = 13;
			e.alpha = 0.f;
			e.filled = true;
		}));

	// 14. Fade in body2 + narration (parallel)
	tl.AddParallel(Actions(
		FadeEntityAction::Create("sB2", 255.f, 0.3f, Easing::EaseOutCubic),
		std::make_unique<TypewriterTextAction>(
			"And in this world, lived a small snake.",
			sf::Vector2f(cx - 275.f * s, cy - 225.f * s),
			(unsigned int)(28 * s), inkColor, 22.f, false)
	));

	// 15. Snake wiggle with body follow-through
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			0.f, 8.f, 0.25f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			0.f, 4.f, 0.28f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			0.f, 2.f, 0.30f, Easing::EaseOutQuad)
	));
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			8.f, -8.f, 0.3f, Easing::EaseInOutQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			4.f, -4.f, 0.33f, Easing::EaseInOutQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			2.f, -2.f, 0.35f, Easing::EaseInOutQuad)
	));
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			-8.f, 0.f, 0.25f, Easing::EaseOutElastic),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			-4.f, 0.f, 0.28f, Easing::EaseOutElastic),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			-2.f, 0.f, 0.30f, Easing::EaseOutElastic)
	));

	// 16. Ink dust + pause
	tl.AddParallel(Actions(
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDust,
			sf::Vector2f(cx, cy + 20.f * s), inkColor, 5),
		std::make_unique<WaitAction>(0.5f)
	));

	// ===================================================================
	// ACT 4 — THE HAPPY LIFE
	// ===================================================================

	// Apple position (to the right of the snake)
	float appleX = cx + 100.f * s;
	float appleY = cy + 20.f * s;

	// 17. Spawn apple (behind snake so head can overlap it)
	tl.Add(std::make_unique<SpawnEntityAction>("apple", EntityShape::Circle,
		[appleX, appleY, appleRed, s](CutsceneEntity& e)
		{
			e.position = {appleX, appleY};
			e.radius = 10.f * s;
			e.color = appleRed;
			e.corruption = 0.1f;
			e.seed = 99;
			e.alpha = 0.f;
			e.filled = true;
			e.isApple = true;
			e.zOrder = -1;
		}));

	// 18. Apple fades in + narration
	tl.AddParallel(Actions(
		FadeEntityAction::Create("apple", 255.f, 0.5f, Easing::EaseOutCubic),
		std::make_unique<TypewriterTextAction>(
			"It ate apples. It was happy.",
			sf::Vector2f(cx - 200.f * s, cy - 190.f * s),
			(unsigned int)(28 * s), inkColor, 22.f, false)
	));

	float moveTargetHeadX = cx + 76.f * s;
	float moveTargetB1X   = cx + 54.f * s;
	float moveTargetB2X   = cx + 32.f * s;

	// 18b. Anticipation — snake coils slightly before pounce
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::PositionX,
			headX, headX - 4.f * s, 0.18f, Easing::EaseOutQuad),
		ScaleToAction::Create("sHead", {0.92f, 1.08f}, 0.18f, Easing::EaseOutQuad)
	));

	// 19. Snake moves toward apple — staggered + scale restore from coil
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::PositionX,
			headX - 4.f * s, moveTargetHeadX, 1.0f, Easing::EaseInOutCubic),
		ScaleToAction::Create("sHead", {1.f, 1.f}, 0.15f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::PositionX,
			b1X, moveTargetB1X, 1.1f, Easing::EaseInOutCubic),
		std::make_unique<AnimateAction>("sB2", AnimProperty::PositionX,
			b2X, moveTargetB2X, 1.2f, Easing::EaseInOutCubic)
	));

	// 20a. First bite — head covers apple, apple shifts right + shrinks
	//       (crescent of apple peeks out past the head = "bitten" look)
	float bitenAppleX = appleX + 8.f * s; // shift right so remnant peeks out
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("apple", AnimProperty::PositionX,
			appleX, bitenAppleX, 0.12f, Easing::EaseOutQuad),
		ScaleToAction::Create("apple", {0.55f, 0.55f}, 0.12f, Easing::EaseOutQuad),
		std::make_unique<SoundAction>("apple_eat"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(appleX, appleY), appleRed, 6)
	));

	// 20b. Pause — the remaining apple is visible as a small piece past the head
	tl.Add(std::make_unique<WaitAction>(0.3f));

	// 20c. Second bite — head lunges forward, swallows the rest
	float bite2X = moveTargetHeadX + 8.f * s;
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::PositionX,
			moveTargetHeadX, bite2X, 0.15f, Easing::EaseOutQuad),
		ScaleToAction::Create("apple", {0.f, 0.f}, 0.15f, Easing::EaseInCubic),
		ScaleToAction::Create("sHead", {1.15f, 1.15f}, 0.15f, Easing::EaseOutQuad),
		std::make_unique<SoundAction>("combo_3x"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(bitenAppleX, appleY), appleRed, 10)
	));

	// 20d. Head snaps back + destroy apple
	tl.AddParallel(Actions(
		ScaleToAction::Create("sHead", {1.f, 1.f}, 0.2f, Easing::EaseOutElastic),
		std::make_unique<DestroyEntityAction>("apple")
	));

	// 20e. Swallow pulse — wave down the body
	tl.AddParallel(Actions(
		ScaleToAction::Create("sB1", {1.12f, 1.12f}, 0.12f, Easing::EaseOutQuad),
		ScaleToAction::Create("sB2", {1.08f, 1.08f}, 0.2f, Easing::EaseOutQuad)
	));
	tl.AddParallel(Actions(
		ScaleToAction::Create("sB1", {1.f, 1.f}, 0.2f, Easing::EaseOutElastic),
		ScaleToAction::Create("sB2", {1.f, 1.f}, 0.2f, Easing::EaseOutElastic)
	));

	// 21. Snake grows! New body segment pops in with EaseOutBack
	float b3X = cx + 10.f * s;
	float b3Y = cy + 10.f * s;
	tl.Add(std::make_unique<SpawnEntityAction>("sB3", EntityShape::Rect,
		[b3X, b3Y, bodySz, inkColor](CutsceneEntity& e)
		{
			e.position = {b3X, b3Y};
			e.width = bodySz;
			e.height = bodySz;
			e.color = inkColor;
			e.corruption = 0.08f;
			e.seed = 17;
			e.alpha = 255.f;
			e.filled = true;
			e.scale = {0.f, 0.f};
		}));
	tl.Add(ScaleToAction::Create("sB3", {1.f, 1.f}, 0.4f, Easing::EaseOutBack));

	// 22. Happy wiggle with body follow-through
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			0.f, 7.f, 0.15f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			0.f, 3.5f, 0.18f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			0.f, 1.8f, 0.20f, Easing::EaseOutQuad),
		std::make_unique<AnimateAction>("sB3", AnimProperty::Rotation,
			0.f, 1.f, 0.22f, Easing::EaseOutQuad)
	));
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			7.f, -7.f, 0.2f, Easing::EaseInOutQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			3.5f, -3.5f, 0.23f, Easing::EaseInOutQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			1.8f, -1.8f, 0.25f, Easing::EaseInOutQuad),
		std::make_unique<AnimateAction>("sB3", AnimProperty::Rotation,
			1.f, -1.f, 0.27f, Easing::EaseInOutQuad)
	));
	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
			-7.f, 0.f, 0.15f, Easing::EaseOutElastic),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Rotation,
			-3.5f, 0.f, 0.18f, Easing::EaseOutElastic),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Rotation,
			-1.8f, 0.f, 0.20f, Easing::EaseOutElastic),
		std::make_unique<AnimateAction>("sB3", AnimProperty::Rotation,
			-1.f, 0.f, 0.22f, Easing::EaseOutElastic)
	));

	// 23. Pause
	tl.Add(std::make_unique<WaitAction>(0.5f));

	// ===================================================================
	// ACT 5 — THE TURN
	// ===================================================================

	// 23b. Dim snake — visual de-emphasis before the tonal shift
	tl.AddParallel(Actions(
		FadeEntityAction::Create("sHead", 200.f, 0.3f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB1", 200.f, 0.3f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB2", 200.f, 0.3f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB3", 200.f, 0.3f, Easing::EaseOutQuad)
	));

	// 24. Clear all previous text
	tl.Add(std::make_unique<ClearPersistentAction>());

	// 25. Dramatic silence
	tl.Add(std::make_unique<WaitAction>(1.0f));

	// 26. Heartbeat
	tl.Add(std::make_unique<SoundAction>("heartbeat"));

	// 26b. Snake snaps back to full visibility
	tl.AddParallel(Actions(
		FadeEntityAction::Create("sHead", 255.f, 0.15f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB1", 255.f, 0.15f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB2", 255.f, 0.15f, Easing::EaseOutQuad),
		FadeEntityAction::Create("sB3", 255.f, 0.15f, Easing::EaseOutQuad)
	));

	// 27. THE pivotal line — only text that waits for input
	tl.Add(std::make_unique<TypewriterTextAction>(
		"But the world had other plans.",
		sf::Vector2f(cx - 220.f * s, cy - 220.f * s),
		(unsigned int)(30 * s), darkInk, 14.f, true));

	// 27b. World inhale — anticipation for first shrink
	tl.Add(ScaleToAction::Create("border", {1.03f, 1.03f}, 0.25f, Easing::EaseOutQuad));

	// 28. First shrink: 1.0 → 0.8 + snake distress begins
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.8f, 0.8f}, 1.5f, Easing::EaseInOutCubic),
		std::make_unique<ShakeAction>(0.4f, 4.f),
		std::make_unique<SoundAction>("world_shrink"),
		std::make_unique<TypewriterTextAction>(
			"The walls began to close in.",
			sf::Vector2f(cx - 200.f * s, cy - 175.f * s),
			(unsigned int)(28 * s), darkInk, 22.f, false),
		std::make_unique<AnimateAction>("sHead", AnimProperty::Corruption,
			0.08f, 0.15f, 1.5f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Corruption,
			0.08f, 0.15f, 1.5f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Corruption,
			0.08f, 0.15f, 1.5f, Easing::EaseInQuad)
		// sB3 omitted — just spawned, stays "clean" until second shrink
	));

	// 29. Pause
	tl.Add(std::make_unique<WaitAction>(0.6f));

	// 29b. World inhale — second anticipation
	tl.Add(ScaleToAction::Create("border", {0.83f, 0.83f}, 0.2f, Easing::EaseOutQuad));

	// 30. Second shrink: 0.8 → 0.65 + visual corruption + snake agitation
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.65f, 0.65f}, 1.2f, Easing::EaseInOutCubic),
		std::make_unique<ShakeAction>(0.5f, 6.f),
		std::make_unique<SoundAction>("world_shrink"),
		std::make_unique<PostProcessAction>(0.3f),
		std::make_unique<AnimateAction>("sHead", AnimProperty::Corruption,
			0.15f, 0.25f, 1.2f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Corruption,
			0.15f, 0.25f, 1.2f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Corruption,
			0.15f, 0.25f, 1.2f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB3", AnimProperty::Corruption,
			0.08f, 0.25f, 1.2f, Easing::EaseInQuad)
	));

	// 31. Snake squeezed back toward center
	tl.AddParallel(Actions(
		MoveAction::Create("sHead", {cx - 12.f * s, headY}, 0.8f, Easing::EaseInOutQuad),
		MoveAction::Create("sB1",   {cx - 34.f * s, b1Y},  0.9f, Easing::EaseInOutQuad),
		MoveAction::Create("sB2",   {cx - 56.f * s, b2Y},  1.0f, Easing::EaseInOutQuad),
		MoveAction::Create("sB3",   {cx - 78.f * s, b3Y},  1.1f, Easing::EaseInOutQuad)
	));

	// 32. Ink drips from above — the world is bleeding
	tl.Add(std::make_unique<ParticleAction>(ParticleSpawnType::InkDrips,
		sf::Vector2f(cx, 20.f * s), darkInk, 10));

	// ===================================================================
	// ACT 6 — THE CRUEL TRUTH
	// ===================================================================

	// 33. Clear text for the final act
	tl.Add(std::make_unique<ClearPersistentAction>());

	// 34. Ramp up visual corruption (animated over 0.8s)
	tl.Add(std::make_unique<AnimatePostProcessAction>(0.3f, 0.6f, 0.8f, Easing::EaseInQuad, true, true));

	// 34b. World inhale — final anticipation
	tl.Add(ScaleToAction::Create("border", {0.68f, 0.68f}, 0.2f, Easing::EaseOutQuad));

	// 35. Third shrink: 0.65 → 0.45 + border color bleeds to crimson + max distress
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.45f, 0.45f}, 1.5f, Easing::EaseInCubic),
		std::make_unique<SoundAction>("earthquake"),
		std::make_unique<ShakeAction>(0.7f, 8.f),
		std::make_unique<AnimateAction>("border", AnimProperty::ColorR,
			(float)inkColor.r, (float)crimson.r, 1.5f, Easing::EaseInCubic),
		std::make_unique<AnimateAction>("border", AnimProperty::ColorG,
			(float)inkColor.g, (float)crimson.g, 1.5f, Easing::EaseInCubic),
		std::make_unique<AnimateAction>("border", AnimProperty::ColorB,
			(float)inkColor.b, (float)crimson.b, 1.5f, Easing::EaseInCubic),
		std::make_unique<AnimateAction>("sHead", AnimProperty::Corruption,
			0.25f, 0.45f, 1.5f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB1", AnimProperty::Corruption,
			0.25f, 0.45f, 1.5f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB2", AnimProperty::Corruption,
			0.25f, 0.45f, 1.5f, Easing::EaseInQuad),
		std::make_unique<AnimateAction>("sB3", AnimProperty::Corruption,
			0.25f, 0.45f, 1.5f, Easing::EaseInQuad)
	));

	// 36. Urgent narration + impact
	tl.AddParallel(Actions(
		std::make_unique<TypewriterTextAction>(
			"And with every apple...",
			sf::Vector2f(cx - 170.f * s, cy - 220.f * s),
			(unsigned int)(28 * s), redInk, 30.f, false),
		std::make_unique<SoundAction>("wall_death"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(cx, cy), redInk, 25)
	));

	// 37. The cruel conclusion
	tl.Add(std::make_unique<TypewriterTextAction>(
		"...the world grew crueler.",
		sf::Vector2f(cx - 210.f * s, cy - 170.f * s),
		(unsigned int)(36 * s), redInk, 28.f, false));

	// 38. Hold — let it sink in
	tl.Add(std::make_unique<WaitAction>(1.5f));

	// ===================================================================
	// ACT 7 — THE TITLE
	// ===================================================================

	// 39. Clear everything
	tl.Add(std::make_unique<ClearPersistentAction>());

	// 40. Full post-processing chaos (ramped over 0.5s)
	tl.Add(std::make_unique<AnimatePostProcessAction>(0.6f, 1.0f, 0.5f, Easing::EaseInCubic, true, true, true));

	// 41. Destroy the world — everything gone in a burst of red ink
	tl.AddParallel(Actions(
		std::make_unique<SoundAction>("phase_advance"),
		std::make_unique<ShakeAction>(0.8f, 12.f),
		std::make_unique<DestroyEntityAction>("border"),
		std::make_unique<DestroyEntityAction>("sHead"),
		std::make_unique<DestroyEntityAction>("sB1"),
		std::make_unique<DestroyEntityAction>("sB2"),
		std::make_unique<DestroyEntityAction>("sB3"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(cx, cy), redInk, 30)
	));

	// 42. Let the ink settle
	tl.Add(std::make_unique<WaitAction>(0.7f));

	// 43. THE TITLE
	tl.Add(std::make_unique<TypewriterTextAction>(
		"HELLO, CRUEL WORLD.",
		sf::Vector2f(cx - 230.f * s, cy - 30.f * s),
		(unsigned int)(48 * s), crimson, 40.f, false));

	// 44. Hold on the title
	tl.Add(std::make_unique<WaitAction>(2.0f));

	// ===================================================================
	// ACT 8 — FADE OUT
	// ===================================================================

	// 45. Fade to black
	tl.Add(std::make_unique<FadeAction>(true, 2.0f));

	// 46. Reset post-processing before gameplay
	tl.Add(std::make_unique<PostProcessAction>(0.f, false, false, false));

	// 47. Mark intro as played so it doesn't replay
	tl.Add(std::make_unique<LambdaAction>([](StateManager& sm)
	{
		sm.introPlayed = true;
	}));

	return tl;
}

std::vector<CutsceneDefs::CutsceneEntry> CutsceneDefs::GetAllEntries()
{
	return {
		{"intro", "The Cruel Beginning", [](const StateManager& sm) { return sm.introPlayed; }}
	};
}

CutsceneTimeline CutsceneDefs::Build(const std::string& l_id, StateManager& l_sm)
{
	if (l_id == "intro")
		return BuildIntroCutscene(l_sm);

	// Unknown cutscene ID — return empty timeline (instantly finishes)
	return CutsceneTimeline{};
}
