#include "stdafx.h"
#include "DrawManager.h"
#include"Component/Draw.h"
#include"Game/Camera.h"
#include"Game/Utilities.h"

DrawManager::DrawManager()
	:renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes },m_camera(nullptr)
{
}
//
//DrawManager::~DrawManager()
//{
//}

util::Convert2DTransform DrawManager::getConverter()
{
	return util::Convert2DTransform(this);
}

void DrawManager::setting(Camera* camera, std::function<bool(IDrawing*)> f)
{
	m_camera = camera;
	canDraw = f;
}

void DrawManager::set3D(IDraw3D* drawing)
{
	m_drawings3D << drawing;
}

void DrawManager::set2D(IDraw2D* drawing)
{
	m_drawings2D << drawing;
}

void DrawManager::remove3D(IDraw3D* drawing)
{
	m_drawings3D.remove(drawing);
}

void DrawManager::remove2D(IDraw2D* drawing)
{
	m_drawings2D.remove(drawing);
}

IDraw2D* DrawManager::get2D(std::function<bool(IDraw2D*)> filter)
{
	for (int32 i = m_drawings2D.size(); i >= 0; --i)if (filter(m_drawings2D[i]))return m_drawings2D[i];
	return nullptr;
}

Array<IDraw2D*> DrawManager::get2Ds(std::function<bool(IDraw2D*)> filter)
{
	Array<IDraw2D*> ret{};
	for (int32 i = m_drawings2D.size(); i >= 0; --i)if (filter(m_drawings2D[i]))ret << m_drawings2D[i];
	return ret;
}

void DrawManager::drop(IDrawing* drawing)
{
	for (auto it = m_drawings3D.begin(); it != m_drawings3D.end();)
	{
		if (drawing == (*it))it = m_drawings3D.erase(it);
		else ++it;
	}

	for (auto it = m_drawings2D.begin(); it != m_drawings2D.end();)
	{
		if (drawing == (*it))it = m_drawings2D.erase(it);
		else ++it;
	}
}

MSRenderTexture DrawManager::getRenderTexture()const
{
	return renderTexture;
}

Camera* DrawManager::getCamera()const
{
	return m_camera;
}

void DrawManager::update()
{
	std::stable_sort(
		m_drawings3D.begin(),
		m_drawings3D.end(),
		[=](const IDraw3D* d1, const IDraw3D* d2) {return d1->distanceFromCamera().length() < d2->distanceFromCamera().length(); }
	);

	std::stable_sort(
		m_drawings2D.begin(),
		m_drawings2D.end(),
		[=](const IDraw2D* d1, const IDraw2D* d2) {	return d1->shallow->shouldReplace(d2->shallow); }
	);
}

void DrawManager::draw()const
{
	{/*
		const auto t0 = Transformer2D{ Mat3x2::Translate(-translate3D),TransformCursor::Yes };
		const auto t1 = Transformer2D{ Mat3x2::Scale(scale3D,scalePos3D),TransformCursor::Yes };*/
		//// 3D シーンにカメラを設定
		Graphics3D::SetCameraTransform(m_camera->getCamera());
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backGroundColor) };

			const ScopedRenderStates3D blend{ BlendState::OpaqueAlphaToCoverage };

			for (const auto& drawing : m_drawings3D)if (drawing->visible)drawing->draw();
		}

		//// 3D シーンを 2D シーンに描画
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			Shader::LinearToScreen(renderTexture);
		}
	}
	for (const auto& drawing : m_drawings2D)if (drawing->visible)
	{
		const auto trans = translate * drawing->dManagerInfluence->value.getMovement();
		const auto t0 = Transformer2D{Mat3x2::Translate(-trans),TransformCursor::Yes };
		const auto t1 = Transformer2D{Mat3x2::Scale(Vec2{1,1}+(scale-Vec2{1,1})*drawing->dManagerInfluence->value.getScale(),scalePos+util::sc()+trans),TransformCursor::Yes};
		const auto t2 = Transformer2D{Mat3x2::Rotate(angle * 1_deg * drawing->dManagerInfluence->value.Rotate)};
		drawing->draw();
	}
}
