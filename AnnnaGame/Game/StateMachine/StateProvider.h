#pragma once
#include"../../Prg/Actions.h"
#include"Inform.h"
#define F(x) std::forward<StateActions>(x)

namespace state
{
	using In = Inform&;
	using A = StateActions&&;

	class StateCreator
	{
	public:
		StateCreator() = default;
		StateCreator(StringView name,
			const std::function<StateActions (In, StateActions&&)>& method = [](In, StateActions&& a) {return std::forward<StateActions>(a); })
			:method(method), name(name)
		{};

		StateCreator& operator = (const std::function<StateActions (In, StateActions&&)>& method)
		{
			this->method = method;
			return *this;
		}

		String name;

		std::function<StateActions (In, StateActions&&)> method;

		StateActions create(StringView name)const;

		StateActions operator()(In info) const
		{
			return F(method(info, std::move(create(name))));
		}
	};

	class SCreatorContainer
	{
	public:
		HashTable<String,StateCreator> creators;

		bool contains(StringView name) { return creators.contains(name); }

		StateCreator& operator[](StringView name)
		{
			if (not creators.contains(name))creators.emplace(name, name);
			return creators[name];
		}

		const StateCreator& operator[](StringView name)const
		{
			return creators.at(name);
		}
	};

	class StateProvider
	{
		StateProvider();
		~StateProvider() {};

		static StateProvider* instance;

		SCreatorContainer dict;
	public:
		static void Init();

		static void Destroy();

		static StateActions&& Get(StringView name, In info);
	};
}
