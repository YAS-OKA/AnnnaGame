#include "../../stdafx.h"
#include "Card.h"
#include"../Chara/Player.h"
#include"../Object.h"
#include"../Utilities.h"
#include"../UI.h"
#include"../Scenes.h"

CardComponent::CardComponent(StringView assetPath, const Borrow<Object>& player, const std::function<double()>& gageRate)
	:assetPath(assetPath), player(player), gageRate(gageRate)
{
}

void CardComponent::start()
{
	Component::start();

	Object* innnerOwner = dynamic_cast<Object*>(owner);

	m_bar = innnerOwner->scene->birth<ui::ProgressBar>();

	m_bar->setting(Vec3{ 0,0,0 }, 70, 5);

	m_bar->transform->setParent(innnerOwner->transform);

	m_bar->visible(false);

	innnerOwner->addComponent<DrawTexture>(innnerOwner->scene->getDrawManager(), assetPath)
		->aspect.setScale(0.4);
}

void CardComponent::update(double dt)
{
	Component::update(dt);

	if (not m_bar->visible() and player)m_bar->visible(true);

	if (m_bar->visible())
	{
		if (owner->owner->isKilled(player))
		{
			player = nullptr;
			m_bar->visible(false);
		}
		else if(gageRate)
		{
			m_bar->rate(gageRate());
		}
	}
}
