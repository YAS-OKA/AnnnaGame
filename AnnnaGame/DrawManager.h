#pragma once

namespace util {
	class Convert2DTransform;
}

class DrawManager
{
private:
	Array<class IDraw3D*> m_drawings3D;
	Array<class IDraw2D*> m_drawings2D;
	std::function<bool(class IDrawing*)> canDraw;
	class Camera* m_camera;
	const MSRenderTexture renderTexture;
	const MSRenderTexture renderTexture2D;
public:
	ColorF backGroundColor = Palette::Black;
	Vec2 scale{ 1,1 };
	Vec2 scalePos{ 0,0 };
	Vec2 translate{ 0,0 };
	double angle = 0;

	/*Vec2 scale3D{ 1,1 };
	Vec2 scalePos3D{ 0,0 };
	Vec2 translate3D{ 0,0 };*/

	DrawManager();
	/*~DrawManager();*/

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
	virtual void draw()const;

};
