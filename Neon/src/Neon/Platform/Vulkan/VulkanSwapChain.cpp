#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanSwapChain.h"

#include <glfw/glfw3.h>

namespace Neon
{
	VulkanSwapChain::~VulkanSwapChain()
	{
		m_Device->GetHandle().waitIdle();
		m_Device->GetHandle().destroySwapchainKHR(m_Handle);
	}

	void VulkanSwapChain::Init(vk::Instance instance, const SharedRef<VulkanDevice>& device)
	{
		m_Instance = instance;
		m_Device = device;
		m_Allocator = VulkanAllocator(device, "SwapChain");

		// Create sync objects
		vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		m_Semaphores.resize(m_TargetMaxFramesInFlight + 1);
		for (auto& semaphore : m_Semaphores)
		{
			semaphore.ImageAcquired = m_Device->GetHandle().createSemaphoreUnique(semaphoreCreateInfo);
			semaphore.RenderComplete = m_Device->GetHandle().createSemaphoreUnique(semaphoreCreateInfo);
		}

		// Wait fences to sync command buffer access
		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		m_WaitFences.resize(m_TargetMaxFramesInFlight);
		for (auto& fence : m_WaitFences)
		{
			fence = m_Device->GetHandle().createFenceUnique(fenceCreateInfo);
		}

		m_QueueNodeIndex = m_Device->GetPhysicalDevice()->GetGraphicsQueueIndex();

		// Create command pool
		vk::CommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.queueFamilyIndex = m_QueueNodeIndex;
		cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_CommandPool = m_Device->GetHandle().createCommandPoolUnique(cmdPoolInfo);

		// Create command buffers
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.commandPool = m_CommandPool.get();
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandBufferCount = m_TargetMaxFramesInFlight;
		m_RenderCommandBuffers = m_Device->GetHandle().allocateCommandBuffersUnique(commandBufferAllocateInfo);
	}

