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
			//当たり判定
			enemy->getComponent<Collider>()->setCategory(ColliderCategory::enemy);
			enemy->getComponent<Draw3D>(U"hitbox")->visible = true;
			//パラメータ読み込み
			enemy->param.LoadFile(U"enemys.txt", U"KirikabuBake");
			//見た目の構築
			auto parts = scene->partsLoader->create(U"asset/motion/snake/snake.json", false);
			//見た目の大きさ設定
			parts->master->transform->scale.setAspect({ 0.3,0.3,1 });
			//パースとミラー
			enemy->addComponent<Convert2DTransformComponent>(parts->transform, scene->getDrawManager(), ProjectionType::Parse);
			enemy->addComponent<Convert2DScaleComponent>(parts->transform, scene->camera->distance(enemy->transform->getPos()), scene->getDrawManager());
			enemy->addComponent<PartsMirrored>(parts);
			//座標をセット
			enemy->transform->setPos({ 8 - 10,3,4 + 10 });
			//モーションのステートマシンを構築
			CharaUtil::SetAnimator(enemy, parts, U"asset/motion/snake/motion.txt", U"Stand",
				{
				{{U"Stand"},U"Walk",true},
				{{U"Stand",U"Walk"},U"Attack"}
				}
			);
			//スキルのセットとAIのセット
			auto info = state::Inform();
			auto s = skill::SkillProvider::Get(U"Tmp");
			s->addInfo<skill::Chara>(U"chara", enemy);
			s->addInfo<skill::InfoV<Vec3>>(U"dir", enemy->transform->getDirection());
			enemy->setSkill(*s, U"Attack");

			EnemyAIProvider::Set(U"Skulnake", enemy, std::move(info));
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
