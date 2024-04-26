#pragma once
#include"../StateMachine/Inform.h"
#include"../../Util/Borrow.h"

class Enemy;

class EnemyAIProvider
{
	EnemyAIProvider();
	~EnemyAIProvider() {}

	static EnemyAIProvider* instance;
public:
	HashTable<String, std::function<void(const Borrow<Enemy>& e, state::Inform&& info)>> aiList;

	static void Init();

	static void Destroy();

	static void Set(StringView name, const Borrow<Enemy>& e, state::Inform&& info);
};
