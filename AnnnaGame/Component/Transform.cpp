﻿#include "../stdafx.h"
#include"../Game/Utilities.h"
#include "Transform.h"

Transform::Vector::Vector() {
	resetVector({ 0,0,0 });
}

void Transform::Vector::calculate()
{
	delta = vec - pre;
	pre = vec;
}

void Transform::Vector::resetVector(const Vec3& v)
{
	vec = pre = v;
	delta = { 0,0,0 };
}

Vec3 Transform::Vector::operator=(const Vec3& v)
{
	vec = v;
	return vec;
}

Vec3 Transform::Vector::operator+(const Vec3& v)
{
	return vec + v;
}

Vec3 Transform::Vector::operator*(const Vec3& v)
{
	return vec* v;
}

Vec3 Transform::Vector::operator*=(const Vec3& v)
{
	vec = vec * v;
	return vec;
}

Transform::Scale::Scale()
{
	aspect = { 1,1,1 };
	dir = { 1,0,0 };
	verticalDir = { 0,1,0 };
}

void Transform::Scale::setScale(double scale, double xRotate, double yRotate, double zRotate)
{
	setAspect({ scale,scale,scale }, xRotate,yRotate,zRotate);
}

void Transform::Scale::setAspect(const Vec3& aspect, double xRotate, double yRotate, double zRotate)
{
	this->aspect = aspect;
	auto q = Quaternion::RotateX(xRotate) * Quaternion::RotateY(yRotate) * Quaternion::RotateZ(zRotate);
	this->dir = q * Vec3 { 1, 0, 0 };
	this->verticalDir = q * Vec3{ 0,1,0 };
}

std::pair<Vec3, Vec3> Transform::Scale::getDir() const
{
	return std::pair<Vec3, Vec3>(dir,verticalDir);
}

void Transform::Scale::addScale(double scale)
{
	addAspect({ scale,scale,scale });
}

void Transform::Scale::addAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect + aspect);
}

void Transform::Scale::giveScale(double scale)
{
	giveAspect({ scale,scale,scale });
}

void Transform::Scale::giveAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect * aspect);
}

void Transform::Scale::operator=(double scale)
{
	setScale(scale);
}

void Transform::Scale::operator=(const Vec3& aspect)
{
	setAspect(aspect);
}

void Transform::Scale::operator+=(double scale)
{
	addScale(scale);
}

void Transform::Scale::operator+=(const Vec3& aspect)
{
	addAspect(aspect);
}

void Transform::Scale::operator*=(double scale)
{
	giveScale(scale);
}

void Transform::Scale::operator*=(const Vec3& aspect)
{
	giveAspect(aspect);
}

Transform::Direction::Direction()
	:vector({ 1,0,0 })
	, vertical({ 0,1,0 })
	,accum(Quaternion(0,0,0,1))
{
}

std::pair<Vec3, Vec3> Transform::Direction::asPair()const
{
	return std::make_pair(vector, vertical);
}

void Transform::Direction::setDirection(const Vec3& dir, double rad)
{
	rotate(Quaternion::FromUnitVectors(vector, dir));
	if (rad != 0)rotate(vector, rad);
}

void Transform::Direction::rotate(Vec3 axis, double rad)
{
	//rotate(Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
	rotate(Quaternion::RotationAxis(axis, rad));
}

void Transform::Direction::rotate(const Quaternion& qua)
{
	q = qua;
	accum *= qua;
	vector = q * vector;
	vertical = q * vertical;
}
////あえて3次元のrotateは使わない
//void Transform::Direction::rotateXY(double rad)
//{
//	q = Quaternion{ 0 * sin(rad / 2), 0 * sin(rad / 2), 1 * sin(rad / 2), cos(rad / 2) };
//	accum *= qua;
//}

void Transform::setParent(Transform* parent, bool maintainAbsPosition)
{
	relation.setParent(parent);

	m_parent = parent;

	if (not maintainAbsPosition) {
		//今までの絶対座標を相対座標にする
		setLocalPos(getPos());
		setLocalDirection(getDirection());
		setLocalPos(m_parent->direction.accum * getLocalPos());
	}
}

