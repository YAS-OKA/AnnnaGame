﻿#include"../../stdafx.h"
#include"Inform.h"

using namespace state;

bool state::Inform::contains(StringView name)
{
	return info.contains(name);
}

void Inform::set(StringView name, Info value)
{
	info.emplace(name, value);
}

Info Inform::get(StringView name, Info&& default_value)
{
	return info.contains(name) ? std::forward<Info>(get(name)) : std::forward<Info>(default_value);
}

Info& Inform::get(StringView name)
{
	if (not info.contains(name))info[name].valid = false;
	return info[name];
}
