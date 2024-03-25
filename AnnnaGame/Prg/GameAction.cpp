#include "../stdafx.h"
#include "GameAction.h"
#include"../Game/Object.h"
#include"../Game/Chara/Character.h"

namespace prg
{
	void EndAction::endAll()
	{
		endOtherIf([](IAction*) {return true; });
	}
	void EndAction::endOtherIf(const std::function<bool(IAction*)>& f)
	{
		this->f = f;
	}
	void EndAction::addException(StringView id)
	{
		exception << actions->getAction<IAction>(id)->lend();
	}
	void EndAction::addException(const Borrow<IAction>& act)
	{
		exception << act;
	}
	void EndAction::addTarget(StringView id)
	{
		targets << actions->getAction<IAction>(id)->lend();
	}

	void EndAction::start()
	{
		if (f)actions->getAll().each([&](IAction* act) {if (f(act))targets << act->lend(); });
	}

	void EndAction::update(double dt)
	{
		//nullじゃないアクションを終了する。
		targets.remove_if([&](const Borrow<IAction>& target) {return (not target) or exception.contains(target); });
		//終了する
		targets.each([&](Borrow<IAction>& target) {if (actions)actions->end(target); });
	}

	FreeFall::FreeFall(Transform* transform, double time)
		:transform(transform),IAction(time)
	{
	}

	FreeFall::FreeFall(Transform* transform, Vec3 initVel, Vec3 acc, double time)
		:initVel(initVel),acc(acc), transform(transform), IAction(time)
	{
	}

	FreeFall::FreeFall(Transform* transform, Vec3 initVel, double acc, double time)
		:initVel(initVel), transform(transform), IAction(time)
	{
		setAcc(acc);
	}

	void FreeFall::setAcc(double a)
	{
		acc = Vec3{ 0,a,0 };
	}

	void FreeFall::start()
	{
		IAction::start();
		vel = initVel;
	}

	void FreeFall::update(double dt)
	{
		IAction::update(dt);
		vel += acc *dt;
		*transform += vel * dt;
	}

	LifeSpan::LifeSpan(Object* target, double time)
		:target(target), IAction(time)
	{}

	void LifeSpan::end()
	{
		if (target)target->die();
		target = nullptr;
	}

	void Hitbox::reset()
	{
		IAction::reset();
		collidedEntitys.clear();
	}

	void Hitbox::start()
	{
		IAction::start();
		collider->collidable = true;
		hitbox->getComponent<Draw3D>(U"hitbox")->visible = true;
	}

	void Hitbox::update(double dt)
	{
		IAction::update(dt);

		collidedEntitys.clear();

		for (const auto& tc : targetCategory)
		{
			for (const auto& entity : collider->intersects(tc))
			{
				collidedEntitys[tc].emplace(entity);
			}
		}
	}

	void Hitbox::end()
	{
		IAction::end();
		collidedEntitys.clear();
		collider->collidable = false;
		hitbox->getComponent<Draw3D>(U"hitbox")->visible = false;
	}

	ShowParam::ShowParam(Character* c, double t)
		:chara(c),IAction(t)
	{}

	void ShowParam::update(double dt)
	{
		const auto& p = chara->param;
		p.printParams();
		Print << U"physics";
		p.physics.printParams();
		Print << U"magic";
		p.magic.printParams();
		Print << U"zokusei";
		p.zokusei.printParams();
	}

}
