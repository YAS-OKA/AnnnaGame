#pragma once
#include"../../Util/Util.h"

namespace sys
{
	//属性
	struct Attribute
	{
		double fire;//火
		double water;//水
		double soil;//地
		double plant;//植物
		double animal;//動物

		void printParams()const;
	};

	//HPとかの構造 maxを超えたらtrue
	struct PassMax
	{
		double value;
		double max;

		PassMax() {};

		PassMax(double threshold, double value = 0);

		void add(double value);

		bool flag()const;

		double residual()const;
	};

	//次のlevelに行くために必要な経験値量を返す
	using PassMaxGenerator = std::function<double(int32 level)>;

	//レベル系の構造
	struct Level
	{
		//現在の経験値
		PassMax exp;

		PassMaxGenerator _generator;
		//レベル
		util::StopMax level;

		Level() {};

		Level(double level,	double max, const PassMaxGenerator& generator);

		bool able_to_up()const;

		bool level_up();
	};
}