Transform* Transform::getParent()const
{
	return relation.getParent();
}

Array<Transform*> Transform::getChildren()const
{
	return relation.getChildren();
}

void Transform::relaseParent(Transform* parent)
{
	parent->relaseChild(this);
	//if (m_parent == parent)m_parent = nullptr;
}

void Transform::relaseChild(Transform* child)
{
	relation.removeChild(child);
}

Transform* Transform::getParent() {
	return m_parent;
}

Transform::Transform()
	:relation(this)
{
	measureVel = measureDirVel = { 0,0,0 };
	m_parent = nullptr;
}

void Transform::setDirection(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	affectChildren();
}

void Transform::setDirAndPreDir(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	frameDir.vec = frameDir.pre = dir;
}

void Transform::setLocalDirection(const Vec3& dir, double verRad)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setDirection(dir,verRad);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(),verRad);
	affectChildren();
}

void Transform::setLocalDirAndPreDir(const Vec3& dir, double verRad)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setDirAndPreDir(dir);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(), verRad);
	frameDir.vec = frameDir.pre = getDirection();;
}

void Transform::setPosAndPrePos(const Vec3& p)
{
	pos.vec = pos.pre = p;
	framePos.vec = framePos.pre = p;
}

void Transform::setPos(const Vec3& p)
{
	pos.vec = p;
	affectChildren();
}

void Transform::setXY(const Vec2& p)
{
	setPos({ p,getPos().z });
}

void Transform::setLocalXY(const Vec2& p)
{
	setLocalPos({ p,getLocalPos().z });
}

void Transform::setLocalPos(const Vec3& p)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setPos(p);
	}
	else {
		pos.vec = p + parent->getPos();
		affectChildren();
	}
}

void Transform::setLocalPosAndPrePos(const Vec3& p)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setPosAndPrePos(p);
		return;
	}

	pos.vec
		= pos.pre
		= framePos.vec
		= framePos.pre
		= p + parent->getPos();
}

void Transform::setX(double x)
{
	setPos({ x,getPos().yz() });
}

void Transform::setLocalX(double x)
{
	setLocalPos({ x, getLocalPos().yz()});
}

void Transform::setLocalY(double y)
{
	const auto& lp = getLocalPos();
	setLocalPos({lp.x,y,lp.z });
}

void Transform::setLocalZ(double z)
{
	setLocalPos({ getLocalPos().xy(),z });
}

void Transform::setY(double y)
{
	const auto& p = getPos();
	setPos({ p.x,y,p.z });
}

void Transform::setZ(double z)
{
	setPos({ getPos().xy(),z });
}

void Transform::addX(double x)
{
	addPos(getPos() + Vec3{ x,0,0 });
}

void Transform::addY(double y)
{
	addPos(getPos() + Vec3{ 0,y,0 });
}

void Transform::addZ(double z)
{
	setPos(getPos() + Vec3{ 0,0,z });
}

void Transform::addPos(const Vec3& pos)
{
	setPos(getPos() + pos);
}
//
//void Transform::setAbsPos(const Vec3& pos)
//{
//	auto local = (pos - getParent()->getPos());
//
//}

void Transform::moveBy(const Vec3& delta)
{
	pos.vec += delta;
	affectChildren();
}

Vec3 Transform::getAspect()const
{
	auto parent = getParent();
	return parentScalingAffectable and parent != nullptr ? parent->getAspect()*scale.aspect.vec : scale.aspect.vec;
}

std::pair<Vec3,Vec3> Transform::getScaleDir() const
{
	if (not rotatableAspect)return { {1,0,0},{0,1,0} };

	auto parent = getParent();
	auto [dir, ver] = scale.getDir();

	if (parentScalingAffectable and parent != nullptr)
	{
		auto q = Quaternion::FromUnitVectorPairs({ {1,0,0},{0,1,0} }, parent->getScaleDir());
		return { q * dir,q * ver };
	}

	return { dir , ver };
}

