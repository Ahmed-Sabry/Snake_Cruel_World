#include "CutsceneLoader.h"
#include "CutsceneActions.h"
#include "CutsceneState.h"
#include "KeyframeAction.h"
#include "BezierPath.h"
#include "ExpressionAction.h"
#include "SceneTransition.h"
#include "StateManager.h"
#include "Easing.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>

using json = nlohmann::json;

// ── Expression evaluator for position strings like "cx - 230" ────

struct ExprContext
{
	float cx, cy, w, h, s;
};

static float EvalToken(const std::string& token, const ExprContext& ctx)
{
	if (token == "cx") return ctx.cx;
	if (token == "cy") return ctx.cy;
	if (token == "w")  return ctx.w;
	if (token == "h")  return ctx.h;
	if (token == "s")  return ctx.s;
	// Try parsing as number
	try { return std::stof(token); }
	catch (...) {
		std::cerr << "CutsceneLoader: unknown expression token '" << token << "'\n";
		return 0.f;
	}
}

static float EvalExpr(const std::string& expr, const ExprContext& ctx)
{
	// Simple arithmetic expression evaluator: number | var | expr op expr
	// Supports +, -, *, / with left-to-right evaluation (no precedence beyond * / before + -)
	std::istringstream ss(expr);
	std::string token;
	float result = 0.f;

	// Two-pass: first handle * and /, then + and -
	// Actually, let's just do left-to-right with proper precedence using a simple approach

	std::vector<float> values;
	std::vector<char> ops;

	// Tokenize
	while (ss >> token)
	{
		if (token == "+" || token == "-" || token == "*" || token == "/")
		{
			ops.push_back(token[0]);
		}
		else
		{
			values.push_back(EvalToken(token, ctx));
		}
	}

	if (values.empty())
		return 0.f;

	// Guard against mismatched operators/values (e.g. trailing operator)
	if (ops.size() >= values.size())
		return values[0];

	// First pass: handle * and /
	std::vector<float> addValues;
	std::vector<char> addOps;
	addValues.push_back(values[0]);

	for (size_t i = 0; i < ops.size(); ++i)
	{
		if (ops[i] == '*')
		{
			addValues.back() *= values[i + 1];
		}
		else if (ops[i] == '/')
		{
			float div = values[i + 1];
			if (std::abs(div) > 0.0001f)
				addValues.back() /= div;
		}
		else
		{
			addOps.push_back(ops[i]);
			addValues.push_back(values[i + 1]);
		}
	}

	// Second pass: handle + and -
	result = addValues[0];
	for (size_t i = 0; i < addOps.size(); ++i)
	{
		if (addOps[i] == '+')
			result += addValues[i + 1];
		else if (addOps[i] == '-')
			result -= addValues[i + 1];
	}

	return result;
}

static float ResolveFloat(const json& j, const ExprContext& ctx)
{
	if (j.is_number())
		return j.get<float>();
	if (j.is_string())
		return EvalExpr(j.get<std::string>(), ctx);
	return 0.f;
}

static sf::Vector2f ResolveVec2(const json& j, const ExprContext& ctx)
{
	if (j.is_array() && j.size() >= 2)
		return {ResolveFloat(j[0], ctx), ResolveFloat(j[1], ctx)};
	return {0.f, 0.f};
}

static sf::Color ResolveColor(const json& j)
{
	if (j.is_array())
	{
		int r = j.size() > 0 ? j[0].get<int>() : 0;
		int g = j.size() > 1 ? j[1].get<int>() : 0;
		int b = j.size() > 2 ? j[2].get<int>() : 0;
		int a = j.size() > 3 ? j[3].get<int>() : 255;
		auto clamp = [](int v) { return (sf::Uint8)std::max(0, std::min(255, v)); };
		return sf::Color(clamp(r), clamp(g), clamp(b), clamp(a));
	}
	return sf::Color::Black;
}

static EntityShape ResolveShape(const std::string& name)
{
	if (name == "rect")    return EntityShape::Rect;
	if (name == "circle")  return EntityShape::Circle;
	if (name == "line")    return EntityShape::Line;
	if (name == "star")    return EntityShape::Star;
	if (name == "text")    return EntityShape::Text;
	if (name == "sprite")  return EntityShape::Sprite;
	return EntityShape::None;
}

