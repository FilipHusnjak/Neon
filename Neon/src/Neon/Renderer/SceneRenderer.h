#pragma once

#include "Neon/Renderer/Camera.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Scene.h"

namespace Neon
{
	struct SceneRendererCamera
	{
		Neon::Camera Camera;
		glm::mat4 ViewMatrix;
		float Near;
		float Far;
		float FOV;
	};

	class SceneRenderer
	{
	public:
		static void Init();

		static void SetViewportSize(uint32 width, uint32 height);

		static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		static void EndScene();

		static void SubmitMesh(SharedRef<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));

		static const SharedRef<RenderPass>& GetGeoPass();

		static void SetFocusPoint(const glm::vec2& point);

		static void* GetFinalImageId();

		static void OnImGuiRender();

		static void Shutdown();

	private:
		static void FlushDrawList();
		static void GeometryPass();
	};
} // namespace Neon
