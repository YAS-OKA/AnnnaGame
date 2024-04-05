#pragma once
#include "../../EC.hpp"

namespace mot
{
	class PartsManager;
}

class PartsMirrored:public Component
{
private:
	double scale = 1;
	int32 firstScale;
	bool active;
	class Transform* transform;//これの方向をもとにする

	double mirroredTime = 0.2;
	double timer = 0;
public:
	mot::PartsManager* pman;
	//パーツマネージャー　最初から反転するか
	PartsMirrored(mot::PartsManager* pman,bool mirrorStart = false);

	void start()override;

	void update(double dt)override;
};
