#pragma once
#include"../../stdafx.h"
#include"Skills.h"
#include"../../Prg/GameAction.h"
#include"../Chara/Character.h"

namespace skill
{
	//スキルエフェクトを相手に付与するのを簡単にする
	class STarget
	{
	public:
		Borrow<skill::Skill> s;
		Borrow<Object> target;
		Borrow<Character> CTarget;
		Actions& se;

		STarget(const Borrow<skill::Skill>& s, const Borrow<Object>& target)
			:s(s), target(target), se(target->ACreate(U"Skill/" + s->name, true))
		{
			if (auto c = dynamic_cast<Character*>(target.get())) CTarget = *c;
		}

		template<class Act, std::enable_if_t<std::is_base_of_v<skill::ISkillEffect, Act>>* = nullptr>
		void operator += (Act&& act)noexcept
		{
			act.setTarget(target);
			if (CTarget)act.setCTarget(CTarget);
			se += std::forward<Act>(act);
		}

		template<class Act, std::enable_if_t < std::is_base_of_v < skill::ISkillEffect, Act >>* = nullptr>
		void operator |= (Act& act)noexcept
		{
			act.setTarget(target);
			if (CTarget)act.setCTarget(CTarget);
			se |= std::forward<Act>(act);
		}

		operator Borrow<Object>()const
		{
			return target;
		}

		Borrow<Object> operator ->()const
		{
			return target;
		}
	};

	class SHitbox :public prg::Hitbox
	{
	public:
		Borrow<skill::Skill> sk;
		String name;

		HashTable<ColliderCategory, HashSet<Entity*>> hitted;

		template <class Shape>
		SHitbox(const Borrow<skill::Skill>& s, StringView name, const Borrow<Object>& chara, const Shape& shape, const Vec3& relative, double time = Math::Inf, HashSet<ColliderCategory> catego = {})
			:Hitbox(chara, shape, relative, time), sk(s), name(name)
		{
			setTarget(catego);
			s->hitboxs.emplace(name, *this);
			s->endHitbox = *this;
		}

		template <class Shape>
		SHitbox(const Borrow<skill::Skill>& s, const Borrow<Object>& chara, const Shape& shape, const Vec3& relative, double time = Math::Inf, HashSet<ColliderCategory> catego = {})
			: Hitbox(chara, shape, relative, time), sk(s), name(Format(s->hitboxs.size()))
		{
			setTarget(catego);
			s->hitboxs.emplace(name, *this);
			s->endHitbox = *this;
		}
		//ヒットしたオブジェクト
		Array<STarget> getHitted(const HashSet<ColliderCategory>& categorys = Collider::AllCategory)const
		{
			Array<STarget> res{};
			for (const auto& category : categorys)
			{
				if (not hitted.contains(category))continue;

				for (auto& entity : hitted.at(category))
				{
					res << STarget(*sk, *dynamic_cast<Object*>(entity));
				}
			}
			return res;
		}
		//ヒット中のオブジェクト
		Array<STarget> getHittings(const HashSet<ColliderCategory>& categorys = Collider::AllCategory)const
		{
			Array<STarget> res{};
			for (auto category : categorys)
			{
				if (not collidedEntitys.contains(category))continue;
				for (const auto& entity : collidedEntitys.at(category))
				{
					res << STarget(*sk, *dynamic_cast<Object*>(entity));
				}
			}
			return res;
		}

		Array<STarget> getNewHittings(const HashSet<ColliderCategory>& categorys = Collider::AllCategory)const
		{
			Array<STarget> res{};
			for (const auto& category : categorys)
			{
				if (not collidedEntitys.contains(category))continue;

				for (const auto& entity : collidedEntitys.at(category))
				{
					//初めてぶつかったオブジェクトなら追加
					if (not (hitted.contains(category) and hitted.at(category).contains(entity)))
						res << STarget(*sk, *dynamic_cast<Object*>(entity));
				}
			}
			return res;
		}
	protected:
		void update(double dt)override
		{
			//hittedに追加
			for (auto& [key, entitys] : collidedEntitys)
			{
				for (auto& entity : entitys)hitted[key].emplace(entity);
			}

			prg::Hitbox::update(dt);
			//死んだエンティティを持っていたら排除する
			for (auto& [key, entitys] : hitted)
			{
				for (auto& entity : entitys)if (IsKilled(entity->lend()))hitted[key].erase(entity);
			}
		}

		void end()override
		{
			prg::Hitbox::end();

			//hittedに追加
			for (auto& [key, entitys] : collidedEntitys)
			{
				for (auto& entity : entitys)hitted[key].emplace(entity);
			}
		}
	};
}
