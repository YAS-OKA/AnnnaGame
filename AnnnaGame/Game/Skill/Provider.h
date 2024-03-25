#pragma once
#include"Skills.h"

namespace skill
{
	//SkillEffectCreatorを組み合わせてSkillを作り、提供するシングルトン
	class SkillProvider
	{
		SkillProvider();
		~SkillProvider() {};

		HashTable<String, Skill::Collback> skillDict;

		static SkillProvider* instance;

		my::Scene* scene;
	public:
		static void Init(my::Scene* scene);

		static void Destroy();

		static Skill* Get(StringView name);
	};
}
