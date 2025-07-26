#pragma once
#include "Cfu.hpp"

namespace Reactor
{
	class Movement
	{
		glm::vec3 requestedMoveVel = { 0, 0, 0 };
		glm::vec3 requestedPanAvel = { 0, 0, 0 };

		glm::vec3 center = { 0, 0, 0 };
		glm::vec3 forward = { 1, 0, 0 };
		glm::vec3 upward = { 0, 1, 0 };

	public:
		void Tick(glm::ivec3 moveAccelWeight, glm::ivec3 panAccelWeight);
		void Pan(glm::vec3 panWeight);
		void Warp(glm::vec3 newCenter, glm::vec3 newForward, glm::vec3 newUpward);

		Cfu GetCfu() const;
	};
}
