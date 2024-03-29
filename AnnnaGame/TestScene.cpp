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
#include"Tools/StateScript.h"

Actions* ac;
Object* obj;
void TestScene::start()
{
	Scene::start();

	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

	camera->type = Camera::Z;

	drawManager.setting(camera);

	obj = birthObjectNonHitbox();

	Actions attack;

	attack.loop = true;

	Actions move;
	move.setEndCondition(ConditionArray());
	attack += std::move(move.endIf(KeyA));
	attack += MyPrint(U"attack!", 1);

	obj->actman.create(U"a", std::move(attack), true);
}

void TestScene::update(double dt)
{
	Scene::update(dt);
	
}
