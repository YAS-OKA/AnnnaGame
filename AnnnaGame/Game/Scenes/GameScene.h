#pragma once
#include"../Scenes.h"

class GameScene :public my::Scene
{
public:
	class Camera* camera;
	class Player* player;

	~GameScene();

	void start()override;

	void update(double dt)override;
};
