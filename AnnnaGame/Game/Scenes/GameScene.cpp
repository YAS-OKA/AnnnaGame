﻿#include "../../stdafx.h"
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
#include"../Objects/Card.h"
#include"../Computer/EnemyAI.h"
#include"../Chara/Enemy.h"

GameScene::~GameScene()
{
	skill::SkillProvider::Destroy();

	EnemyAIProvider::Destroy();
}

void GameScene::start()
{
	Scene::start();
	skill::SkillProvider::Init(*this);
	EnemyAIProvider::Init();
	
	auto s = Size{ 700,700 };
	const MSRenderTexture uvChecker{ s,ColorF{0.12,0.27,0.12}.removeSRGBCurve() };
	auto ground = birthObject<Object>(Box{ 500,2,500 }, { 0,0,0 });
	ground->getComponent<Collider>(U"hitbox")->setCategory(ColliderCategory::object);
	auto ground_texture = ground->addComponent<Draw3D>(&drawManager, MeshData::OneSidedPlane({ 500,500 },{8,8}));
	//地面の模様作成
	{
		// レンダーターゲットを renderTexture に変更する
		const ScopedRenderTarget2D target{ uvChecker };
		const ScopedRenderStates2D blend{ BlendState::AdditiveRGB };

		for (int32 i = 0; i < 200; ++i)
		{
			auto r = Random<int32>(1, 3);
			Vec2 pos = RandomVec2(Rect{ r, r, s.x - r, s.y - r });
			Circle{ pos,r }.draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
		}

		for (int32 i = 0; i < 300; ++i)
		{
			auto wh = Random<int32>(4, 8);
			Vec2 pos = RandomVec2(Rect{ 6, 6, s.x - 6, s.y - 6 });
			double theta = Random<double>(0, 90);
			RectF{ pos,wh }.rotated(2 * theta * Math::Pi).draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
		}

		for (int32 i = 0; i < 400; ++i)
		{
			auto r = Random<double>(4, 10);
			Vec2 pos = RandomVec2(Rect{ 6, 6, s.x - 6, s.y - 6 });
			double theta = Random<double>(0, 120);
			Triangle{ pos,r }.rotated(2 * theta * Math::Pi).draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
		}

		// 2D 描画をフラッシュ
		Graphics2D::Flush();

		// マルチサンプル・テクスチャをリゾルブ
		uvChecker.resolve();
	}
	ground_texture->tex = uvChecker;
	//プレイヤー生成
	auto player = birthObject<Player>(Box(3, 5, 3),{0,0,0});
	//*2はデバッグ用　
	player->transform->setY(2.5);

	player->getComponent<Collider>()->setCategory(ColliderCategory::hero);

	auto h = player->getComponent<Draw3D>(U"hitbox");

	h->color = ColorF{ Palette::Skyblue,0.7 }.removeSRGBCurve();

	h->visible = true;

	player->param.LoadFile(U"Player.txt", U"Annna");

	//カメラ
	camera = birth<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,12,-25 }));
	drawManager.setting(camera);
	//プレイヤーをターゲット
	camera->setFollowTarget(player);
	camera->type = Camera::DistanceType::Z;

	player->ACreate(U"cons", true).add([p = player](double) {Print << p->transform->getPos(); });

	auto parts = partsLoader->create(U"asset/motion/sara/model.json", false);

	parts->master->transform->scale.setAspect({ 0.4,0.4,1 });

	this->player = player;
	
	player->addComponent<Convert2DTransformComponent>(parts->transform, getDrawManager(), ProjectionType::Parse);
	
	player->addComponent<Convert2DScaleComponent>(parts->transform, camera->distance(player->transform->getPos()), getDrawManager());

	player->addComponent<PartsMirrored>(parts);

	auto info = state::Inform();

	auto decoder = std::make_shared<CmdDecoder>();

	DecoderSet(decoder.get()).motionScriptCmd(parts,nullptr);

	decoder->input(U"load asset/motion/sara/motion1.txt Stand")->decode()->execute();//モーションをセット
	decoder->input(U"load asset/motion/sara/motion1.txt Jump")->decode()->execute();
	decoder->input(U"load asset/motion/sara/motion1.txt Attack")->decode()->execute();
	decoder->input(U"load asset/motion/sara/motion1.txt Run")->decode()->execute();
	decoder->input(U"load asset/motion/sara/motion1.txt Knockback")->decode()->execute();

	info.set(U"MotionCmdDecoder", state::Info(decoder));//デコーダーを渡す
	info.set(U"StandMotionCmd", state::Info(U"start Stand"));
	info.set(U"JumpMotionCmd", state::Info(U"start Jump"));
	info.set(U"AttackMotionCmd", state::Info(U"start Attack"));
	info.set(U"RunMotionCmd", state::Info(U"start Run true"));
	info.set(U"KnockbackMotionCmd", state::Info(U"start Knockback"));

	info.set(U"parts", state::Info(parts));

	player::SetPlayerAnimator(player, std::move(info));

	info = state::Inform();

	player->setAttackSkill(0, U"旅人/たたく");

	player->behaviorSetting(std::move(info));

	/*mot::Parts* hair = nullptr;
	for (const auto& p : parts->partsArray)
	{
		if (p->name == U"kami")hair = p;
	}

	double g = 90;
	double haba = 50;
	
	player->ACreate(U"HairSwing", true)
		+= FuncAction(
		[=, v = 0.0, mirror = player->getComponent<PartsMirrored>()](double dt)mutable {
			const auto& ang = hair->getAngle();
			const auto& sokudo = player->transform->getVel().xy();
			double kansei = 100 * sokudo.x * (haba - Min(haba, Abs(ang))) / haba;

			if (mirror->getMirrored())kansei *= -1;
			
			v += (-ang * g + kansei) * dt;

			hair->setAngle(ang + Math::Sign(v) * Log(Abs(v)+1) * dt);
		}
	);*/

	EnemyFactory::LoadCreator();

	EnemyFactory fact(*this);

	fact.create(U"Skulnake");

	//for (auto k : step(0)) {
	//	auto enemy = birthObject<Enemy>(Box(3, 5, 3), { 0,0,0 });

	//	enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);

	//	enemy->getComponent<Draw3D>(U"hitbox")->visible = true;

	//	enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");

	//	auto eparts = partsLoader->create(U"asset/motion/sara/model.json", false);

	//	eparts->master->transform->scale.setAspect({ 0.4,0.4,1 });

	//	enemy->addComponent<Convert2DTransformComponent>(eparts->transform, getDrawManager(), ProjectionType::Parse);

	//	enemy->addComponent<Convert2DScaleComponent>(eparts->transform, camera->distance(enemy->transform->getPos()), getDrawManager());

	//	enemy->addComponent<PartsMirrored>(eparts);

	//	enemy->transform->setPos({ 8 * (k%10) - 10,3,4*(k/10)+10 });

	//	decoder = std::make_shared<CmdDecoder>();

	//	DecoderSet(decoder.get()).motionScriptCmd(eparts,nullptr);

	//	decoder->input(U"load asset/motion/sara/motion1.txt Stand")->decode()->execute();//モーションをセット
	//	decoder->input(U"load asset/motion/sara/motion1.txt Jump")->decode()->execute();
	//	decoder->input(U"load asset/motion/sara/motion1.txt Attack")->decode()->execute();
	//	decoder->input(U"load asset/motion/sara/motion1.txt Run")->decode()->execute();
	//	decoder->input(U"load asset/motion/sara/motion1.txt Knockback")->decode()->execute();

	//	info = state::Inform();

	//	info.set(U"MotionCmdDecoder", state::Info(decoder));//デコーダーを渡す
	//	info.set(U"StandMotionCmd", state::Info(U"start Stand"));
	//	info.set(U"JumpMotionCmd", state::Info(U"start Jump"));
	//	info.set(U"AttackMotionCmd", state::Info(U"start Attack"));
	//	info.set(U"RunMotionCmd", state::Info(U"start Run true"));
	//	info.set(U"KnockbackMotionCmd", state::Info(U"start Knockback"));
	//	info.set(U"parts", state::Info(eparts));

	//	player::SetPlayerAnimator(enemy, std::move(info));

	//	info = state::Inform();

	//	auto s = skill::SkillProvider::Get(U"Tmp");
	//	s->addInfo<skill::Chara>(U"chara", enemy);
	//	s->addInfo<skill::InfoV<Vec3>>(U"dir", enemy->transform->getDirection());
	//	enemy->setSkill(*s, U"Attack");

	//	EnemyAIProvider::Set(U"Sample", enemy, std::move(info));
	//	enemy->name = U"Enemy";
	//}
	//カード
	auto card = birth();
	card->addComponent<CardComponent>(U"カード裏.png", player,
		[gage = player->getComponent<Field<util::StopMax>>(U"A0Gage")] {
			return gage->value.value / gage->value.max;
		});
	card->transform->setPos({ 1000,30,0 });

	player->name = U"Player";
}

void GameScene::update(double dt)
{
	Scene::update(dt);
	Scene::updateTransform(dt);
}
