#include "../../stdafx.h"
#include "EnemyAI.h"
#include"../Chara/Enemy.h"
#include "../Chara/Player.h"
#include"../Utilities.h"
#include"../../Component/Transform.h"
#include"../../Prg/GameAction.h"
#include"../Scenes.h"
#include"../GameSystem/Setting.h"
#include"../Skill/Provider.h"
#include"../../Motions/MotionCmd.h"
#include"../../Util/CmdDecoder.h"
#include"../../Component/PartsCont/PartsMirrored.h"
#include"../../Motions/Parts.h"

void setSimple(const Borrow<Enemy>&, state::Inform&&);

EnemyAIProvider* EnemyAIProvider::instance = nullptr;

EnemyAIProvider::EnemyAIProvider()
{
	aiList[U"Sample"]=setSimple;
}

void EnemyAIProvider::Init()
{
	if (instance)return;//すでにインスタンスが生成されていたらここで終了
	instance = new EnemyAIProvider();
}

void EnemyAIProvider::Destroy()
{
	if (instance)delete instance;
	instance = nullptr;
}

void EnemyAIProvider::Set(StringView name, const Borrow<Enemy>& e, state::Inform&& info)
{
	if (instance)
	{
		if (not instance->aiList.contains(name))throw Error{ U"存在しないAIが指定されました。\nAI名:" + name };
		instance->aiList[name](e, std::forward<state::Inform>(info));
	}
}
//サンプル状態遷移
void setSimple(const Borrow<Enemy>& e, state::Inform&& info)
{
	using namespace prg;
	using namespace state;
	using namespace setting;

	auto target = e->addComponentNamed<Field<Borrow<Player>>>(U"Target");

	auto param = e->getComponent<Field<HashTable<String, Info>>>(U"StateMachineParam");
	if (not param)param = e->addComponentNamed<Field<HashTable<String, Info>>>(U"StateMachineParam");
	param->value[U"attack"] = false;
	param->value[U"attackTimer"] = 1.0;
	param->value[U"unOperatable"] = false;

	auto motionState = e->getComponent<Field<HashTable<String, bool>>>(U"AnimatorParam");

	SCreatorContainer dict;

	//f1
	dict[U"SampleAI"] = [&](In info, A act)
		{
			act |= dict[U"Operatable"](info);
			act |= dict[U"UnOperatable"](info);

			act.relate(U"Operatable", U"UnOperatable")
				.andActiveIf([=] {return param->value[U"unOperatable"].getValue<bool>(); });

			return F(act);
		};
	//f2
	dict[U"Operatable"] = [&](In info, A act)
		{
			act |= dict[U"Stand"](info);
			act |= dict[U"Attack"](info);
			act |= dict[U"Walk"](info);
			
			act.relate( U"Stand", U"Walk").andActiveIf([=] {return target->value; });
			act.relate(U"Walk", U"Attack").andActiveIf([=]
				{
				return param->value[U"attack"].getValue<bool>() and param->value[U"attackTimer"].getValue<double>() > 1.0;
				});

			return F(act);
		};
	dict[U"UnOperatable"] = [&](In info, A act)
		{
			//今は何もしない

			return F(act);
		};
	//f3
	dict[U"Stand"] = [&](In info, A act)
		{
			act |= MyPrint(U"EnemyStanding")
				+ FuncAction([=, players = e->scene->find<Player>()](double dt) {
						param->value[U"attackTimer"] = param->value[U"attackTimer"].getValue<double>() + dt;
						
						for (const auto& player : players)
						{
							//近くにプレイヤーがいたらターゲットに(半径はinfoで設定できるようにする)
							if ((e->transform->getPos() - player->transform->getPos()).lengthSq() < 2500)
							{
								target->value = player;
							}
						}
					});

			return F(act);
		};
	dict[U"Walk"] = [&](In info, A act)
		{
			auto up = std::make_shared<bool>(false);
			auto down = std::make_shared<bool>(false);
			auto left = std::make_shared<bool>(false);
			auto right = std::make_shared<bool>(false);

			act |= use::Move4D(e->transform, 6,
					FuncCondition([=] {return *up; }),
					FuncCondition([=] {return *down; }),
					FuncCondition([=] {return *left; }),
					FuncCondition([=] {return *right; }))
				+ FuncAction(
				[=](double dt) {
					param->value[U"attackTimer"] = param->value[U"attackTimer"].getValue<double>() + dt;

					const auto& dpos = target->value->transform->getPos() - e->transform->getPos();

					motionState->value[U"run"] = e->transform->getVel().xz().lengthSq() > 1;

					*up = false;
					*down = false;
					*left = false;
					*right = false;

					//ターゲットが殺された or ターゲットが認識範囲外に出たら　null
					if (target->value->owner->isKilled(target->value) or dpos.lengthSq() > 2500)
					{
						target->value = nullptr;
					}
					else 
					{
						//十分近づいたらあたっく
						if ((target->value->transform->getPos() - e->transform->getPos()).lengthSq() < 5) {
							param->value[U"attack"] = true;
						}
						else
						{
							*up = dpos.z >= 1;
							*down = dpos.z <= -1;
							*left = dpos.x <= -1;
							*right = dpos.x >= 1;
						}
					}
				},
				[=]{ motionState->value[U"run"] = false;}
			);

			return F(act);
		};
	dict[U"Attack"] = [&](In info, A act)
		{
			auto& tmp = info.get(U"AttackSkillName");
			Optional<String> attackSkillName = none;
			if (tmp.valid)attackSkillName = tmp.getValue<String>();

			act |= FuncAction([=, skills = e->getSkills()] {
				motionState->value[U"attack"] = true;
				if (skills->value.contains(U"Attack")) {
					auto& s = skills->value[U"Attack"];
					auto dir = s->getInfo<skill::InfoV<Vec3>>(U"dir");
					if (not dir)s->addInfo<skill::InfoV<Vec3>>(U"dir", e->transform->getDirection());
					else *dir = e->transform->getDirection();
					e->startAction(U"Attack");
				}
			},
			[=] {
				param->value[U"attack"] = false;
				param->value[U"attackTimer"] = 0.0;
				motionState->value[U"attack"] = false;
			}, 1);

			return F(act);
		};

	auto freeFall = FreeFall(e->transform, { 0,0,0 }, -27)
		.startIfNot<Touch>(e->getComponent<Collider>(U"bottom"), ColliderCategory::object)
		.endIf<Touch>(e->getComponent<Collider>(U"bottom"), ColliderCategory::object)
		.andIf([=] {return e->transform->pos.delta.y < 0; }, 1);

	//自由落下
	e->ACreate(U"FreeFall", true, true) += std::move(freeFall);

	e->ACreate(U"State", true) += dict[U"SampleAI"](info);
}
