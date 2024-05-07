#include "../stdafx.h"
#include "Motion.h"
#include"../Util/Util.h"

namespace
{
	HashSet<String> simpleMotion{U"SetAngle", U"SetZ"};
}

void mot::MotionScript::Load(const Borrow<PartsManager>& pMan,const String& motionName, const String& text)
{
	DecoderSet(&deco).motionScriptCmd(pMan);
	String target = U"";
	String time = U"0";
	String len = U"0";
	for (const auto& cLine : text.split_lines())
	{
		//前処理 
		String line=U"";
		const auto& e = cLine.indexOf(U"//");
		if (e == 0)continue;											//先頭からコメントが始まってたら次の行へ
		for (int32 i = 0; i < e and i < cLine.size(); ++i)line << cLine[i];//文中のコメントの削除
		auto token = line.replace(U" ", U",").split(U',');				//空白を,に変換　,で文字列を分割 トークン化
		token.remove_if([](const String& str) {return str == U""; });	//なんも書いてないトークンを削除
		if (token.isEmpty()) {											//トークンがなければtimeにlenを加算して次の行へ
			time = Format(Eval(time + U"+" + len));
			continue;
		}
		//主処理
		if (token[0] == U"JOINT")
		{
			target = token[1];
			continue;
		}
		else if (token[0] == U"TIME")
		{
			time = token[1];
			continue;
		}
		else if (token[0] == U"LEN")
		{
			len = token[1];
			continue;
		}

		if (simpleMotion.contains(token[0]))
		{
			//lenは第3引数に入れる
			token.insert(token.begin() + 2, len);
		}
		else if (token[0] == U"Pause")
		{
			//lenは第7引数に入れる
			token.insert(token.begin() + 6, len);
		}
		else
		{
			//lenは第４引数に入れる
			token.insert(token.begin() + 3, len);
		}
		//3要素をセット
		token.insert(token.begin() + 1, { target,motionName,time });

		deco.input(token)->decode()->execute();
	}
}

bool mot::MotionScript::LoadFile(const Borrow<PartsManager>& pMan, const String& path, const String& motionName)
{
	auto reader = TextReader{ path };
	if (not reader)return false;
	String inputText = U"";
	bool find = false;
	for (const auto& line : TextReader{ path }.readAll().split_lines())
	{
		if (find)
		{
			if (line[0] == U'#')break;
			inputText += line;
			inputText += U"\n";
		}

		if (line[0] == U'#')
		{
			if (motionName == util::slice(line, 1, line.size()))
			{
				find = true;
			}
			else
			{
				find = false;
			}
		}
	}

	if(not inputText.isEmpty()) Load(pMan, motionName, inputText);
	return true;
}

mot::PartsMotion::PartsMotion(const Borrow<Parts>& target, double time)
	:TimeAction(time), target(target)
{
}

mot::Rotate::Rotate(const Borrow<Parts>& target, double angle, double time)
	:PartsMotion(target, time), ang(angle)
{
}

void mot::Rotate::update(double dt)
{
	PartsMotion::update(dt);

	target->setAngle(target->getAngle() + ang * dtPerTime(dt));
}

