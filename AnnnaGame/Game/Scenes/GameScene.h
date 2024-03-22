#pragma once
#include"../Scenes.h"

class GameScene :public my::Scene
{
public:
	void start()override;

	class Camera* camera;
	class Player* player;

	void update(double dt)override;
};
