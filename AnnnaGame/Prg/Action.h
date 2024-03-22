#pragma once
#include"Condition.h"
#include"../Util/Borrow.h"

namespace prg
{
	class ConditionArray;

	class Actions;

	class IAction :public Borrowable
	{
	public:
		IAction(double time = Math::Inf);

		void setStartCondition(ConditionArray&& condition);

		void setEndCondition(ConditionArray&& condition);
		//開始条件をセット
		template<class C = FuncCondition, class Self, class... Args>
		auto startIf(this Self&& self, Args&& ...args)->decltype(self)
		{
			self.startCondition.set<C>(args...);
			return std::forward<Self>(self);
		}
		//開始条件を追加
		template<class C = FuncCondition, class Self, class... Args>
		auto andStartIf(this Self&& self, Args&& ...args)->decltype(self)
		{
			self.startCondition.add<C>(args...);
			return std::forward<Self>(self);
		}
		//終了条件をセット
		template<class C = FuncCondition, class Self, class... Args>
		auto endIf(this Self&& self, Args&& ...args)->decltype(self)
		{
			self.endCondition.set<C>(args...);
			return std::forward<Self>(self);
		}
		//終了条件を追加
		template<class C = FuncCondition, class Self, class... Args>
		auto andEndIf(this Self&& self, Args&& ...args)->decltype(self)
		{
			self.endCondition.add<C>(args...);
			return std::forward<Self>(self);
		}
		//他のアクションと開始終了を合わせる
		template<class Self>
		auto same(this Self&& self,const Borrow<IAction>& other)->decltype(self)
		{
			self.startIf(other, ActState::start);
			self.endIf(other, ActState::end);
			return std::forward<Self>(self);
		}

		bool isStarted();

		bool isEnded();

		bool isActive();

		double updatePriority;

		double timer;

		double timeScale;

		ConditionArray startCondition;
		ConditionArray endCondition;
	protected:
		String id;
		bool started = false;
		bool ended = false;
		friend Actions;

		virtual void reset();

		virtual void start()
		{
			timer = 0;
			started = true;
		};
		virtual void end()
		{
			ended = true;
		};

		virtual void update(double dt);
	};

	//IActionはWaitとしても使える
	using Wait = IAction;

	class TimeAction :public IAction
	{
	public:
		double time;

		double progress()const;
		//dtが0～timeまでdtを加算していったとき、その和が1となるようなdt/timeを出力する
		double dtPerTime(double dt)const;

		TimeAction(double time = 0);
	};

	class MyPrint final :public IAction
	{
	public:
		MyPrint(const String& text, double time = Math::Inf);

		String text;

	private:
		void update(double dt);
	};

	class FuncAction :public IAction
	{
	public:
		using IAction::IAction;

		using Self = FuncAction*;
		using FullUpdate = std::function<void(double, Self)>;
		using SimpleUpdate = std::function<void(double)>;
		using TermEvent = std::function<void()>;
		using UpdateFunc = std::variant< FullUpdate, SimpleUpdate>;

		UpdateFunc upd = [](double) {};
		TermEvent ini = [] {};
		TermEvent fin = [] {};

		FuncAction(const UpdateFunc& upd, double time = Math::Inf);

		FuncAction(const TermEvent& ini, double time = 0);

		FuncAction(const TermEvent& ini, const UpdateFunc& upd, double time = Math::Inf);

		FuncAction(const TermEvent& ini, const UpdateFunc& upd, const TermEvent& fin, double time = Math::Inf);
	private:
		void update(double dt);
		void start();
		void end();
	};
}
