#include "../stdafx.h"
#include "Condition.h"
#include"Action.h"

#include"../EC.hpp"
#include"../Prg/GameAction.h"
#include"../Game/UI.h"

using namespace prg;

ICondition::ICondition(size_t waitFrame)
	:flag(false), waitFrame(waitFrame), frameCounter(0)
{
}

void ICondition::forced(bool _flag)
{
	flag = _flag;
}

void ICondition::countFrame()
{
	frameCounter++;
}

void ICondition::setWaitFrame(size_t frame)
{
	waitFrame = frame;
}

bool ICondition::enoughFrame() const
{
	return waitFrame <= frameCounter;
}

bool prg::ICondition::commonCheck()const
{
	return enoughFrame() and (flag or check());
}

void ICondition::reset()
{
	flag = false;
	frameCounter = 0;
}

bool ICondition::check()const
{
	return false;
}

void prg::ConditionArray::countFrame()
{
	ICondition::countFrame();
	for (auto& conditionTable : conditions.arr)
	{
		for (auto& [_, condition] : conditionTable.second)
			condition->countFrame();
	}
}

bool ConditionArray::check()const
{
	if (conditions.arr.empty())return false;//無条件の場合、自分(flag)が条件となる
	
	//すべてand または すべてor
	bool tmp = checkType == Type::Any;

	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			if (tmp == condition->commonCheck())return tmp;

	return not tmp;
}

void ConditionArray::reset()
{
	ICondition::reset();
	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			condition->reset();
}

Hit::Hit(const Borrow<Hitbox>& box, HashSet<ColliderCategory> target, size_t waitFrame)
	:box(box), target(target), ICondition(waitFrame)
{
}

Hit::Hit(const Borrow<Hitbox>& box, const ColliderCategory& target, size_t waitFrame)
	:box(box), target({ target }), ICondition(waitFrame)
{
}

bool Hit::check()const
{
	for (auto& c : target)
	{
		return box->collidedEntitys.contains(c) and not box->collidedEntitys.at(c).empty();
	}
	return false;
}

ButtonChecker::ButtonChecker(ui::Button* button, size_t waitFrame)
	:button(button), ICondition(waitFrame)
{}

void ButtonChecker::setButton(ui::Button* button)
{
	ButtonChecker::button = button;
}

bool ButtonChecker::check()const
{
	return button->pushed();
}

prg::TimeCondition::TimeCondition(const Borrow<IAction>& _act, double time, size_t waitFrame)
	:ICondition(waitFrame), act(_act), time(time)
{
}

bool prg::TimeCondition::check()const
{
	return act and act->timer >= time;
}

prg::FuncCondition::FuncCondition(const Borrow<IAction>& act, const ActState& state, bool Not, size_t waitFrame)
	:ICondition(waitFrame)
{
	if (state == ActState::start)
		if (Not)	m_function = [=] {return not(act->isStarted()); };
		else m_function = [=] {return act->isStarted(); };
	else if (state == ActState::active)
		if (Not) m_function = [=] {return not(act->isActive()); };
		else m_function = [=] {return act->isActive(); };
	else
		if (Not) m_function = [=] {return not(act->isEnded()); };
		else m_function = [=] {return act->isEnded(); };
}

bool prg::FuncCondition::check() const
{
	return m_function();
}
