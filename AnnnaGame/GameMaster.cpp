﻿#include "stdafx.h"
#include "GameMaster.h"
#include"Component/Transform.h"
#include"Game/Scenes.h"
#include"Game/Scenes/GameScene.h"
#include"Game/Object.h"
#include"Motions/MotionCreator.h"
#include"TestScene.h"
#include"Game/Scenes/Title.h"

struct GameMaster::Impl
{
	EntityManager manager;
	double timeScale = 1.0;
	Borrow<GameMaster> master;
	Borrow<my::Scene> scene;
	Impl()
	{
		//scene = manager.birth<TestScene>();

		//scene = manager.birth<mot::PartsEditor>();

		//scene = manager.birth<Title>();

		scene = manager.birth<GameScene>();
	}

	void update(double dt)
	{
#if _DEBUG
		//シーンをチェンジ
		if (KeyN.down())
		{
			if (scene.cast<GameScene>())
			{
				manager.kill(scene);
				scene = manager.birth<mot::PartsEditor>();
			}
			else if (scene.cast<mot::PartsEditor>())
			{
				manager.kill(scene);
				scene = manager.birth<GameScene>();
			}
		}
#endif

		manager.update(dt);
	}
};

void GameMaster::start()
{
	p_impl = std::make_shared<Impl>();
	p_impl->master = *this;
}

#if _DEBUG&&0
bool push = false;
#endif

void GameMaster::update(double dt)
{
#if _DEBUG&&0
	if (Key0.down())push = true;
	if (not push)dt = 0;
#endif
	p_impl->update(dt);
}
