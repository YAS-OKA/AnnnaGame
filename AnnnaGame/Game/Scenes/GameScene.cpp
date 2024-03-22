#include "../../stdafx.h"
#include "GameScene.h"
#include"../../Component/Transform.h"
#include"../../Component/Draw.h"
#include"../Camera.h"
#include"../../Prg/Init.h"
#include"../Chara/Player.h"
#include"../UI.h"
#include"../../Component/CameraAffect/Convert2DTransformComponent.h"
#include"../../Component/CameraAffect/Convert2DScaleComponent.h"
#include"../Skill/Provider.h"

void GameScene::start()
{
	Scene::start();
	skill::SkillProvider::Init(this);
	TextureAsset::Register(U"uv", U"example/texture/uv.png");

	auto ground = birthObject<Object>(Box{ 500,1,500 }, { 0,0,0 });
	auto ground_texture = ground->addComponent<Draw3D>(&drawManager, MeshData::OneSidedPlane({ 500,500 }));

	ground_texture->setAssetName(U"uv");
	//プレイヤー生成
	auto player = birthObject<Player>(Box(3, 5, 3),{0,0,0});

	player->transform->setY(2.5);

	auto h = player->getComponent<Draw3D>(U"hitbox");

	h->color = ColorF{ Palette::Skyblue,0.7 }.removeSRGBCurve();

	h->visible = true;

	player->param.LoadFile(U"Player.txt", U"Annna");

	//カメラ
	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,10,-30 }));
	drawManager.setting(camera);
	//プレイヤーをターゲット
	camera->setFollowTarget(player);

	auto parts = partsLoader->create(U"asset/motion/test/test.json");

	this->player = player;
	
	player->addComponent<Convert2DTransformComponent>(parts->transform, getDrawManager(), ProjectionType::Parse);

	player->addComponent<Convert2DScaleComponent>(parts->transform, camera->distance(player->transform->getPos(), Camera::DistanceType::XYZ), getDrawManager(), Camera::DistanceType::XYZ);

	auto enemy = birthObject<Character>(Box(3, 5, 3), { 8,3,0 });

	enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);

	enemy->getComponent<Draw3D>(U"hitbox")->visible = true;

	enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");

	//カードUI
	auto ui = birthObjectNonHitbox<ui::Card>();

	player->name = U"Player";
	enemy->name = U"Enemy";
}

void GameScene::update(double dt)
{
	Scene::update(dt);
}
