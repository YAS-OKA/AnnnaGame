#pragma once

#include"../Component/Draw.h"

#include"BunchoUI.hpp"

using namespace BunchoUI;

template<>
class Draw2D<UIManager> :public IDraw2D
{
public:
	UIManager uiManager;

	template<class ...Args>
	Draw2D<UIManager>(DrawManager* drawManager, Args&&... args)
		:IDraw2D(drawManager), uiManager(args...)
	{}

	void update(double)override
	{
		uiManager.update();
	}

	void draw()const override
	{
		uiManager.draw();
	}
};

using UICanvas = Draw2D<UIManager>;
