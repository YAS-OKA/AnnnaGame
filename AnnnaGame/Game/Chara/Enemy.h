#pragma once
#include"Character.h"

class Enemy :public Character
{
public:
	using Character::Character;

	void start()override;
};

class EnemyFactory
{
public:
	using ptr = Borrow<Enemy>;

	static void LoadCreator();

	static HashTable<String, std::function<ptr(ptr)>> m_creator;

	Borrow<my::Scene> m_scene;

	EnemyFactory(const Borrow<my::Scene>& scene);

	Borrow<Enemy> create(StringView name);
};
