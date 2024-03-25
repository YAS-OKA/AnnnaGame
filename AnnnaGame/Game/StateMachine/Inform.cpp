#include"../../stdafx.h"
#include"Inform.h"

using namespace state;

void Inform::set(StringView name, Info value)
{
	info.emplace(name, value);
}

Info Inform::get(StringView name, Info&& default_value)
{
	return info.contains(name) ? info[name] : std::forward<Info>(default_value);
}
