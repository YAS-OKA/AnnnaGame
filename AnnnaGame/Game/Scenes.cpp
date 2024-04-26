#include "../stdafx.h"
#include "Scenes.h"
#include"Object.h"
#include"../Motions/Parts.h"

namespace my
{
	void createCollider(Entity* ent, const Box& shape, const Vec3& relative)
	{
		auto col = ent->addComponentNamed<Collider>(U"hitbox", shape, relative);
		auto box = col->hitbox.boudingBox();
		auto s = shape;
		s.h = Min(1.0, shape.h * 0.05);
		s.w = shape.w * 0.7;
		s.d = shape.d * 0.7;
		ent->addComponentNamed<Collider>(U"bottom", s, box.bottomCenter());
		ent->addComponentNamed<Collider>(U"top", s, box.topCenter());
	}
	void createCollider(Entity* ent, const Cylinder& shape, const Vec3& relative)
	{
		auto col = ent->addComponentNamed<Collider>(U"hitbox", shape, relative);
		auto box = col->hitbox.boudingBox();
		auto s = shape;
		s.h = Min(1.0, shape.h * 0.05);
		s.r = shape.r * 0.7;
		ent->addComponentNamed<Collider>(U"bottom", s, box.bottomCenter());
		ent->addComponentNamed<Collider>(U"top", s, box.topCenter());
	}

	void Scene::hitboxVisible(const MeshData& data)
	{
	}
	Scene::Scene()
		:drawManager(ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve())
	{
		partsLoader = new mot::LoadParts(this);
	}

	Scene::~Scene()
	{
		delete partsLoader;
	}

	EntityManager* Scene::getEntityManager()
	{
		return &entityManager;
	}

	DrawManager* Scene::getDrawManager()
	{
		return &drawManager;
	}

	void Scene::update(double dt)
	{
		entityManager.update(dt);

		drawManager.update();

		drawManager.draw();
	}

	void Scene::updateTransform(double dt)
	{
		for (auto& t : entitysTransform)t.second->calUpdate(dt);
		//for (auto& ent : entityManager.allEntitys())ent->getComponent<Transform>(U"original")->calUpdate(dt);
	}
}
