#pragma once
#include"Parts.h"
#include"../Prg/Action.h"
#include"../Util/CmdDecoder.h"

namespace mot
{
	class MotionScript
	{
		//LoadのときINS命令でファイルの全文を要求するときこいつを参照する
		Optional<String> AllText;
	public:
		CmdDecoder deco;

		prg::Actions* CreateMotions(const Borrow<PartsManager>& pMan, const String& text);

		void Load(const Borrow<PartsManager>& pMan, const String& motionName, const String& text);

		bool LoadFile(const Borrow<PartsManager>& pMan, const String& path,const String& motionName);

		static Optional<String> GetMotionScrOf(const String& text, StringView motionName, bool removeBackBlank = false);

		static Array<String> GetMotionNames(FilePathView scriptPath);
	};

	class PartsMotion :public prg::TimeAction
	{
	public:
		Borrow<Parts> target;

		PartsMotion(const Borrow<Parts>& target, double time = 0);
	};

	class Rotate :public PartsMotion
	{
	public:
		double ang;

		Rotate(const Borrow<Parts>& target, double angle, double time = 0);

		void update(double dt)override;
	};

	class RotateTo :public PartsMotion
	{
		bool firstRotateDirectionIsDesignated;
	public:
		double angle;

		const int32 rotation;

		Optional<bool> clockwizeRotation;

		Borrow<Rotate> impl;

		Actions acts;
		//clockwizeRotationは時計回りか反時計回りか。rotationは何回転するか。
		RotateTo(const Borrow<Parts>& target, double angle, double time = 0,Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		void start()override;

		void update(double dt)override;

		void reset()override;
	};

	class Move :public PartsMotion
	{
	public:
		Vec2 move;

		Move(const Borrow<Parts>& target, double moveX, double moveY, double time = 0);

		void update(double dt)override;
	};

	class MoveTo :public PartsMotion
	{
	public:
		Vec2 dest;

		Borrow<Move> impl;

		Actions acts;

		MoveTo(const Borrow<Parts>& target, double destX, double destY, double time = 0);

		void start()override;

		void update(double dt)override;
	};

	class AddZ :public PartsMotion
	{
	public :
		double z;

		AddZ(const Borrow<Parts>& target, double z, double time = 0);

		void update(double dt)override;
	};

	class SetZ :public PartsMotion
	{
	public:
		double z;

		SetZ(const Borrow<Parts>& target, double z, double time = 0);

		void update(double dt)override;
	};

	class AddScale :public PartsMotion
	{
	public:
		Vec2 scale;

		AddScale(const Borrow<Parts>& target, const Vec2& scale, double time = 0);

		void update(double dt)override;
	};

	class SetScale :public PartsMotion
	{
	public:
		Vec2 scale;

		SetScale(const Borrow<Parts>& target, double sX, double sY, double time = 0);

		void update(double dt)override;
	};

	class SetRotateCenter :public PartsMotion
	{
	public:
		Vec2 pos;

		SetRotateCenter(const Borrow<Parts> target, double x, double y);

		void start()override;
	};

	class PauseTo :public PartsMotion
	{
	private:
		Optional<Vec2> parentDirectionInit;
		Optional<double>parentAngleInit;
		Vec2 directionInit;
		Vec2 moveInit;
		Borrow<Move> move;
		Borrow<RotateTo> rotateTo;
		Vec2 calDestination()const;
	public:
		Vec2 scale;
		Vec2 dest;
		double ang;
		const int32 rotation;
		Optional<bool> clockwizeRotation;

		Actions motions;

		PauseTo(const Borrow<Parts>& target, double destX, double destY, double sX, double sY, double angle, double time = 0, Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		void start()override;

		void update(double dt)override;
	};
}
