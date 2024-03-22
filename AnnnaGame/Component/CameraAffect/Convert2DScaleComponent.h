#pragma once
#include"../../EC.hpp"

#include"../../Game/Utilities.h"

class Convert2DScaleComponent:public Component
{
public:
	util::Convert2DScale converter;

	Transform* target;

	Transform* ownerTransform;

	Convert2DScaleComponent(Transform* convTarget, const util::Convert2DScale& converter);

	Convert2DScaleComponent(Transform* convTarget, double baseLength,DrawManager* dm, const Camera::DistanceType& type = Camera::DistanceType::Screen);

	void convert();

	void start()override;

	void update(double dt)override;	
};

