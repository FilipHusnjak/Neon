#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VK_CHECK_RESULT(f)                                                                                                         \
	{                                                                                                                              \
		vk::Result res = (f);                                                                                                      \
		NEO_CORE_ASSERT(res == vk::Result::eSuccess);                                                                                       \
	}\
