#include "neopch.h"

#include "Neon/Renderer/MeshFactory.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Neon
{
	SharedRef<StaticMesh> MeshFactory::CreateBox(const glm::vec3& size)
	{
		std::vector<StaticMesh::Vertex> vertices;
		vertices.resize(8);
		vertices[0].Position = {-size.x, -size.y, size.z};
		vertices[1].Position = {size.x, -size.y, size.z};
		vertices[2].Position = {size.x, size.y, size.z};
		vertices[3].Position = {-size.x, size.y, size.z};
		vertices[4].Position = {-size.x, -size.y, -size.z};
		vertices[5].Position = {size.x, -size.y, -size.z};
		vertices[6].Position = {size.x, size.y, -size.z};
		vertices[7].Position = {-size.x, size.y, -size.z};

		vertices[0].Normal = {-1.f, -1.f, 1.f};
		vertices[1].Normal = {1.f, -1.f, 1.f};
		vertices[2].Normal = {1.f, 1.f, 1.f};
		vertices[3].Normal = {-1.f, 1.f, 1.f};
		vertices[4].Normal = {-1.f, -1.f, -1.f};
		vertices[5].Normal = {1.f, -1.f, -1.f};
		vertices[6].Normal = {1.f, 1.f, -1.f};
		vertices[7].Normal = {-1.f, 1.f, -1.f};

		std::vector<Index> indices;
		indices.resize(12);
		indices[0] = {0, 1, 2};
		indices[1] = {2, 3, 0};
		indices[2] = {1, 5, 6};
		indices[3] = {6, 2, 1};
		indices[4] = {7, 6, 5};
		indices[5] = {5, 4, 7};
		indices[6] = {4, 0, 3};
		indices[7] = {3, 7, 4};
		indices[8] = {4, 5, 1};
		indices[9] = {1, 0, 4};
		indices[10] = {3, 2, 6};
		indices[11] = {6, 7, 3};

		return SharedRef<StaticMesh>::Create("BoxMesh", vertices, indices);
	}

	SharedRef<StaticMesh> MeshFactory::CreateSphere(float radius)
	{
		std::vector<StaticMesh::Vertex> vertices;
		std::vector<Index> indices;

		constexpr float latitudeBands = 10.f;
		constexpr float longitudeBands = 10.f;

		for (float latitude = 0.f; latitude <= latitudeBands; latitude++)
		{
			const float theta = latitude * (float)M_PI / latitudeBands;
			const float sinTheta = glm::sin(theta);
			const float cosTheta = glm::cos(theta);

			for (float longitude = 0.f; longitude <= longitudeBands; longitude++)
			{
				const float phi = longitude * 2.f * (float)M_PI / longitudeBands;
				const float sinPhi = glm::sin(phi);
				const float cosPhi = glm::cos(phi);

				StaticMesh::Vertex vertex;
				vertex.Normal = {cosPhi * sinTheta, cosTheta, sinPhi * sinTheta};
				vertex.Position = {radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z};
				vertices.push_back(vertex);
			}
		}

		for (uint32 latitude = 0; latitude < (uint32)latitudeBands; latitude++)
		{
			for (uint32 longitude = 0; longitude < (uint32)longitudeBands; longitude++)
			{
				const uint32 first = (latitude * ((uint32)longitudeBands + 1)) + longitude;
				const uint32 second = first + (uint32)longitudeBands + 1;

				indices.push_back({first, first + 1, second});
				indices.push_back({second, first + 1, second + 1});
			}
		}

		return SharedRef<StaticMesh>::Create("SphereMesh", vertices, indices);
	}

	static void CalculateRing(size_t segments, float radius, float x, float dx, float height, float actualRadius,
							  std::vector<StaticMesh::Vertex>& vertices)
	{
		float segIncr = 1.f / (float)(segments - 1);
		for (size_t s = 0; s < segments; s++)
		{
			float y = glm::cos(float(M_PI * 2) * s * segIncr) * radius;
			float z = glm::sin(float(M_PI * 2) * s * segIncr) * radius;

			StaticMesh::Vertex& vertex = vertices.emplace_back();
			vertex.Position = glm::vec3(actualRadius * x + height * dx, actualRadius * y, actualRadius * z);
		}
	}

	SharedRef<StaticMesh> MeshFactory::CreateCapsule(float radius, float height)
	{
		constexpr size_t subdivisionsHeight = 8;
		constexpr size_t ringsBody = subdivisionsHeight + 1;
		constexpr size_t ringsTotal = subdivisionsHeight + ringsBody;
		constexpr size_t numSegments = 12;
		constexpr float radiusModifier = 0.021f; // Needed to ensure that the wireframe is always visible

		std::vector<StaticMesh::Vertex> vertices;
		std::vector<Index> indices;

		vertices.reserve(numSegments * ringsTotal);
		indices.reserve((numSegments - 1) * (ringsTotal - 1) * 2);

		float bodyIncr = 1.f / (float)(ringsBody - 1);
		float ringIncr = 1.f / (float)(subdivisionsHeight - 1);

		for (int r = 0; r < subdivisionsHeight / 2; r++)
		{
			CalculateRing(numSegments, glm::sin(float(M_PI) * r * ringIncr), glm::sin(float(M_PI) * (r * ringIncr - 0.5f)), -0.5f,
						  height, radius + radiusModifier, vertices);
		}

		for (int r = 0; r < ringsBody; r++)
		{
			CalculateRing(numSegments, 1.f, 0.f, r * bodyIncr - 0.5f, height, radius + radiusModifier, vertices);
		}

		for (int r = subdivisionsHeight / 2; r < subdivisionsHeight; r++)
		{
			CalculateRing(numSegments, glm::sin(float(M_PI) * r * ringIncr), glm::sin(float(M_PI) * (r * ringIncr - 0.5f)), 0.5f,
						  height, radius + radiusModifier, vertices);
		}

		for (int r = 0; r < ringsTotal - 1; r++)
		{
			for (int s = 0; s < numSegments - 1; s++)
			{
				Index& index1 = indices.emplace_back();
				index1.V1 = (uint32)(r * numSegments + s + 1);
				index1.V2 = (uint32)(r * numSegments + s + 0);
				index1.V3 = (uint32)((r + 1) * numSegments + s + 1);

				Index& index2 = indices.emplace_back();
				index2.V1 = (uint32)((r + 1) * numSegments + s + 0);
				index2.V2 = (uint32)((r + 1) * numSegments + s + 1);
				index2.V3 = (uint32)(r * numSegments + s);
			}
		}

		return SharedRef<StaticMesh>::Create("CapsuleMesh", vertices, indices);
	}
} // namespace Neon
