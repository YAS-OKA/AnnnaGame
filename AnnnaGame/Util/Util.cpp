#include"../stdafx.h"
#include"Util.h"

namespace util
{
	StopMax::StopMax(double max, double value)
		:value(value), max(max) {}

	void StopMax::add(double additional)
	{
		value += additional;
		value = value > max ? max : value;
	}

	bool StopMax::additionable()const
	{
		return value < max;
	}

}
