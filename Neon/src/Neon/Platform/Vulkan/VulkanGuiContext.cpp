#include "neopch.h"

#include "Neon/Core/Application.h"
#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "VulkanGuiContext.h"

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

	void VulkanGuiContext::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

		ImFont* pFont = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\segoeui.ttf)", 15.f);
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

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

		// Headers
		colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
		colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
		colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetHandle());

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
		auto& commandBuffer = VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics, true);
		ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)commandBuffer->GetHandle());
		VulkanContext::Get()->SubmitCommandBuffer(commandBuffer);

		device->GetHandle().waitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanGuiContext::Shutdown()
	{
		auto device = VulkanContext::GetDevice()->GetHandle();

		device.waitIdle();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		m_ImGuiDescPool.reset();
	}

	void VulkanGuiContext::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanGuiContext::End()
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		uint32 width = swapChain.GetWidth();
		uint32 height = swapChain.GetHeight();

		vk::CommandBuffer renderCommandBuffer = (VkCommandBuffer)VulkanContext::Get()->GetPrimaryRenderCommandBuffer()->GetHandle();

		// Update viewport state
		vk::Viewport viewport = {};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		// Update scissor state
		vk::Rect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = width;
		scissor.extent.height = height;

		vk::ClearValue clearValues[2];
		std::array<float, 4> clearColor = {0.2f, 0.2f, 0.2f, 1.0f};
		clearValues[0].color = vk::ClearColorValue(clearColor);
		clearValues[1].depthStencil = {1.0f, 0};

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.renderPass = swapChain.GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

		renderCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		renderCommandBuffer.setViewport(0, 1, &viewport);

		renderCommandBuffer.setScissor(0, 1, &scissor);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderCommandBuffer);

		renderCommandBuffer.endRenderPass();

		ImGuiIO& io = ImGui::GetIO();
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

} // namespace Neon
