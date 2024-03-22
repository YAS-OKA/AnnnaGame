#include "../stdafx.h"
#include "Actions.h"
using namespace prg;

Actions::Actions()
	:IAction()
{
	endCondition.set([borrowed = lend()] {return borrowed->isAllFinished(); });
}

Actions::~Actions()
{
	clear();
}

void prg::Actions::clear()
{
	for (auto& action : update_list)delete action;
	update_list.clear();
}

int32 Actions::getIndex(IAction* action)
{
	int32 count = 0;
	for (auto& a : update_list)
	{
		if (a == action)
		{
			for (int32 i = 0; i < separate.size() - 1; i++)
			{
				if (separate[i] <= count and count < separate[i + 1])
				{
					return separate[i];
				}
			}
		}
		++count;
	}
	return -1;
}

int32 prg::Actions::getActiveIndex() const
{
	return activeIndex;
}

int32 prg::Actions::getActiveNum() const
{
	return activeNum;
}

Array<IAction*> prg::Actions::getAll()const
{
	return update_list;
}

Array<IAction*> Actions::getAction(const int32& index)
{
	auto [s, e] = _getArea(index);
	Array<IAction*>acts;
	for (auto itr = update_list.begin() + s, en = update_list.begin() + e; itr != en; ++itr)
		acts << (*itr);
	return acts;
}

IAction* prg::Actions::operator[](const String& id)
{
	return getAction(id);
}

Array<IAction*> prg::Actions::operator[](const int32& index)
{
	return getAction(index);
}

int32 Actions::getLoopCount()const
{
	return loopCount;
}

void Actions::restart()
{
	reset();
	start();
}

void prg::Actions::start()
{
	start(false);
}

void Actions::start(bool startFirstActions)
{
	start(0, startFirstActions);
}

void prg::Actions::start(const int32& startIndex, bool startFirstActions)
{
	IAction::start();

	activeIndex = startIndex;

	int32 sepInd = 1;

	for (int32 n = 0; n < update_list.size(); ++n)
	{
		if (separate[sepInd] <= n)
		{
			++sepInd;
		}
		//前の区間のアクションが全て終わったら開始 (自分の区間がactiveIndexになったら開始) また、setInなので同じ条件クラスが重複しない
		update_list[n]->startCondition.addIn(U"isMyTurn", [=] {return sepInd - 1 == activeIndex; });
	}

	if (startFirstActions)_startCheck();
}

void Actions::reset()
{
	IAction::reset();
	activeNum = update_list.size();
	activeIndex = 0;
	loopCount = 0;
	for (auto& action : update_list)
	{
		action->reset();
	}
}

bool Actions::isAllFinished()
{
	return activeNum == 0;
}

void Actions::end()
{
	IAction::end();
	//実行中のアクションは終了させる
	for (auto& action : update_list)
	{
		if (action->isActive())action->end();
	}
	//if (initialize)reset();
}

void Actions::update(double dt)
{
	if (stopped)return;

	IAction::update(dt);
	dt *= timeScale;
	_sort();

	_startCheck();

	_update(dt);

	_endCheck();

	if (isAllFinished())
	{
		if (loop)
		{
			int32 tmp = ++loopCount;
			reset();
			startCondition.forced();
			loopCount = Min(tmp, maxLoopCount);
		}
	}
	else if (_isEnded(_getArea(activeIndex))) ++activeIndex;
}

void prg::Actions::_insert(int32 septIndex, IAction* action)
{
#if _DEBUG
	if (isActive())throw Error{ U"Actionsが実行中にActionの追加が行われました" };//実行時エラーなのでリリース時はthrowしない
#endif

	update_list.insert(update_list.begin() + separate[septIndex], action);

	++activeNum;
}

void prg::Actions::_sort()
{
	for (int32 i = activeIndex; i < separate.size() - 1; ++i)
	{
		const auto& [st, en] = _getArea(i);
		//優先度ソート 同区間内でソート
		std::stable_sort(update_list.begin() + st, update_list.begin() + en, [](const IAction* ac1, const IAction* ac2) {
			return ac1->updatePriority < ac2->updatePriority;
		});
	}
}

void prg::Actions::_startCheck()
{
	for (auto it = update_list.begin(), end = update_list.end(); it != end; ++it)
	{
		auto& act = (*it);
		if ((not act->started) and (not act->ended))
		{
			if (act->startCondition.check()) act->start();

			act->startCondition.countFrame();//カウント
		}
	}
}

void prg::Actions::_update(double dt)
{
	for (auto it = update_list.begin(), en = update_list.end(); it != en; ++it)
	{
		if ((*it)->isActive())(*it)->update(dt);
	}
}

void prg::Actions::_endCheck()
{
	for (auto it = update_list.begin(), end = update_list.end(); it != end; ++it)
	{
		auto& act = (*it);
		if (act->isActive()) {
			if (act->endCondition.check())
			{
				act->end();
				activeNum--;
			}
			act->endCondition.countFrame();//カウント
		}
	}
}

bool prg::Actions::_isEnded(std::tuple<int32, int32>se) const
{
	const auto& [start, end] = se;

	for (auto it = update_list.begin() + start, en = update_list.begin() + end; it != en; ++it)
		if (not (*it)->isEnded())
			return false;

	return true;
}

std::tuple<int32, int32> prg::Actions::_getArea(const int32& activeIndex) const
{
	return std::tuple<int32, int32>(
		separate[activeIndex],
		separate.size() - 1 > activeIndex ? separate[activeIndex + 1] : separate[activeIndex]);
}
