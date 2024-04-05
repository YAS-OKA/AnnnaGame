#pragma once
#include"Parts.h"
#include"../Prg/Action.h"
#include"../Util/CmdDecoder.h"

namespace mot
{
	static Array<String> MotionKind
	{
		U"rotate",
		U"rotateTo",
		U"move",
		U"moveTo",
		U"addZ",
		U"setZ",
		U"addScale",
		U"setScale",
	};

	class MotionScript
	{
	public:
		CmdDecoder deco;

		void Load(PartsManager* pMan,const String& motionName,const String& text);

		bool LoadFile(PartsManager* pMan, const String& path,const String& motionName);
	};

	class PartsMotion :public prg::TimeAction
	{
	public:
		Parts* target;

		PartsMotion(Parts* target, double time = 0);
	};

	//class Rotate :public PartsMotion
	//{
	//public:
	//	const double ang;

	//	Rotate(Parts* target, double angle, double time = 0);

	//	void start()override;

	//	void update(double dt)override;
	//};

	class RotateTo :public PartsMotion
	{
	public:
		const double angle;

		const int32 rotation;

		const bool clockwizeRotation;

		//clockwizeRotationは時計回りか反時計回りか。rotationは何回転するか。
		RotateTo(Parts* target, double angle, double time = 0,bool clockwizeRotation = true, int32 rotation = 0);

		void update(double dt)override;
	};

	/*class Move :public PartsMotion;
	{
	publ ic:
		Vec2 move;

		Move(Parts* target, const Vec2& move, double time = 0);

		void update(double dt)override;
	};*/

	class MoveTo :public PartsMotion
	{
	public:
		Vec2 dest;

		MoveTo(Parts* target, double destX,double destY, double time = 0);

		void update(double dt)override;
	};

	class AddZ :public PartsMotion
	{
	public :
		double z;

		AddZ(Parts* target, double z, double time = 0);

		void update(double dt)override;
	};

	class SetZ :public PartsMotion
	{
	public:
		double z;

		SetZ(Parts* target, double z, double time = 0);

		void update(double dt)override;
	};

	class AddScale :public PartsMotion
	{
	public:
		Vec2 scale;

		AddScale(Parts* target, const Vec2& scale, double time = 0);

		void update(double dt)override;
	};

	class SetScale :public PartsMotion
	{
	public:
		Vec2 scale;

		SetScale(Parts* target, const Vec2& scale, double time = 0);

		void update(double dt)override;
	};
}
