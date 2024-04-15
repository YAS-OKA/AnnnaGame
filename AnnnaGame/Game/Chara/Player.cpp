#include "../../stdafx.h"
#include "Player.h"
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

namespace uni{
	template<class T1, class T2, class T3, class T4>
	ActCluster Move4D(Transform* transform, double speed, const T1& upCondition, const T2& downCondition, const T3& leftCondition, const T4& rightCondition)
	{		
		return prg::Move4D(transform,
					speed,
					upCondition,
					downCondition,
					leftCondition,
					rightCondition).startIf([] {return true; })
			* Look4D(transform,
					upCondition,
					downCondition,
					leftCondition,
					rightCondition).startIf([] {return true; });
	}
}

void Player::start()
{
	Character::start();

	//ACreate(U"Constant", true) += ActCluster()
	//	* MyPrint(U"下があたった").activeIf<Touch>(getComponent<Collider>(U"bottom"), ColliderCategory::object)
	//	* MyPrint(U"上があたった").activeIf<Touch>(getComponent<Collider>(U"top"), ColliderCategory::object)
	//	* FuncAction([=](double dt) {transform->addY(2 * dt); }).activeIf(KeyZ, KeyState::p)
	//	* FuncAction([=](double dt) {transform->addY(-2 * dt); }).activeIf(KeyX, KeyState::p);
	
	//attack
	{
		auto& attack = ACreate(U"attack", true, true);
		auto s = skill::SkillProvider::Get(U"旅人/たたく");//スキル取得
		s->addInfo<skill::Chara>(U"chara", this);
		attack.add([=] { s->act(); }).startIf(KeyK);//スキル発動
	}
}

void Player::update(double dt)
{
	Character::update(dt);
	/*auto p = getComponent<PartsMirrored>();

	Print << p->pman->find(U"rarm")->getPos();*/
}

void Player::behaviorSetting(state::Inform&& info)
{
	using namespace prg;
	using namespace state;
	using namespace setting;

	SCreatorContainer dict;

	auto freeFall = FreeFall(transform, { 0,0,0 }, -27)
		.startIfNot<Touch>(getComponent<Collider>(U"bottom"), ColliderCategory::object)
		.endIf<Touch>(getComponent<Collider>(U"bottom"), ColliderCategory::object)
		.andIf([&] {return transform->pos.delta.y < 0; },1);

	auto ff = freeFall.lend();

	auto param = getComponent<Field<HashTable<String, Info>>>(U"PlayerAnimatorParam");// ->value[U"jump"];

	addComponentNamed<Field<size_t>>(U"Select", 3);
	//自由落下
	ACreate(U"FreeFall", true, true) += std::move(freeFall);
	//かまえ０～２　Run　のいずれかを選択
	ACreate(U"Select", true, true) += FuncAction(
			[type = getComponent<Field<size_t>>(U"Select")](double)
				{
					if (SubLeft.down())type->value = 0;
					if (SubUp.down())type->value = 1;
					if (SubRight.down())type->value = 2;
					if (SubDown.down())type->value = 3;
					Print << U"選択:{}"_fmt(type->value);
				});
	//f1
	dict[U"PlayerBehabior"] = [&](In info, A act)
		{
			act |= dict[U"Operatable"](info);
			act |= dict[U"UnOperatable"](info);

			return F(act);
		};
	//f2
	dict[U"Operatable"] = [&](In info, A act)
		{
			act |= dict[U"Run"](info);
			act |= dict[U"Jump"](info);
			act |= dict[U"Air"](info);
			act |= dict[U"Avoid"](info);
			for (auto k : step(3)) {
				act |= dict[U"SetUp{}"_fmt(k)](info);
				act |= dict[U"Attack{}"_fmt(k)](info);
			}

			auto allState = act.getAllState();

			//Jumpの条件 Run,SetUp0～2から遷移可能
			auto excep = HashSet<String>{ U"Avoid",U"Air" };//これらステートはのぞく
			for (auto k : step(3))excep.emplace(U"Attack{}"_fmt(k));
			act.relate(allState.removed_if([&](String id) {return excep.contains(id); }), U"Jump")
				.andIf(Jump)
				.andIf<Touch>(getComponent<Collider>(U"bottom"), ColliderCategory::object);

			//Airの条件 すべてから遷移可能
			act.relate(allState.removed(U"Air") , U"Air")
				.andIf(ff, ActState::active);//自由落下がアクティブなら

			//Avoidの条件　Jumpを押した後nフレーム内に敵の攻撃があたったら

			//Runの条件
			auto select = getComponent<Field<size_t>>(U"Select");
			excep.emplace(U"Jump");//Avoid,Air,Jump,Attacks1~2の時はのぞく
			act.relate(allState.removed(U"Run").remove_if([&](String id) { return excep.contains(id); }), U"Run")
				.andIf([=] {return select->value == 3; });

			//SetUp0～2の条件
			for (auto k : step(3))
			{
				act.relate(allState.removed(U"SetUp{}"_fmt(k)).remove_if([&](String id) {return excep.contains(id); }), U"SetUp{}"_fmt(k))
					.andIf([=] {return select->value == k; });
			}

			//Attack0～2の条件
			Array<Input> keys{ SubLeft,SubUp,SubRight };
			for (auto k : step(3))
			{
				act.relate(U"SetUp{}"_fmt(k), U"Attack{}"_fmt(k))
					.andIf([gage = getComponent<Field<util::StopMax>>(U"A{}Gage"_fmt(k)),k] {
						Print << U"{}:{}"_fmt(k, gage->value.value);
						return not gage->value.additionable(); })
					.andIf(keys[k], KeyState::d);
			}

			return F(act);
		};
	dict[U"UnOperatable"] = [&](In info, A act)
		{
			//何もしない
			act += MyPrint(U"操作できない");

			return F(act);
		};
	//f3
	dict[U"Jump"] = [&](In info, A act)
		{
			
			Actions jump;
			jump |= FuncAction([=] {
					ff->initVel = { 0,21,0 };
					ff->startCondition.forced();
					param->value[U"jump"] = true;//モーションを開始
					}, [=] {
					ff->initVel = { 0,0,0 };
					}, none);

			act |= std::move(jump);

			act.endIf([borrow = act.lend()] {return borrow->isAllFinished(); });

			return F(act);
		};
	dict[U"Air"] = [&](In info, A act)
		{
			act |= uni::Move4D(transform, 13,
					FuncCondition(setting::Up, KeyState::p),
					FuncCondition(setting::Down, KeyState::p),
					FuncCondition(setting::Left, KeyState::p),
					FuncCondition(setting::Right, KeyState::p)
			)
				+ FuncAction([] {}, [=] {
				param->value[U"jump"] = false;
					}, none);//モーションを終了

			act.endIfNot(ff, ActState::start,1);//自由落下がスタートしていない状態だったら終わる

			return F(act);
		};
	dict[U"Run"] = [&](In info, A act)
		{
			act |= uni::Move4D(transform, 31,
					FuncCondition(setting::Up, KeyState::p),
					FuncCondition(setting::Down, KeyState::p),
					FuncCondition(setting::Left, KeyState::p),
					FuncCondition(setting::Right, KeyState::p))
				+ FuncAction(
					[=](double)
					{
						if (transform->getVel().xz().lengthSq() > 1)
						{
							param->value[U"run"] = true;
						}
						else
						{
							if (param->value[U"run"].getValue<bool>())
							{
								Print << U"";
							}
							param->value[U"run"] = false;
						}
					},[=]
					{
						param->value[U"run"] = false;
					})
				+ MyPrint(U"Run");

			return F(act);
		};
	dict[U"Avoid"] = [&](In info, A act)
		{
			act += MyPrint(U"よける",0.5);
			act.endIf([borrow = act.lend()] {return borrow->isAllFinished(); });
			return F(act);
		};
	for (auto k : step(3)) dict[U"SetUp{}"_fmt(k)] = [=](In info, A act)
		{
			auto& constant = act.add<Actions>();
			constant += uni::Move4D(transform, 9,
					FuncCondition(setting::Up, KeyState::p),
					FuncCondition(setting::Down, KeyState::p),
					FuncCondition(setting::Left, KeyState::p),
					FuncCondition(setting::Right, KeyState::p))
				+ FuncAction(
					[k,gage = addComponentNamed<Field<util::StopMax>>(U"A{}Gage"_fmt(k), util::StopMax(100))](double dt) {
						gage->value.add(40 * dt);
						Print << U"ため{}:{}"_fmt(k, gage->value.value);
					});
			constant.setEndCondition();

			return F(act);
		};
	for (auto k : step(3)) dict[U"Attack{}"_fmt(k)] = [=](In info, A act)
		{
			act |= FuncAction(
				[=,gage=getComponent<Field<util::StopMax>>(U"A{}Gage"_fmt(k))] {
					gage->value.value = 0;
					param->value[U"attack"] = true;
				},[=] {
					param->value[U"attack"] = false;
				}, 1)
				+ MyPrint(U"attack!", 1);

			act.endIf([borrow = act.lend()] {return borrow->isAllFinished(); });

			return F(act);
		};

	ACreate(U"State", true) += dict[U"PlayerBehabior"](info);
}