// ── Action parsers ───────────────────────────────────────────────

static CutsceneActionPtr ParseAction(const json& j, const ExprContext& ctx);

static CutsceneActionPtr ParseWait(const json& j)
{
	return std::make_unique<WaitAction>(j.value("duration", 1.0f));
}

static CutsceneActionPtr ParseFade(const json& j)
{
	bool toBlack = j.value("toBlack", true);
	float duration = j.value("duration", 1.0f);
	sf::Color color = j.contains("color") ? ResolveColor(j["color"]) : sf::Color::Black;
	return std::make_unique<FadeAction>(toBlack, duration, color);
}

static CutsceneActionPtr ParseText(const json& j, const ExprContext& ctx)
{
	std::string text = j.value("text", "");
	sf::Vector2f pos = j.contains("position") ? ResolveVec2(j["position"], ctx) : sf::Vector2f{0.f, 0.f};
	unsigned int charSize = j.contains("charSize")
		? (unsigned int)ResolveFloat(j["charSize"], ctx) : 28;
	sf::Color color = j.contains("color") ? ResolveColor(j["color"]) : sf::Color(60, 50, 45);
	float charsPerSec = j.value("charsPerSec", 20.0f);
	bool waitForInput = j.value("waitForInput", true);
	return std::make_unique<TypewriterTextAction>(text, pos, charSize, color, charsPerSec, waitForInput);
}

static CutsceneActionPtr ParseWaitForInput(const json&)
{
	return std::make_unique<WaitForInputAction>();
}

static CutsceneActionPtr ParseSpawn(const json& j, const ExprContext& ctx)
{
	std::string name = j.value("name", "entity");
	EntityShape shape = ResolveShape(j.value("shape", "none"));

	// Capture overrides for the setup function
	json overrides = j.contains("overrides") ? j["overrides"] : j;
	ExprContext capturedCtx = ctx;

	auto setup = [overrides, capturedCtx](CutsceneEntity& e)
	{
		if (overrides.contains("position"))
		{
			e.position = ResolveVec2(overrides["position"], capturedCtx);
		}
		if (overrides.contains("scale"))
		{
			auto s = overrides["scale"];
			if (s.is_array() && s.size() >= 2)
			{
				e.scale.x = s[0].get<float>();
				e.scale.y = s[1].get<float>();
			}
		}
		if (overrides.contains("color"))
			e.color = ResolveColor(overrides["color"]);
		if (overrides.contains("alpha"))
			e.alpha = overrides["alpha"].get<float>();
		if (overrides.contains("width"))
			e.width = ResolveFloat(overrides["width"], capturedCtx);
		if (overrides.contains("height"))
			e.height = ResolveFloat(overrides["height"], capturedCtx);
		if (overrides.contains("radius"))
			e.radius = ResolveFloat(overrides["radius"], capturedCtx);
		if (overrides.contains("corruption"))
			e.corruption = overrides["corruption"].get<float>();
		if (overrides.contains("seed"))
			e.seed = overrides["seed"].get<unsigned int>();
		if (overrides.contains("zOrder"))
			e.zOrder = overrides["zOrder"].get<int>();
		if (overrides.contains("visible"))
			e.visible = overrides["visible"].get<bool>();
		if (overrides.contains("filled"))
			e.filled = overrides["filled"].get<bool>();
		if (overrides.contains("hasEyes"))
			e.hasEyes = overrides["hasEyes"].get<bool>();
		if (overrides.contains("isApple"))
			e.isApple = overrides["isApple"].get<bool>();
		if (overrides.contains("rotation"))
			e.rotation = overrides["rotation"].get<float>();
		if (overrides.contains("text"))
			e.text = overrides["text"].get<std::string>();
		if (overrides.contains("charSize"))
			e.charSize = (unsigned int)ResolveFloat(overrides["charSize"], capturedCtx);
		if (overrides.contains("parent"))
			e.parent = overrides["parent"].get<std::string>();
		if (overrides.contains("flipX"))
			e.flipX = overrides["flipX"].get<bool>();
		if (overrides.contains("flipY"))
			e.flipY = overrides["flipY"].get<bool>();
		if (overrides.contains("texturePath"))
		{
			e.texturePath = overrides["texturePath"].get<std::string>();
			e.LoadTexture();
		}
	};

	return std::make_unique<SpawnEntityAction>(name, shape, setup);
}

