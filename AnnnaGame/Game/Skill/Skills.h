#pragma once
#include"../GameSystem/GameSystem.h"
//#include"../../Prg/Init.h"
#include"../../Util/TypeContainer.hpp"
#include"SkillInfo.h"
#include"../Object.h"

class Character;

namespace my { class Scene; }

using namespace sys;

namespace skill {
	template<class Type>
	using PileProcess = std::function<void(Type*,Array<Type*>)>;

	//スキルの効果クラス　インターフェース
	struct ISkillEffect :public prg::IAction
	{
		class Skill* skill;

		Object* _target = nullptr;
		Character* _CTarget = nullptr;

		double time;//これは今だけ　　後々アクションが自分の終了時間を計算する関数を作る。

		ISkillEffect(Skill* skill, double time);

		ISkillEffect(Skill* skill);

		virtual void start()override;

		virtual void end()override;

		virtual void setTarget(Object* obj);

		virtual void setCTarget(Character* obj);

		void setTime(double t);

		//重ね掛け処理をセットする
		template <class Self>
		Self& setPileProseccing(this Self& self, const PileProcess<Self>& func)
		{
			//自分と同じ効果クラスの配列をfuncに渡す
			self.m_pileProcess = [=] { func(self.target->skillEffects.getArray<Self>(&self)); };
			return self;
		}
		//setPileProcessを呼ぶ　
		template <class Self>
		Self& operator()(this Self& self, const PileProcess<Self>& func)
		{
			//自分と同じ効果クラスの配列をfuncに渡す
			return self.setPileProcessing(func);
		}
	private:
		//効果の重ね掛けの処理
		std::function<void()> m_pileProcess = nullptr;
		//targetのスキルエフェクト保管庫に自分を追加
		template<class Self>
		void addTargetEffects(this Self& self)
		{
			self._target->skillEffects.addPtr<Self>(&self);
		}
		//targetのスキルエフェクト保管庫から自分を取り除く
		template<class Self>
		void removeTargetEffects(this Self& self)
		{
			self._target->skillEffects.remove<Self>(&self);
		}
	};

	enum class EffectType
	{
		non,
		burn
	};

	template<EffectType t>
	struct SEffect :ISkillEffect
	{
		std::unique_ptr<Actions> effects;

		SEffect(Skill* s = nullptr)
			:ISkillEffect(s , 0)
		{
			effects = std::make_unique<Actions>();
			endIf([borrowed = lend()] {return borrowed->effects->isAllFinished(); });
		};

		//ただのアクションもスキルエフェクトにできる
		template<class Act, std::enable_if_t<std::is_base_of_v<IAction, Act>>* = nullptr >
		SEffect& operator +=(Act&& other)
		{
			*effects += std::forward<Act>(other);
			return *this;
		}
		template<class Act, std::enable_if_t<std::is_base_of_v<IAction, Act>>* = nullptr >
		SEffect& operator |=(Act&& other)
		{
			*effects |= std::forward<Act>(other);
			return *this;
		}
		template<class Act, std::enable_if_t<std::is_base_of_v<IAction, Act>>* = nullptr >
		SEffect&& operator =(Act&& other)
		{
			*effects += std::forward<Act>(other);
			return std::move(*this);
		}

		SEffect&& operator =(Actions&& other)
		{
			effects.reset(new Actions(std::move(other)));
			return std::move(*this);
		}

		virtual void setTarget(Object* obj)override
		{
			ISkillEffect::setTarget(obj);
			_setTarget(obj, effects.get());//安全なアクセスなので生ポインタを渡す
		};

		virtual void setCTarget(Character* chara)override
		{
			ISkillEffect::setCTarget(chara);
			_setCTarget(chara, effects.get());//安全なアクセスなので生ポインタを渡す
		}

