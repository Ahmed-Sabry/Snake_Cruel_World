#include "CutsceneScene.h"
#include <algorithm>
#include <cmath>

CutsceneEntity& CutsceneScene::Spawn(const std::string& l_name, EntityShape l_shape)
{
	// Remove any existing entity with this name to prevent duplicates
	Destroy(l_name);

	CutsceneEntity entity;
	entity.name = l_name;
	entity.shape = l_shape;
	m_entities.push_back(std::move(entity));
	m_sortDirty = true;
	return m_entities.back();
}

CutsceneEntity* CutsceneScene::Get(const std::string& l_name)
{
	for (auto& e : m_entities)
	{
		if (e.name == l_name)
			return &e;
	}
	return nullptr;
}

const CutsceneEntity* CutsceneScene::Get(const std::string& l_name) const
{
	for (const auto& e : m_entities)
	{
		if (e.name == l_name)
			return &e;
	}
	return nullptr;
}

const CutsceneEntity* CutsceneScene::GetConst(const std::string& l_name) const
{
	return Get(l_name);
}

void CutsceneScene::Destroy(const std::string& l_name)
{
	auto it = std::remove_if(m_entities.begin(), m_entities.end(),
				   [&](const CutsceneEntity& e) { return e.name == l_name; });
	if (it != m_entities.end())
	{
		m_entities.erase(it, m_entities.end());
		m_sortDirty = true;
	}
}

void CutsceneScene::Clear()
{
	m_entities.clear();
	m_sortDirty = false;
}

void CutsceneScene::Update(float l_dt)
{
	for (auto& entity : m_entities)
	{
		entity.totalTime += l_dt;
		for (auto& cb : entity.updateCallbacks)
			cb(l_dt, entity.totalTime);
	}
}

void CutsceneScene::Render(sf::RenderTarget& l_target, const sf::Font& l_font)
{
	if (m_sortDirty)
	{
		std::stable_sort(m_entities.begin(), m_entities.end(),
						 [](const CutsceneEntity& a, const CutsceneEntity& b)
						 { return a.zOrder < b.zOrder; });
		m_sortDirty = false;
	}

	for (const auto& entity : m_entities)
	{
		sf::Transform worldTransform = sf::Transform::Identity;
		if (!entity.parent.empty())
			worldTransform = BuildWorldTransform(entity, 0);
		entity.Render(l_target, l_font, worldTransform);
	}
}

sf::Transform CutsceneScene::GetWorldTransform(const std::string& l_name) const
{
	const CutsceneEntity* entity = GetConst(l_name);
	if (!entity)
		return sf::Transform::Identity;
	return BuildWorldTransform(*entity, 0);
}

sf::Transform CutsceneScene::BuildWorldTransform(const CutsceneEntity& l_entity, int l_depth) const
{
	// Guard against circular parent chains
	if (l_depth > 16)
		return sf::Transform::Identity;

	sf::Transform local;
	if (!l_entity.parent.empty())
	{
		const CutsceneEntity* parentEntity = GetConst(l_entity.parent);
		if (parentEntity)
		{
			// Build parent's world transform first
			sf::Transform parentWorld = BuildWorldTransform(*parentEntity, l_depth + 1);
			// Parent's local transform: translate + rotate + scale around parent position
			sf::Transform parentLocal;
			parentLocal.translate(parentEntity->position);
			parentLocal.rotate(parentEntity->rotation);
			parentLocal.scale(parentEntity->scale);
			return parentWorld * parentLocal;
		}
	}
	return sf::Transform::Identity;
}
