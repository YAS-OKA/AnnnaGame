#include"../../stdafx.h"
#include"StateProvider.h"
#include"../../Prg/GameAction.h"
//#include"../../Prg/StateActions.h"

using namespace state;

StateActions state::StateCreator::create(StringView name)
{
	StateActions act(name);
	return act;
}

StateProvider* StateProvider::instance = nullptr;

void state::StateProvider::Init()
{
	if (instance)return;
	instance = new StateProvider();
}

void state::StateProvider::Destroy()
{
	if (instance)delete instance;
	instance = nullptr;
}

StateProvider::StateProvider()
{

}

StateActions&& state::StateProvider::Get(StringView name, const Inform& info)
{	
	return std::forward<StateActions>(instance and instance->dict.contains(name) ?
		instance->dict[name](info) : StateActions(name)
	);
}
