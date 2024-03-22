#pragma once
#include"../Game/Object.h"
#include"../Util/DataSaver.h"
#include"../Prg/Action.h"
#include"../Game/Utilities.h"
#include"../Util/CmdDecoder.h"

namespace my
{
	class Scene;
}

namespace tool
{
	class ObjScript
	{
	public:
		CmdDecoder deco;

		my::Scene* scene;

		ObjScript(my::Scene* scene) :scene(scene)
		{
		}

		Object* Load(const String& path, const String& name)
		{
			TextReader reader{ path };
			return _loadImpl(reader.readAll(), name);
		};

		Object* create(const DataSaver& data,Optional<String> objName);

	private:
		Object* _loadImpl(const String& text, const String& nest);
	};
}

namespace objScriptCmdAction
{
	class SetObject:public prg::IAction
	{
	public:
		double x, y, z, angle;

		SetObject(double x, double y,double z, double angle)
			:IAction(0), x(x), y(y),z(z), angle(angle)
		{
		}
	};

	class AttachDrawing:public SetObject
	{
	public:
		double r, g, b, a, scaleX, scaleY;
		Object* obj=nullptr;

		AttachDrawing(double x, double y, double z, double angle, double r, double g, double b, double a, double scaleX, double scaleY)
			:SetObject(x, y, z, angle), r(r), g(g), b(b), a(a), scaleX(scaleX), scaleY(scaleY)
		{}
	};

	class AttachTexture :public AttachDrawing
	{
	public:
		const String path;

		AttachTexture(String path, double x, double y, double r = 255, double g = 255, double b = 255, double a = 1, double angle = 0, double z = 0, double scaleX = 1, double scaleY = 1)
			:path(path), AttachDrawing(x, y, z, angle, r, g, b, a,scaleX, scaleY) {};

		AttachTexture* build(Object* obj)
		{
			this->obj = obj;
			return this;
		};
	protected:
		void start()override;
	};

	class AttachRectF :public AttachDrawing
	{
	public:
		RectF fig;

		AttachRectF(double x, double y, double w, double h, double r = 255, double g = 255, double b = 255, double a = 1, double angle = 0, double z = 0, double scaleX = 1, double scaleY = 1)
			:AttachDrawing(x, y, z, angle, r, g, b, a, scaleX, scaleY), fig(RectF{ 0,0,w,h })
		{
		};

		AttachRectF* build(Object* obj)
		{
			this->obj = obj;
			return this;
		}
	private:
		void start()override;
	};

	class AttachTriangle :public AttachDrawing
	{
	public:
		Triangle fig;

		AttachTriangle(double x, double y, double deg, double len1, double len2, double r = 255, double g = 255, double b = 255, double a = 1, double angle = 0, double z = 0, double scaleX = 1, double scaleY = 1)
			:AttachDrawing(x, y, z, angle, r, g, b, a, scaleX, scaleY)
		{
			deg *= 1_deg;
			fig = Triangle{ {0,0},util::polar(-deg / 2,len1),util::polar(deg / 2,len2) };
		}

		AttachTriangle* build(Object* obj)
		{
			this->obj = obj;
			return this;
		}
	private:
		void start()override;
	};

	class AttachCircle:public AttachDrawing
	{
	public:
		Circle fig;

		AttachCircle(double x, double y, double radius, double r = 255, double g = 255, double b = 255, double a = 1, double angle = 0, double z = 0, double scaleX = 1, double scaleY = 1)
			:AttachDrawing(x, y, z, angle, r, g, b, a,scaleX, scaleY), fig(Circle{ radius })
		{
		}

		AttachCircle* build(Object* obj)
		{
			this->obj = obj;
			return this;
		}

	private:
		void start()override;
	};

	class MakeGUI :public prg::IAction
	{

	};
}
