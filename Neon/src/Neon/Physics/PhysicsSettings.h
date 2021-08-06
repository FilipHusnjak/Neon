#pragma once

#include <glm/glm.hpp>

namespace Neon
{
	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	enum class DebugType : int8_t
	{
		DebugToFile = 0,
		LiveDebug = 1
	};

	struct PhysicsSettings
	{
		float FixedTimestep = 1.0f / 60.f;
		glm::vec3 Gravity = {0.f, -9.81f, 0.f};
		BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		glm::vec3 WorldBoundsMin = glm::vec3(0.f);
		glm::vec3 WorldBoundsMax = glm::vec3(1.f);
		uint32_t WorldBoundsSubdivisions = 2;
		FrictionType FrictionModel = FrictionType::Patch;
		uint32_t SolverIterations = 6;
		uint32_t SolverVelocityIterations = 1;
		bool DebugOnPlay = true;
		DebugType DebugType = DebugType::LiveDebug;
	};
}