static CutsceneActionPtr ParseDestroy(const json& j)
{
	return std::make_unique<DestroyEntityAction>(j.value("name", ""));
}

static CutsceneActionPtr ParseAnimate(const json& j, const ExprContext& ctx)
{
	std::string entity = j.value("entity", "");
	std::string propStr = j.value("property", "positionX");
	float duration = j.value("duration", 1.0f);
	EasingFunc easing = Easing::FromName(j.value("easing", "EaseOutQuad"));

	// Handle 2D properties ("position", "scale")
	if (propStr == "position" || propStr == "scale")
	{
		sf::Vector2f target = j.contains("to") ? ResolveVec2(j["to"], ctx) : sf::Vector2f{0.f, 0.f};

		if (propStr == "position")
			return MoveAction::Create(entity, target, duration, easing);
		else
			return ScaleToAction::Create(entity, target, duration, easing);
	}

	// Single property
	AnimProperty prop = AnimPropertyUtil::FromName(propStr);
	float to = j.contains("to") ? ResolveFloat(j["to"], ctx) : 0.f;

	// Deferred (from current value) or explicit from
	if (j.contains("from"))
	{
		float from = ResolveFloat(j["from"], ctx);
		return std::make_unique<AnimateAction>(entity, prop, from, to, duration, easing);
	}

	// Use deferred (reads current value at Start)
	if (prop == AnimProperty::Alpha)
		return FadeEntityAction::Create(entity, to, duration, easing);
	if (prop == AnimProperty::Rotation)
		return RotateAction::Create(entity, to, duration, easing);

	// Camera properties: read current value from camera, not entity
	if (prop == AnimProperty::CameraX || prop == AnimProperty::CameraY ||
		prop == AnimProperty::CameraZoom || prop == AnimProperty::CameraRotation)
	{
		float fromValue = 0.f;
		if (CutsceneState::s_active)
		{
			auto& cam = CutsceneState::s_active->GetCamera();
			switch (prop)
			{
			case AnimProperty::CameraX:        fromValue = cam.position.x; break;
			case AnimProperty::CameraY:        fromValue = cam.position.y; break;
			case AnimProperty::CameraZoom:     fromValue = cam.zoom; break;
			case AnimProperty::CameraRotation: fromValue = cam.rotation; break;
			default: break;
			}
		}
		return std::make_unique<AnimateAction>(entity, prop, fromValue, to, duration, easing);
	}

	// Generic deferred single (entity-based properties)
	return std::make_unique<DeferredSingleAnimAction>(
		entity, prop, to, 0.f, duration, easing,
		[prop](const CutsceneEntity& e) -> float
		{
			switch (prop)
			{
			case AnimProperty::PositionX: return e.position.x;
			case AnimProperty::PositionY: return e.position.y;
			case AnimProperty::ScaleX:    return e.scale.x;
			case AnimProperty::ScaleY:    return e.scale.y;
			case AnimProperty::Rotation:  return e.rotation;
			case AnimProperty::Alpha:     return e.alpha;
			case AnimProperty::ColorR:    return (float)e.color.r;
			case AnimProperty::ColorG:    return (float)e.color.g;
			case AnimProperty::ColorB:    return (float)e.color.b;
			case AnimProperty::Corruption: return e.corruption;
			default: return 0.f;
			}
		});
}

static CutsceneActionPtr ParseKeyframe(const json& j, const ExprContext& ctx)
{
	std::string entity = j.value("entity", "");
	std::string propStr = j.value("property", "positionX");

	// 2D keyframes
	if (propStr == "position" || propStr == "scale")
	{
		AnimProperty propX = (propStr == "position") ? AnimProperty::PositionX : AnimProperty::ScaleX;
		AnimProperty propY = (propStr == "position") ? AnimProperty::PositionY : AnimProperty::ScaleY;

		std::vector<Keyframe2D> keyframes;
		if (j.contains("keyframes"))
		{
			for (const auto& kf : j["keyframes"])
			{
				Keyframe2D k;
				k.time = kf.value("time", 0.f);
				k.value = kf.contains("value") ? ResolveVec2(kf["value"], ctx) : sf::Vector2f{0.f, 0.f};
				k.easing = Easing::FromName(kf.value("easing", "EaseOutQuad"));
				keyframes.push_back(k);
			}
		}
		return KeyframeTrack::Create2D(entity, propX, propY, std::move(keyframes));
	}

	// Single property keyframes
	AnimProperty prop = AnimPropertyUtil::FromName(propStr);
	std::vector<Keyframe> keyframes;
	if (j.contains("keyframes"))
	{
		for (const auto& kf : j["keyframes"])
		{
			Keyframe k;
			k.time = kf.value("time", 0.f);
			k.value = kf.contains("value") ? ResolveFloat(kf["value"], ctx) : 0.f;
			k.easing = Easing::FromName(kf.value("easing", "EaseOutQuad"));
			keyframes.push_back(k);
		}
	}
	return KeyframeTrack::Create(entity, prop, std::move(keyframes));
}

