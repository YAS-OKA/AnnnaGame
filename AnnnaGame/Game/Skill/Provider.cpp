#include"../../stdafx.h"
#include"SkillInfo.h"
#include"Provider.h"
#include"SkillsActor.h"

using namespace skill;

SkillProvider* SkillProvider::instance = nullptr;

SkillProvider::SkillProvider()
{
	using C = ColliderCategory;
	using enum EffectType;
	auto& d = skillDict;

	d[U"Tmp"] = [](const Borrow<Skill>& s, Actions& act)
		{
			const double hitableTime = 1;//のちのちテキストファイルとかに書くべきパラメータ
			
			act += SHitbox(s, s->getInfo<Chara>()->v, Box(2, 2, 2), Vec3{ 1 * s->getInfo<InfoV<Vec3>>(U"dir")->v.x,1.5,0 }, hitableTime, HashSet<C>{ C::hero });

			act |= FuncAction(
				[=, hitbox = s->getHitbox()](double) {
					for (auto& target : hitbox->getNewHittings({ C::hero }))
					{
						auto damage = [] { return 5; };
						////効果を付与
						Actions se;
						se += Damage(s, damage)
							>> MyPrint(U"いたっ！", 0.8);
						//本当はs->getInfoでパワーを取得する
						se |= Knockback(s, 1, [=] {
							return Vec3{ s->getInfo<InfoV<Vec3>>(U"dir")->v.x,0,0 };
							}
						, [] {return 20; });

						target += SEffect<non>(s) = std::move(se);
					}
				}, hitableTime);
		};

	//アタックスキル
	d[U"旅人/たたく"] = [](const Borrow<Skill>& s, Actions& act)
		{
			const double hitableTime = 1;//のちのちテキストファイルとかに書くべきパラメータ

			act += SHitbox(s, s->getInfo<Chara>()->v, Box(2, 2, 2), Vec3{ 7* s->getInfo<InfoV<Vec3>>(U"dir")->v.x,3,0 }, hitableTime, HashSet<C>{ C::enemy });

			act |= FuncAction(
				[=, hitbox = s->getHitbox()](double) {
					for (auto& target : hitbox->getNewHittings({ C::enemy }))
					{
						auto damage = [] { return 10; };
						auto dotDamage = [] {return 1; };
						////効果を付与
						Actions se;
						se += Damage(s, damage)
							>> MyPrint(U"いたっ！", 0.8)
							+ DotDamage(s, 1, 7, dotDamage)
							+ MyPrint(U"メラメラ", 7)
							>> MyPrint(U"治った！", 2);
						//本当はs->getInfoでパワーを取得する
						se |= Knockback(s, 1, [=] {return
							Vec3{ s->getInfo<InfoV<Vec3>>(U"dir")->v.x,0,0 };
							}, [] {return 20; });

						target += SEffect<burn>(s) = std::move(se);
					}
				}, hitableTime
			);
		};
}

void SkillProvider::Init(const Borrow<my::Scene>& scene)
{
	if (instance)return;//すでにインスタンスが生成されていたらここで終了
	instance = new SkillProvider();
	instance->scene = scene;
}

void SkillProvider::Destroy()
{
	if (instance)delete instance;
	instance = nullptr;
}

Borrow<Skill> SkillProvider::Get(StringView name)
{
	if (instance)
	{
		if (not instance->skillDict.contains(name))throw Error{ U"存在しないスキルが指定されました。\nスキル名:" + name };
		return instance->scene->birth<Skill>(name, instance->skillDict[name]);
	}

	throw Error{ U"SkillProviderのインスタンスが生成されていないため、スキルを提供できませんでした。" };
}
