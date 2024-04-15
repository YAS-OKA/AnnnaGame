#include"../../stdafx.h"
#include"StateProvider.h"
#include"../../Prg/GameAction.h"
//#include"../../Prg/StateActions.h"

using namespace state;

StateActions state::StateCreator::create(StringView name)const
{
	StateActions act(name);
	return act;
}
