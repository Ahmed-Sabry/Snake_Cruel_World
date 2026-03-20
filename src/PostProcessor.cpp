#include "PostProcessor.h"
#include "TextureGenerator.h"

PostProcessor::PostProcessor()
	: m_available(false),
	  m_initialized(false),
	  m_rendering(false),
	  m_vignetteStrength(0.3f),
	  m_vignetteRadius(0.7f),
	  m_enableInkBleed(false),
	  m_inkBleedAmount(0.0f),
	  m_enableChromatic(false),
	  m_chromaticAmount(0.0f),
	  m_enablePsychedelic(false),
	  m_psychedelicIntensity(0.0f),
	  m_time(0.0f)
{
}

bool PostProcessor::Init(unsigned int l_width, unsigned int l_height)
{
	m_resolution = sf::Vector2f((float)l_width, (float)l_height);

	if (!sf::Shader::isAvailable())
	{
		m_available = false;
		m_initialized = true;
		return false;
	}

	if (!m_rtA.create(l_width, l_height) || !m_rtB.create(l_width, l_height))
	{
		m_available = false;
		m_initialized = true;
		return false;
	}

	m_available = LoadShaders();
	if (m_available)
	{
		m_paperNoiseTex = TextureGenerator::GenerateNoiseTexture(256, 256, 42);
		m_paperNoiseTex.setRepeated(true);
	}

	m_spriteA.setTexture(m_rtA.getTexture());
	m_spriteB.setTexture(m_rtB.getTexture());

	m_initialized = true;
	return m_available;
}

bool PostProcessor::LoadShaders()
{
	bool ok = true;

	// Paper overlay
	ok &= m_paperShader.loadFromFile("content/shaders/paper.frag", sf::Shader::Fragment);
	// Vignette
	ok &= m_vignetteShader.loadFromFile("content/shaders/vignette.frag", sf::Shader::Fragment);
	// Ink bleed
	ok &= m_inkBleedShader.loadFromFile("content/shaders/inkbleed.frag", sf::Shader::Fragment);
	// Chromatic aberration
	ok &= m_chromaticShader.loadFromFile("content/shaders/chromatic.frag", sf::Shader::Fragment);
	// Psychedelic hue shift
	ok &= m_psychedelicShader.loadFromFile("content/shaders/psychedelic.frag", sf::Shader::Fragment);

	return ok;
}

void PostProcessor::Configure(const LevelConfig& l_config)
{
	m_vignetteStrength = 0.3f + l_config.corruption * 0.5f;
	m_vignetteRadius = 0.7f - l_config.corruption * 0.3f;
	m_enableInkBleed = l_config.enableInkBleed;
	m_inkBleedAmount = l_config.corruption * 2.0f;
	m_enableChromatic = l_config.enableChromatic;
	m_chromaticAmount = 1.0f + l_config.corruption * 2.0f;
	m_enablePsychedelic = l_config.enablePsychedelic;
	m_psychedelicIntensity = l_config.corruption * 0.5f;
}

void PostProcessor::Update(float l_dt)
{
	m_time += l_dt;
}

void PostProcessor::Begin()
{
	if (!m_available) return;
	m_rendering = true;
	m_rtA.clear(sf::Color::Transparent);
}

void PostProcessor::End()
{
	if (!m_available) return;
	m_rtA.display();
	m_rendering = false;
}

sf::RenderTarget& PostProcessor::GetTarget()
{
	return m_rtA;
}

void PostProcessor::Apply(Window& l_window)
{
	if (!m_available)
		return;

	bool pingA = true; // true = source is m_rtA, draw to m_rtB

	// Pass 1: Paper texture overlay
	{
		sf::RenderTexture& dst = pingA ? m_rtB : m_rtA;
		sf::Sprite& srcSprite = pingA ? m_spriteA : m_spriteB;

		m_paperShader.setUniform("scene", sf::Shader::CurrentTexture);
		m_paperShader.setUniform("paperTex", m_paperNoiseTex);
		m_paperShader.setUniform("paperOpacity", 0.08f);
		m_paperShader.setUniform("resolution", m_resolution);

		dst.clear();
		dst.draw(srcSprite, &m_paperShader);
		dst.display();
		pingA = !pingA;
	}

	// Pass 2: Vignette
	{
		sf::RenderTexture& dst = pingA ? m_rtB : m_rtA;
		sf::Sprite& srcSprite = pingA ? m_spriteA : m_spriteB;

		m_vignetteShader.setUniform("scene", sf::Shader::CurrentTexture);
		m_vignetteShader.setUniform("resolution", m_resolution);
		m_vignetteShader.setUniform("vignetteStrength", m_vignetteStrength);
		m_vignetteShader.setUniform("vignetteRadius", m_vignetteRadius);

		dst.clear();
		dst.draw(srcSprite, &m_vignetteShader);
		dst.display();
		pingA = !pingA;
	}

	// Pass 3: Ink bleed (levels 7+)
	if (m_enableInkBleed)
	{
		sf::RenderTexture& dst = pingA ? m_rtB : m_rtA;
		sf::Sprite& srcSprite = pingA ? m_spriteA : m_spriteB;

		m_inkBleedShader.setUniform("scene", sf::Shader::CurrentTexture);
		m_inkBleedShader.setUniform("resolution", m_resolution);
		m_inkBleedShader.setUniform("time", m_time);
		m_inkBleedShader.setUniform("wobbleAmount", m_inkBleedAmount);

		dst.clear();
		dst.draw(srcSprite, &m_inkBleedShader);
		dst.display();
		pingA = !pingA;
	}

	// Pass 4: Chromatic aberration (level 9, 10)
	if (m_enableChromatic)
	{
		sf::RenderTexture& dst = pingA ? m_rtB : m_rtA;
		sf::Sprite& srcSprite = pingA ? m_spriteA : m_spriteB;

		m_chromaticShader.setUniform("scene", sf::Shader::CurrentTexture);
		m_chromaticShader.setUniform("resolution", m_resolution);
		m_chromaticShader.setUniform("amount", m_chromaticAmount);

		dst.clear();
		dst.draw(srcSprite, &m_chromaticShader);
		dst.display();
		pingA = !pingA;
	}

	// Pass 5: Psychedelic hue shift (level 9)
	if (m_enablePsychedelic)
	{
		sf::RenderTexture& dst = pingA ? m_rtB : m_rtA;
		sf::Sprite& srcSprite = pingA ? m_spriteA : m_spriteB;

		m_psychedelicShader.setUniform("scene", sf::Shader::CurrentTexture);
		m_psychedelicShader.setUniform("resolution", m_resolution);
		m_psychedelicShader.setUniform("time", m_time);
		m_psychedelicShader.setUniform("intensity", m_psychedelicIntensity);

		dst.clear();
		dst.draw(srcSprite, &m_psychedelicShader);
		dst.display();
		pingA = !pingA;
	}

	// Draw final result to window
	sf::Sprite& finalSprite = pingA ? m_spriteA : m_spriteB;
	l_window.Draw(finalSprite);
}

void PostProcessor::SetBlackoutParams(bool l_active, float l_headX, float l_headY)
{
	// Reserved for future blackout shader integration
	(void)l_active;
	(void)l_headX;
	(void)l_headY;
}
