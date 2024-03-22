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
#include"Tools/ObjScript.h"

Actions* ac;
Object* obj;
void TestScene::start()
{
	Scene::start();
	//カメラ
	//camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

	//camera->type = Camera::Z;

	//drawManager.setting(camera);

	//auto player = birthObjectNonHitbox();

	//player->transform->setY(2.5);
	//
	////カメラ
	//camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,10,-30 }));
	//drawManager.setting(camera);
	////プレイヤーをターゲット
	//camera->setFollowTarget(player);

	//auto parts = partsLoader->create(U"asset/motion/test/test.json");

	//player->addComponent<Convert2DTransformComponent>(parts->transform, getDrawManager(), ProjectionType::Parse);

	//player->addComponent<Convert2DScaleComponent>(parts->transform, camera->distance(player->transform->getPos(), Camera::DistanceType::XYZ), getDrawManager(), Camera::DistanceType::XYZ);

	//TextReader reader{ U"asset/motion/test/script/Test.txt" };

	//MotionScript().Load(parts, U"test", reader.readAll());

	//auto act=player->ACreate(U"Mawaru");

	//act->startIf([] {return KeyK.down(); });

	//act->add([=] {
	//	parts->find(U"body")->startAction(U"test");
	//	});

	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

	camera->type = Camera::Z;

	drawManager.setting(camera);

	obj = birthObjectNonHitbox();

	obj->ACreate(U"あいさつ").startIf([] {return KeyA.down(); })
		+= MyPrint(U"hello", 1)
		+ MyPrint(U"world", 1)
		+ MyPrint(U"c++").endIf([] {return KeyA.down(); }, 1)
		>> MyPrint(U"こんにちは", 1)
		+ MyPrint(U"世界", 1);
}

void TestScene::update(double dt)
{
	Scene::update(dt);
	
}
