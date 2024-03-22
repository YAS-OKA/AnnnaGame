#include "../stdafx.h"
#include "Scenes.h"
#include"Object.h"
#include"../Motions/Parts.h"

namespace my
{
	Scene::Scene()
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
		for (auto& ent : entityManager.allEntitys())ent->getComponent<Transform>(U"original")->calUpdate(dt);
	}
}
