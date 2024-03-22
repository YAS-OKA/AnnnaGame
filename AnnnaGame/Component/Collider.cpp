#include "../stdafx.h"
#include "Collider.h"
#include"../Game/Camera.h"
//#include"Draw.h"
#include"../Game/Object.h"
#include"../DrawManager.h"
#include"../Game/Utilities.h"

CollideBox::CollideFigure::CollideFigure(const Figure& fig)
	:figure(fig)//,layerd(layerd)
{
}

Figure CollideBox::CollideFigure::getFigure()const
{
	return figure;
}

void CollideBox::CollideFigure::setFigure(const Figure& fig)
{
	figure = fig;
	//四角形だったらtlに
	if (fig.shape.index() == 1) {
		const auto& r = figure.getRectF();
		parent->relative = { Vec2(r.w,r.h)/2,0};
	}
};

void CollideBox::CollideFigure::operator=(const Figure& fig)
{
	setFigure(fig);
}

Polygon CollideBox::CollideFigure::getScaledFig()const
{
	//return  figure.asPolygon().scaledAt((parent->t->getPos() + parent->relative).xy(), parent->t->getAspect().xy());
	auto poly = figure.asPolygon();
	auto rad = util::getRad(parent->t->getScaleDir().first.xy());
	poly.rotateAt(parent->t->getPos().xy() ,-rad);
	poly.scaleAt(parent->t->getPos().xy(), parent->t->getAspect().xy());
	poly.rotateAt(parent->t->getPos().xy(), rad);
	return poly;
}

bool CollideBox::CollideFigure::intersects(const CollideFigure& fig)const
{
	auto scaledFig = getScaledFig();//figure.asPolygon().scaled(parent->t->getAspect().xy());
	//お互い階層を持ってたら
	//if (layerd and fig.layerd)
	//{
	//	//階層を考慮した当たり判定
	//	double z, other_z;
	//	z = parent->t->getPos().z + parent->relative.z;
	//	other_z = fig.parent->t->getPos().z + fig.parent->relative.z;
	//	if (z == other_z)
	//	{
	//		return scaledFig.intersects(
	//			fig.getScaledFig()
	//		);
	//	}
	//}
	//else
	{
		//階層を考慮しない
		return scaledFig.intersects(
			fig.getScaledFig()
		);
	}
}

void CollideBox::CollideFigure::setWH(double w, double h)
{
	RectF rect = figure.getRectF();
	rect.h = h;
	rect.w = w;
	setFigure(rect);
}

void CollideBox::CollideFigure::setR(double r)
{
	Circle circle = figure.getCircle();
	circle.r = r;
	figure = circle;
}

void CollideBox::CollideFigure::setPos(const Vec3& pos)
{
	parent->relative.z = pos.z;
	figure.setCenter(pos.xy());
}

void CollideBox::CollideFigure::rotateAt(const Vec2& pos, double rad)
{
	if (rad == 0)return;
	this->rad += rad;
	this->rad = Math::Fmod(this->rad, 360_deg);
	if (this->rad < 0)this->rad = 360_deg - this->rad;

	parent->relative = { parent->relative.xy().rotateAt(pos, rad),parent->relative.z };
	figure.rotate(rad);
}

void CollideBox::CollideFigure::setAngle(const Vec2& pos, double rad)
{
	rad = Math::Fmod(rad, 360_deg);

	rotateAt(pos, rad - this->rad);
}

//void CollideBox::CollideFigure::fitDrawing()
//{
//	Vec2 aspect_ = (d->transformScaleAffectable ? d->transform->getAspect() * d->aspect.aspect.vec : d->aspect.aspect.vec).xy();
//	double rotation = d->transformDirectionAffectable ? d->transform->getDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle() + d->direction.vector.xy().getAngle() : d->direction.vector.xy().getAngle();
//	rotation -= Vec2{ 1,0 }.getAngle();
//
//	auto p = d->getDrawPos().xy();
//
//	figure.setCenter(p+parent->relative.xy());
//	figure.rotateAt(d->rotateCenter.xy()+figure.center(), rotation);
//	figure.scale(aspect_.x, aspect_.y);
//}

