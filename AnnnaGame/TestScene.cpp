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

	DataSaver a;
	Print << a.preProcess(TextReader{ U"DataSaverTest.txt" }.readAll());

	//obj = birthObjectNonHitbox();

	//obj->ACreate(U"あいさつ").startIf([] {return KeyA.down(); })
	//	+= MyPrint(U"hello", 1)
	//	+ MyPrint(U"world", 1)
	//	+ MyPrint(U"c++").endIf([] {return KeyA.down(); },1)
	//	>> MyPrint(U"こんにちは", 1)
	//	+ MyPrint(U"世界", 1);
}

void TestScene::update(double dt)
{
	Scene::update(dt);
	
}
