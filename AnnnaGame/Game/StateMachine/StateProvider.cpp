#include"../../stdafx.h"
#include"StateProvider.h"
#include"../../Prg/GameAction.h"
#define F(x) std::forward<Actions>(x)

using namespace state;

Actions&& state::StateCreator::create(StringView name)
{
	Actions act;
	act.setId(name);
	return std::move(act);
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
	using A = Actions&&;

	dict[U"Player"]
		= [&](In info, A act)
		{
			act |= dict[U"Operable"](info)
				+ dict[U"InOperable"](info);
			return F(act);
		};
}

Actions&& state::StateProvider::Get(StringView name, const Inform& info)
{	
	return std::forward<Actions>(instance and instance->dict.contains(name) ?
		instance->dict[name](info) : Actions()
	);
}
