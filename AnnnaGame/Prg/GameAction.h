#pragma once
#include"Action.h"
#include"../Component/Transform.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Component/Collider.h"

class Object;
class Collider;
class Character;

namespace prg
{
	//アクションを終了させる
	class EndAction final :public IAction
	{
	public:
		Borrow<class Actions> actions;

		Array<Borrow<IAction>>targets{};

		template<class... Args>
		EndAction(const Borrow<Actions>& actions, Args&& ...ids)
			:IAction(0), actions(actions)
		{
			setTargets(ids...);
		}
		template<class ...Args>
		void withoutId(Args&& ...ids)
		{
			for (const String& id : std::initializer_list<String>{ ids... }) addException(id);
		}

		template<class ...Args>
		void without(Args&& ...acts)
		{
			for (const Borrow<IAction>& act : std::initializer_list<Borrow<IAction>>{ acts... }) addException(act);
		}

		void endAll();

		//fに渡されるIActionのnull判定はしなくていい
		void endOtherIf(const std::function<bool(IAction*)>& f);

		template<class... Args>
		void setTargets(Args&& ...ids)
		{
			for (const String& id : std::initializer_list<String>{ ids... }) addTarget(id);
		}
	protected:
		Array<Borrow<IAction>> exception{};
		std::function<bool (IAction*)> f = nullptr;
		void addException(StringView id);
		void addException(const Borrow<IAction>& act);
		void addTarget(StringView ids);
		void start()override;
		void update(double dt)override;
	};

	class FreeFall :public IAction
	{
	public:
		Vec3 initVel{ 0,0,0 };
		Vec3 acc{ 0,0,0 };

		FreeFall(Transform* transform, double time=Math::Inf);

		FreeFall(Transform* transform, Vec3 initVel, Vec3 acc, double time = Math::Inf);

		FreeFall(Transform* transform, Vec3 initVel, double acc, double time = Math::Inf);

		void setAcc(double a);

		Transform* transform=nullptr;
	protected:
		Vec3 vel;
		virtual void start()override;
		virtual void update(double dt)override;
	};
	//このアクションは終了すると対象のオブジェクトが死ぬ
	class LifeSpan :public IAction
	{
	public:
		Object* target;

		LifeSpan(Object* target, double time = 0);
	protected:
		void end()override;
	};

	class Hitbox:public IAction
	{
	public:
		Collider* collider;

		Object* hitbox;
		//衝突するカテゴリー
		HashSet<ColliderCategory> targetCategory{};
		//衝突したエンティティを保管 一フレームごとに更新
		HashTable<ColliderCategory, HashSet<Entity*>> collidedEntitys{};

		template <class Shape>
		Hitbox(Object* chara,const Shape& shape, const Vec3& relative, double time = Math::Inf)
			:IAction(time)
		{
			hitbox = chara->scene->birthObject(shape, Vec3{ 0,0,0 });
			hitbox->transform->setParent(chara->transform);
			hitbox->transform->setLocalPos(relative);
			collider = hitbox->getComponent<Collider>();
			collider->collidable = false;
			auto vis = hitbox->getComponent<Draw3D>(U"hitbox");
			vis->color = ColorF{ Palette::Yellow }.removeSRGBCurve();
			vis->visible = false;
			//このアクションが終わったらヒットボックスオブジェクトをkill
			hitbox->ACreate(U"killSelf", true) += LifeSpan(hitbox).same(*this);
		}
		
		Hitbox* setTarget(HashSet<ColliderCategory> category) {
			targetCategory = category;
			return this;
		}
		
	protected:
		virtual void reset()override;
		virtual void start()override;
		virtual void update(double dt)override;
		virtual void end()override;
	};

	class ShowParam:public IAction
	{
	public:
		Character* chara;
		ShowParam(Character* chara, double time = Math::Inf);

	protected:
		void update(double dt)override;
	};
}
