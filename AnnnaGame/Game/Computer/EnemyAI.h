#pragma once
#include"../StateMachine/Inform.h"

class Enemy;

class EnemyAIProvider
{
	EnemyAIProvider();
	~EnemyAIProvider() {}

	static EnemyAIProvider* instance;
public:
	HashTable<String, std::function<void(Enemy* e, state::Inform&& info)>> aiList;

	static void Init();

	static void Destroy();

	static void Set(StringView name, Enemy* e, state::Inform&& info);
};
