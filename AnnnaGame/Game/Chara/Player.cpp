#include "../../stdafx.h"
#include "Player.h"
#include"../Utilities.h"
#include"../../Component/Transform.h"
#include"../../Prg/GameAction.h"
#include"../Scenes.h"
#include"../GameSystem/Setting.h"
#include"../Skill/Provider.h"

void Player::start()
{
	Character::start();

	using namespace prg;
	//sousa WASD
	{
		ACreate(U"move", true).add([&](double dt)
		{
			Vec3 dir{ 0,0 ,0 };
			if (setting::Down.pressed())dir += {0, 0, -1};
			if (setting::Up.pressed())dir += {0, 0, 1};
			if (setting::Right.pressed())dir += {1, 0, 0};
			if (setting::Left.pressed())dir += {-1, 0, 0};
			dir.setLength(param.quickness);
			transform->moveBy(dir * dt);
		});
	}
	//attack
	{
		auto& attack = ACreate(U"attack");
		attack.startIf([] {return KeyK.down(); });
		auto s = skill::SkillProvider::Get(U"旅人/たたく");//スキル取得
		s->addInfo<skill::Chara>(U"chara", this);
		attack.add([=] { s->act(); });//スキル発動
	}
	//jump
	{
		ACreate(U"jump").startIf([] {return setting::Jump.down(); }).endIf([] {return setting::Jump.down(); }, 1)
			+= FreeFall(transform, Vec3{ 0,15,0 }, -20);
	}
}

void Player::update(double dt)
{
	Character::update(dt);
}
