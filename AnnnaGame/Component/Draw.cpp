#include "../stdafx.h"
#include "Draw.h"
#include"Transform.h"
#include"../DrawManager.h"

IDrawing::IDrawing(DrawManager* manager)
	:manager(manager)
{
}

IDrawing::~IDrawing()
{
	manager->drop(this);
}

void IDrawing::start()
{
	//cameraInfluence = owner->addComponentNamed<Field<Influence>>(U"cameraInfluence", Influence());
	transform=owner->getComponent<Transform>();
}

Vec3 IDrawing::getDrawPos() const
{
	Vec3 localPos = transformScaleAffectable?relative*transform->getAspect():relative;
	//回転
	auto q1 = direction.accum;

	auto rq = q1.inverse();

	if (transformDirectionAffectable) {
		q1 = q1 * transform->get2Direction().accum;
	}
	localPos = rq * localPos;//基準に戻す
	localPos = q1 * localPos;//目的の方向を向かせる

	return transform->getPos() + localPos;
}

Vec3 IDrawing::distanceFromCamera()const
{
	return getDrawPos() - manager->getCamera()->transform->getPos();
}

Draw3D::Draw3D(DrawManager* manager, const MeshData& data)
	:IDraw3D(manager)
	,mesh(data)
{
}

void Draw3D::setAssetName(const String& name)
{
	assetName = name;
}

void Draw3D::draw()const
{
	mesh.draw(
		getDrawPos()
		, transform->get2Direction().accum
		, TextureAsset(assetName)
		,color.removeSRGBCurve()
		);
}

Billboard::Billboard(DrawManager* manager)
	:Draw3D(manager, MeshData::Billboard({ 1,1 }))
{}

void Billboard::draw()const
{
	auto size = TextureAsset(assetName).size();
	double x = size.x;
	double y = size.y;
	auto max = Math::Max(x, y);
	Vec2 aspect_ = size / max;

	mesh.draw(
			manager->getCamera()->getCamera().billboard(getDrawPos(), aspect_ * transform->getAspect().xy())
		, TextureAsset(assetName)
		, color
	);
}

IDraw2D::IDraw2D(DrawManager* manager)
	:IDrawing(manager)
{
}

IDraw2D::~IDraw2D()
{
	delete _scale_calculation;
	delete shallow;
}

Vec2 IDraw2D::getScale() const
{
	Vec2 aspect_ = (transformScaleAffectable ? transform->getAspect() * aspect.aspect.vec : aspect.aspect.vec).xy();

	return aspect_ * (*_scale_calculation)();
}

Vec2 IDraw2D::getScalePos() const
{
	return _scale_calculation->getScalePos();
}

DrawShallow* IDraw2D::getShallow()const
{
	return shallow;
}

Transformer2D IDraw2D::getTransformer() const
{
	Vec2 aspect_ = getScale();//(transformScaleAffectable ? transform->getAspect() * aspect.aspect.vec : aspect.aspect.vec).xy();
	Vec2 aspDir = (transformScaleAffectable ? Vec3(Quaternion::FromUnitVectorPairs({ {1,0,0,},{0,1,0} }, transform->getScaleDir()) * aspect.dir) : aspect.dir).xy();
	double rotation = transformDirectionAffectable ? transform->getDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle() + direction.vector.xy().getAngle() : direction.vector.xy().getAngle();
	rotation -= Vec2{ 1,0 }.getAngle();
	//motioneditorでxscaleを0にするとバグるので対策
	if (aspect_.x == 0)aspect_.x = zero;
	if (aspect_.y == 0)aspect_.y = zero;

	auto drawPos = getDrawPos();

	double scaleRad = util::getRad(aspDir);

	const auto& scaleMat = Mat3x2::Rotate(scaleRad-rotation) * Mat3x2::Scale(aspect_, getScalePos()) * Mat3x2::Rotate(rotation-scaleRad);

	return Transformer2D{
		scaleMat
		* Mat3x2::Rotate(rotation,rotateCenter.xy())
		* Mat3x2::Translate(drawPos.xy() - viewport.tl())
		,TransformCursor::Yes
	};
}

draw_helper::ScaleCalculation2D::ScaleCalculation2D(Transform* transform, DrawManager* m, double baseLength, const Camera::DistanceType& type)
	:converter(baseLength, m, type),transform(transform)
{
}

double draw_helper::ScaleCalculation2D::operator() ()const
{
	//カメラとの距離が0だったら0
	return converter.distanceRate(transform->getPos());
}

draw_helper::DrawShallow::DrawShallow(IDraw2D* owner)
	:owner(owner)
{
}

bool draw_helper::DrawShallow::shouldReplace(DrawShallow* other) const
{
	const auto& camera=owner->manager->getCamera();
	//相手の描画が優先される(あとから描写される)場合trueを返すというようにすればいい
	if (layer == other->layer)
	{
		return camera->distance(owner->getDrawPos()) > camera->distance(other->owner->getDrawPos());
	}
	else
	{
		return layer < other->layer;
	}

	return false;
}

