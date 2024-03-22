#include"GameSystem.h"

void sys::Attribute::printParams() const
{
	Print << U"fire:" << fire;
	Print << U"water:" << water;
	Print << U"soil:" << soil;
	Print << U"plang:" << plant;
	Print << U"animal:" << animal;
}

sys::PassMax::PassMax(double threshold, double value)
	:max(threshold), value(value)
{}

void sys::PassMax::add(double additional)
{
	value += additional;
}

bool sys::PassMax::flag()const
{
	return value >= max;
}

double sys::PassMax::residual()const
{
	return max - value;
}

sys::Level::Level(double level, double max, const std::function<double(int32)>& gene)
	:level(max, level), _generator(gene), exp(gene(level + 1))
{}

bool sys::Level::able_to_up()const
{
	return exp.flag() and level.additionable();
}

bool sys::Level::level_up()
{
	if (not able_to_up())return false;

	//レベルアップ
	level.add(1);
	//次のレベルに行くための経験値量をセット
	auto ex_value = -exp.residual();
	exp.max = _generator(level.value + 1);
	//超過expを加算
	exp.add(ex_value);

	return true;
}
