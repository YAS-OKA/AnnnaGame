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

		Borrow<my::Scene> scene;
	public:
		static void Init(const Borrow<my::Scene>& scene);

		static void Destroy();

		static Borrow<Skill> Get(StringView name);
	};
}
