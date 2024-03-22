#pragma once
#include"../Game/Object.h"

namespace motionEditor {
	class Impl;

	class MotionEditor :public Object
	{
	public:
		void start()override;
		void update(double dt);

		Impl* impl;
	};

}
