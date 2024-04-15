#pragma once
#include"../../EC.hpp"

class CardComponent :public Component
{
public:

	void start()override;

	void update(double dt);
};
