#include "../stdafx.h"
#include "Actions.h"
using namespace prg;

Actions::Actions(StringView id)
{
	setId(id);
	endCondition.addIn(U"isAllFinished", [borrowed = lend()] {return borrowed->isAllFinished(); });
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

void prg::Actions::swapOne(size_t index,IAction* act)
{
	delete update_list[index];
	update_list[index] = act;
}

bool prg::Actions::empty() const
{
	return update_list.empty();
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

IAction* prg::Actions::getAction(const int32& index1, const int32& index2)
{
	return getAction(index1)[index2];
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
	reset();

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

void prg::Actions::start(IAction* act)
{
	for (auto it = update_list.begin(), en = update_list.end(); it != en; ++it) {
		if ((*it) == act and not (*it)->isActive()) {
			(*it)->start();
			activeNum++;
			return;
		}
	}
}

void Actions::reset()
{
	IAction::reset();
	notFinishedActNum = update_list.size();
	activeIndex = 0;
	loopCount = 0;
	for (auto& action : update_list)
	{
		action->reset();
	}
}

bool Actions::isAllFinished()
{
	return notFinishedActNum == 0;
}

void Actions::end()
{
	IAction::end();
	//実行中のアクションは終了させる
	for (auto& action : update_list)
	{
		if (action->isActive())action->end();
	}
}

void prg::Actions::end(IAction* act)
{
	for (auto it = update_list.begin(), en = update_list.end(); it != en; ++it) {
		if ((*it) == act and (*it)->isActive()) {
			(*it)->end();
			activeNum--;
			notFinishedActNum--;
			return;
		}
	}
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
	if (isActive())throw Error{ U"Actions実行中にActionの追加が行われました" };//実行時エラーなのでリリース時はthrowしない
#endif

	update_list.insert(update_list.begin() + separate[septIndex], action);

	action->setOwner(*this);

	++notFinishedActNum;
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
			if (act->startCondition.commonCheck())
			{
				act->start();
				activeNum++;
			}
			else {
				act->startCondition.countFrame();//カウント
			}
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
			if (act->endCondition.commonCheck())
			{
				act->end();
				activeNum--;
				notFinishedActNum--;
			}
			else {
				act->endCondition.countFrame();//カウント
			}
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

prg::StateActions::StateActions(StringView id)
{
	this->id = id;
	setEndCondition();//リセット
}

void prg::StateActions::start(const int32& startIndex, bool startFirstAction)
{
	IAction::start();
}

void prg::StateActions::update(double dt)
{
	if (stopped)return;

	IAction::update(dt);
	dt *= timeScale;
	_sort();

	_startCheck();

	_update(dt);

	_endCheck();

	for (auto it = update_list.begin(), en = update_list.end(); it != en; ++it)
	{
		auto& act = (*it);
		if (act->isEnded()) { act->reset(); }
	}
}

IAction& prg::StateActions::relate(StringView from, StringView to)
{
	auto fa = getAction<IAction>(from);
	auto ta = getAction<IAction>(to);
	ta->startIf(fa->lend(), ActState::active);//fromアクションがアクティブか、という条件
	return *ta;
}

IAction& prg::StateActions::relate(Array<String> froms, StringView to)
{
	auto ta = getAction<IAction>(to);
	ConditionArray c(Type::Any);
	for (auto& from : froms)
	{
		auto fa = getAction<IAction>(from);
		c.add(fa->lend(), ActState::active);
	}
	ta->startIf<ConditionArray>(std::move(c));
	return *ta;
}

IAction& StateActions::relate(IAction* from_act, IAction* to_act)
{
	return to_act->startIf(from_act->lend(), ActState::active);
}

void prg::StateActions::duplicatable(String a, String b)
{
	canDuplicate[a].emplace(b);
	canDuplicate[b].emplace(a);
}

IAction& prg::StateActions::operator[](StringView name)
{
	return *getAction(name);
}

Array<String> prg::StateActions::getAllState() const
{
	Array<String> res{};

	for (const auto& act : getAll())res << act->id;

	return res;
}

void prg::StateActions::_startCheck()
{
	Array<IAction*> preStartAct;
	Array<IAction*> startAct;
	Array<IAction*> activeAct;
	//スタートするものとアクティブなものを探す
	for (auto it = update_list.begin(), end = update_list.end(); it != end; ++it)
	{
		auto& act = (*it);
		if ((not act->started) and (not act->ended))
		{
			if (act->startCondition.commonCheck())
			{
				preStartAct << act;
			}

			act->startCondition.countFrame();//カウント
		}
		else if (act->isActive())
		{
			activeAct << act;
		}
	}
	//startActはすでに優先度順に並んでいる。
	//ここで同時スタートの問題を解決する。
	for (auto it = preStartAct.begin(), end = preStartAct.end(); it != end; ++it)
	{
		auto& act = (*it);

		for (auto it2 = startAct.begin(), end2 = startAct.end(); it2 != end2; ++it2)
		{
			auto& startedAct = (*it2);
			//同時に実行できないステートがあったら
			if (not (canDuplicate.contains(startedAct->id) and canDuplicate[startedAct->id].contains(act->id)))
				goto next;//スタートしない。
		}

		startAct << act;
		act->start();
		activeNum++;

	next://スタートしない
		continue;
	}
	//もしアクティブなものとスタートするものが一つもなければ
	if (activeAct.isEmpty() and startAct.isEmpty())
	{
		if (not update_list.isEmpty())
		{
			update_list.front()->start();
			activeNum++;
		}
	}
	//すでにアクティブだったものを終わらせる
	for (auto it = activeAct.begin(), en = activeAct.end(); it != en; ++it)
	{
		auto& act = (*it);
		for (auto it2 = startAct.begin(), end2 = startAct.end(); it2 != end2; ++it2)
		{
			auto& startedAct = (*it2);
			//同時に実行できないのであれば終了
			if (not (canDuplicate.contains(startedAct->id) and canDuplicate[startedAct->id].contains(act->id)))
			{
				end(act);
			}
		}		
	}
}

