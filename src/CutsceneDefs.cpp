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
		std::make_unique<WaitAction>(0.3f)
	));

	// 6. Narration
	tl.Add(std::make_unique<TypewriterTextAction>(
		"A world, drawn on notebook paper.",
		sf::Vector2f(cx - 230.f * s, cy - 220.f * s),
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
		}));

	// 14. Fade in body2 + narration (parallel)
	tl.AddParallel(Actions(
		FadeEntityAction::Create("sB2", 255.f, 0.3f, Easing::EaseOutCubic),
		std::make_unique<TypewriterTextAction>(
			"And in this world, lived a small snake.",
			sf::Vector2f(cx - 275.f * s, cy - 175.f * s),
			(unsigned int)(28 * s), inkColor, 22.f, false)
	));

	// 15. Snake wiggle — gives it personality!
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		0.f, 5.f, 0.25f, Easing::EaseOutQuad));
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		5.f, -5.f, 0.3f, Easing::EaseOutQuad));
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		-5.f, 0.f, 0.25f, Easing::EaseOutElastic));

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

	// 17. Spawn apple
	tl.Add(std::make_unique<SpawnEntityAction>("apple", EntityShape::Star,
		[appleX, appleY, redInk, s](CutsceneEntity& e)
		{
			e.position = {appleX, appleY};
			e.radius = 10.f * s;
			e.color = redInk;
			e.corruption = 0.1f;
			e.seed = 99;
			e.alpha = 0.f;
		}));

	// 18. Apple fades in + narration
	tl.AddParallel(Actions(
		FadeEntityAction::Create("apple", 255.f, 0.5f, Easing::EaseOutCubic),
		std::make_unique<TypewriterTextAction>(
			"It ate apples. It was happy.",
			sf::Vector2f(cx - 200.f * s, cy - 130.f * s),
			(unsigned int)(28 * s), inkColor, 22.f, false)
	));

	// 19. Snake moves toward apple — staggered via duration differences
	//     Head arrives first, each body segment takes slightly longer
	float moveTargetHeadX = cx + 76.f * s;
	float moveTargetB1X   = cx + 54.f * s;
	float moveTargetB2X   = cx + 32.f * s;

	tl.AddParallel(Actions(
		std::make_unique<AnimateAction>("sHead", AnimProperty::PositionX,
			headX, moveTargetHeadX, 1.0f, Easing::EaseInOutCubic),
		std::make_unique<AnimateAction>("sB1", AnimProperty::PositionX,
			b1X, moveTargetB1X, 1.1f, Easing::EaseInOutCubic),
		std::make_unique<AnimateAction>("sB2", AnimProperty::PositionX,
			b2X, moveTargetB2X, 1.2f, Easing::EaseInOutCubic)
	));

	// 20. Apple eaten! Destruction + celebration
	tl.AddParallel(Actions(
		std::make_unique<DestroyEntityAction>("apple"),
		std::make_unique<SoundAction>("apple_eat"),
		std::make_unique<SoundAction>("combo_3x"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(appleX, appleY), redInk, 10)
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
			e.scale = {0.f, 0.f};
		}));
	tl.Add(ScaleToAction::Create("sB3", {1.f, 1.f}, 0.4f, Easing::EaseOutBack));

	// 22. Happy wiggle
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		0.f, 4.f, 0.15f, Easing::EaseOutQuad));
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		4.f, -4.f, 0.2f, Easing::EaseOutQuad));
	tl.Add(std::make_unique<AnimateAction>("sHead", AnimProperty::Rotation,
		-4.f, 0.f, 0.15f, Easing::EaseOutElastic));

	// 23. Pause
	tl.Add(std::make_unique<WaitAction>(0.5f));

	// ===================================================================
	// ACT 5 — THE TURN
	// ===================================================================

	// 24. Clear all previous text
	tl.Add(std::make_unique<ClearPersistentAction>());

	// 25. Dramatic silence
	tl.Add(std::make_unique<WaitAction>(1.0f));

	// 26. Heartbeat
	tl.Add(std::make_unique<SoundAction>("heartbeat"));

	// 27. THE pivotal line — only text that waits for input
	tl.Add(std::make_unique<TypewriterTextAction>(
		"But the world had other plans.",
		sf::Vector2f(cx - 220.f * s, cy - 220.f * s),
		(unsigned int)(30 * s), darkInk, 14.f, true));

	// 28. First shrink: 1.0 → 0.8
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.8f, 0.8f}, 1.5f, Easing::EaseInOutCubic),
		std::make_unique<ShakeAction>(0.4f, 4.f),
		std::make_unique<SoundAction>("world_shrink"),
		std::make_unique<TypewriterTextAction>(
			"The walls began to close in.",
			sf::Vector2f(cx - 200.f * s, cy - 175.f * s),
			(unsigned int)(28 * s), darkInk, 22.f, false)
	));

	// 29. Pause
	tl.Add(std::make_unique<WaitAction>(0.6f));

	// 30. Second shrink: 0.8 → 0.65 + visual corruption begins
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.65f, 0.65f}, 1.2f, Easing::EaseInOutCubic),
		std::make_unique<ShakeAction>(0.5f, 6.f),
		std::make_unique<SoundAction>("world_shrink"),
		std::make_unique<PostProcessAction>(0.3f)
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

	// 34. Ramp up visual corruption
	tl.Add(std::make_unique<PostProcessAction>(0.6f, true, true));

	// 35. Third shrink: 0.65 → 0.45 + border turns crimson
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.45f, 0.45f}, 1.5f, Easing::EaseInCubic),
		std::make_unique<SoundAction>("earthquake"),
		std::make_unique<ShakeAction>(0.7f, 8.f),
		std::make_unique<LambdaAction>([crimson](StateManager& sm)
		{
			(void)sm;
			if (CutsceneState::s_active)
			{
				auto* border = CutsceneState::s_active->GetScene().Get("border");
				if (border)
					border->color = crimson;
			}
		})
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

	// 40. Full post-processing chaos
	tl.Add(std::make_unique<PostProcessAction>(1.0f, true, true, true));

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
	tl.Add(std::make_unique<WaitAction>(0.3f));

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
