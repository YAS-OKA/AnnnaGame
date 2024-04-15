#pragma once
#include"../EC.hpp"

//フィールド 汎用的
template<class T>
class Field :public Component
{
public:
	Field(){}
	Field(const T& init)
	{
		value = init;
	}

	operator T()
	{
		return value;
	}

	T value;
};