	void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
	{
		VkSurfaceKHR surface;
		glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &surface);
		m_Surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(surface), m_Instance);

		vk::PhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetHandle();
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		std::vector<VkBool32> supportsPresent(queueFamilyProperties.size());
		for (uint32_t i = 0; i < supportsPresent.size(); i++)
		{
			physicalDevice.getSurfaceSupportKHR(i, m_Surface.get(), &supportsPresent[i]);
		}

		FindSurfaceFormatAndColorSpace();
	}

	void VulkanSwapChain::Create(uint32* width, uint32* height, bool vsync /*= false*/)
	{
		vk::SwapchainKHR oldSwapchain = m_Handle;

		vk::PhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetHandle();

		// Get physical device surface properties and formats
		auto deviceSurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Surface.get());

		// Get available present modes
		auto presentModes = physicalDevice.getSurfacePresentModesKHR(m_Surface.get());
		NEO_CORE_ASSERT(!presentModes.empty(), "There are no supported present modes");

		vk::Extent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (deviceSurfaceCapabilities.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = *width;
			swapchainExtent.height = *height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = deviceSurfaceCapabilities.currentExtent;
			*width = deviceSurfaceCapabilities.currentExtent.width;
			*height = deviceSurfaceCapabilities.currentExtent.height;
		}

		m_Width = *width;
		m_Height = *height;

		// Select a present mode for the swapchain

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (auto presentMode : presentModes)
			{
				if (presentMode == vk::PresentModeKHR::eMailbox)
				{
					swapchainPresentMode = vk::PresentModeKHR::eMailbox;
					break;
				}
				if (swapchainPresentMode == vk::PresentModeKHR::eImmediate)
				{
					swapchainPresentMode = vk::PresentModeKHR::eImmediate;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = deviceSurfaceCapabilities.minImageCount + 1;
		if ((deviceSurfaceCapabilities.maxImageCount > 0) &&
			(desiredNumberOfSwapchainImages > deviceSurfaceCapabilities.maxImageCount))
		{
			desiredNumberOfSwapchainImages = deviceSurfaceCapabilities.maxImageCount;
		}

		// Find the transformation of the surface
		vk::SurfaceTransformFlagBitsKHR preTransform;
		if (deviceSurfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		{
			// We prefer a non-rotated transform
			preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		}
		else
		{
			preTransform = deviceSurfaceCapabilities.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		// Simply select the first composite alpha format available
		std::vector<vk::CompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			vk::CompositeAlphaFlagBitsKHR::ePostMultiplied, vk::CompositeAlphaFlagBitsKHR::eInherit};

		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (deviceSurfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		vk::SwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.surface = m_Surface.get();
		swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCreateInfo.imageFormat = m_ColorFormat;
		swapchainCreateInfo.imageColorSpace = m_ColorSpace;
		swapchainCreateInfo.imageExtent = {swapchainExtent.width, swapchainExtent.height};
		swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		swapchainCreateInfo.preTransform = preTransform;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.presentMode = swapchainPresentMode;
		swapchainCreateInfo.oldSwapchain = oldSwapchain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (deviceSurfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
		{
			swapchainCreateInfo.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
		}

		// Enable transfer destination on swap chain images if supported
		if (deviceSurfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
		{
			swapchainCreateInfo.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
		}

		m_Handle = m_Device->GetHandle().createSwapchainKHR(swapchainCreateInfo);

		// Cleanup old swap chain
		if (oldSwapchain)
		{
			for (auto& buffer : m_Buffers)
			{
				buffer = {};
			}
			m_Device->GetHandle().destroySwapchainKHR(oldSwapchain);
		}

		std::vector<vk::Image> images = m_Device->GetHandle().getSwapchainImagesKHR(m_Handle);

		// Get the swap chain buffers containing the image and imageview
		m_Buffers.resize(images.size());
		for (uint32_t i = 0; i < images.size(); i++)
		{
			vk::ImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.format = m_ColorFormat;
			colorAttachmentView.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
											  vk::ComponentSwizzle::eA};
			colorAttachmentView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = vk::ImageViewType::e2D;
			colorAttachmentView.image = images[i];

			m_Buffers[i].Image = images[i];
			m_Buffers[i].View = m_Device->GetHandle().createImageViewUnique(colorAttachmentView);
		}

		CreateDepthStencil();

		vk::Format depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();

		// Render Pass
		std::array<vk::AttachmentDescription, 2> attachments = {};
		// Color attachment
		attachments[0].format = m_ColorFormat;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
		// Depth attachment
		attachments[1].format = depthFormat;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[1].initialLayout = vk::ImageLayout::eUndefined;
		attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		vk::SubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits{};
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = static_cast<uint32>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		m_RenderPass = m_Device->GetHandle().createRenderPassUnique(renderPassInfo);

		CreateFramebuffers();

		m_ImageIndexToFrameIndex.clear();
		m_ImageIndexToSemaphoreIndex.clear();

		uint32 frameCounter = 0;
		for (uint32 imageIndex = 0; imageIndex < images.size(); imageIndex++)
		{
			m_ImageIndexToFrameIndex[imageIndex] = frameCounter;
			frameCounter++;
			frameCounter %= m_TargetMaxFramesInFlight;
			frameCounter = std::min(frameCounter, m_TargetMaxFramesInFlight);
		}

		m_FreeSemaphoreIndices.clear();
		for (uint32 i = 0; i <= m_TargetMaxFramesInFlight; i++)
		{
			m_FreeSemaphoreIndices.push_back(i);
		}
	}

	void VulkanSwapChain::OnResize(uint32 width, uint32 height)
	{
		auto device = m_Device->GetHandle();

		device.waitIdle();
		Create(&width, &height);
		device.waitIdle();
	}

	void VulkanSwapChain::BeginFrame()
	{
		uint32 semaphoreIndex = m_FreeSemaphoreIndices.front();
		m_FreeSemaphoreIndices.pop_back();
		VK_CHECK_RESULT(AcquireNextImage(m_Semaphores[semaphoreIndex].ImageAcquired.get(), &m_CurrentSwapChainImageIndex));
		m_CurrentFrameIndex = m_ImageIndexToFrameIndex[m_CurrentSwapChainImageIndex];

		if (m_ImageIndexToSemaphoreIndex.find(m_CurrentFrameIndex) != m_ImageIndexToSemaphoreIndex.end())
		{
			m_FreeSemaphoreIndices.push_back(m_ImageIndexToSemaphoreIndex[m_CurrentFrameIndex]);
		}
		m_ImageIndexToSemaphoreIndex[m_CurrentFrameIndex] = semaphoreIndex;

		// Use a fence to wait until the command buffer has finished execution before using it again
		VK_CHECK_RESULT(m_Device->GetHandle().waitForFences(m_WaitFences[m_CurrentFrameIndex].get(), VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(m_Device->GetHandle().resetFences(1, &m_WaitFences[m_CurrentFrameIndex].get()));
	}

	void VulkanSwapChain::Present()
	{
		// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		// The submit info structure specifices a command buffer queue submission batch
		vk::SubmitInfo submitInfo = {};
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.pWaitSemaphores = &m_Semaphores[m_ImageIndexToSemaphoreIndex[m_CurrentFrameIndex]].ImageAcquired.get();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_Semaphores[m_ImageIndexToSemaphoreIndex[m_CurrentFrameIndex]].RenderComplete.get();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &m_RenderCommandBuffers[m_CurrentFrameIndex].get();
		submitInfo.commandBufferCount = 1;

		// Submit to the graphics queue passing a wait fence
		m_Device->GetGraphicsQueue().submit(submitInfo, m_WaitFences[m_CurrentFrameIndex].get());

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted
		vk::Result result = QueuePresent(m_Device->GetGraphicsQueue(), m_CurrentSwapChainImageIndex,
										 m_Semaphores[m_ImageIndexToSemaphoreIndex[m_CurrentFrameIndex]].RenderComplete.get());

		if (result != vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR)
		{
			if (result == vk::Result::eErrorOutOfDateKHR)
			{
				// Swap chain is no longer compatible with the surface and needs to be recreated
				OnResize(m_Width, m_Height);
			}
			else
			{
				VK_CHECK_RESULT(result);
			}
		}
	}

	vk::Result VulkanSwapChain::AcquireNextImage(vk::Semaphore imageAcquiredSemaphore, uint32* imageIndex)
	{
		NEO_CORE_ASSERT(imageIndex);
		vk::ResultValue resultValue =
			m_Device->GetHandle().acquireNextImageKHR(m_Handle, UINT64_MAX, imageAcquiredSemaphore, nullptr);
		*imageIndex = resultValue.value;
		return resultValue.result;
	}

	vk::Result VulkanSwapChain::QueuePresent(vk::Queue queue, uint32 imageIndex, vk::Semaphore waitSemaphore /*= VK_NULL_HANDLE*/)
	{
		vk::PresentInfoKHR presentInfo = {};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Handle;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore)
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &waitSemaphore;
		}
		return queue.presentKHR(presentInfo);
	}

	void VulkanSwapChain::CreateFramebuffers()
	{
		// Setup Framebuffer
		vk::ImageView ivAttachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		ivAttachments[1] = m_DepthStencil.ImageView.get();

		vk::FramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = m_RenderPass.get();
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = ivAttachments;
		frameBufferCreateInfo.width = m_Width;
		frameBufferCreateInfo.height = m_Height;
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
		m_Framebuffers.resize(m_Buffers.size());
		for (uint32_t i = 0; i < m_Framebuffers.size(); i++)
		{
			ivAttachments[0] = m_Buffers[i].View.get();
			m_Framebuffers[i] = m_Device->GetHandle().createFramebufferUnique(frameBufferCreateInfo);
		}
	}

	void VulkanSwapChain::CreateDepthStencil()
	{
		vk::Format depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();

		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = depthFormat;
		imageCreateInfo.extent = {m_Width, m_Height, 1};
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;

		vk::Device device = m_Device->GetHandle();
		m_DepthStencil.Image = device.createImageUnique(imageCreateInfo);
		vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(m_DepthStencil.Image.get());

		m_Allocator.Allocate(memReqs, m_DepthStencil.DeviceMemory);
		device.bindImageMemory(m_DepthStencil.Image.get(), m_DepthStencil.DeviceMemory.get(), 0);

		vk::ImageViewCreateInfo imageViewCI{};
		imageViewCI.viewType = vk::ImageViewType::e2D;
		imageViewCI.image = m_DepthStencil.Image.get();
		imageViewCI.format = depthFormat;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (depthFormat >= vk::Format::eD16Unorm)
		{
			imageViewCI.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}

		m_DepthStencil.ImageView = device.createImageViewUnique(imageViewCI);
	}

	void VulkanSwapChain::FindSurfaceFormatAndColorSpace()
	{
		vk::PhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetHandle();

		// Get list of supported surface formats
		auto deviceSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_Surface.get());

		NEO_CORE_ASSERT(!deviceSurfaceFormats.empty(), "There are no supported surface formats");

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((deviceSurfaceFormats.size() == 1) && (deviceSurfaceFormats[0].format == vk::Format::eUndefined))
		{
			m_ColorFormat = vk::Format::eB8G8R8A8Unorm;
			m_ColorSpace = deviceSurfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto surfaceFormat : deviceSurfaceFormats)
			{
				if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm)
				{
					m_ColorFormat = surfaceFormat.format;
					m_ColorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_ColorFormat = deviceSurfaceFormats[0].format;
				m_ColorSpace = deviceSurfaceFormats[0].colorSpace;
			}
		}
	}
} // namespace Neon
