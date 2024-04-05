#pragma once
#include<Siv3D.hpp>
#include"Util/PCTree.hpp"
#include"Util/TypeContainer.hpp"

class Entity;

class Priority
{
	//値が大きい方が先に更新される
	double priority = 0;
public:
	double getPriority() const { return priority; };
	void setPriority(double priority) { Priority::priority = priority; };
};

//コンポーネントの基底クラス
class Component
{
public:
	Component() {}
	virtual ~Component() {}
	virtual void start() {}
	virtual void update(double dt) {}

	Priority priority;

	Entity* owner = nullptr;
	//追加番号　priorityが等しい場合、これをもとにソート
	int32 index;

private:
	friend Entity;

	std::type_index compType{ typeid(Component) };
	String id{U""};
};

class EntityManager;

//コンポーネントを適用した基底クラス
//HashTableを使っているので計算量は基本O(1)？
//同一の型のコンポーネントを追加することは可能　その場合idを指定しないとGetComponentはできない（バグ防止）
//型が重複してるならGetComponentArrを使うことを推奨
class Entity
{
public:
	EntityManager* owner;

	util::PCRelationship<Entity> relation;

	HashSet<Entity*> sameDestiny;

	Entity()
		:relation(this), owner(nullptr)
	{}
	//killされた直後実行する
	virtual void onTrashing() {};

	virtual void start() {};

	virtual void update(double dt) {};

	//相手が死ぬと自分も死ぬ。自分が死ぬと相手も死ぬ
	void setSameDestiny(Entity* other)
	{
		sameDestiny.emplace(other);
		other->sameDestiny.emplace(this);
	}
	//相手が死ぬと自分も死ぬ
	void followDestiny(Entity* owner)
	{
		relation.setParent(owner);
	}

	//コンポーネントをアップデート
	virtual void update_components(double dt)
	{
		if (components.garbages.size() > 0)components.deleteGarbages();

		std::stable_sort(allComponentForUpdate.begin(), allComponentForUpdate.end(), [=](const Component* com1, const Component* com2) {return _replace_flag(com1, com2); });

		for (auto& com : allComponentForUpdate)com->update(dt);
	};

	//一致するコンポーネントを削除 下のオーバーロードの関数を呼び出す
	void remove(Component* com)
	{
		remove(com->compType, com->id);
	}
	//コンポーネントの削除 コンポーネントの型が重複している場合下のif文でid指定で消す 削除したコンポーネントはgarbagesへ
	template<class T>
	void remove(const String& id = U"")
	{
		if (id == U"")
		{
			const auto& remSet = components.remove<T>();

			for (auto itr = allComponentForUpdate.begin(); itr != allComponentForUpdate.end();)
			{
				if (remSet.contains(*itr))
				{
					itr = allComponentForUpdate.erase(itr);
					--componentsNum;
				}
				else
					++itr;
			}
		}

		remove(typeid(T), id);
	}

	void remove(const std::type_index& compType, const String& id)
	{
		auto rem = components.remove(compType, id);
		//nullptr だったら終了
		if (not rem)return;
		
		for (auto itr = allComponentForUpdate.begin(),en = allComponentForUpdate.end();itr!=en;++itr)
		{
			if (rem==*itr)
			{
				allComponentForUpdate.erase(itr);
				--componentsNum;
				return;
			}
		}
	}

	//コンポーネントの追加　idがかぶったら上書き
	template<class T, class... Args>
	T* addComponentNamed(const String& id, Args&&... args)
	{
		auto component = components.addIn<T>(id, args...);
		component->owner = this;
		component->start();
		component->index = componentsNum;
		component->compType = typeid(T);
		component->id = id;
		allComponentForUpdate << component;
		componentsNum++;
		return component;
	}
	//コンポーネントの追加
	template<class T,class... Args>
	T* addComponent(Args&&... args)
	{
		auto component = components.add<T>(args...);
		component->owner = this;
		component->start();
		component->index = componentsNum;
		component->compType = typeid(T);
		component->id = Format(int32(components.ids[typeid(T)]) - 1);
		componentsNum++;
		allComponentForUpdate << component;
		return component;
	}
	
	//コンポーネントの取得　型の重複はなし
	template<class T>
	T* getComponent()
	{
		return components.get<T>();
	}

	template<class T>
	const T* getComponent() const
	{
		return components.get<T>();
	}

	//コンポーネントの取得 id指定。使いどころは複数のコンポーネントが同一の型で重複しているときとか。
	template<class T>
	T* getComponent(const String& id)
	{
		return components.get<T>(id);
	}

	String name = U"";

	Priority priority;
private:
	TypeContainer<Component> components;
	int32 componentsNum = 0;
	//すべてのコンポーネントをここにぶち込む priorityでソートするため 所有権は持たない
	Array<Component*> allComponentForUpdate;
	
