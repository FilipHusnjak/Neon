#pragma once

namespace Neon
{
	class PhysicsMaterial : public RefCounted
	{
	public:
		static SharedRef<PhysicsMaterial> CreateMaterial(float staticFriction = 1.f, float dynamicFriction = 0.7f,
														 float restitution = 0.5f, float density = 10.f);

	public:
		float StaticFriction;
		float DynamicFriction;
		float Restitution;
		float Density;

	public:
		PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution, float density);
		virtual void* GetHandle() const = 0;
	};
} // namespace Neon
