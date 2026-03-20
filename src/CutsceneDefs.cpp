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
	sf::Color inkColor(60, 50, 45);
	sf::Color darkInk(45, 35, 30);
	sf::Color redInk(180, 50, 40);

	// 1. Fade from black
	tl.Add(std::make_unique<FadeAction>(false, 1.5f));

	// 2. Spawn border entity (represents the world)
	tl.Add(std::make_unique<SpawnEntityAction>("border", EntityShape::Rect,
		[cx, cy, inkColor](CutsceneEntity& e)
		{
			float w = 500.f, h = 350.f;
			e.position = {cx - w / 2.f, cy - h / 2.f + 20.f};
			e.width = w;
			e.height = h;
			e.color = inkColor;
			e.corruption = 0.15f;
			e.seed = 42;
			e.alpha = 0.f;
			e.zOrder = -1;
		}));
	tl.Add(FadeEntityAction::Create("border", 255.f, 0.8f, Easing::EaseOutCubic));

	// 3. "Once upon a time, there was a snake."
	tl.Add(std::make_unique<TypewriterTextAction>(
		"Once upon a time, there was a snake.",
		sf::Vector2f(cx - 260.f, cy - 130.f),
		32, inkColor, 22.f, true));

	// 4. Spawn snake entity — small ink rectangle at center, fade in
	tl.Add(std::make_unique<SpawnEntityAction>("snake", EntityShape::Rect,
		[cx, cy, inkColor](CutsceneEntity& e)
		{
			e.position = {cx - 15.f, cy + 10.f};
			e.width = 30.f;
			e.height = 30.f;
			e.color = inkColor;
			e.corruption = 0.08f;
			e.seed = 7;
			e.alpha = 0.f;
		}));
	tl.Add(FadeEntityAction::Create("snake", 255.f, 0.5f));

	// 5. Animate snake moving right + ink splat + sound
	tl.AddParallel(Actions(
		MoveAction::Create("snake", {cx + 120.f, cy + 10.f}, 1.5f, Easing::EaseInOutCubic),
		std::make_unique<SoundAction>("apple_eat"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(cx + 80.f, cy + 25.f), inkColor, 12)
	));

	// 6. "It lived in a world drawn on notebook paper."
	tl.Add(std::make_unique<TypewriterTextAction>(
		"It lived in a world drawn on notebook paper.",
		sf::Vector2f(cx - 310.f, cy - 75.f),
		28, inkColor, 24.f, true));

	// 7. "The world was small."
	tl.Add(std::make_unique<TypewriterTextAction>(
		"The world was small.",
		sf::Vector2f(cx - 150.f, cy - 25.f),
		28, darkInk, 18.f, false));

	// 8. Animate border shrinking + screen shake
	tl.AddParallel(Actions(
		ScaleToAction::Create("border", {0.7f, 0.7f}, 2.0f, Easing::EaseInOutCubic),
		std::make_unique<ShakeAction>(0.4f, 4.f),
		std::make_unique<SoundAction>("world_shrink")
	));

	// 9. "And getting smaller."
	tl.Add(std::make_unique<TypewriterTextAction>(
		"And getting smaller.",
		sf::Vector2f(cx - 155.f, cy + 25.f),
		28, sf::Color(120, 50, 40), 16.f, true));

	// 10. Ink drips from top + pause
	tl.AddParallel(Actions(
		std::make_unique<ParticleAction>(ParticleSpawnType::InkDrips,
			sf::Vector2f(cx, 20.f), darkInk, 8),
		std::make_unique<WaitAction>(1.5f)
	));

	// 11. "The snake didn't know it yet..."
	tl.Add(std::make_unique<TypewriterTextAction>(
		"The snake didn't know it yet...",
		sf::Vector2f(cx - 220.f, cy + 85.f),
		28, inkColor, 16.f, false));

	// 12. Dramatic pause
	tl.Add(std::make_unique<WaitAction>(1.0f));

	// 13. Screen shake + crash + border turns red + "...but the world was cruel."
	tl.AddParallel(Actions(
		std::make_unique<ShakeAction>(0.6f, 8.f),
		std::make_unique<SoundAction>("wall_death"),
		std::make_unique<ParticleAction>(ParticleSpawnType::InkSplat,
			sf::Vector2f(cx, cy), redInk, 20),
		std::make_unique<TypewriterTextAction>(
			"...but the world was cruel.",
			sf::Vector2f(cx - 215.f, cy + 140.f),
			36, redInk, 30.f, false),
		std::make_unique<LambdaAction>([redInk](StateManager& sm)
		{
			(void)sm;
			if (CutsceneState::s_active)
			{
				auto* border = CutsceneState::s_active->GetScene().Get("border");
				if (border)
					border->color = redInk;
			}
		})
	));

	// 14. Hold
	tl.Add(std::make_unique<WaitAction>(2.0f));

	// 15. Fade to black
	tl.Add(std::make_unique<FadeAction>(true, 2.0f));

	// Mark intro as played so it doesn't replay
	tl.Add(std::make_unique<LambdaAction>([](StateManager& sm)
	{
		sm.introPlayed = true;
	}));

	return tl;
}

CutsceneTimeline CutsceneDefs::Build(const std::string& l_id, StateManager& l_sm)
{
	if (l_id == "intro")
		return BuildIntroCutscene(l_sm);

	// Unknown cutscene ID — return empty timeline (instantly finishes)
	return CutsceneTimeline{};
}
