#include "Movement.hpp"
#include "Common/Log.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <numbers>

namespace Reactor
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("movement");

		constexpr float movementVelMax  = 25.f;
		constexpr float movementAcc     = 100.f;
		constexpr float rotationAvelMax = 0.25f;
		constexpr float rotationAacc    = 1.0f;
		constexpr float tickInterval    = 0.01f;

		constexpr auto pi = std::numbers::pi_v<float>;

		template<class Item>
		Item ClampMagnitude(Item item, float maxMagnitude)
		{
			float magnitude = glm::length(item);
			if (maxMagnitude < magnitude)
			{
				item *= maxMagnitude / magnitude;
			}
			return item;
		}
	}

	void Movement::Tick(glm::ivec3 moveAccelWeight, glm::ivec3 panAccelWeight)
	{
		auto rotationAaccTick   = rotationAacc    * tickInterval;
		auto movementAccTick    = movementAcc     * tickInterval;
		auto movementVelMaxTick = movementVelMax  * tickInterval;
		auto rightward = glm::cross(forward, upward);

		requestedPanAvel.x += ClampMagnitude(panAccelWeight.x * rotationAvelMax - requestedPanAvel.x, rotationAaccTick);
		requestedPanAvel.y += ClampMagnitude(panAccelWeight.y * rotationAvelMax - requestedPanAvel.y, rotationAaccTick);
		requestedPanAvel.z += ClampMagnitude(panAccelWeight.z * rotationAvelMax - requestedPanAvel.z, rotationAaccTick);
		if (glm::length(requestedPanAvel) > 0)
		{
			Pan(requestedPanAvel * -tickInterval);
		}

		requestedMoveVel.x += ClampMagnitude(moveAccelWeight.x * movementVelMax - requestedMoveVel.x, movementAccTick);
		requestedMoveVel.y += ClampMagnitude(moveAccelWeight.y * movementVelMax - requestedMoveVel.y, movementAccTick);
		requestedMoveVel.z += ClampMagnitude(moveAccelWeight.z * movementVelMax - requestedMoveVel.z, movementAccTick);
		{
			auto planarVelTick = ClampMagnitude(requestedMoveVel * tickInterval, movementVelMaxTick);
			center += forward   * planarVelTick.x;
			center += upward    * planarVelTick.y;
			center += rightward * planarVelTick.z;
		}

		// lrh("(", center[0], ", ", center[1], ", ", center[2], "), (", forward[0], ", ", forward[1], ", ", forward[2], "), (", upward[0], ", ", upward[1], ", ", upward[2], ")");
	}

	void Movement::Pan(glm::vec3 panWeight)
	{
		auto rightward = glm::cross(forward, upward);
		auto panPlanar = rightward * panWeight.x + upward * panWeight.y;
		auto kernel = glm::cross(forward, panPlanar) + forward * panWeight.z;
		auto magnitude = glm::length(panWeight);
		if (magnitude)
		{
			auto angle = magnitude * 2 * pi;
			forward = glm::rotate(glm::mat4(1.0), angle, kernel) * glm::vec4(forward, 1.f);
			upward  = glm::rotate(glm::mat4(1.0), angle, kernel) * glm::vec4(upward , 1.f);
			// TODO: maybe normalize?
		}
	}

	void Movement::Warp(glm::vec3 newCenter, glm::vec3 newForward, glm::vec3 newUpward)
	{
		center = newCenter;
		forward = glm::normalize(newForward);
		auto rightward = glm::cross(forward, newUpward);
		upward = glm::normalize(glm::cross(rightward, forward));
	}

	Cfu Movement::GetCfu() const
	{
		return glm::mat3(center, forward, upward);
	}
}
