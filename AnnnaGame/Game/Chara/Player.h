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
	void start()override;

	void update(double dt)override;

	void behaviorSetting(state::Inform&& info);

	void setAttackSkill(size_t cardsNum,StringView skillName);
};

namespace player
{

	void SetPlayerAnimator(const Borrow<Object>& obj, state::Inform&& info);
}
