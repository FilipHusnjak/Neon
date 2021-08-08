#include "neopch.h"

#include "Neon/Core/Application.h"
#include "Neon/Physics/Physics.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	PrimitiveComponent::PrimitiveComponent(Actor* owner)
		: ActorComponent(owner)
	{
		m_CheckerboardTex = Texture2D::Create("assets/editor/Checkerboard.tga", {TextureUsageFlagBits::ShaderRead});
	}

	PrimitiveComponent::~PrimitiveComponent()
	{
		if (m_RootPhysicsBody && m_RootPhysicsBody->GetHandle())
		{
			Physics::GetCurrentScene()->RemovePhysicsBody(m_RootPhysicsBody);
		}
	}

	void PrimitiveComponent::TickComponent(float deltaSeconds)
	{
		ActorComponent::TickComponent(deltaSeconds);

		if (SceneRenderer::GetSelectedActor() == m_Owner)
		{
			if (m_RootPhysicsBody)
			{
				m_RootPhysicsBody->RenderCollision();
			}
		}
	}

	void PrimitiveComponent::CreatePhysicsBody(PhysicsBodyType bodyType, const std::string& boneName /*= std::string()*/, const SharedRef<PhysicsMaterial>& material)
	{
		NEO_CORE_ASSERT(m_Owner);

		if (m_RootPhysicsBody && m_RootPhysicsBody->GetHandle())
		{
			Physics::GetCurrentScene()->RemovePhysicsBody(m_RootPhysicsBody);
		}
		m_RootPhysicsBody = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, m_Owner->GetTransform(), material);
	}

	void PrimitiveComponent::AddForce(const glm::vec3& force, const std::string& boneName /*= std::string()*/)
	{
		m_RootPhysicsBody->AddForce(force);
	}

	void PrimitiveComponent::RenderGui()
	{
		ImGui::BeginTable("##meshfiletable", 3);

		ImGui::TableSetupColumn("##meshfileTitle", 0, 100);
		ImGui::TableSetupColumn("##meshfile", 0, 700);
		ImGui::TableSetupColumn("##meshfileButton", 0, 40);

		ImGui::TableNextColumn();
		ImGui::Text("File Path");

		ImGui::TableNextColumn();
		if (GetMesh())
		{
			ImGui::InputText("##meshfilepath", (char*)GetMesh()->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
		}
		else
		{
			ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::TableNextColumn();
		if (ImGui::Button("...##openmesh"))
		{
			std::string file = Application::Get().OpenFile();
			if (!file.empty())
			{
				RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(GetMesh()));
				LoadMesh(file);
			}
		}

		ImGui::EndTable();

		if (GetMesh() && ImGui::TreeNodeEx("Materials"))
		{
			RenderMeshProperties(GetMesh());
		}
	}

	void PrimitiveComponent::RenderMeshProperties(SharedRef<Mesh> mesh) const
	{
		NEO_CORE_ASSERT(mesh);

		ImGui::BeginTable("##meshmaterialssizetable", 2);

		ImGui::TableNextColumn();
		ImGui::Text("Size");

		ImGui::TableNextColumn();
		ImGui::InputText("##meshmaterialssize", (char*)std::to_string(mesh->GetMaterials().size()).c_str(), 10);

		ImGui::EndTable();

		for (uint32 i = 0; i < mesh->GetMaterials().size(); i++)
		{
			Material& material = mesh->GetMaterials()[i];
			MaterialProperties materialProperties = material.GetProperties();

			std::string name = "Element " + std::to_string(i);
			if (ImGui::TreeNodeEx(name.c_str()))
			{
				if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::Image(materialProperties.UseAlbedoMap > 0.5 ? material.GetTexture2D("u_AlbedoTextures")->GetRendererId()
																	   : m_CheckerboardTex->GetRendererId(),
								 ImVec2(64, 64));
					ImGui::PopStyleVar();

					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
						if (!filename.empty())
						{
							SharedRef<Texture2D> albedoMap =
								Texture2D::Create(filename, {TextureUsageFlagBits::ShaderRead, TextureFormat::SRGBA8});
							material.SetTexture2D("u_AlbedoTextures", albedoMap, 0);
						}
					}

					ImGui::SameLine();
					ImGui::BeginGroup();
					bool useAlbedoMap = materialProperties.UseAlbedoMap > 0.5f;
					if (ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap))
					{
						materialProperties.UseAlbedoMap = useAlbedoMap ? 1.f : 0.f;
					}
					ImGui::EndGroup();

					ImGui::SameLine();
					ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(materialProperties.AlbedoColor),
									  ImGuiColorEditFlags_NoInputs);
				}

				if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::Image(materialProperties.UseNormalMap > 0.5 ? material.GetTexture2D("u_NormalTextures")->GetRendererId()
																	   : m_CheckerboardTex->GetRendererId(),
								 ImVec2(64, 64));
					ImGui::PopStyleVar();

					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
						if (!filename.empty())
						{
							SharedRef<Texture2D> normalMap =
								Texture2D::Create(filename, {TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA8});
							material.SetTexture2D("u_NormalTextures", normalMap, 0);
							materialProperties.UseNormalMap = 1.f;
						}
					}

					ImGui::SameLine();
					ImGui::BeginGroup();
					bool useNormalMap = materialProperties.UseNormalMap > 0.5f;
					if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
					{
						materialProperties.UseNormalMap = useNormalMap ? 1.f : 0.f;
					}
					ImGui::EndGroup();
				}

				if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::Image(materialProperties.UseMetalnessMap > 0.5
									 ? material.GetTexture2D("u_MetalnessTextures")->GetRendererId()
									 : m_CheckerboardTex->GetRendererId(),
								 ImVec2(64, 64));
					ImGui::PopStyleVar();

					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
						if (!filename.empty())
						{
							SharedRef<Texture2D> metalnessMap =
								Texture2D::Create(filename, {TextureUsageFlagBits::ShaderRead, TextureFormat::SRGBA8});
							material.SetTexture2D("u_MetalnessTextures", metalnessMap, 0);
							materialProperties.UseMetalnessMap = 1.f;
						}
					}

					ImGui::SameLine();
					ImGui::BeginGroup();
					bool useMetalnessMap = materialProperties.UseMetalnessMap > 0.5f;
					if (ImGui::Checkbox("Use##MetalnessMap", &useMetalnessMap))
					{
						materialProperties.UseMetalnessMap = useMetalnessMap ? 1.f : 0.f;
					}
					ImGui::EndGroup();

					ImGui::SameLine();
					ImGui::SliderFloat("##MetalnessInput", &materialProperties.Metalness, 0.0f, 1.0f);
				}

				if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::Image(materialProperties.UseRoughnessMap > 0.5
									 ? material.GetTexture2D("u_RoughnessTextures")->GetRendererId()
									 : m_CheckerboardTex->GetRendererId(),
								 ImVec2(64, 64));
					ImGui::PopStyleVar();

					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
						if (!filename.empty())
						{
							SharedRef<Texture2D> roughnessMap =
								Texture2D::Create(filename, {TextureUsageFlagBits::ShaderRead, TextureFormat::SRGBA8});
							material.SetTexture2D("u_RoughnessTextures", roughnessMap, 0);
							materialProperties.UseRoughnessMap = 1.f;
						}
					}

					ImGui::SameLine();
					ImGui::BeginGroup();
					bool useRoughnessMap = materialProperties.UseRoughnessMap > 0.5f;
					if (ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap))
					{
						materialProperties.UseRoughnessMap = useRoughnessMap ? 1.f : 0.f;
					}
					ImGui::EndGroup();

					ImGui::SameLine();
					ImGui::SliderFloat("##RoughnessInput", &materialProperties.Roughness, 0.0f, 1.0f);
				}

				ImGui::TreePop();

				material.SetProperties(materialProperties);
			}

			ImGui::Spacing();
			ImGui::Spacing();
		}

		ImGui::TreePop();
	}

} // namespace Neon
