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
#include"../StateMachine/Inform.h"

void GameScene::start()
{
	Scene::start();
	skill::SkillProvider::Init(this);
	TextureAsset::Register(U"uv", U"example/texture/uv.png");

	auto ground = birthObject<Object>(Box{ 500,0.5,500 }, { 0,0,0 });
	ground->getComponent<Collider>(U"hitbox")->setCategory(ColliderCategory::object);
	auto ground_texture = ground->addComponent<Draw3D>(&drawManager, MeshData::OneSidedPlane({ 500,500 }));

	ground_texture->setAssetName(U"uv");
	//プレイヤー生成
	auto player = birthObject<Player>(Box(3, 5, 3),{0,0,0});
	//*2はデバッグ用　
	player->transform->setY(2.5 * 2);

	auto h = player->getComponent<Draw3D>(U"hitbox");

	h->color = ColorF{ Palette::Skyblue,0.7 }.removeSRGBCurve();

	h->visible = true;

	player->param.LoadFile(U"Player.txt", U"Annna");

	player->behaviorSetting(state::Inform());

	//カメラ
	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,12,-25 }));
	drawManager.setting(camera);
	//プレイヤーをターゲット
	camera->setFollowTarget(player);
	camera->type = Camera::DistanceType::Z;

	auto parts = partsLoader->create(U"asset/motion/sara/model2.json");

	parts->master->transform->scale.setAspect({ 0.5,0.5,1 });

	this->player = player;
	
	player->addComponent<Convert2DTransformComponent>(parts->transform, getDrawManager(), ProjectionType::Parse);
	
	player->addComponent<Convert2DScaleComponent>(parts->transform, camera->distance(player->transform->getPos()), getDrawManager());

	auto enemy = birthObject<Character>(Box(3, 5, 3), {0,0,0});

	enemy->transform->setPos({ 8,3,0 });

	enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);

	enemy->getComponent<Draw3D>(U"hitbox")->visible = true;

	enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");

	auto eparts = partsLoader->create(U"asset/motion/circle.json");

	eparts->master->transform->scale.setAspect({ 0.6,0.6,1 });

	enemy->addComponent<Convert2DTransformComponent>(eparts->transform, getDrawManager(), ProjectionType::Parse);

	enemy->addComponent<Convert2DScaleComponent>(eparts->transform, camera->distance(enemy->transform->getPos()), getDrawManager());

	//drawManager.debugDraw = [=] {
	//	auto p = parts->find(U"ubody");
	//	auto e = eparts->find(U"a");
	//	Print << p->transform->getPos();
	//	Print << e->transform->getPos();
	//	};

	//カードUI
	auto ui = birthObjectNonHitbox<ui::Card>();

	player->name = U"Player";
	enemy->name = U"Enemy";
}

void GameScene::update(double dt)
{
	Scene::update(dt);
}
