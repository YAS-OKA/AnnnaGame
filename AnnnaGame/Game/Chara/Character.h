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

class Character :public Object
{
public:
	CharacterParams param;

	virtual void start();

	virtual void update(double dt)override;
};
