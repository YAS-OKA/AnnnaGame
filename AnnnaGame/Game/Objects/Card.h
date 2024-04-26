#pragma once
#include"../../EC.hpp"
#include"../../Util/Util.h"

namespace ui
{
	class ProgressBar;
}

class Object;

class CardComponent :public Component
{
public:
	Borrow<ui::ProgressBar> m_bar;

	std::function<double()> gageRate = nullptr;

	String assetPath = U"";

	Borrow<Object> player;

	CardComponent(StringView assetPath, const Borrow<Object>& player, const std::function<double()>& gageRate);

	void start()override;

	void update(double dt);
};
