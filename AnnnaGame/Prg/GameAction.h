#pragma once
#include"Action.h"
#include"../Component/Transform.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Component/Collider.h"
#include"../Game/Utilities.h"

class Object;
class Collider;
class Character;

//今回のゲームで使うアクション
namespace prg
{
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

			hitbox->ACreate(U"killSelf", true) +=LifeSpan(hitbox).same(*this);
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
