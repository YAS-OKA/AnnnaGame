#include"../../stdafx.h"
#include"SkillInfo.h"
#include"Provider.h"
#include"SkillsActor.h"

using namespace skill;

SkillProvider* SkillProvider::instance = nullptr;

void SkillProvider::Init(my::Scene* scene)
{
	if (instance)return;//すでにインスタンスが生成されていたらここで終了
	instance = new SkillProvider();
	instance->scene = scene;

	using C = ColliderCategory;
	using enum EffectType;
	auto& d = instance->skillDict;

	d[U"Tmp"] = [](Skill* s, Actions& act)
		{
		};

	//アタックスキル
	d[U"旅人/たたく"] = [](Skill* s, Actions& act)
		{
			const double hitableTime = 1;//のちのちテキストファイルとかに書くべきパラメータ

			act += SHitbox(s, s->getInfo<Chara>()->v, Box(2, 2, 2), Vec3{ 7,3,0 }, hitableTime, HashSet<C>{ C::enemy });

			act |= FuncAction(
				[=, hitbox = s->getHitbox()](double) {
						for (auto& target : hitbox->getNewHittings({ C::enemy }))
						{
							auto damage = [](Damage*) { return 10; };
							auto dotDamage = [](DotDamage*) {return 1; };

							////効果を付与
							target += SEffect<burn>(s)
								= Damage(s, damage)
								>> MyPrint(U"いたっ！", 0.8)
								+ DotDamage(s, 1, 7, dotDamage)
								+ MyPrint(U"メラメラ", 7)
								>> MyPrint(U"治った！", 2);
						}
				}, hitableTime
			);
		};
}

void SkillProvider::Destroy()
{
	if (instance)delete instance;
	instance = nullptr;
}

Skill* SkillProvider::Get(StringView name)
{
	if (instance)
	{
		if (not instance->skillDict.contains(name))throw Error{ U"存在しないスキルが指定されました。\nスキル名:" + name };
		return instance->scene->birthObjectNonHitbox<Skill>(name, instance->skillDict[name]);
	}

	throw Error{ U"SkillProviderのインスタンスが生成されていないため、スキルを提供できませんでした。" };
}
