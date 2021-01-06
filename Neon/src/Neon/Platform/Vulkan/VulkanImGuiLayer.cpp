#include "neopch.h"

#include "Core/Application.h"
#include "VulkanContext.h"
#include "VulkanImGuiLayer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Neon
{
	static void CheckVkResult(VkResult err)
	{
		if (err == 0)
		{
			return;
		}

		NEO_CORE_ERROR("[vulkan] Error: VkResult = %d", err);
		if (err < 0)
		{
			abort();
		}
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

		ImFont* pFont = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\segoeui.ttf)", 18.0f);
		io.FontDefault = io.Fonts->Fonts.back();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		VulkanImGuiLayer* instance = this;
		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		auto vulkanContext = VulkanContext::Get();
		auto device = VulkanContext::GetDevice();

		// Create Descriptor Pool
		vk::DescriptorPoolSize poolSizes[] = {{vk::DescriptorType::eSampler, 1000},
											  {vk::DescriptorType::eCombinedImageSampler, 1000},
											  {vk::DescriptorType::eSampledImage, 1000},
											  {vk::DescriptorType::eStorageImage, 1000},
											  {vk::DescriptorType::eUniformTexelBuffer, 1000},
											  {vk::DescriptorType::eStorageTexelBuffer, 1000},
											  {vk::DescriptorType::eUniformBuffer, 1000},
											  {vk::DescriptorType::eStorageBuffer, 1000},
											  {vk::DescriptorType::eUniformBufferDynamic, 1000},
											  {vk::DescriptorType::eStorageBufferDynamic, 1000},
											  {vk::DescriptorType::eInputAttachment, 1000}};
		vk::DescriptorPoolCreateInfo imguiPoolCreateInfo = {};
		imguiPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		imguiPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		imguiPoolCreateInfo.poolSizeCount = (uint32)IM_ARRAYSIZE(poolSizes);
		imguiPoolCreateInfo.pPoolSizes = poolSizes;

		m_ImGuiDescPool = device->GetHandle().createDescriptorPoolUnique(imguiPoolCreateInfo);

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo imguiInitInfo = {};
		imguiInitInfo.Instance = VulkanContext::GetInstance();
		imguiInitInfo.PhysicalDevice = device->GetPhysicalDevice()->GetHandle();
		imguiInitInfo.Device = device->GetHandle();
		imguiInitInfo.QueueFamily = device->GetPhysicalDevice()->GetGraphicsQueueIndex();
		imguiInitInfo.Queue = device->GetGraphicsQueue();
		imguiInitInfo.PipelineCache = nullptr;
		imguiInitInfo.DescriptorPool = m_ImGuiDescPool.get();
		imguiInitInfo.Allocator = nullptr;
		imguiInitInfo.MinImageCount = 2;
		imguiInitInfo.ImageCount = vulkanContext->GetSwapChain().GetImageCount();
		imguiInitInfo.CheckVkResultFn = CheckVkResult;
		ImGui_ImplVulkan_Init(&imguiInitInfo, vulkanContext->GetSwapChain().GetRenderPass());

		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		/*io.Fonts->AddFontDefault();
		io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
		ImFont* font = io.Fonts->AddFontFromFileTTF(R"(c:\Windows\Fonts\ArialUni.ttf)", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != NULL);*/

		// Upload Fonts
		VkCommandBuffer commandBuffer = VulkanContext::GetDevice()->GetCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		VulkanContext::GetDevice()->FlushCommandBuffer(commandBuffer);

		device->GetHandle().waitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanImGuiLayer::OnDetach()
	{
		auto device = VulkanContext::GetDevice()->GetHandle();

		device.waitIdle();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
	}

} // namespace Neon
