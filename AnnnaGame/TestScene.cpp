#include "stdafx.h"
#include "TestScene.h"
#include"Game/UI.h"
#include"Game/Utilities.h"
#include"Prg/Init.h"
#include"Motions/Motion.h"
#include"Component/CameraAffect/Convert2DScaleComponent.h"
#include"Component/CameraAffect/Convert2DTransformComponent.h"
#include"Motions/MotionCreator.h"
#include"Motions/MotionEditor.h"
#include"Game/Objects/Card.h"
#include"Game/Chara/Player.h"

Actions* ac;
Object* obj;
void TestScene::start()
{
	Scene::start();

	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

	camera->type = Camera::Z;

	drawManager.setting(camera);

	//obj = birthObjectNonHitbox();

	//Actions attack;

	//attack.loop = true;

	//Actions move;
	//move.setEndCondition(ConditionArray());
	//attack += std::move(move.endIf(KeyA));
	//attack += MyPrint(U"attack!", 1);

	//obj->actman.create(U"a", std::move(attack), true);

	auto player = birthObject<Object>(Box(3, 5, 3), { 0,0,0 });

	auto card= birthObjectNonHitbox();
	card->addComponent<CardComponent>(U"カード裏.png", player, [] {return 0.4; });
	card->transform->setPos({ 100,100,0 });
}

void TestScene::update(double dt)
{
	Scene::update(dt);
}
