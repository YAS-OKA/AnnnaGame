﻿#include"../../stdafx.h"
#include"../Scenes.h"
#include"Skills.h"
#include"../../Prg/GameAction.h"
#include"SkillsActor.h"

namespace skill
{
	template<class Effect, class T>
	T CalFormula(const Formula<Effect, T>& formula, Effect* e)
	{
		if (formula.index() == 0)return std::get<0>(formula)(e);
		else return std::get<1>(formula)();
	}
}

//skillEffect
using namespace skill;

//キャラのパラメータをいじるスキル効果のインターフェース
ParamMod::ParamMod(Skill* s, double time)
	:ISkillEffect(s, time)
{
};

void ParamMod::start()
{
	ISkillEffect::start();
	if (_CTarget) {
		params = &_CTarget->param;
	}
	else if (_target) {
		if (auto target = dynamic_cast<Character*>(_target))
			params = &target->param;
	}
};

//ダメージ
Damage::Damage(Skill* s, Formula<Damage> damage)
	:ParamMod(s, 0), damage(damage) {}

void Damage::start()
{
	ParamMod::start();
	if (params)params->hp -= CalFormula(damage, this);
}

//相手にノックバックの速度を与える
void Knockback::start()
{
	ISkillEffect::start();
}

//継続ダメージ
DotDamage::DotDamage(Skill* s, double span, double time, Formula<DotDamage> damage)
	: ParamMod(s, time), timeSpan(span), damage(damage)
{
}
//何回ダメージを与えたか
int32 DotDamage::getCount()
{
	//効果の長さが0のとき
	if (time == 0)return timeSpan == 0 ? 1 : 0;

	if (timeSpan == 0)throw Error{ U"0秒間隔でダメージを与えてます。DotDamageの誤った使い方" };

	return int32(timer / timeSpan);
}

void DotDamage::start()
{
	ParamMod::start();

	m_count = 0;
}

void DotDamage::update(double dt)
{
	ParamMod::update(dt);

	const auto& tmp = getCount();
	while (tmp > m_count)
	{
		//ダメージを食らわせる
		if (params)params->hp -= CalFormula(damage, this);
		m_count++;
	}
}


ISkillEffect::ISkillEffect(Skill* skill, double time)
	:skill(skill)
{
	setTime(time);
}

ISkillEffect::ISkillEffect(Skill* skill)
	:skill(skill)
{
}

void ISkillEffect::start()
{
	IAction::start();
	if (m_pileProcess)m_pileProcess();
	addTargetEffects();
}

void ISkillEffect::end()
{
	IAction::end();
	removeTargetEffects();
}

void ISkillEffect::setTarget(Object* obj)
{
	_target = obj;
}

void ISkillEffect::setCTarget(Character* chara)
{
	_CTarget = chara;
}

void ISkillEffect::setTime(double t)
{
	endIf<TimeCondition>(*this, t);
	time = t;
}

Skill::Skill(StringView name, const Skill::Collback& collback)
	:collback(collback),name(name)
{
}

Borrow<SHitbox> Skill::getHitbox(int32 i)
{
	if (i == -1)return endHitbox;
	return hitboxs[Format(i)];
}

Borrow<SHitbox> Skill::getHitbox(StringView name)
{
	return hitboxs[name];
}

void Skill::build()
{
	//もし変更が起こっていたら まだ判定してないけど
	collback(this, ACreate(name));
}

void Skill::act()
{
	build();
	actman.act(name);
}