#pragma once

namespace Neon
{
	class PhysicsMaterial : public RefCounted
	{
	public:
		float StaticFriction;
		float DynamicFriction;
		float Restitution;

	public:
		PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);
		virtual void* GetHandle() const = 0;
	};
} // namespace Neon
