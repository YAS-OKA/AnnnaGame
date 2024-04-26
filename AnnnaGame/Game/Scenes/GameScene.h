#pragma once
#include"../Scenes.h"

class GameScene :public my::Scene
{
public:
	Borrow<class Camera> camera;
	Borrow<class Player> player;

	~GameScene();

	void start()override;

	void update(double dt)override;
};
