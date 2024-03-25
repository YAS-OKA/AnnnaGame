#pragma once

namespace state
{
	class IV {};

	template<class T>
	class Value :public IV
	{
	public:
		Value(Value&&) = default;
		Value(T&& v) :v(std::forward<T>(v)) {}
		T v;

		operator T ()
		{
			return v;
		}
	};

	class Info {
		IV* v = nullptr;
	public:
		Info() {};
		Info(Info&&) = default;

		template<class V>
		Info(V v)
			:v(new Value<V>(v))
		{
		}

		template<class V>
		Info(V&& v)
			: v(new Value<V>(std::forward<V>(v)))
		{
		}

		~Info() { if (v)delete v; }

		template<class V>
		operator V()
		{
			return *dynamic_cast<Value<V>*>(v);
		};
	};


	class Inform
	{
		HashTable<String, Info> info;
	public:
		void set(StringView name, Info value);

		Info get(StringView name, Info&& default_value);
	};
}
