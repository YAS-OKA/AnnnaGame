#include "../../stdafx.h"
#include "PartsMirrored.h"
#include"../../Motions/Parts.h"

PartsMirrored::PartsMirrored(const Borrow<mot::PartsManager>& pman, bool m)
	:pman(pman), active(false), mirrored(false)
{
	if (m)
	{
		pman->dm->drawing.scale.x *= -1;
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
		if ((mirrored and transform->direction.vector.x > 0.5) or
			((not mirrored) and transform->direction.vector.x < -0.5))
		{
			active = true;
			firstScale = pman->dm->drawing.scale.x;
			timer = 0;
		}
	}
	//反転中
	if (active)
	{
		timer += dt;
		if (timer >= mirroredTime) {
			timer = mirroredTime;
			mirrored = !mirrored;
			active = false;
		}
		pman->dm->drawing.scale.x = firstScale * (1 - 2 * timer / mirroredTime);
	}
}
