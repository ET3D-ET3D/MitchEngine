#include "Animation.h"

namespace ma {
	Animation::Animation() {
	}


	Animation::~Animation() {
	}

	void Animation::Init() {
		FPS = 60.f;
		CurrentFrame = glm::vec2(0, 0);
		GridSize = glm::vec2(1, 1);
		FramesAccumulated = 0;
	}

	void Animation::SetAnimationInfo(int InWidth, int InHeight, int InColumns, int InRows) {
		CurrentFrame = glm::vec2(2, 4);
		GridSize = glm::vec2(InColumns, InRows);
	}

}