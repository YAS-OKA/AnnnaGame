﻿#include "stdafx.h"
#include "TestScene.h"
#include"Game/UI.h"
#include"Game/Utilities.h"
#include"Prg/Init.h"
#include"Motions/Motion.h"
#include"Component/CameraAffect/Convert2DScaleComponent.h"
#include"Component/CameraAffect/Convert2DTransformComponent.h"
#include"Motions/MotionCreator.h"
#include"Motions/BodyEditor.h"
#include"Game/Objects/Card.h"
#include"Game/Chara/Player.h"

Actions* ac;
Object* obj;
Array<Object*> en;
void TestScene::start()
{
	Scene::start();

	camera = birth<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

	camera->type = Camera::Z;

	drawManager.setting(camera);

	//obj = birth();

	//Actions attack;

	//attack.loop = true;

	//Actions move;
	//move.setEndCondition(ConditionArray());
	//attack += std::move(move.endIf(KeyA));
	//attack += MyPrint(U"attack!", 1);

	//obj->actman.create(U"a", std::move(attack), true);


	auto player = birthObject<Object>(Box(3, 5, 3), { 0,0,0 });
	obj = player;

	for (auto k : step(72))
	{
		en << birthObject<Object>(Box(1, 1, 1), { 0,0,0 });
		en.back()->transform->setParent(player->transform);
		
	}

	auto card= birth();
	card->addComponent<CardComponent>(U"カード裏.png", player, [] {return 0.4; });
	card->transform->setPos({ 100,100,0 });

}

void TestScene::update(double dt)
{
	Scene::update(dt);
	for (auto e : en)
		e->transform->getPos();
	obj->transform->setPos({ 1,1,1 });
}