void Player::setPlayerAnimator(state::Inform&& info)
{
	using namespace prg;
	using namespace state;
	using namespace setting;

	auto param = addComponentNamed<Field<HashTable<String, Info>>>(U"PlayerAnimatorParam");
	param->value[U"run"] = false;
	param->value[U"jump"] = false;
	param->value[U"attack"] = false;

	SCreatorContainer dict;

	dict[U"PlayerAnimator"] = [&](In info, A act)->A
		{
			act |= dict[U"Stand"](info);//default
			act |= dict[U"Run"](info);
			act |= dict[U"Jump"](info);
			act |= dict[U"Attack"](info);

			act.relate({ U"Stand",U"Jump",U"Attack" }, U"Run")
				.activeIf([=] {return param->value[U"run"].getValue<bool>(); });
				
			act.relate({ U"Stand",U"Run" }, U"Jump")
				.activeIf([=] {return param->value[U"jump"].getValue<bool>(); });

			act.relate({ U"Stand",U"Run" }, U"Attack")
				.activeIf([=] {return param->value[U"attack"].getValue<bool>(); });

			return F(act);
		};
	//モーションのセット
	for (const auto& s : Array<String>{ U"Stand",U"Run",U"Jump",U"Attack"})
	{
		dict[s] = [=](In info, A act)->A
			{
				//モーションがあるならセット
				if (info.contains(U"MotionCmdDecoder") and info.contains(s + U"MotionCmd"))
				{
					act |= FuncAction(
						[decoder = info.get(U"MotionCmdDecoder").getValue<util::sPtr<CmdDecoder>>()
						, cmd = info.get(s + U"MotionCmd").getValue<String>()]
						{
							decoder->input(cmd)->decode()->execute();
						},
						[pman = info.get(U"parts").getValue<util::sPtr<mot::PartsManager>>()
						, motionName = info.get(s + U"MotionCmd").getValue<String>().split(' ')[1]] {
							for (const auto& p : pman->partsArray)
							{
								if (p->actman[motionName].isActive())p->actman[motionName].end();
							}
						}, none);
				}

				return F(act);
			};
	}

	ACreate(U"Animator", true) += dict[U"PlayerAnimator"](info);
}