static CutsceneActionPtr ParseBezier(const json& j, const ExprContext& ctx)
{
	std::string entity = j.value("entity", "");
	float duration = j.value("duration", 1.0f);
	EasingFunc easing = Easing::FromName(j.value("easing", "EaseInOutCubic"));
	bool orientToPath = j.value("orientToPath", false);

	BezierPath path;
	if (j.contains("path"))
	{
		for (const auto& seg : j["path"])
		{
			sf::Vector2f p0 = seg.contains("p0") ? ResolveVec2(seg["p0"], ctx) : sf::Vector2f{0.f, 0.f};
			sf::Vector2f p3 = seg.contains("p3") ? ResolveVec2(seg["p3"], ctx) : sf::Vector2f{0.f, 0.f};

			if (seg.contains("c1") && seg.contains("c2"))
			{
				// Cubic
				sf::Vector2f c1 = ResolveVec2(seg["c1"], ctx);
				sf::Vector2f c2 = ResolveVec2(seg["c2"], ctx);
				path.AddCubic(p0, c1, c2, p3);
			}
			else if (seg.contains("control"))
			{
				// Quadratic
				sf::Vector2f ctrl = ResolveVec2(seg["control"], ctx);
				path.AddQuadratic(p0, ctrl, p3);
			}
		}
	}

	return BezierMove::Create(entity, std::move(path), duration, easing, orientToPath);
}

static CutsceneActionPtr ParseExpression(const json& j, const ExprContext& ctx)
{
	std::string entity = j.value("entity", "");
	AnimProperty prop = AnimPropertyUtil::FromName(j.value("property", "positionY"));

	ExprFunc expr;
	if (j.contains("expr"))
	{
		const auto& e = j["expr"];
		std::string func = e.value("func", "sin");
		float freq = e.value("frequency", 1.0f);
		float amplitude = e.value("amplitude", 10.0f);
		float phase = e.value("phase", 0.0f);
		float offset = e.contains("offset") ? ResolveFloat(e["offset"], ctx) : 0.f;

		if (func == "sin")
			expr = Expr::Sin(freq, amplitude, phase, offset);
		else if (func == "cos")
			expr = Expr::Cos(freq, amplitude, phase, offset);
		else if (func == "triangle")
			expr = Expr::Triangle(freq, amplitude, offset);
		else if (func == "sawtooth")
			expr = Expr::Sawtooth(freq, amplitude, offset);
		else if (func == "breathing")
			expr = Expr::Breathing(offset, amplitude, 1.f / std::max(0.001f, freq));
		else
			expr = Expr::Constant(offset);
	}
	else
	{
		expr = Expr::Constant(0.f);
	}

	return ExpressionAnim::Create(entity, prop, std::move(expr));
}

static CutsceneActionPtr ParseCamera(const json& j, const ExprContext& ctx)
{
	std::string action = j.value("action", "pan");

	if (action == "follow")
	{
		std::string entity = j.value("entity", "");
		float smoothing = j.value("smoothing", 5.0f);
		return std::make_unique<CameraFollowAction>(entity, smoothing);
	}
	if (action == "stopFollow")
	{
		return std::make_unique<CameraStopFollowAction>();
	}
	if (action == "pan")
	{
		sf::Vector2f to = j.contains("to") ? ResolveVec2(j["to"], ctx) : sf::Vector2f{0.f, 0.f};
		// Instant-set pan; for smooth panning, use animate with cameraX/cameraY in JSON
		return std::make_unique<LambdaAction>([to](StateManager&)
		{
			if (CutsceneState::s_active)
				CutsceneState::s_active->GetCamera().position = to;
		});
	}
	if (action == "zoom")
	{
		float to = j.contains("to") ? j["to"].get<float>() : 1.0f;
		// Instant-set zoom; for smooth zooming, use animate with cameraZoom in JSON
		return std::make_unique<LambdaAction>([to](StateManager&)
		{
			if (CutsceneState::s_active)
				CutsceneState::s_active->GetCamera().zoom = to;
		});
	}

	return std::make_unique<WaitAction>(0.f);
}

