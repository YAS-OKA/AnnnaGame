﻿#pragma once
#include"../Util/TypeContainer.hpp"
#include"../Util/Borrow.h"

namespace prg
{
	class ICondition
	{
	public:
		ICondition(size_t waitFrame = 0);
		virtual ~ICondition() {};
		void forced(bool flag = true);

		virtual void countFrame();

		void setWaitFrame(size_t frame);

		bool enoughFrame()const;

		bool commonCheck()const;

		virtual void reset();
	protected:
		virtual bool check()const;

		bool flag;
	private:
		size_t waitFrame;
		size_t frameCounter;
	};

	enum class Type {
		Every,
		Any
	};

	class ConditionArray :public ICondition
	{
	private:
		TypeContainer<ICondition> conditions;
	public:
		using ICondition::ICondition;
		ConditionArray(ConditionArray&&) = default;
		ConditionArray& operator = (ConditionArray&&) = default;

		ConditionArray(const ConditionArray&) = delete;
		ConditionArray& operator = (const ConditionArray&) = delete;

		void countFrame()override;

		template<class C = FuncCondition, class... Args>
		C* add(Args&& ...args)
		{
			return conditions.add<C>(args...);
		}
		template<class C = FuncCondition, class... Args>
		C* addIn(const String& id, Args&& ...args)
		{
			return conditions.addIn<C>(id, args...);
		}

		template<class C = FuncCondition, class... Args>
		C* set(Args&& ...args)
		{
			conditions.removeAll();
			return conditions.set<C>(args...);
		}

		template<class C = FuncCondition, class... Args>
		C* setIn(const String& id, Args&& ...args)
		{
			conditions.removeAll();
			return conditions.setIn<C>(id, args...);
		}

		template<class C = FuncCondition>
		C* get(Optional<StringView> id = none)
		{
			if (id)return conditions.get<C>(*id);
			else return conditions.get<C>();
		}

		void reset()override;
		//Every すべてのconditionがtrueならtrue
		//Any いずれかのconditionがtrueならtrue
		Type checkType = Type::Every;

	protected:
		virtual bool check()const override;
	};

	class TimeCondition :public ICondition
	{
	public:
		Borrow<class IAction> act;

		double time;

		TimeCondition(const Borrow<IAction>& act, double time, size_t waitFrame = 0);

	protected:
		bool check()const override;
	};

	enum class ActState
	{
		start, active, end
	};

	//関数でチェック
	class FuncCondition :public ICondition
	{
	public:
		//ラムダ式などで関数を渡す　関数の戻り値はbool
		FuncCondition(std::function<bool()> _function, size_t waitFrame = 0)
			:ICondition(waitFrame), m_function{ _function } {}

		FuncCondition(const Borrow<class IAction>& act, const ActState& state, bool Not = false, size_t waitFrame = 0);

		std::function<bool()> m_function;
	protected:
		bool check()const override;
	};

}

#include"../Component/Collider.h"

class Entity;

namespace prg
{
	class Hitbox;

	class Hit :public ICondition
	{
		Borrow<Hitbox> box;
	public:
		HashSet<ColliderCategory> target;

		Hit(const Borrow<Hitbox>& box, HashSet<ColliderCategory> targets = {}, size_t waitFrame = 0);
		Hit(const Borrow<Hitbox>& box, const ColliderCategory& target, size_t waitFrame = 0);

	protected:
		bool check()const override;
	};
}

namespace ui
{
	class Button;
}

namespace prg
{
	class ButtonChecker :public ICondition
	{
	public:
		ui::Button* button;

		ButtonChecker(ui::Button* button, size_t waitFrame = 0);
		void setButton(ui::Button* button);

	protected:
		bool check()const override;
	};
}