	//優先度で入れ替えを行うかどうか
	bool _replace_flag(const Component* s, const Component* other)
	{
		if (s->priority.getPriority() != other->priority.getPriority())return s->priority.getPriority() > other->priority.getPriority();
		return s->index < other->index;
	};
	
	template<class T>
	Optional<String> _get_id(Component* com)
	{
		if (not components.arr.contains(typeid(T)))return none;
		for (const auto& component : components.arr[typeid(T)])
		{
			if (component.second == com)
			{
				return component.first;
			}
		}
		return none;
	}
};

class EntityManager{
	using EntityContainer= Array<std::pair<std::type_index, Entity*>>;

	EntityContainer pre;
	EntityContainer entitys;
	Array<Entity*> garbages;
	HashSet<Entity*> trashList;
private:
	EntityContainer MargedEntitys() {
		EntityContainer ret;
		return ret.append(entitys).append(pre);
	};

	void freshGarbages(){
		if (garbages.isEmpty())return;
		for (auto& ent : garbages)delete ent;
		garbages.clear();
	}

public:
	~EntityManager() {
		clean();
		freshGarbages();
	}

	void update(double dt) {
		//消去
		freshGarbages();

		//preからentitysに移す
		entitys.append(pre);
		pre.clear();

		//ソート
		std::stable_sort(
			entitys.begin(),
			entitys.end(),
			[=](const auto ent1, const auto ent2) {return ent1.second->priority.getPriority() > ent2.second->priority.getPriority(); }
		);

		//更新
		for (auto& entity : entitys){
			entity.second->update(dt);
			entity.second->update_components(dt);
		}

		//killされたentityをがーべじへ
		for (auto it = entitys.begin(); it != entitys.end();)	{
			if (trashList.contains(it->second))	{
				garbages << it->second;
				it = entitys.erase(it);
			}
			else {
				++it;
			}			
		}
		for (auto it = pre.begin(); it != pre.end();)	{
			if (trashList.contains(it->second)) {
				garbages << it->second;
				it = pre.erase(it);
			}
			else {
				++it;
			}
		}
		trashList.clear();
	}

	Array<Entity*> allEntitys() {
		Array<Entity*> result;
		for (auto& ent : MargedEntitys())result << ent.second;
		return result;
	};

	template<class T>
	Array<T*> find(Optional<String> name=none)
	{
		Array<T*> result;
		std::type_index info = typeid(T);
		for (auto& ent : MargedEntitys())
		{
			if (ent.first == info)
			{
				//名前が指定されていない場合
				if (!name)result << static_cast<T*>(ent.second);
				//名前が指定されている場合
				else if (*name == ent.second->name)result << static_cast<T*>(ent.second);
			}
		}
		return result;
	}

	template<class T>
	T* findOne(Optional<String> name=none)
	{
		auto ent = find<T>(name);
		if (ent.isEmpty())return nullptr;
		else return ent[0];
	}

	//Entityを作る
	template<class T = Entity, class... Args>
	T* birth(Args&&... args) {
		auto entity = new T(args...);
		pre << std::pair<std::type_index, Entity*>(typeid(T), entity);
		entity->owner = this;
		entity->start();
		return entity;
	}

	template<class T = Entity, class... Args>
	T* birthNonStart(Args&&... args) {
		auto entity = new T(args...);
		pre << std::pair<std::type_index, Entity*>(typeid(T), entity);
		entity->owner = this;
		return entity;
	}

	//すべてガーベージに
	void clean()
	{
		for (auto& ent : MargedEntitys())garbages << ent.second;
		pre.clear();
		entitys.clear();
	}

	bool isKilled(Entity* entity)const
	{
		return trashList.contains(entity) or garbages.contains(entity);
	}

	//一致するEntityをgarbagesへ
	void kill(Entity* ent) {
		//子をkillする
		for (auto& child: ent->relation.getChildren())
		{
			kill(child);
		}
		//一蓮托生もkillする
		for (auto& other : ent->sameDestiny)
		{
			other->sameDestiny.erase(ent);
			kill(other);
		}
		//trashListへ
		if (not trashList.contains(ent)) {
			ent->onTrashing();
			trashList.emplace(ent);
		}
	}
	//名前が一致するEntityをgarbagesへ
	void kill(const String& name)
	{
		for (auto it = entitys.begin(), en = entitys.end(); it != en;++it)
		{
			if ((*it).second->name==name) {
				kill((*it).second);
			}
		}
		for (auto it = pre.begin(), en = pre.end(); it != en;++it)
		{
			if ((*it).second->name == name) {
				kill((*it).second);
			}
		}
	}
};

static bool IsKilled(Entity* entity)
{
	return entity->owner->isKilled(entity);
}
