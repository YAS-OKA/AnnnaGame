#include "stdafx.h"
#include "GameMaster.h"
#include"Component/Transform.h"
#include"Game/Scenes.h"
#include"Game/Scenes/GameScene.h"
#include"Game/Object.h"
#include"Motions/MotionCreator.h"
#include"TestScene.h"

struct GameMaster::Impl
{
	EntityManager manager;
	double timeScale = 1.0;
	GameMaster* master;
	my::Scene* scene;
	Impl()
	{
		//scene = manager.birth<TestScene>();

		//scene = manager.birth<mot::PartsEditor>();

		scene=manager.birth<GameScene>();
	}

	void update(double dt)
	{
		manager.update(dt);
	}
};

void GameMaster::start()
{
	p_impl = std::make_shared<Impl>();
	p_impl->master = this;
}

void GameMaster::update(double dt)
{
	p_impl->update(dt);
}
