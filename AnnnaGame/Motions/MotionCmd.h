#pragma once
#include"../Prg/Init.h"
#include"../Figure.h"
#include"../Util/Util.h"

namespace mot
{
	//セットしたsceneにパーツを生成
	class MakeParts :public prg::IAction
	{
	private:
		class Parts* createdParts = nullptr;
	public:
		String name;

		Optional<String> path = none;

		Figure fig;

		double x, y;

		class PartsManager* pmanager;

		bool createHitbox = false;

		MakeParts(String name, String path, double x, double y)
			:name(name), IAction(0), path(path), x(x), y(y) {};

		MakeParts(String name, String figure, double x, double y, double w, double h)
			:name(name), IAction(0), x(x), y(y) {
			if (figure == U"rect")
			{
				fig = RectF{ -w / 2,-h / 2,w,h };
			}
		};

		MakeParts(String name, String figure, double x, double y, double deg, double len1, double len2)
			:name(name), IAction(0), x(x), y(y)
		{
			if (figure == U"tri") {
				deg *= 1_deg;
				fig = Triangle{ {0,0},util::polar(-deg / 2,len1),util::polar(deg / 2,len2) };
				fig.setCenter(0, 0);
			}
		}

		MakeParts(String name, String figure, double x, double y, double r)
			:name(name), IAction(0), x(x), y(y)
		{
			if (figure == U"cir") {
				fig = Circle{ r };
			}
		}

		MakeParts* build(PartsManager* pmanager, bool createHitbox)
		{
			this->pmanager = pmanager;
			this->createHitbox = createHitbox;
			return this;
		};
		//keepParts=falseの場合、現在のcreatedPartsを返して、このクラスのcreatedParts=nullptrにする。
		Parts* getCreatedParts(bool keepParts = false);

	private:
		virtual void start()override;
	};

	class FindParts :public prg::IAction
	{
	private:
		class Parts* foundParts = nullptr;
	public:
		String name;

		class PartsManager* pmanager;

		FindParts(const String& name)
			:name(name) {};

		FindParts* build(PartsManager* pmanager)
		{
			this->pmanager = pmanager;
			return this;
		};

		Parts* getFindParts(bool keepParts = false);
	private:
		virtual void start()override;
	};

	class KillParts :public prg::IAction
	{
	private:
		HashSet<class Parts*> killedParts;
	public:
		String name;

		bool killChildren;

		class PartsManager* pmanager;

		KillParts(const String& name, bool killChildren = false)
			:name(name), killChildren(killChildren)
		{
		}

		KillParts* build(PartsManager* pmanager)
		{
			this->pmanager = pmanager;
			return this;
		}

		HashSet<Parts*> getKilledParts(bool keepParts = false);
	private:
		virtual void start()override;
	};

	template<class T>
	class SetMotion:public prg::IAction
	{
	public:
		PartsManager* pMan;
		String targetName;
		String motionName;
		double time;
		T* action=nullptr;

		template<class... Args>
		SetMotion(const String& target, const String& motionName,double time, Args&& ...args)
			:targetName(target),motionName(motionName),time(time), action(new T(nullptr, args...))
		{
		}

		SetMotion<T>* build(PartsManager* pMan)
		{
			this->pMan = pMan;
			return this;
		}

		T* getCreatedMotion()const
		{
			return action;
		};
	private:
		void start()
		{
			auto targetParts = pMan->find(targetName);
			if (not targetParts)return;
			action->target = targetParts;
			auto& motion = targetParts->actman[motionName];//motionNameが含まれていたらそいつを返すし、いなければ新しく生成して下でセットする。
			action->startIf<TimeCondition>(motion, time);
			motion.addActParallel(action);
		}
	};

	class LoadMotionScript:public prg::IAction
	{
	public:
		PartsManager* pman;
		FilePath path;
		String motionName;

		LoadMotionScript(FilePath path,String motionName);
//load asset/motion/sara/motion.txt tmp
		LoadMotionScript* build(PartsManager* pman);

	protected:
		void start();
	};

	class WriteMotionScript :public prg::IAction
	{
	public:
		PartsManager* pman;
		FilePath path;
		String motionName;
		Optional<String> time;
		Optional<String> len;

		WriteMotionScript(FilePath path, String motionName, Optional<String> time = none, Optional<String> len = none);

		WriteMotionScript* build(PartsManager* pmana);

		Array<String> createMotionText()const;

	protected:
		void start();
	};

	class StartMotion :public prg::IAction
	{
	public:
		bool loop;
		PartsManager* pMan;
		String motionName;

		StartMotion(String motionName, bool loop = false);

		StartMotion* build(PartsManager* pMan)
		{
			this->pMan = pMan;
			return this;
		}
	private:
		void start();
	};
}
