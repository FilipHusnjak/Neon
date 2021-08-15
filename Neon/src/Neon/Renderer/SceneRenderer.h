#pragma once

#include "Neon/Renderer/Camera.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/LightComponent.h"

namespace Neon
{
	class SceneRenderer
	{
	public:
		static void Init();

		static void InitializeScene(SharedRef<Scene> scene);
		const static SharedRef<Scene>& GetActiveScene();

		static void SetSelectedActor(SharedRef<Actor> actor);
		static SharedRef<Actor> GetSelectedActor();

		template<typename T, typename... Args>
		static SharedRef<Actor> CreateActor(UUID uuid, const std::string& name, Args&&... args)
		{
			NEO_CORE_ASSERT(s_Data.ActiveScene);
			return s_Data.ActiveScene->CreateActor<T>(uuid, name, args...);
		}

		static void DestroyActor(SharedRef<Actor> actor);

		static void SetViewportSize(uint32 width, uint32 height);

		static void BeginScene(Camera* camera);
		static void EndScene();

		static void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f), bool wireframe = false);
		static void SubmitLight(const Light& light);

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

	private:
		struct SceneRendererData
		{
			SharedRef<Scene> ActiveScene = nullptr;
			SharedRef<Actor> SelectedActor = {};

			std::vector<Light> Lights;

			struct SceneInfo
			{
				Camera* SceneCamera;
				std::string EnvironmentPath;
			} SceneData;

			SharedRef<RenderPass> GeoPass;
			SharedRef<RenderPass> PostProcessingPass;

			SharedRef<Shader> PostProcessingShader;
			SharedRef<GraphicsPipeline> PostProcessingPipeline;

			SharedRef<TextureCube> EnvUnfilteredTextureCube;
			SharedRef<TextureCube> EnvFilteredTextureCube;
			SharedRef<TextureCube> IrradianceTextureCube;

			glm::vec2 FocusPoint = {0.5f, 0.5f};

			struct MeshDrawCommand
			{
				SharedRef<Mesh> Mesh;
				glm::mat4 Transform;
				bool Wireframe;
			};

			std::vector<MeshDrawCommand> MeshDrawList;

			Material SkyboxMaterial;
			SharedRef<GraphicsPipeline> SkyboxGraphicsPipeline;

			SharedRef<Shader> EnvUnfilteredComputeShader;
			SharedRef<ComputePipeline> EnvUnfilteredComputePipeline;

			SharedRef<Shader> EnvFilteredComputeShader;
			SharedRef<ComputePipeline> EnvFilteredComputePipeline;

			SharedRef<Shader> IrradianceComputeShader;
			SharedRef<ComputePipeline> IrradianceComputePipeline;

			SharedRef<Texture2D> BRDFLUT;

			float CurrentTime = 0;
		};

		static SceneRendererData s_Data;
	};
} // namespace Neon
