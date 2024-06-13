#pragma once
#include"../Game/Object.h"

namespace bodyEditor {
	class Impl;

	class BodyEditor :public Object
	{
	public:
		void start()override;
		void update(double dt);

		Impl* impl;
	};

}
