#include "../../stdafx.h"
#include "PartsMirrored.h"
#include"../../Motions/Parts.h"

PartsMirrored::PartsMirrored(mot::PartsManager* pman, bool m)
	:pman(pman), active(false)
{
	if (m)
	{
		const auto& s = pman->master->getScale();
		pman->master->setScale({ s.x * -1,s.y });
	}
}

void PartsMirrored::start()
{
	Component::start();
	transform = owner->getComponent<Transform>();
}

void PartsMirrored::update(double dt)
{
	Component::update(dt);
	//反転を検知
	if (not active)
	{
		if (transform->direction.vector.x < 0)
		{
			active = true;
			firstScale = pman->master->getScale().x;
			timer = 0;
		}
	}
	//反転中
	if (active)
	{
		timer += dt;
		if (timer >= mirroredTime) {
			timer = mirroredTime;
			active = false;
		}
		pman->master->setScale({ firstScale * (1 - 2 * timer / mirroredTime),pman->master->getScale().y });		
	}
}