mot::RotateTo::RotateTo(const Borrow<Parts>& target, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PartsMotion(target, time), angle(angle), clockwizeRotation(clockwizeRotation), rotation(rotation)
{
	firstRotateDirectionIsDesignated = (bool)clockwizeRotation;//noneでなければtrue
}
//0~360
double seikika(double theta) {
	if (0 <= theta)return Fmod(theta, 360.0);//正規化
	else return 360 + Fmod(theta, 360.0);
}

void mot::RotateTo::start()
{
	PartsMotion::start();

	if (acts.empty())impl = acts.add<Rotate>(target, 0, time);

	double delta = seikika(angle) - seikika(target->getAngle());

	if (not clockwizeRotation)//回転が短い方向を求める
	{
		clockwizeRotation = (delta < 0) ^ (abs(delta) < 180);
	}

	if (*clockwizeRotation)
	{
		//時計回り
		impl->ang = delta < 0 ? delta + 360 : delta;
		impl->ang += 360 * rotation;
	}
	else
	{
		//反時計回り
		impl->ang = delta > 0 ? delta - 360 : delta;
		impl->ang -= 360 * rotation;
	}

	acts.start(true);
}

void mot::RotateTo::update(double dt)
{
	PartsMotion::update(dt);

	acts.update(dt);
}

void mot::RotateTo::reset()
{
	PartsMotion::reset();

	if (not firstRotateDirectionIsDesignated)clockwizeRotation = none;
}

mot::Move::Move(const Borrow<Parts>& target, double moveX, double moveY, double time)
	:PartsMotion(target, time), move({ moveX,moveY })
{
}

void mot::Move::update(double dt)
{
	PartsMotion::update(dt);

	target->setPos(target->getPos() + move * dtPerTime(dt));
}

mot::MoveTo::MoveTo(const Borrow<Parts>& target, double destX, double destY, double time)
	:PartsMotion(target, time),dest({ destX,destY })
{
}

void mot::MoveTo::start()
{
	PartsMotion::start();

	if (acts.empty())impl = acts.add<Move>(target, 0, 0, time);

	impl->move = dest - target->getPos();

	acts.start(true);
}

void mot::MoveTo::update(double dt)
{
	PartsMotion::update(dt);

	acts.update(dt);
}

mot::AddZ::AddZ(const Borrow<Parts>& target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::AddZ::update(double dt)
{
	PartsMotion::update(dt);

	target->setZ(target->getZ() + z * dtPerTime(dt));
}

mot::SetZ::SetZ(const Borrow<Parts>& target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::SetZ::update(double dt)
{
	PartsMotion::update(dt);

}

mot::AddScale::AddScale(const Borrow<Parts>& target, const Vec2& scale, double time)
	:PartsMotion(target,time),scale(scale)
{
}

void mot::AddScale::update(double dt)
{
	PartsMotion::update(dt);

}

mot::SetScale::SetScale(const Borrow<Parts>& target, double sX, double sY, double time)
	:PartsMotion(target,time),scale({sX,sY})
{
}

void mot::SetScale::update(double dt)
{
	PartsMotion::update(dt);

}

Vec2 mot::PauseTo::calDestination()const
{
	//目標から回転後の座標を引く
	if (target->parent)
	{
		const auto& parentAspect = target->parent->transform->getAspect().xy();
		const auto& d = dest * parentAspect;
		return (d
			.rotatedAt(d + (target->getRotatePos() * parentAspect).rotate(ang * 1_deg), (target->getAngle() - ang) * 1_deg)//目標座標を初期角度まで回転させる
			.rotated(target->parent->transform->getDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle())//親の回転を考慮
			- target->getPos());//親や自分の回転をなくした場合のずれを返す
	}
	//たぶん親がいないなんてことはない(masterpartsに命令を出さない限り)
	return dest;
}

mot::PauseTo::PauseTo(const Borrow<Parts>& target, double destX, double destY, double sX, double sY, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PartsMotion(target, time), ang(angle), clockwizeRotation(clockwizeRotation), rotation(rotation), dest({ destX,destY }), scale({ sX,sY }),parentDirectionInit(none)
{
}

void mot::PauseTo::start()
{
	PartsMotion::start();

	if (motions.empty())
	{
		rotateTo=motions.addParallel<RotateTo>(target, ang, time, clockwizeRotation, rotation);
		motions.addParallel<SetScale>(target, scale.x, scale.y, time);
		move = motions.addParallel<Move>(target, 0, 0, time);
	}
	move->move = calDestination();

	moveInit = move->move;

	if (target->parent)parentDirectionInit = target->parent->transform->getDirection().xy();

	motions.start(true);
}

void mot::PauseTo::update(double dt)
{
	if (parentDirectionInit)
	{
		move->move = moveInit.rotated(target->parent->transform->getDirection().xy().getAngle() - parentDirectionInit->getAngle());
	}

	PartsMotion::update(dt);
	motions.update(dt);
}
