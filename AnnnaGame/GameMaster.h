#pragma once
#include"EC.hpp"

class GameMaster final :public Entity
{
public:
	void start()override;
	void update(double dt)override;
private:
	struct Impl;
	std::shared_ptr<Impl> p_impl;
};