//CollideBox::CameraAffectedCollider::CameraAffectedCollider(const Figure& fig, IDraw2D* drawing, bool layerd)
//	:CollideFigure(fig, layerd), drawing(drawing)
//{
//}
//
//void CollideBox::CameraAffectedCollider::setPos(const Vec3& pos)
//{/*
//	const auto& cameraPos = drawing->manager->getCamera()->transform->getPos().xy();
//	const auto& moveInfluence = drawing->influence.getMovement();
//	CollideFigure::setPos(cameraPos * ({}));*/
//}

void CollideBox::update()
{
	std::visit([this](auto& x) { x.setPos(relative  + t->getPos()); }, shape);
}

bool CollideBox::intersects(const CollideBox& other)const
{
	return std::visit([this](auto& x) {return this->intersects(x); }, other.shape);
}

bool CollideBox::intersects(const Box& box)const
{
	switch (shape.index())
	{
	case 0:
		return std::get<0>(shape).intersects(box);
	case 1:
		return intersects(box, std::get<1>(shape));
	default:
		return false;
	}
}

bool CollideBox::intersects(const Cylinder& cylinder)const
{
	switch (shape.index())
	{
	case 0:
		return intersects(std::get<0>(shape), cylinder);
	case 1:
		auto m = std::get<1>(shape);
		return m.bottomCenter().y <= cylinder.topCenter().y and m.topCenter().y >= cylinder.bottomCenter().y
			and (m.center.xz() - cylinder.center.xz()).length() <= m.r + cylinder.r;
	}
	return false;
}

bool CollideBox::intersects(const Box& b, const Cylinder& c)const
{
	auto c1 = b.center;
	auto c2 = c.center;
	return b.bottomCenter().y <= c.topCenter().y and b.topCenter().y >= c.bottomCenter().y
		and c2.z - c.r <= c1.z + b.d / 2.0 and c2.z + c.r >= c1.z - b.d / 2.0
		and c2.x - c.r <= c1.x + b.w / 2.0 and c2.x + c.r >= c1.x - b.w / 2.0;
}

bool CollideBox::intersects(const CollideFigure& fig)const
{
	if (shape.index() == 2)
	{
		const auto& m = std::get<2>(shape);
		return m.intersects(fig);
	}
	else {
		return false;
	}
}

namespace {
	//当たり判定を分類。決まったカテゴリーのcolliderとしか当たり判定をしない
	HashTable<ColliderCategory, Array<Collider*>> clasificaly;
}

Collider::~Collider()
{
	clasificaly[category].remove(this);
}

void Collider::start()
{
	transform = owner->getComponent<Transform>();
	hitbox.t = transform;
	if (transform == nullptr)throw Error{ U"Transformが取得できませんでした。" };

	clasificaly[category] << this;
}

void Collider::update(double dt)
{
	hitbox.update();
}

Array<Entity*> Collider::intersects(const HashSet<ColliderCategory> targets)const
{
	Array<Entity*> ret;
	for (const auto& category : targets)ret.append(intersects(category));
	return ret;
}

Array<Entity*> Collider::intersects(const ColliderCategory& targetCategory)const
{
	Array<Entity*> ret;
	for (const auto& target : clasificaly[targetCategory])
	{
		if (intersects(target))	ret << target->owner;
	}
	return ret;
}

Array<Entity*> Collider::intersectsAll()const
{
	Array<Entity*>ret;
	for (const auto& [_,targets]: clasificaly)
	{
		for(const auto& target:targets)if (intersects(target)) ret << target->owner;
	}
	return ret;
}

bool Collider::intersects(const Collider* target)const
{
	if (target == this)return false;

	return hitbox.intersects(target->hitbox) and target->collidable and collidable;
}

void Collider::setCategory(const ColliderCategory& _category)
{
	clasificaly[category].remove(this);
	category = _category;
	clasificaly[category] << this;
}

const ColliderCategory Collider::getCategory()const
{
	return category;
}
