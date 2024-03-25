#pragma once
#include"../../Prg/Actions.h"
#include"Inform.h"

namespace state
{
	using In = const Inform&;

	class StateCreator
	{
	public:
		StateCreator() = default;
		StateCreator(StringView name,
			const std::function<Actions && (In, Actions&&)>& method = [](In, Actions&& a) {return std::forward<Actions>(a); })
			:method(method), name(name)
		{};

		StateCreator& operator = (const std::function<Actions && (In, Actions&&)>& method)
		{
			this->method = method;
			return *this;
		}

		String name;

		std::function<Actions&& (In, Actions&&)> method;

		Actions&& create(StringView name);

		Actions&& operator()(In info)
		{
			return std::forward<Actions>(method(info, create(name)));
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

		static Actions&& Get(StringView name, const Inform& info);
	};
}