Vec3 Transform::getPos()const
{
	auto parent = getParent();
	//親がいなければ座標をそのまま返す
	if (parent == nullptr)return pos.vec;

	auto q = Quaternion::FromUnitVectorPairs({ {1,0,0},{0,1,0} }, parent->getScaleDir());
	return parent->getPos() + q * ((q.inverse() * (pos.vec - parent->pos.vec)) * parent->getAspect());
}

Vec2 Transform::getXY()const
{
	return getPos().xy();
}

Vec3 Transform::getLocalPos()const
{
	auto parent = getParent();
	return parent != nullptr ? getPos() - parent->getPos() : getPos();
}

Vec3 Transform::getVel()const
{
	return measureVel;
}

Vec3 Transform::getDirection()const
{
	return direction.vector;
}

Transform::Direction Transform::get2Direction()const
{
	return direction;
}

Vec3 Transform::getLocalDirection()const
{
	auto parent = getParent();
	if (parent == nullptr)return getDirection();
	//親がいる場合
	auto q = Quaternion::FromUnitVectors(parent->getDirection(), getDirection());
	return q*Vec3{ 1,0,0 };
}

Vec3 Transform::getAngulerVel()const
{
	return measureDirVel;
}


void Transform::affectChildren()
{
	calculate();
	if (affectToChildren)
	{
		for (auto& child : getChildren())
		{
			if (child->followParent)child->moveBy(pos.delta);
			if (child->parentRotationAffectable)child->rotateAt(pos.vec, direction.q);
		}
	}
	direction.q.set(0, 0, 0, 1);
}

void Transform::calculate()
{
	pos.calculate();
}

void Transform::rotateAt(const Vec3& center, Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(center, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotateAt(const Vec3& center,const Quaternion& qua)
{
	direction.rotate(qua);
	pos.vec = qua * (pos.vec - center) + center;
	affectChildren();
}

void Transform::rotate(Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(pos.vec, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotate(const Quaternion& q)
{
	rotateAt(pos.vec, q);
}

void Transform::rotate(const std::pair<Vec3, Vec3>& from, const std::pair<Vec3, Vec3>& to)
{
	rotate(Quaternion::FromUnitVectorPairs(from, to));
}

void Transform::scaleAt(const Vec3& pos, const Vec3& asp)
{
	Vec3 delta = getPos() - pos;
	scale.setAspect(asp);
	setPos(delta * asp + pos);
}

void Transform::scaleAt(const Vec3& pos, double s)
{
	scaleAt(pos, { s,s,s });
}

void Transform::calUpdate(double dt)
{
	if (dt == 0)return;
	//速度を求める
	measureVel = (getPos() - framePos.vec) / dt;
	measureDirVel = (getDirection() - frameDir.vec) / dt;
	//framePos,frameDirの更新
	framePos.vec = getPos();
	frameDir.vec = getDirection();
}

Vec3 Transform::operator+(const Vec3& vec)
{
	return pos + vec;
}

Vec3 Transform::operator+=(const Vec3& vec)
{
	setPos(*this + vec);
	return pos.vec;
}

void Transform::scaleXYAt(const Vec2& _pos, double scale, double rad)
{
	scaleXYAt(_pos, scale, rad);
}

void Transform::scaleXYAt(const Vec2& _pos, const Vec2& scale, double rad)
{
	//Vec2 tmp{ util::invXY(getPos().xy() - _pos).rotate(rad) };
	////現在地を_posを中心に回転した時の座標を得る
	//auto xx = tmp.y;
	//auto yy = tmp.x;

	Vec2 relative = (getPos().xy() - _pos).rotate(rad);

	relative *= scale;

	setXY(relative.rotate(-rad) + _pos);

	//this->scale.aspect.vec.x = scale.x;
	//this->scale.aspect.vec.x = scale.y;
	this->scale.setAspect({ scale,this->scale.aspect.vec.z }, 0, 0, rad);
}