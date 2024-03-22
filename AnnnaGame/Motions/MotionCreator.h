#pragma once
#include"../EC.hpp"
#include"../Game/Scenes.h"

namespace mot
{
	class PartsEditor :public my::Scene
	{
	public:
		void start()override;
		class Impl;
		Impl* impl;
	};

	class MotionCreator :public my::Scene
	{
	public:
		void start()override;
		class Impl;
		Impl* impl;
	};
}
