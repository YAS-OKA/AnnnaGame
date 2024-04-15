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
#include"../../Component/PartsCont/PartsMirrored.h"
#include"../StateMachine/Inform.h"
#include"../../Motions/MotionCmd.h"
#include"../../Util/CmdDecoder.h"

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

	//カメラ
	camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,12,-25 }));
	drawManager.setting(camera);
	//プレイヤーをターゲット
	camera->setFollowTarget(player);
	camera->type = Camera::DistanceType::Z;

	auto parts = std::shared_ptr<PartsManager>(partsLoader->create(U"asset/motion/sara/model1.json"));

	parts->master->transform->scale.setAspect({ 0.4,0.4,1 });

	this->player = player;
	
	player->addComponent<Convert2DTransformComponent>(parts->transform, getDrawManager(), ProjectionType::Parse);
	
	player->addComponent<Convert2DScaleComponent>(parts->transform, camera->distance(player->transform->getPos()), getDrawManager());

	player->addComponent<PartsMirrored>(parts.get());

	auto info = state::Inform();

	auto decoder = std::make_shared<CmdDecoder>();

	DecoderSet(decoder.get()).motionScriptCmd(parts.get());

	decoder->input(U"load asset/motion/sara/motion.txt Stand")->decode()->execute();//モーションをセット
	decoder->input(U"load asset/motion/sara/motion.txt Jump")->decode()->execute();
	decoder->input(U"load asset/motion/sara/motion.txt Attack")->decode()->execute();
	decoder->input(U"load asset/motion/sara/motion.txt Run")->decode()->execute();

	info.set(U"MotionCmdDecoder", state::Info(decoder));//デコーダーを渡す
	info.set(U"StandMotionCmd", state::Info(U"start Stand"));
	info.set(U"JumpMotionCmd", state::Info(U"start Jump"));
	info.set(U"AttackMotionCmd", state::Info(U"start Attack"));
	info.set(U"RunMotionCmd", state::Info(U"start Run true"));
	info.set(U"parts", state::Info(parts));

	player->setPlayerAnimator(std::move(info));

	player->behaviorSetting(state::Inform());

	auto enemy = birthObject<Character>(Box(3, 5, 3), {0,0,0});

	enemy->transform->setPos({ 8,3,0 });

	enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);

	enemy->getComponent<Draw3D>(U"hitbox")->visible = true;

	enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");

	auto eparts = partsLoader->create(U"asset/motion/circle.json");

	eparts->master->transform->scale.setAspect({ 0.6,0.6,1 });

	enemy->addComponent<Convert2DTransformComponent>(eparts->transform, getDrawManager(), ProjectionType::Parse);

	enemy->addComponent<Convert2DScaleComponent>(eparts->transform, camera->distance(enemy->transform->getPos()), getDrawManager());

	//カードUI
	auto ui = birthObjectNonHitbox<ui::Card>();

	player->name = U"Player";
	enemy->name = U"Enemy";
}

void GameScene::update(double dt)
{
	Scene::update(dt);
	Scene::updateTransform(dt);
}