static CutsceneActionPtr ParseTransition(const json& j)
{
	TransitionType type = Transition::FromName(j.value("kind", "fade"));
	float duration = j.value("duration", 1.0f);
	sf::Color color = j.contains("color") ? ResolveColor(j["color"]) : sf::Color::Black;
	EasingFunc easing = Easing::FromName(j.value("easing", "EaseInOutCubic"));
	return Transition::Create(type, duration, color, easing);
}

static CutsceneActionPtr ParseSound(const json& j)
{
	return std::make_unique<SoundAction>(j.value("sound", ""));
}

static CutsceneActionPtr ParseShake(const json& j)
{
	return std::make_unique<ShakeAction>(j.value("duration", 0.5f), j.value("intensity", 5.0f));
}

static CutsceneActionPtr ParseParticles(const json& j, const ExprContext& ctx)
{
	std::string kindStr = j.value("kind", "inkDust");
	ParticleSpawnType kind = ParticleSpawnType::InkDust;
	if (kindStr == "inkSplat") kind = ParticleSpawnType::InkSplat;
	else if (kindStr == "inkDrips") kind = ParticleSpawnType::InkDrips;

	sf::Vector2f pos = j.contains("position") ? ResolveVec2(j["position"], ctx) : sf::Vector2f{0.f, 0.f};
	sf::Color color = j.contains("color") ? ResolveColor(j["color"]) : sf::Color(60, 50, 45);
	int count = j.value("count", 10);

	return std::make_unique<ParticleAction>(kind, pos, color, count);
}

static CutsceneActionPtr ParseBackground(const json& j)
{
	sf::Color paperTone = j.contains("paperTone") ? ResolveColor(j["paperTone"]) : sf::Color(245, 235, 220);
	sf::Color inkTint = j.contains("inkTint") ? ResolveColor(j["inkTint"]) : sf::Color(60, 50, 45);
	float corruption = j.value("corruption", 0.02f);
	return std::make_unique<SetBackgroundAction>(paperTone, inkTint, corruption);
}

static CutsceneActionPtr ParsePostProcess(const json& j)
{
	float corruption = j.value("corruption", 0.0f);
	bool inkBleed = j.value("inkBleed", false);
	bool chromatic = j.value("chromatic", false);
	bool psychedelic = j.value("psychedelic", false);

	if (j.contains("duration"))
	{
		float fromCorruption = j.value("fromCorruption", 0.0f);
		float toCorruption = j.value("toCorruption", corruption);
		float duration = j.value("duration", 1.0f);
		EasingFunc easing = Easing::FromName(j.value("easing", "EaseOutQuad"));
		return std::make_unique<AnimatePostProcessAction>(
			fromCorruption, toCorruption, duration, easing, inkBleed, chromatic, psychedelic);
	}

	return std::make_unique<PostProcessAction>(corruption, inkBleed, chromatic, psychedelic);
}

static CutsceneActionPtr ParseClearPersistent(const json&)
{
	return std::make_unique<ClearPersistentAction>();
}

static CutsceneActionPtr ParseParallel(const json& j, const ExprContext& ctx)
{
	std::vector<CutsceneActionPtr> actions;
	if (j.contains("actions"))
	{
		for (const auto& actionJson : j["actions"])
		{
			auto action = ParseAction(actionJson, ctx);
			if (action)
				actions.push_back(std::move(action));
		}
	}
	return std::make_unique<ParallelAction>(std::move(actions));
}

