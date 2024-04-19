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
	ui::ProgressBar* m_bar=nullptr;

	std::function<double()> gageRate = nullptr;

	String assetPath = U"";

	Object* player = nullptr;

	CardComponent(StringView assetPath, Object* player, const std::function<double()>& gageRate);

	void start()override;

	void update(double dt);
};
