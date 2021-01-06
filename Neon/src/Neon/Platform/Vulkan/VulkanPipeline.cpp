#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"

namespace Neon
{

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		vk::Device device = VulkanContext::GetDevice()->GetHandle();

		NEO_CORE_ASSERT(m_Specification.Shader, "Shader not initialized!");
		SharedRef<VulkanShader> vulkanShader = SharedRef<VulkanShader>(m_Specification.Shader);

		NEO_CORE_ASSERT(m_Specification.Pass, "RenderPass not initialized!");
		SharedRef<VulkanRenderPass> vulkanRenderPass = SharedRef<VulkanRenderPass>(m_Specification.Pass);

		vk::DescriptorSetLayout descriptorSetLayout = vulkanShader->GetDescriptorSetLayout();

		// TODO: Push constant ranges

		// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
		// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
		vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
		if (descriptorSetLayout)
		{
			pPipelineLayoutCreateInfo.setLayoutCount = 1;
			pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
		}

		m_PipelineLayout = device.createPipelineLayoutUnique(pPipelineLayoutCreateInfo);

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo = {};
		// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
		pipelineCreateInfo.layout = m_PipelineLayout.get();
		// Renderpass this pipeline is attached to
		pipelineCreateInfo.renderPass = vulkanRenderPass->GetHandle();

		// Construct the different states making up the pipeline

		// Input assembly state describes how primitives are assembled
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		// Rasterization state
		vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.polygonMode = vk::PolygonMode::eFill;
		rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationState.frontFace = vk::FrontFace::eClockwise;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Color blend state describes how blend factors are calculated (if used)
		// We need one blend attachment state per color attachment (even if blending is not used)
		vk::PipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
												 vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		blendAttachmentState[0].blendEnable = VK_FALSE;
		vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		// Viewport state sets the number of viewports and scissor used in this pipeline
		// Note: This is actually overriden by the dynamic states (see below)
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Enable dynamic states
		// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
		// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
		// For this example we will set the viewport and scissor using dynamic states
		std::vector<vk::DynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(vk::DynamicState::eViewport);
		dynamicStateEnables.push_back(vk::DynamicState::eScissor);
		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStateEnables.size());

		// Depth and stencil state containing depth and stencil compare and test operations
		// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = vk::StencilOp::eKeep;
		depthStencilState.back.passOp = vk::StencilOp::eKeep;
		//depthStencilState.back.compareOp = vk::CompareOp::eAlways;
		depthStencilState.front = depthStencilState.back;

		// Multi sampling state
		vk::PipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

		// Vertex input descriptor
		VertexBufferLayout& layout = m_Specification.Layout;

		vk::VertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = layout.GetStride();
		vertexInputBinding.inputRate = vk::VertexInputRate::eVertex;

		// Input attribute bindings describe shader attribute locations and memory layouts
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttribs(layout.GetElementCount());

		uint32_t location = 0;
		for (auto element : layout)
		{
			vertexInputAttribs[location].binding = 0;
			vertexInputAttribs[location].location = location;
			vertexInputAttribs[location].format = ShaderDataTypeToVulkanFormat(element.Type);
			vertexInputAttribs[location].offset = element.Offset;

			location++;
		}

		// Vertex input state used for pipeline creation
		vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
		if (!vertexInputAttribs.empty())
		{
			vertexInputState.vertexBindingDescriptionCount = 1;
			vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
			vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32>(vertexInputAttribs.size());
			vertexInputState.pVertexAttributeDescriptions = vertexInputAttribs.data();
		}

		const auto& shaderStages = vulkanShader->GetShaderStages();
		// Set pipeline shader stage info
		pipelineCreateInfo.stageCount = static_cast<uint32>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		// Assign the pipeline states to the pipeline creation info structure
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;

		// TODO: Pipeline cache??
		/*
		vk::PipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		VkPipelineCache pipelineCache;
		device.createPipelineCache
		VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));*/

		// Create rendering pipeline using the specified states
		m_Handle = device.createGraphicsPipelineUnique(nullptr, pipelineCreateInfo);
	}

} // namespace Neon
