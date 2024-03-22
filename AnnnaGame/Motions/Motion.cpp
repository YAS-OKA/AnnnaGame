#include "../stdafx.h"
#include "Motion.h"
#include"../Game/Utilities.h"

namespace
{
	HashSet<String> simpleMotion{U"SetAngle", U"SetZ"};
}

void mot::MotionScript::Load(PartsManager* pMan,const String& motionName, const String& text)
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

void mot::MotionScript::LoadFile(PartsManager* pMan, const String& path, const String& motionName)
{
	auto text = TextReader{ path }.readAll();
	String inputText = U"";
	bool find = false;
	for (const auto& line : text.split_lines())
	{
		if (find)
		{
			inputText += line;
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
}

mot::PartsMotion::PartsMotion(Parts* target, double time)
	:TimeAction(time), target(target)
{
}

//mot::Rotate::Rotate(Parts* target, double angle, double time)
//	:PartsMotion(target, time), ang(angle)
//{
//}
//
//void mot::Rotate::start()
//{
//	PartsMotion::start();
//}
//
//void mot::Rotate::update(double dt)
//{
//	PartsMotion::update(dt);
//
//	target->setAngle(target->getAngle() + ang * dtPerTime(dt));
//}

mot::RotateTo::RotateTo(Parts* target, double angle, double time, bool clockwizeRotation, double rotation)
	:PartsMotion(target, time), angle(angle),clockwizeRotation(clockwizeRotation),rotation(rotation)
{
}

void mot::RotateTo::update(double dt)
{
	PartsMotion::update(dt);

	if (timer >= time)target->setAngle(angle);
	else {
		target->setAngle(target->getAngle() + dt * (angle - target->getAngle()) / (time - timer));
	}
}
//
//mot::Move::Move(Parts* target, const Vec2& move, double time)
//	:PartsMotion(target,time),move(move)
//{
//}
//
//void mot::Move::update(double dt)
//{
//	PartsMotion::update(dt);
//
//	target->setPos(target->getPos() + move * dtPerTime(dt));
//}

mot::MoveTo::MoveTo(Parts* target, const Vec2& dest, double time)
	:PartsMotion(target,time),dest(dest)
{
}

void mot::MoveTo::update(double dt)
{
	PartsMotion::update(dt);

	if (timer >= time)
		target->setPos(dest);
	else 
		target->setPos(target->getPos() + dt * (dest - target->getPos()) / (time - timer));
}

mot::AddZ::AddZ(Parts* target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::AddZ::update(double dt)
{
	PartsMotion::update(dt);

	target->setZ(target->getZ() + z * dtPerTime(dt));
}

mot::SetZ::SetZ(Parts* target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::SetZ::update(double dt)
{
	PartsMotion::update(dt);

}

mot::AddScale::AddScale(Parts* target, const Vec2& scale, double time)
	:PartsMotion(target,time),scale(scale)
{
}

void mot::AddScale::update(double dt)
{
	PartsMotion::update(dt);

}

mot::SetScale::SetScale(Parts* target, const Vec2& scale, double time)
	:PartsMotion(target,time),scale(scale)
{
}

void mot::SetScale::update(double dt)
{
	PartsMotion::update(dt);

}