		void _setTarget(Object* obj, Actions* effects)
		{
			for (auto& act : effects->getAll())
			{
				if (auto effect = dynamic_cast<ISkillEffect*>(act))
				{
					if (not effect->_target)effect->setTarget(obj);
				}
				else if (auto actions = dynamic_cast<Actions*>(act))
				{
					_setTarget(obj, actions);
				}
			}
		}
		void _setCTarget(Character* chara, Actions* effects)
		{
			for (auto& act : effects->getAll())
			{
				if (auto effect = dynamic_cast<ISkillEffect*>(act))
				{
					if (not effect->_target)effect->setCTarget(chara);
				}
				else if (auto actions = dynamic_cast<Actions*>(act))
				{
					_setCTarget(chara, actions);
				}
			}
		}
	protected:
		void start()override
		{
			ISkillEffect::start();
			effects->start();
		}
		void update(double dt)override {
			ISkillEffect::update(dt);
			effects->update(dt);
		}
		void end()override { ISkillEffect::end(); effects->end(); }
		void reset()override { ISkillEffect::reset(); effects->reset(); }
	};

	class SHitbox;

	class Skill:public Object
	{
	private:
		//スキルの情報(動的に渡したい情報も含む)
		TypeContainer<SkillInfo> info;
	public:
		//最後に追加したヒットボックス
		Borrow<SHitbox> endHitbox;
		//技名
		String name;
		////効果を参照できるように格納しておく
		//WeakTypeContainer<ISkillEffect> effects;
		//このコールバックを呼び出して効果を作る
		using Collback = std::function<void(Skill*,Actions& act)>;
		Collback collback;
		//ヒットボックスを格納できる
		HashTable<String, Borrow<SHitbox>> hitboxs;

		Skill(StringView name, const Collback& collback);

		Borrow<SHitbox> getHitbox(int32 i = -1);
		Borrow<SHitbox> getHitbox(StringView name);
	
		//SkillInfoの追加
		template<class T,class... Args, std::enable_if_t<std::is_base_of_v<SkillInfo, T>>* = nullptr>
		void addInfo(StringView infoName, Args ...args)
		{
			info.addIn<T>(infoName, args...);
		}

		template<class T, std::enable_if_t<std::is_base_of_v<SkillInfo, T>>* = nullptr>
		T* getInfo(Optional<String> infoName = none)
		{
			if (infoName)return info.get<T>(*infoName);
			else return info.get<T>();
		}

		//スキルをビルドする
		void build();

		void act();
	};
}

namespace skill
{
	//数式の型
	template<class Effect, class T = double>
	using Formula = std::variant<std::function<T(Effect*)>, std::function<T(void)>>;

	struct ParamMod :ISkillEffect
	{
		CharacterParams* params = nullptr;
		ParamMod(Skill* s, double time = 0);

		void start()override;
	};

	//ダメージ
	struct Damage :ParamMod
	{
		Damage(Skill* s, Formula<Damage> damage);

		Formula<Damage> damage;
	protected:
		void start()override;
	};
	//相手にノックバックの速度を与える
	struct Knockback :ISkillEffect
	{
		Formula<Knockback, Vec3> dir;

		Formula<Knockback> power;

	protected:
		void start()override;

	};
	//継続ダメージ
	struct DotDamage :ParamMod
	{
		DotDamage(Skill* s, double span, double time, Formula<DotDamage> damage = [](DotDamage*) {return 0; });

		//何回ダメージを与えたか
		int32 getCount();

		Formula<DotDamage> damage;

	protected:
		//ダメージが入る間隔
		double timeSpan;

		int32 m_count;
		void start()override;

		void update(double dt)override;
	};

	//属性値の操作 効果が終了したらもとに戻す
	struct AttributeMod :ParamMod
	{
		Attribute delta;

		Attribute* atr;
		//最初に属性を変化
		Formula<AttributeMod, Attribute> f;

	protected:
		void start()override;

		void end()override;
	};
	//フレーム更新ごとに属性値を操作　効果が終了したらもとに戻す
	struct AttributeModFrame :ParamMod
	{
		//この効果クラスが変化させた量
		Attribute delta;

		Attribute* atr;
		//最初に属性を変化
		Formula<AttributeMod, Attribute> f;

	protected:
		void update(double dt)override;

		void end()override;
	};

	class Pile
	{

	};
}
