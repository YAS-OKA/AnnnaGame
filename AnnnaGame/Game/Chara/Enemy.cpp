#include "../../stdafx.h"
#include "Enemy.h"
#include "../Scenes.h"
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

EnemyFactory::EnemyFactory(const Borrow<my::Scene>& scene)
	:m_scene(scene)
{
}

Borrow<Enemy> EnemyFactory::create(StringView name)
{
	if (m_creator.empty())throw Error{ U"クリエーターを構築してください。" };

	if (not(m_scene and m_creator.contains(name)))return Borrow<Enemy>();
	//敵を生成
	auto creature = m_scene->birthObject<Enemy>(Box{0,0,0}, {0,0,0});
	//敵を加工
	m_creator[name](creature);
	
	return creature;
}

HashTable<String, std::function<Borrow<Enemy>(Borrow<Enemy>)>> EnemyFactory::m_creator{};

/*プロトタイプ宣言*/
namespace //creatorなどが利用する関数とか
{

}

/*creatorの実装*/
void EnemyFactory::LoadCreator()
{
	/*テンプレート*/
	m_creator[U""] = [](Borrow<Enemy> enemy)
		{


			return enemy;
		};

	//骸骨蛇
	m_creator[U"Skulnake"] = [](Borrow<Enemy> enemy)
		{
			const auto& scene = enemy->scene;
			//敵の基本設定
			enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);

			enemy->getComponent<Draw3D>(U"hitbox")->visible = true;

			enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");
			//見た目の構築
			auto eparts = scene->partsLoader->create(U"asset/motion/snake/snake.json", false);

			eparts->master->transform->scale.setAspect({ 0.4,0.4,1 });

			enemy->addComponent<Convert2DTransformComponent>(eparts->transform, scene->getDrawManager(), ProjectionType::Parse);

			enemy->addComponent<Convert2DScaleComponent>(eparts->transform, scene->camera->distance(enemy->transform->getPos()), scene->getDrawManager());

			enemy->addComponent<PartsMirrored>(eparts);
			//座標をセット
			enemy->transform->setPos({ 8 - 10,3,4 + 10 });

			//モーションの構築
			//auto decoder = std::make_shared<CmdDecoder>();

			//DecoderSet(decoder.get()).motionScriptCmd(eparts, nullptr);

			//decoder->input(U"load asset/motion/sara/motion1.txt Stand")->decode()->execute();//モーションをセット
			//decoder->input(U"load asset/motion/sara/motion1.txt Jump")->decode()->execute();
			//decoder->input(U"load asset/motion/sara/motion1.txt Attack")->decode()->execute();
			//decoder->input(U"load asset/motion/sara/motion1.txt Run")->decode()->execute();
			//decoder->input(U"load asset/motion/sara/motion1.txt Knockback")->decode()->execute();

			//state::Inform info;

			//info.set(U"MotionCmdDecoder", state::Info(decoder));//デコーダーを渡す
			//info.set(U"StandMotionCmd", state::Info(U"start Stand"));
			//info.set(U"JumpMotionCmd", state::Info(U"start Jump"));
			//info.set(U"AttackMotionCmd", state::Info(U"start Attack"));
			//info.set(U"RunMotionCmd", state::Info(U"start Run true"));
			//info.set(U"KnockbackMotionCmd", state::Info(U"start Knockback"));
			//info.set(U"parts", state::Info(eparts));

			//player::SetPlayerAnimator(enemy, std::move(info));

			//スキルのセットとAIのセット
			/*info = state::Inform();
			auto s = skill::SkillProvider::Get(U"Tmp");
			s->addInfo<skill::Chara>(U"chara", enemy);
			s->addInfo<skill::InfoV<Vec3>>(U"dir", enemy->transform->getDirection());
			enemy->setSkill(*s, U"Attack");

			EnemyAIProvider::Set(U"Sample", enemy, std::move(info));*/
			enemy->name = U"Enemy";

			return enemy;
		};

}

/*Enemyの共通処理　start*/
void Enemy::start()
{
	Character::start();

}

/*実装*/
namespace //creatorなどが利用する関数とか
{

}
