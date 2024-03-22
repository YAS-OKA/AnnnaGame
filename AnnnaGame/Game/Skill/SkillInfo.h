#pragma once
#include"../Chara/Character.h"

namespace skill
{
	namespace {
		//スキルインフォ
		struct SkillInfo {};
	}

	//一つの値を持つスキルインフォ
	template<class Value>struct InfoV :SkillInfo
	{
		Value v;

		InfoV(const Value& v) :v(v) {}
	};
	using Chara = InfoV<Character*>;
	using Pro = InfoV<size_t>;
}
