#pragma once

#include "Neon/Renderer/Camera.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Entity.h"

namespace Neon
{
	struct SceneRendererCamera
	{
		Neon::Camera Camera;
		float Near;
		float Far;
		float FOV;
	};

	class SceneRenderer
	{
	public:
		static void Init();

		static void InitializeScene(SharedRef<Scene> scene);
		const static SharedRef<Scene>& GetActiveScene();

		static void SetSelectedEntity(Entity entity);
		static Entity GetSelectedEntity();

		static void SetViewportSize(uint32 width, uint32 height);

		static void BeginScene(const SceneRendererCamera& camera);
		static void EndScene();

		static void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f));

		static const SharedRef<RenderPass>& GetGeoPass();

		static const SharedRef<TextureCube>& GetRadianceTex();
		static const SharedRef<TextureCube>& GetIrradianceTex();
		static const SharedRef<Texture2D>& GetBRDFLUTTex();

		static void SetFocusPoint(const glm::vec2& point);

		static void* GetFinalImageId();

		static void OnImGuiRender();

		static void CreateEnvironmentMap(const std::string& filepath);

		static void Shutdown();

	private:
		static void FlushDrawList();
		static void GeometryPass();
		static void PostProcessingPass();
	};
} // namespace Neon