static CutsceneActionPtr ParseAction(const json& j, const ExprContext& ctx)
{
	std::string type = j.value("type", "");

	if (type == "wait")           return ParseWait(j);
	if (type == "fade")           return ParseFade(j);
	if (type == "text")           return ParseText(j, ctx);
	if (type == "waitForInput")   return ParseWaitForInput(j);
	if (type == "spawn")          return ParseSpawn(j, ctx);
	if (type == "destroy")        return ParseDestroy(j);
	if (type == "animate")        return ParseAnimate(j, ctx);
	if (type == "keyframe")       return ParseKeyframe(j, ctx);
	if (type == "bezier")         return ParseBezier(j, ctx);
	if (type == "expression")     return ParseExpression(j, ctx);
	if (type == "camera")         return ParseCamera(j, ctx);
	if (type == "transition")     return ParseTransition(j);
	if (type == "sound")          return ParseSound(j);
	if (type == "shake")          return ParseShake(j);
	if (type == "particles")      return ParseParticles(j, ctx);
	if (type == "background")     return ParseBackground(j);
	if (type == "postProcess")    return ParsePostProcess(j);
	if (type == "clearPersistent") return ParseClearPersistent(j);
	if (type == "parallel")       return ParseParallel(j, ctx);

	std::cerr << "CutsceneLoader: Unknown action type '" << type << "'" << std::endl;
	return std::make_unique<WaitAction>(0.f);
}

// ── Public API ───────────────────────────────────────────────────

static CutsceneTimeline BuildFromJson(const json& doc, StateManager& l_sm)
{
	CutsceneTimeline tl;

	// Build expression context from window size
	sf::Vector2u winSize = l_sm.GetWindow().GetWindowSize();
	ExprContext ctx;
	ctx.w = (float)winSize.x;
	ctx.h = (float)winSize.y;
	ctx.cx = ctx.w / 2.f;
	ctx.cy = ctx.h / 2.f;

	// Reference resolution scaling
	float refW = 1366.f, refH = 768.f;
	if (doc.contains("referenceResolution") && doc["referenceResolution"].is_array()
		&& doc["referenceResolution"].size() >= 2)
	{
		refW = doc["referenceResolution"][0].get<float>();
		refH = doc["referenceResolution"][1].get<float>();
	}
	ctx.s = std::min(ctx.w / refW, ctx.h / refH);

	// Parse timeline
	if (doc.contains("timeline") && doc["timeline"].is_array())
	{
		for (const auto& actionJson : doc["timeline"])
		{
			std::string type = actionJson.value("type", "");
			if (type == "parallel")
			{
				std::vector<CutsceneActionPtr> actions;
				if (actionJson.contains("actions"))
				{
					for (const auto& subAction : actionJson["actions"])
					{
						auto action = ParseAction(subAction, ctx);
						if (action)
							actions.push_back(std::move(action));
					}
				}
				tl.AddParallel(std::move(actions));
			}
			else
			{
				auto action = ParseAction(actionJson, ctx);
				if (action)
					tl.Add(std::move(action));
			}
		}
	}

	return tl;
}

CutsceneTimeline CutsceneLoader::LoadFromFile(const std::string& l_path, StateManager& l_sm)
{
	std::ifstream file(l_path);
	if (!file.is_open())
	{
		std::cerr << "CutsceneLoader: Failed to open " << l_path << std::endl;
		return CutsceneTimeline{};
	}

	try
	{
		json doc = json::parse(file);
		return BuildFromJson(doc, l_sm);
	}
	catch (const json::exception& e)
	{
		std::cerr << "CutsceneLoader: JSON parse error in " << l_path << ": " << e.what() << std::endl;
		return CutsceneTimeline{};
	}
}

CutsceneTimeline CutsceneLoader::LoadFromString(const std::string& l_json, StateManager& l_sm)
{
	try
	{
		json doc = json::parse(l_json);
		return BuildFromJson(doc, l_sm);
	}
	catch (const json::exception& e)
	{
		std::cerr << "CutsceneLoader: JSON parse error: " << e.what() << std::endl;
		return CutsceneTimeline{};
	}
}

bool CutsceneLoader::ReadMetadata(const std::string& l_path, CutsceneMetadata& l_out)
{
	std::ifstream file(l_path);
	if (!file.is_open())
		return false;

	try
	{
		json doc = json::parse(file);
		l_out.id = doc.value("id", "");
		l_out.displayName = doc.value("displayName", l_out.id);
		return !l_out.id.empty();
	}
	catch (...)
	{
		return false;
	}
}
