#pragma once
#include"../Object.h"
#include"../GameSystem/GameSystem.h"
#include"../../Component/Field.h"

using namespace util;

struct CharaProperty
{
	//攻撃力
	double power;
	//技の復帰力
	double ready_rate;
	//防御力
	double guard;

	void printParams()const;
};

struct CharacterParams
{
	double hp;
	//物理攻撃関連
	CharaProperty physics{};
	//魔法攻撃関連
	CharaProperty magic{};
	//レベル
	sys::Level level;
	//すばやさ
	double quickness;
	//属性
	sys::Attribute zokusei;

	void printParams()const;

	void LoadFile(const String& path, const String& name);
};

namespace skill
{
	class Skill;
}

class Character :public Object
{
public:
	CharacterParams param;

	virtual void start();

	virtual void update(double dt)override;

	Borrow<Field<HashTable<String, Borrow<skill::Skill>>>> getSkills();

	Borrow<skill::Skill> setSkill(const Borrow<skill::Skill>& skill, StringView name);
};

#include"../../Util/CmdDecoder.h"

namespace CharaUtil
{
	struct AnimeArg
	{
		Array<String> from;
		String to;//モーション名も兼ねている
		bool loop = true;
	};

	void SetAnimator(const Borrow<Object>& obj, Borrow<mot::PartsManager> pman, FilePath scrPath, Optional<String> standard, Array<AnimeArg> args);
}
