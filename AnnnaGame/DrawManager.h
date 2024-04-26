﻿#pragma once

namespace util {
	class Convert2DTransform;
}

struct PSFog
{
	Float3 fogColor;
	float fogCoefficient;
};

class DrawManager
{
private:
	Array<class IDraw3D*> m_drawings3D;
	Array<class IDraw2D*> m_drawings2D;
	std::function<bool(class IDrawing*)> canDraw;
	class Camera* m_camera;
	const MSRenderTexture renderTexture;
	PixelShader ps;
	double fogParam = 0.6;
	ConstantBuffer<PSFog> cb;
public:
	ColorF backGroundColor;
	Vec2 scale{ 1,1 };
	Vec2 scalePos{ 0,0 };
	Vec2 translate{ 0,0 };
	double angle = 0;

	std::function<void()>debugDraw=nullptr;

	/*Vec2 scale3D{ 1,1 };
	Vec2 scalePos3D{ 0,0 };
	Vec2 translate3D{ 0,0 };*/

	DrawManager(const ColorF& backGround = { 0,0,0,1 });
	DrawManager(Camera* camera, const ColorF& backGround = { 0,0,0,1 });
	DrawManager(Camera* camera, const MSRenderTexture& renderTexture, const ColorF& backGround = { 0,0,0,1 });

	void setting(Camera* camera, std::function<bool(IDrawing*)> f = [](IDrawing*) {return true; });
	void set3D(IDraw3D* drawing);
	void set2D(IDraw2D* drawing);
	IDraw2D* get2D(std::function<bool(IDraw2D*)> filter = [](IDraw2D*) {return true; });
	Array<IDraw2D*> get2Ds(std::function<bool(IDraw2D*)> filter = [](IDraw2D*) {return true; });
	void drop(IDrawing* drawing);
	void remove3D(IDraw3D* drawing);
	void remove2D(IDraw2D* drawing);
	MSRenderTexture getRenderTexture()const;
	Camera* getCamera()const;
	util::Convert2DTransform getConverter();
	virtual void update();
	virtual void draw(bool draw3D=true)const;

};
