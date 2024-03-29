#include "../../stdafx.h"
#include "Player.h"
#include"../Utilities.h"
#include"../../Component/Transform.h"
#include"../../Prg/GameAction.h"
#include"../Scenes.h"
#include"../GameSystem/Setting.h"
#include"../Skill/Provider.h"

void Player::start()
{
	Character::start();

	//ACreate(U"FreeFall", true) += FreeFall(transform, { 0,0,0 }, -20).startIfNot<TouchGround>().endIf<TouchGround>();

	////attack
	//{
	//	auto& attack = ACreate(U"attack", true, true);
	//	auto s = skill::SkillProvider::Get(U"旅人/たたく");//スキル取得
	//	s->addInfo<skill::Chara>(U"chara", this);
	//	attack.add([=] { s->act(); }).startIf(KeyK);//スキル発動
	//}
	//jump
	{
		//ACreate(U"jump",true,true)
		//	+= FreeFall(transform, Vec3{ 0,15,0 }, -20).startIf([] {return setting::Jump.down(); }).endIf([] {return setting::Jump.down(); }, 1);
	}
}

void Player::update(double dt)
{
	Character::update(dt);
}

void Player::behaviorSetting(const state::Inform& info)
{
	state::SCreatorContainer dict;

	using namespace prg;
	using namespace state;
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
			act |= dict[U"Avoid"](info);
			//攻撃1,2,3を追加
			Array<String> attacks{};
			for (auto k : step(3)) {
				auto s= U"Attack{}"_fmt(k);
				attacks << s;
				act |= dict[s](info);
			}
			//走る　攻撃１　攻撃２　攻撃３　はそれぞれに移行可能
			act.relate(attacks, U"Run").andIf(setting::SubDown);
			act.relate(attacks.removed_at(0).append({ U"Run" }), attacks[0]).andIf(setting::SubLeft);
			act.relate(attacks.removed_at(1).append({ U"Run" }), attacks[1]).andIf(setting::SubUp);
			act.relate(attacks.removed_at(2).append({ U"Run" }), attacks[2]).andIf(setting::SubRight);

			return F(act);
		};
	dict[U"UnOperatable"] = [&](In info, A act)
		{
			//何もしない
			act += MyPrint(U"操作できない");

			return F(act);
		};
	//f3
	dict[U"Run"] = [&](In info, A act)
		{
			act |= Move4D(transform, 35,
					FuncCondition(setting::Up, KeyState::p),
					FuncCondition(setting::Down, KeyState::p),
					FuncCondition(setting::Left, KeyState::p),
					FuncCondition(setting::Right, KeyState::p));
			return F(act);
		};
	dict[U"Jump"] = [&](In info, A act)
		{
			act += FuncAction([&] {

				});

			return F(act);
		};
	dict[U"Avoid"] = [&](In info, A act)
		{
			act += MyPrint(U"よける");
			return F(act);

		};
	for (auto k : step(3)) dict[U"Attack{}"_fmt(k)] = [=](In info, A act)
		{
			auto gage = addComponentNamed<Field<util::StopMax>>(U"A{}Gage"_fmt(k), util::StopMax(100));

			Actions attack;

			Array<Input> keys{ setting::SubLeft, setting::SubUp, setting::SubRight };

			attack.loop = true;

			auto& move = attack.add<Actions>(U"かまえ");
			move += Move4D(transform, 15,
					FuncCondition(setting::Up, KeyState::p),
					FuncCondition(setting::Down, KeyState::p),
					FuncCondition(setting::Left, KeyState::p),
					FuncCondition(setting::Right, KeyState::p))
				+ FuncAction([=](double dt) {gage->value.add(40 * dt); Print << U"ため{}：{}"_fmt(k, gage->value.value); });
			move.setEndCondition();
			move.endIf([=] {return not gage->value.additionable(); }).andIf(keys[k], KeyState::d, 1);

			attack += FuncAction([=] {gage->value.value = 0; })
				+ MyPrint(U"attack!", 1);

			act += std::move(attack);

			return F(act);
		};

	ACreate(U"State", true) += dict[U"PlayerBehabior"](info);

}
