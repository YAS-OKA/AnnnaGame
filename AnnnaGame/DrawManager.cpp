#include "stdafx.h"
#include "DrawManager.h"
#include"Component/Draw.h"
#include"Game/Camera.h"
#include"Game/Utilities.h"

DrawManager::DrawManager(const ColorF& backGround)
	:renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes },m_camera(nullptr),backGroundColor(backGround)
{
	ps = HLSL{ U"example/shader/hlsl/forward_fog.hlsl", U"PS" }
	| GLSL{ U"example/shader/glsl/forward_fog.frag", {{ U"PSPerFrame", 0 }, { U"PSPerView", 1 }, { U"PSPerMaterial", 3 }, { U"PSFog", 4 }} };
	cb={ { backGroundColor.removeSRGBCurve().rgb(), 0.0f } };
	cb->fogCoefficient = 0.004;
}

DrawManager::DrawManager(const Borrow<Camera>& camera, const ColorF& backGround)
	:m_camera(camera), renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }, backGroundColor(backGround)
{
	ps = HLSL{ U"example/shader/hlsl/forward_fog.hlsl", U"PS" }
	| GLSL{ U"example/shader/glsl/forward_fog.frag", {{ U"PSPerFrame", 0 }, { U"PSPerView", 1 }, { U"PSPerMaterial", 3 }, { U"PSFog", 4 }} };
	cb={ { backGroundColor.removeSRGBCurve().rgb(), 0.0f } };
	cb->fogCoefficient = 0.004;
}

DrawManager::DrawManager(const Borrow<Camera>& camera, const MSRenderTexture& renderTexture, const ColorF& backGround)
	:m_camera(camera),renderTexture{renderTexture},backGroundColor(backGround)
{
	ps = HLSL{ U"example/shader/hlsl/forward_fog.hlsl", U"PS" }
	| GLSL{ U"example/shader/glsl/forward_fog.frag", {{ U"PSPerFrame", 0 }, { U"PSPerView", 1 }, { U"PSPerMaterial", 3 }, { U"PSFog", 4 }} };
	cb = { { backGroundColor.removeSRGBCurve().rgb(), 0.0f } };
	cb->fogCoefficient = 0.004;
}

util::Convert2DTransform DrawManager::getConverter()
{
	return util::Convert2DTransform(this);
}

void DrawManager::setting(const Borrow<Camera>& camera, std::function<bool(const Borrow<IDrawing>&)> f)
{
	m_camera = camera;
	canDraw = f;
}

void DrawManager::set3D(const Borrow<IDraw3D>& drawing)
{
	m_drawings3D << drawing;
}

void DrawManager::set2D(const Borrow<IDraw2D>& drawing)
{
	m_drawings2D << drawing;
}

void DrawManager::remove3D(const Borrow<IDraw3D>& drawing)
{
	m_drawings3D.remove(drawing);
}

void DrawManager::remove2D(const Borrow<IDraw2D>& drawing)
{
	m_drawings2D.remove(drawing);
}

Borrow<IDraw2D> DrawManager::get2D(std::function<bool(const Borrow<IDraw2D>&)> filter)
{
	for (int32 i = m_drawings2D.size(); i >= 0; --i)if (filter(m_drawings2D[i]))return m_drawings2D[i];
	return nullptr;
}

Array<Borrow<IDraw2D>> DrawManager::get2Ds(std::function<bool(const Borrow<IDraw2D>&)> filter)
{
	Array<Borrow<IDraw2D>> ret{};
	for (int32 i = m_drawings2D.size(); i >= 0; --i)if (filter(m_drawings2D[i]))ret << m_drawings2D[i];
	return ret;
}

void DrawManager::drop(const Borrow<IDrawing>& drawing)
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

Borrow<Camera> DrawManager::getCamera()const
{
	return m_camera;
}

void DrawManager::update()
{

	//深さ 座標　計算
	for (auto it = m_drawings3D.begin(), en = m_drawings3D.end(); it != en; ++it)
	{
		(*it)->cal_drawPos();
		(*it)->cal_distanceFromCamera();
	}

	std::stable_sort(
		m_drawings3D.begin(),
		m_drawings3D.end(),
		[=](const auto& d1, const auto& d2) {return d1->distanceFromCamera() < d2->distanceFromCamera(); }
	);
	//深さ 座標　計算
	for (auto it = m_drawings2D.begin(), en = m_drawings2D.end(); it != en; ++it)
	{
		(*it)->cal_drawPos();
		(*it)->shallow->cal_depth();
	}

	std::stable_sort(
		m_drawings2D.begin(),
		m_drawings2D.end(),
		[=](const auto& d1, const auto& d2) {	return d1->shallow->shouldReplace(d2->shallow); }
	);
}

void DrawManager::draw(bool draw3D)const
{
	if(draw3D)
	{
		//// 3D シーンにカメラを設定
		{
			Graphics3D::SetCameraTransform(m_camera->getCamera());
			Graphics3D::SetPSConstantBuffer(4, cb);

			const ScopedCustomShader3D shader{ ps };
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
#if _DEBUG
	if(debugDraw)debugDraw();
#endif
}
