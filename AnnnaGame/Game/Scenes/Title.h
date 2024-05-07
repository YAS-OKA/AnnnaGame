#pragma once
#include"../Scenes.h"

class Title:public my::Scene
{
public:
	void start()override;

	void update(double dt)override;
};
