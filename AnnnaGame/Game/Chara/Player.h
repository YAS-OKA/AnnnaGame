#pragma once
#include"Character.h"
#include"../StateMachine/Inform.h"

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

	void behaviorSetting(state::Inform&& info);

	void setPlayerAnimator(state::Inform&& info);
};
