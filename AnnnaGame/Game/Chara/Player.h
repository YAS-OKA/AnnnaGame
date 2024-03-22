#pragma once
#include"Character.h"

enum class SpotedSkill
{
	up,left,right,neutral
};



class Player:public Character
{
public:
	SpotedSkill skillState{ SpotedSkill::neutral };

	void start()override;

	void update(double dt)override;
};

