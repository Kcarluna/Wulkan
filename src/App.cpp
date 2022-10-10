// NOTE(__LUNA__): This belongs in a .cpp file... not .h
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// NOTE(__LUNA__): Look into .glTF files
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "App.h"

void App::run()
{
	// NOTE(__LUNA__): Do i need to put App::?
	App::initVk();
	App::mainLoop();
	App::cleanup();
}

// NOTE(__LUNA__): WHAT IS THIS NONSENSE? | https://en.cppreference.com/w/cpp/utility/hash
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const &vertex) const {
			// NOTE(__LUNA__): pos and color, shifted right, are XOR'd then shifted left and finally XOR'd with coords
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texture_coord) << 1);
		}
	};
}

static VkResult createDebugMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *info,
		const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *messenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, info, allocator, messenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data)
{
	(void) type;
	(void) user_data;
	// NOTE(__LUNA__): Filter messages based on severity
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << data->pMessage << std::endl << std::endl;
	}

	return VK_FALSE;
}

void App::createMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		// NOTE(__LUNA__): Enable verbosity, warnings, and errors
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
		.pUserData = NULL
	};
	if (createDebugMessenger(m_instance, &info, NULL, &m_messenger) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create debug messenger");
	}
}

void populateMessenger(VkDebugUtilsMessengerCreateInfoEXT &info)
{
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	// NOTE(__LUNA__): Enable verbosity, warnings, and errors
	info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	info.pfnUserCallback = debugCallback,
	info.pUserData = NULL;
}

static void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks *allocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, messenger, allocator);
	}
}

bool App::validateLayers()
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	std::vector<VkLayerProperties> layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
	for (const char *layer: m_validation_layers) {
		for (const auto &available_layers: layers) {
			if (strcmp(layer, available_layers.layerName) == 0) {
				return true;
			}
		}
	}
	return false;
}

bool supportedInstanceExtension(const char *extension)
{
	uint32_t extension_count;
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.data());

	// NOTE(__LUNA__): WHY IS THIS NOT WORKINGLAKJG:D | WAT
	for (const auto &supported_extension: extensions) {
		if (strcmp(extension, supported_extension.extensionName) == 0) {
			return true;
		}
	}
	return false;
}

void App::createInstance()
{
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Triangle",
		// NOTE(__LUNA__): Version? Wuts dat
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};
	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info
	};
	uint32_t extension_count = 0;
	const char **extensions;
	extensions = glfwGetRequiredInstanceExtensions(&extension_count);
	std::vector<const char *> extension_list;
	for (size_t i = 0; i < extension_count; i++) {
		extension_list.push_back(extensions[i]);
	}

#ifdef __APPLE__
	extension_list.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extension_list.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif // __APPLE__

	instance_info.enabledExtensionCount = static_cast<size_t>(extension_list.size());
	instance_info.ppEnabledExtensionNames = extension_list.data();

	// NOTE(__LUNA__): Putting this in if loop causes it to be prematurely deleted
	VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
	// NOTE (__LUNA__): Validation mainly for debug... disabled in release
	if (m_validate) {
		if (!validateLayers()) {
			throw std::runtime_error("ERROR: Validation layers requested, but not available");
		}
		// NOTE(__LUNA__): Enable validation layers
		instance_info.enabledLayerCount = static_cast<size_t>(m_validation_layers.size());
		instance_info.ppEnabledLayerNames = m_validation_layers.data();

		// NOTE(__LUNA__): Enable debug extensions
		extension_list.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance_info.enabledExtensionCount = static_cast<size_t>(extension_list.size());
		instance_info.ppEnabledExtensionNames = extension_list.data();

		// NOTE(__LUNA__): Populate debugger
		populateMessenger(debug_info);
		instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_info;
	} else {
		instance_info.enabledLayerCount = 0;
		instance_info.pNext = NULL;
	}

	for (size_t i = 0; i < extension_list.size(); i++) {
		if (!supportedInstanceExtension(extension_list[i])) {
			throw std::runtime_error("ERROR: Attempting to load unsupported extension");
		}
	}

	if (vkCreateInstance(&instance_info, NULL, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create instance");
	}
}

// NOTE(__LUNA__): Surface is a 'renderer' built on top of the window?
void App::createSurface()
{
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = glfwGetWin32Window(m_window.getWindow()),
		.hinstance = GetModuleHandle(NULL)
	};
	if (vkCreateWin32SurfaceKHR(m_instance, &info, NULL, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create window surface");
	}
#endif // _WIN32
	if (glfwCreateWindowSurface(m_instance, m_window.getWindow(), NULL, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create window surface");
	}
}

Queue App::findQueue(VkPhysicalDevice device)
{
	Queue queue;
	uint32_t queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, NULL);
	std::vector<VkQueueFamilyProperties> queues(queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues.data());

	// NOTE(__LUNA__): Can optimize (for performance) to check present queue that supports both window and surface
	VkBool32 present_support;
	for (size_t i = 0; i < queues.size(); i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queue.graphics = i;
		}
		if (present_support) {
			queue.present = i;
		}
		if (queue.complete()) {
			break;
		}
	}

	return queue;
}

bool App::supportedDeviceExtension(VkPhysicalDevice device)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extensions.data());

	// NOTE(__LUNA__): 'Ticking' off extensions.. also can use a nested for loop like instance extensions
	std::set<std::string> required_extensions(m_device_extensions.begin(), m_device_extensions.end());
	for (const auto &extension: extensions) {
		required_extensions.erase(extension.extensionName);
	}
	return required_extensions.empty();
}

Swap_Chain App::supportedSwapChain(VkPhysicalDevice device)
{
	Swap_Chain details;
	// NOTE(__LUNA__): Query capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
	// NOTE(__LUNA__): Query formats
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, NULL);
	if (format_count > 0) {
		// NOTE(__LUNA__): Look into vector.resize()... Does it optimize?
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
	}
	// NOTE(__LUNA__): Query present_modes
	uint32_t present_modes_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, NULL);
	if (present_modes_count > 0) {
		details.present_modes.resize(present_modes_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, details.present_modes.data());
	}

	return details;
}

// TODO(__LUNA__): Implement this function | Interesting resource: http://disq.us/p/2ifaubf
//	int candidate_device_score(VkPhysicalDevice device)
//	{
//		// NOTE(__LUNA__): No geometryShaders means pass! | NO GEOM SHADERS MACOS :) gweat
//		if (!device->deviceFeatures.geometryShader) {
//			return 0;
//		}
//		int score = 0;
//		// NOTE (__LUNA__): Discrete GPUs are best?
//		if (device->deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
//			score += 1000;
//		}
//		// NOTE(__LUNA__): Max size affects graphics quality
//		score += device->deviceProperties.limits.maxImageDimension2D;
//		return score;
//	}

// NOTE(__LUNA__): Pick best current device for rendering
bool App::supportedDevice(VkPhysicalDevice device)
{
	Queue queue = App::findQueue(device);
	bool supported_extension = App::supportedDeviceExtension(device);
	bool swap_chain_support;
	if (supported_extension) {
		Swap_Chain swap_chain = App::supportedSwapChain(device);
		swap_chain_support = !swap_chain.formats.empty() && !swap_chain.present_modes.empty();
	}
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	return queue.complete() && supported_extension && swap_chain_support && features.samplerAnisotropy;
}

VkSampleCountFlagBits App::getMaxSamples()
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(m_physical_device, &properties);

	VkSampleCountFlags count = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
	if (count & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (count & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (count & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (count & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (count & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (count & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat App::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags flags)
{
	for (auto format: candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & flags) == flags) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & flags) == flags) {
			return format;
		}
	}
	throw std::runtime_error("ERROR: Failed to find supported format");
}

uint32_t App::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

	// NOTE(__LUNA__): Loop over list of available memory properties
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
		// NOTE(__LUNA__): Loop until finding a matching type | Check both available memory for reading and writing
		if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	// NOTE(__LUNA__): Runtime error statisfies non-void returning warning
	throw std::runtime_error("ERROR: Failed to find a suitable memory type");
}

// NOTE(__LUNA__): Implicitly destroyed...
void App::createPhysicalDevice()
{
	uint32_t device_count;
	vkEnumeratePhysicalDevices(m_instance, &device_count, NULL);
	if (device_count == 0) {
		throw std::runtime_error("ERROR: Failed to find GPUs with Vulkan support");
	}
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

	// TODO(__LUNA__): Implement this function
	//		std::multimap<int, VkPhysicalDevice> candidates
	//		for (const auto &candidate: devices) {
	//			int score = candidate_device_score(candidate);
	//			candidates.insert(std::make_pair(score, candidate);
	//		}
	for (const auto &device: devices) {
		if (App::supportedDevice(device)) {
			m_physical_device = device;
			m_ms_aa_samples = App::getMaxSamples();
			break;
		}
	}
	if (m_physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("ERROR: Failed to find a suitable GPU");
	}
}

// NOTE(__LUNA__): Same as create instance, but device specific
void App::createLogicalDevice()
{
	// NOTE(__LUNA__): findQueue queried multiple times for simplicity
	Queue queue = findQueue(m_physical_device);

	std::vector<VkDeviceQueueCreateInfo> infos;
	std::set<uint32_t> queues = {queue.graphics.value(), queue.present.value()};

	float priority = 1.0f;
	for (const uint32_t queue_type: queues) {
		VkDeviceQueueCreateInfo queue_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queue_type,
			.queueCount = 1,
			.pQueuePriorities = &priority,
		};
		infos.push_back(queue_info);
	}

	// NOTE(__LUNA__): Look at todo.. would give more refined control here
	VkPhysicalDeviceFeatures device_features = {
		.samplerAnisotropy = VK_TRUE,
		// NOTE(__LUNA__): Sample shading... blends colors at a performance cost
		.sampleRateShading = VK_TRUE
	};

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(infos.size()),
		.pQueueCreateInfos = infos.data(),
		.pEnabledFeatures = &device_features,
	};
	// NOTE(__LUNA__): Fixes validation error | Not sure if macos specific..
#ifdef __APPLE__
	const char *device_extension = "VK_KHR_portability_subset";
	m_device_extensions.push_back(device_extension);
#endif // __APPLE__

	device_info.enabledExtensionCount = static_cast<size_t>(m_device_extensions.size());
	device_info.ppEnabledExtensionNames = m_device_extensions.data();
	if (m_validate) {
		device_info.enabledLayerCount = static_cast<size_t>(m_validation_layers.size());
		device_info.ppEnabledLayerNames = m_validation_layers.data();
	} else {
		device_info.enabledLayerCount = 0;
	}
	if (vkCreateDevice(m_physical_device, &device_info, NULL, &m_logical_device) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create logical device");
	}
	// NOTE(__LUNA__): Queue families are the same so index only passed once... meaning the 0?
	vkGetDeviceQueue(m_logical_device, queue.graphics.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_logical_device, queue.present.value(), 0, &m_present_queue);
}

// NOTE(__LUNA__): Capabilities
VkExtent2D App::swapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	VkExtent2D extent;
	// NOTE(__LUNA__): What is this doing?
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	int w, h;
	// NOTE(__LUNA__): Returns pixel value
	glfwGetFramebufferSize(m_window.getWindow(), &w, &h);
	extent = {
		.width  = static_cast<uint32_t>(w),
		.height = static_cast<uint32_t>(h)
	};
	// NOTE(__LUNA__): Clamp calculated pixel count with max and min allowed
	extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return extent;
}

// NOTE(__LUNA__): Formats
VkSurfaceFormatKHR swapChainFormat(const std::vector<VkSurfaceFormatKHR> &formats)
{
	for (const auto &format: formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	// NOTE(__LUNA__): Fallback is returning the first available format
	return formats[0];
}

// NOTE(__LUNA__): Present_modes
VkPresentModeKHR swapChainPresentMode(const std::vector<VkPresentModeKHR> &present_modes)
{
	for (const auto &present_mode: present_modes) {
		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return present_mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

void App::createSwapChain()
{
	Swap_Chain chain = supportedSwapChain(m_physical_device);

	VkExtent2D chain_extent = swapChainExtent(chain.capabilities);
	VkSurfaceFormatKHR chain_format = swapChainFormat(chain.formats);
	VkPresentModeKHR chain_present = swapChainPresentMode(chain.present_modes);

	// NOTE(__LUNA__): Sticking with min would cause driver to 'stall' as it waits to complete rendering
	uint32_t image_count = chain.capabilities.minImageCount + 1;
	// NOTE(__LUNA__): 0 here would signify there is no max.. so would it queue indefinetely?
	if (chain.capabilities.maxImageCount > 0 && image_count > chain.capabilities.maxImageCount) {
		image_count = chain.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = image_count,
		.imageFormat = chain_format.format,
		.imageColorSpace = chain_format.colorSpace,
		.imageExtent = chain_extent,
		// NOTE(__LUNA__) 1 for 2D
		.imageArrayLayers = 1,
		// NOTE(__LUNA__): Usage in this case is just rendering images | Postprocessing would require another pipeline... along with every other process in the rendering phase... vertex, frag, rasterization, blending, Vulkan is fully customizablefunctionalities are up to the programmer to include
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	};
	Queue queue = findQueue(m_physical_device);
	uint32_t queues[] = {queue.graphics.value(), queue.present.value()};
	// NOTE(__LUNA__): NOTE about optimizing happens here.. if graphics and present don't share indices explicit transferring ownership of images would be required. Thus optimizing performance
	if (queue.graphics != queue.present) {
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = queues;
	} else {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = NULL;
	}
	// NOTE(__LUNA__): Transforms like rotate or flip occur here
	info.preTransform = chain.capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = chain_present;
	// NOTE(__LUNA__): Clip pixels while rendering
	info.clipped = VK_TRUE;
	// NOTE(__LUNA__): Fallback for main swap chain disruptions
	info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_logical_device, &info, NULL, &m_swap_chain) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create Swap Chain");
	}

	// NOTE(__LUNA__): Store the images for rendering
	vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, NULL);
	m_swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, m_swap_chain_images.data());

	m_format = chain_format.format;
	m_extent = chain_extent;
}

VkImageView App::createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mip_levels)
{
	VkImageView image_view;
	VkImageViewCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		// NOTE(__LUNA__): VK_COMPONENT_SWIZZLE_IDENTITY is 0... so default?
		.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		// NOTE(__LUNA__): No mipmapping levels or multiple layers
		.subresourceRange.aspectMask = flags,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = mip_levels,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1
	};
	if (vkCreateImageView(m_logical_device, &info, NULL, &image_view) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create Image View");
	}
	return image_view;
}

void App::createImageViews()
{
	m_swap_chain_image_views.resize(m_swap_chain_images.size());
	for (uint32_t i = 0; i < m_swap_chain_images.size(); i++) {
		m_swap_chain_image_views[i] = App::createImageView(m_swap_chain_images[i], m_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

// NOTE(__LUNA__): Reference on render passes https://www.youtube.com/watch?v=x2SGVjlVGhE
void App::createRenderPass()
{
	VkAttachmentDescription color_attachment = {
		.format = m_format,
		.samples = m_ms_aa_samples,
		.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		// NOTE(__LUNA__): Msaa images are not presented directly
		.finalLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkAttachmentReference color_attachment_reference = {
		// NOTE(__LUNA__): Attachment to shader in layout (location = 0)
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkAttachmentDescription color_attachment_resolve = {
		.format = m_format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		// NOTE(__LUNA__): Resolve will present the image
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	VkAttachmentReference color_attachment_resolve_reference = {
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkAttachmentDescription depth_attachment = {
		// NOTE(__LUNA__): Why don't we have a mem var depth buffer format like we do for color i.e. m_format
		.format = App::findDepthFormat(),
		.samples = m_ms_aa_samples,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		// NOTE(__LUNA__): Only difference between attachments is .finalLayout
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	VkAttachmentReference depth_attachment_reference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_reference,
		.pDepthStencilAttachment = &depth_attachment_reference,
		.pResolveAttachments = &color_attachment_resolve_reference
	};
	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};
	std::array<VkAttachmentDescription, 3> attachments = {color_attachment, depth_attachment, color_attachment_resolve};
	VkRenderPassCreateInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};
	if (vkCreateRenderPass(m_logical_device, &render_pass_info, NULL, &m_render_pass) != VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to create render pass");
	}
}

VkShaderModule App::createShaderModule(const std::vector<char> &bytecode)
{
	// NOTE(__LUNA__): Why is module syntax highlighted? Concern?
	VkShaderModule module;
	VkShaderModuleCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = bytecode.size(),
		.pCode = reinterpret_cast<const uint32_t *>(bytecode.data())
	};
	if (vkCreateShaderModule(m_logical_device, &info, NULL, &module) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create shader module");
	}
	return module;
}

static std::vector<char> loadShaderFile(const std::string &file_path)
{
	// NOTE(__LUNA__): ios::ate starts at end of file, helps with allocating memory
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		// NOTE(__LUNA__): Don't like
		throw std::runtime_error("ERROR: Failed to open file");
	}
	size_t size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), size);
	file.close();
	return buffer;
}

void App::createDescriptorLayout()
{
	VkDescriptorSetLayoutBinding uniform_layout_binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		// NOTE(__LUNA__): Used for image sampling
		.pImmutableSamplers = NULL
	};
	VkDescriptorSetLayoutBinding sampler_layout_binding = {
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL
	};

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniform_layout_binding, sampler_layout_binding};
	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	if (vkCreateDescriptorSetLayout(m_logical_device, &layout_info, NULL, &m_descriptor_layout) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create descriptor set layout");
	}
}

void App::createGraphicsPipeline()
{
	auto vert_shader = loadShaderFile("../shaders/vert.spv");
	auto frag_shader = loadShaderFile("../shaders/frag.spv");

	VkShaderModule vert_shader_module = createShaderModule(vert_shader);
	VkShaderModule frag_shader_module = createShaderModule(frag_shader);

	VkPipelineShaderStageCreateInfo vert_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vert_shader_module,
		// NOTE(__LUNA__): Can combine multiple shaders info one using entry points
		.pName = "main",
		// NOTE(__LUNA__): Enable/disable compile-time shader
		.pSpecializationInfo = NULL,
	};
	VkPipelineShaderStageCreateInfo frag_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = frag_shader_module,
		.pName = "main",
		.pSpecializationInfo = NULL
	};
	VkPipelineShaderStageCreateInfo shader_infos[] = {vert_info, frag_info};

	auto bindings = Vertex::getBindingDescription();
	auto attributes = Vertex::getAttributeDescription();
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindings,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
		.pVertexAttributeDescriptions = attributes.data()
	};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		// NOTE(__LUNA__): Enables use of element buffer
		.primitiveRestartEnable = VK_FALSE
	};
	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		// NOTE(__LUNA__): What is dynamic state and scissor
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data()
	};
	VkPipelineViewportStateCreateInfo viewport_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	// NOTE(__LUNA__): Were these ever necessary here?
	//		viewport_state_info.pViewports = &viewport;
	//		viewport_state_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		// NOTE(__LUNA__): If true, geometry never passes through rasterizer
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f
	};
	// TODO(__LUNA__): Multissampling is more efficient than rendering high resolution and then downscaling
	VkPipelineMultisampleStateCreateInfo multisample_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = m_ms_aa_samples,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 0.2f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
	};
	// NOTE(__LUNA__): Only really used for 'static' rendering | i.e. only one framebuffer
	VkPipelineColorBlendAttachmentState blender = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |VK_COLOR_COMPONENT_A_BIT,
		// NOTE(__LUNA__): Enable blend for what? Transparency?
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD
	};
	// NOTE(__LUNA__): Support for multiple framebuffers
	//		blender.blendEnable = VK_TRUE;
	//		blender.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//		blender.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//		blender.colorBlendOp = VK_BLEND_OP_ADD;
	//		blender.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//		blender.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//		blender.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blender_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		// NOTE(__LUNA__): Enabling this enables bitwise blending
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &blender,
		.blendConstants[0] = 0.0f,
		.blendConstants[1] = 0.0f,
		.blendConstants[2] = 0.0f,
		.blendConstants[3] = 0.0f,
	};
	VkPipelineLayoutCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		// NOTE(__LUNA__): Using descriptor layout from UBO
		.pSetLayouts = &m_descriptor_layout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL,
	};
	if (vkCreatePipelineLayout(m_logical_device, &pipeline_info, NULL, &m_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create pipeline layout");
	}
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		// NOTE(__LUNA__): Lower depth is kept? Further depth is discarded?
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		// NOTE(__LUNA__): Range to keep fragments that are within it
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
		.stencilTestEnable = VK_FALSE,
		.front = {},
		.back = {}
	};

	VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shader_infos,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pViewportState = &viewport_state_info,
		.pRasterizationState = &rasterizer_info,
		.pMultisampleState = &multisample_info,
		.pDepthStencilState = &depth_stencil_info,
		.pColorBlendState = &blender_info,
		.pDynamicState = &dynamic_state_info,
		.layout = m_pipeline_layout,
		.renderPass = m_render_pass,
		.subpass = 0,
		// NOTE(__LUNA__): Enable pipeline derivatives
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};
	if (vkCreateGraphicsPipelines(m_logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_info, NULL, &m_graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create graphics pipeline!");
	}

	// NOTE(__LUNA__): Can destroy this on the stack because this is the end of the graphics pipeline
	vkDestroyShaderModule(m_logical_device, vert_shader_module, NULL);
	vkDestroyShaderModule(m_logical_device, frag_shader_module, NULL);
}

void App::createFramebuffers()
{
	m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());
	for (size_t i = 0; i < m_swap_chain_image_views.size(); i++) {
		// NOTE(__LUNA__): Does this only make sense when you have seperate attachments | Yes image_views and depth_buffers
		//			VkImageView attachments[] = {
		//				swap_chain_image_views[i]
		//			};
		std::array<VkImageView, 3> attachments = {m_color_image_view, m_depth_image_view, m_swap_chain_image_views[i]};
		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_render_pass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width  = m_extent.width,
			.height = m_extent.height,
			.layers = 1
		};
		if (vkCreateFramebuffer(m_logical_device, &framebuffer_info, NULL, &m_swap_chain_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("ERROR: Failed to create framebuffer");
		}
	}
}

void App::createCommandPool()
{
	Queue indices = findQueue(m_physical_device);

	VkCommandPoolCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphics.value()
	};
	if (vkCreateCommandPool(m_logical_device, &info, NULL, &m_command_pool) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create command pool");
	}
}

bool hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat App::findDepthFormat()
{
	return App::findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void App::createColorResources()
{
	VkFormat color_format = m_format;
	// NOTE(__LUNA__): 1 mipmap because multiple samples per image... color and depth. Also color buffer not used as texture
	App::createImage(m_extent.width, m_extent.height, 1, m_ms_aa_samples, color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_color_image, m_color_image_memory);
	m_color_image_view = App::createImageView(m_color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void App::createDepthResources()
{
	// NOTE(__LUNA__): This was extracted to helper function... Why? | Setup global depth variable?
	VkFormat depth_format = App::findDepthFormat();
	App::createImage(m_extent.width, m_extent.height, 1, m_ms_aa_samples, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depth_image, m_depth_image_memory);
	m_depth_image_view = App::createImageView(m_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	App::transitionImageLayout(m_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void App::createImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags flags, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &memory)
{
	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = 1,
		.mipLevels = mip_levels,
		.arrayLayers = 1,
		.format = format,
		.tiling = tiling,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = flags,
		// NOTE(__LUNA__): Related to multisampling
		.samples = samples,
		// NOTE(__LUNA__): Look into Sparse images
		.flags = 0,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	if (vkCreateImage(m_logical_device, &image_info, NULL, &image) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create image");
	}
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(m_logical_device, image, &memory_requirements);

	VkMemoryAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		// NOTE(__LUNA__): VK_KHR_dedicated_allocation seems to be more efficient... but this checks for availability
		.memoryTypeIndex = App::findMemoryType(memory_requirements.memoryTypeBits, properties)
	};
	if (vkAllocateMemory(m_logical_device, &allocate_info, NULL, &memory) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to allcoate image memory");
	}
	vkBindImageMemory(m_logical_device, image, memory, 0);
}

void App::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer command_buffer = App::beginSingleTimeCommands();
	VkBufferImageCopy region = {
		// NOTE(__LUNA__): Tightly packed pixels
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,

		.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.imageSubresource.mipLevel = 0,
		.imageSubresource.baseArrayLayer = 0,
		.imageSubresource.layerCount = 1,

		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1}
	};
	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	App::endSingleTimeCommands(command_buffer);
}

// NOTE(__LUNA__): Mipmaps are generally pregenerated and stored in file along with texture file | Look into stb_image_resize
void App::generateMipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mip_levels)
{
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &properties);
	if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("ERROR: Image format does not support linear blitting");
	}

	VkCommandBuffer command_buffer = App::beginSingleTimeCommands();

	// NOTE(__LUNA__): Barrier for synchronization
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.image = image,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.subresourceRange.levelCount = 1
	};
	int32_t mip_width  = width;
	int32_t mip_height = height;
	for (uint32_t i = 1; i < mip_levels; i++) {
		// NOTE(__LUNA__): Transition image from dst to src, from write to read
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		// NOTE(__LUNA__): Waiting to copy buffer to image
		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							 0, NULL, 0, NULL, 1, &barrier);
		VkImageBlit blit = {
			.srcOffsets[0] = {0, 0, 0},
			.srcOffsets[1] = {mip_width, mip_height, 1},
			.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.srcSubresource.mipLevel = i - 1,
			.srcSubresource.baseArrayLayer = 0,
			.srcSubresource.layerCount = 1,

			.dstOffsets[0] = {0, 0, 0},
			.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1},
			.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.dstSubresource.mipLevel = i,
			.dstSubresource.baseArrayLayer = 0,
			.dstSubresource.layerCount = 1
		};
		// NOTE(__LUNA__): Transition same image from src to dst for blitting
		vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		// NOTE(__LUNA__): Transition from transfer to shader read
		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, NULL, 0, NULL, 1, &barrier);
		if (mip_width > 1) mip_width /= 2;
		if (mip_height > 1) mip_height /= 2;
	}
	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						 0, NULL, 0, NULL, 1, &barrier);

	App::endSingleTimeCommands(command_buffer);
}

void App::createTextureImage(const char *file_path)
{
	int width, height, channels;
	unsigned char *pixels = stbi_load(file_path, &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize size = width * height * 4;
	m_mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	if (!pixels) {
		throw std::runtime_error("ERROR: Failed to load texture image");
	}

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	App::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(m_logical_device, staging_buffer_memory, 0, size, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(size));
	vkUnmapMemory(m_logical_device, staging_buffer_memory);

	stbi_image_free(pixels);

	// NOTE(__LUNA__): Mipmaps are both src and dst bits
	App::createImage(width, height, m_mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image, m_texture_image_memory);

	App::transitionImageLayout(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mip_levels);
	App::copyBufferToImage(staging_buffer, m_texture_image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	// NOTE(__LUNA__): Query physical device to see if format supported
	App::generateMipmaps(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, width, height, m_mip_levels);
	// NOTE(__LUNA__): Image transitioned during the generation of mipmap
//	App::transitionImageLayout(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mip_levels);

	vkDestroyBuffer(m_logical_device, staging_buffer, NULL);
	vkFreeMemory(m_logical_device, staging_buffer_memory, NULL);
}

void App::createTextureImageView()
{
	m_texture_image_view = App::createImageView(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mip_levels);
}

void App::createTextureSampler()
{
	// NOTE(__LUNA__): Mipmapping looks a little wonky? On both tut and my code | Found error in shaders | NEVER UPDATED
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(m_physical_device, &properties);
	VkSamplerCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		// NOTE(__LUNA__): Higher quality images
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		// NOTE(__LUNA__): If enabled, texels will be compared to value... used in shadow maps
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		// NOTE(__LUNA__): LOD = level of detail
		.mipLodBias = 0.0f,
		.minLod = 0.0f,
		.maxLod = static_cast<float>(m_mip_levels)
	};
	if (vkCreateSampler(m_logical_device, &info, NULL, &m_texture_sampler) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create texture sampler");
	}
}

// NOTE(__LUNA__): Abstract buffer creation
void App::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer &buffer, VkDeviceMemory &memory)
{
	// NOTE(__LUNA__): Create VB
	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage_flags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	if (vkCreateBuffer(m_logical_device, &buffer_info, NULL, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create buffer");
	}

	// NOTE(__LUNA__): Allocate VB
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(m_logical_device, buffer, &memory_requirements);

	VkMemoryAllocateInfo buffer_memory_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		// NOTE(__LUNA__): HOST_COHERENT_BIT slightly worse performance than explicitly 'flushing' memory
		.memoryTypeIndex = App::findMemoryType(memory_requirements.memoryTypeBits, property_flags)
	};
	if (vkAllocateMemory(m_logical_device, &buffer_memory_info, NULL, &memory) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to allocate buffer memory");
	}
	// NOTE(__LUNA__): Bind VB
	// NOTE(__LUNA__): Last parameter is offset value
	vkBindBufferMemory(m_logical_device, buffer, memory, 0);
}

VkCommandBuffer App::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocation_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = m_command_pool,
		.commandBufferCount = 1
	};

	VkCommandBuffer buffer;
	vkAllocateCommandBuffers(m_logical_device, &allocation_info, &buffer);

	VkCommandBufferBeginInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(buffer, &buffer_info);

	return buffer;
}

void App::endSingleTimeCommands(VkCommandBuffer buffer)
{
	// NOTE(__LUNA__): Check recording... just like other vkEndCommandBuffer
	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to record copy buffer");
	}

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &buffer
	};
	vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	// NOTE(__LUNA__): Fences here would optimize
	vkQueueWaitIdle(m_graphics_queue);
	vkFreeCommandBuffers(m_logical_device, m_command_pool, 1, &buffer);
}

// NOTE(__LUNA__): Challenge; add setupCommandBuffer and flushCommandBuffer functions to record and dispatch commands
void App::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels)
{
	// NOTE(__LUNA__): Format used for explicit depth buffer transition
//	(void) format;

	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

	VkCommandBuffer buffer = App::beginSingleTimeCommands();
	// NOTE(__LUNA__): Barriers mostly used for synchronization purposes
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = mip_levels,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.srcAccessMask = 0,
		.dstAccessMask = 0
	};
	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::runtime_error("ERROR: Unsupported layout transition");
	}
	vkCmdPipelineBarrier(buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

	App::endSingleTimeCommands(buffer);
}

// NOTE(__LUNA__): Optimization would be to create seperate command pools for temporary buffers i.e. staging buffers
void App::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
	VkCommandBuffer buffer = App::beginSingleTimeCommands();
		VkBufferCopy copy = {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size
		};
		vkCmdCopyBuffer(buffer, src_buffer, dst_buffer, 1, &copy);
	App::endSingleTimeCommands(buffer);
}

// NOTE(__LUNA__): Abstract create___Buffers | Am I using template correctomundo?
template<typename T>
void App::createBuffers(const std::vector<T> &input, VkBuffer &buffer, VkDeviceMemory &memory, VkBufferUsageFlags flags)
{
	VkDeviceSize size = sizeof(input[0]) * input.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	App::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(m_logical_device, staging_buffer_memory, 0, size, 0, &data);
		memcpy(data, input.data(), (size_t) size);
	vkUnmapMemory(m_logical_device, staging_buffer_memory);

	App::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

	App::copyBuffer(staging_buffer, buffer, size);
	vkDestroyBuffer(m_logical_device, staging_buffer, NULL);
	vkFreeMemory(m_logical_device, staging_buffer_memory, NULL);
}

void App::loadModels(const char *file_path)
{
	tinyobj::attrib_t attribute;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &warn, &err, file_path)) {
		throw std::runtime_error(warn + err);
	}
	std::unordered_map<Vertex, uint32_t> unique_vertices;
	for (const auto &shape: shapes) {
		// NOTE(__LUNA__): Assume unique vertex entry | auto increment indices
		for (const auto &index: shape.mesh.indices) {
			Vertex vertex = {
				// NOTE(__LUNA__): Multiply by 3 to convert float to vec3
				.pos = {attribute.vertices[3 * index.vertex_index + 0],
						attribute.vertices[3 * index.vertex_index + 1],
						attribute.vertices[3 * index.vertex_index + 2]},
				.color = {1.0f, 1.0f, 1.0f},
				.texture_coord = {attribute.texcoords[2 * index.texcoord_index + 0],
								  // NOTE(__LUNA__): Invert y texture_coord
								  1.0f - attribute.texcoords[2 * index.texcoord_index + 1]}
			};
			// NOTE(__LUNA__): This could be done more efficiently?
			if (unique_vertices.count(vertex) == 0) {
				unique_vertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}
			m_indices.push_back(unique_vertices[vertex]);
		}
	}
}

void App::createVertexBuffers()
{
	App::createBuffers(m_vertices, m_vertex_buffer, m_vertex_buffer_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void App::createIndexBuffers()
{
	App::createBuffers(m_indices, m_index_buffer, m_index_buffer_memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void App::createUniformBuffers()
{
	VkDeviceSize size = sizeof(UBO);

	m_uniform_buffers.resize(FRAMES);
	m_uniform_buffers_memory.resize(FRAMES);

	// NOTE(__LUNA__): Updates every frame so no need for staging buffer... or copy buffer?
	for (size_t i = 0; i < FRAMES; i++) {
		// NOTE(__LUNA__): This is a cross between createBuffer and createBuffers
		App::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniform_buffers[i], m_uniform_buffers_memory[i]);
	}
}

void App::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
	pool_sizes[0] = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = static_cast<uint32_t>(FRAMES)
	};
	pool_sizes[1] = {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = static_cast<uint32_t>(FRAMES)
	};

	VkDescriptorPoolCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
		.pPoolSizes = pool_sizes.data(),
		.maxSets = static_cast<uint32_t>(FRAMES),
		// NOTE(__LUNA__): Flags similar to command pool
		.flags = 0
	};
	if (vkCreateDescriptorPool(m_logical_device, &info, NULL, &m_descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create descriptor pool");
	}
}

void App::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(FRAMES, m_descriptor_layout);
	VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = m_descriptor_pool,
		.descriptorSetCount = static_cast<uint32_t>(FRAMES),
		.pSetLayouts = layouts.data()
	};
	m_descriptor_sets.resize(FRAMES);
	if (vkAllocateDescriptorSets(m_logical_device, &allocate_info, m_descriptor_sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to allocate descriptor sets");
	}
	for (size_t i = 0; i < FRAMES; i++) {
		VkDescriptorBufferInfo buffer_info = {
			.buffer = m_uniform_buffers[i],
			.offset = 0,
			// NOTE(__LUNA__): Rewrite whole buffer... More optimally would be to rewrite neccessary parts with offset and .range
			.range = VK_WHOLE_SIZE
		};

		VkDescriptorImageInfo image_info = {
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.imageView = m_texture_image_view,
			.sampler = m_texture_sampler
		};

		std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
		descriptor_writes[0] = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_descriptor_sets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			// NOTE(__LUNA__): Three possible buffers...
			.pBufferInfo = &buffer_info,
			.pImageInfo = NULL,
			.pTexelBufferView = NULL
		};
		descriptor_writes[1] = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_descriptor_sets[i],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			// NOTE(__LUNA__): Three possible buffers...
			.pBufferInfo = NULL,
			.pImageInfo = &image_info,
			.pTexelBufferView = NULL
		};
		vkUpdateDescriptorSets(m_logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, NULL);
	}
}

void App::updateUniformBuffer(uint32_t frame)
{
//	// TODO(__LUNA__): Try with glfwGetTime | Static for the win
	static auto last = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	float dt = std::chrono::duration<float, std::chrono::seconds::period>(now - last).count();
	last = now;

	static glm::vec3 camera_position = glm::vec3(2.0f, 2.0f, 2.0f);
	static glm::vec3 camera_front = glm::vec3(-2.0f, -2.0f, -2.0f);
	static glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);

	float yaw = m_input.getYaw();
	float pitch = m_input.getPitch();

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	camera_front = glm::normalize(direction);
	// NOTE(__LUNA__): Am I doing this wrong by throwing these asynchronous tasks onto the stack? i.e. Callbacks
	float camera_speed = 2.5f * dt;
	if (m_input.moveUp()) {
		camera_position += camera_speed * camera_front;
	}
	if (m_input.moveLeft()) {
		camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
	}
	if (m_input.moveDown()) {
		camera_position -= camera_speed * camera_front;
	}
	if (m_input.moveRight()) {
		camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
	}

	UBO ubo = {
		// NOTE(__LUNA__): Rotate around identity matrix in the z axis at 90 deg *RADIANS???* per second
		.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		.view = glm::lookAt(camera_position, camera_position + camera_front, camera_up),
		.proj  = glm::perspective(glm::radians(m_input.getFov()), static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height), 0.1f, 10.0f)
	};
	// NOTE(__LUNA__): Invert y coord
	ubo.proj[1][1] *= -1;

	void *data;
	vkMapMemory(m_logical_device, m_uniform_buffers_memory[frame], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_logical_device, m_uniform_buffers_memory[frame]);
}

void App::createCommandBuffers()
{
	m_command_buffers.resize(FRAMES);
	VkCommandBufferAllocateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size())
	};
	if (vkAllocateCommandBuffers(m_logical_device, &info, m_command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to allocate command buffer(s)");
	}
}

void App::createSyncObjects()
{
	m_image_semaphores.resize(FRAMES);
	m_render_semaphores.resize(FRAMES);
	m_fences.resize(FRAMES);
	VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		// NOTE(__LUNA__): Workaround to the indefinite fence on the first frame
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		if (vkCreateSemaphore(m_logical_device, &semaphore_info, NULL, &m_image_semaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_logical_device, &semaphore_info, NULL, &m_render_semaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_logical_device, &fence_info, NULL, &m_fences[i]) != VK_SUCCESS) {
			throw std::runtime_error("ERROR: Failed to create sync objects");
		}
	}
}

void App::recordCommandBuffer(uint32_t index)
{
	VkCommandBuffer command_buffer = m_command_buffers[m_current_frame];

	VkCommandBufferBeginInfo command_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		// NOTE(__LUNA__): Used for secondary command buffers
		.pInheritanceInfo = NULL
	};
	if (vkBeginCommandBuffer(command_buffer, &command_buffer_info) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to record command buffer");
	}
	// NOTE(__LUNA__): What is going on here? | Is it nested struct???
	std::array<VkClearValue, 2> values = {};
	// NOTE(__LUNA__): Order here should match attachment order | i.e. color then depth
	values[0] = {
		.color = {{0.2f, 0.3f, 0.3f, 1.0f}}
	};
	values[1] = {
		.depthStencil = {1.0f, 0}
	};
	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_render_pass,
		.framebuffer = m_swap_chain_framebuffers[index],
		.renderArea.offset = {0, 0},
		.renderArea.extent = m_extent,
		.clearValueCount = static_cast<uint32_t>(values.size()),
		.pClearValues = values.data()
	};
	// NOTE(__LUNA__): No secondary buffers
	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

	VkBuffer vertex_buffers[] = {m_vertex_buffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
	vkCmdBindIndexBuffer(command_buffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT32);

	// NOTE(__LUNA__: Scissor reference https://vulkan-tutorial.com/images/viewports_scissors.png
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width  = static_cast<float>(m_extent.width),
		.height = static_cast<float>(m_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = m_extent
	};

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_sets[m_current_frame], 0, NULL);
	// NOTE(__LUNA__): Vertex, instance, vertexIndex, instanceIndex
	// NOTE(__LUNA__): UPDATED use vertex buffer to draw
	vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
	vkCmdEndRenderPass(command_buffer);
	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to record command buffer");
	}
}

void App::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window.getWindow(), &width, &height);
	// NOTE(__LUNA__): Enter loop if window is minimized
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_window.getWindow(), &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_logical_device);

	App::cleanupSwapChain();

	App::createSwapChain();
	App::createImageViews();
	App::createColorResources();
	App::createDepthResources();
	App::createFramebuffers();
}

void App::draw()
{
	// NOTE(__LUNA__): VK_True waits for all fences MAX sets a timeout
	vkWaitForFences(m_logical_device, 1, &m_fences[m_current_frame], VK_TRUE, UINT64_MAX);

	uint32_t index;
	VkResult result = vkAcquireNextImageKHR(m_logical_device, m_swap_chain, UINT64_MAX, m_image_semaphores[m_current_frame], VK_NULL_HANDLE, &index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		App::recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("ERROR: Failed to acquire Swapchain image");
	}

	App::updateUniformBuffer(m_current_frame);

	vkResetFences(m_logical_device, 1, &m_fences[m_current_frame]);

	vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
	App::recordCommandBuffer(index);

	VkSemaphore wait_semaphores[] = {m_image_semaphores[m_current_frame]};
	VkSemaphore signal_semaphores[] = {m_render_semaphores[m_current_frame]};
	VkPipelineStageFlags stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = &stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_command_buffers[m_current_frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores
	};
	if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_fences[m_current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to submit draw command buffer");
	}
	VkSwapchainKHR swap_chains[] = {m_swap_chain};

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swap_chains,
		.pImageIndices = &index,
		// NOTE(__LUNA__): Check if swapchains presented successfully
		.pResults = NULL
	};
	result = vkQueuePresentKHR(m_present_queue, &present_info);

	// NOTE(__LUNA__): Check if swapchain is capable of presenting or recreating it for optimal results
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.resized()) {
		m_window.set_resized(false);
		App::recreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to present Swapchain image!");
	}

	m_current_frame = (m_current_frame + 1) % FRAMES;
}

void App::initVk()
{
	// NOTE(__LUNA__): I can take the App:: off and it works fine... Do I keep them on for convention? i.e. They are private members of App::
	App::createInstance();
	if (m_validate) App::createMessenger();
	App::createSurface();

	App::createPhysicalDevice();
	App::createLogicalDevice();

	App::createSwapChain();
	App::createImageViews();
	App::createRenderPass();
	App::createDescriptorLayout();
	App::createGraphicsPipeline();

	App::createCommandPool();
	App::createColorResources();
	App::createDepthResources();
	App::createFramebuffers();
	App::createTextureImage("../textures/viking_room.png");
	App::createTextureImageView();
	App::createTextureSampler();
	App::loadModels("../models/viking_room.obj");
	App::createVertexBuffers();
	App::createIndexBuffers();
	App::createUniformBuffers();
	App::createDescriptorPool();
	App::createDescriptorSets();
	App::createCommandBuffers();

	App::createSyncObjects();
}

void App::mainLoop()
{
	m_input.setWindow(m_window.getWindow());
	float last = 0.0f;
	while (!m_window.close()) {
		float current = glfwGetTime();
		float dt = current - last;
		last = current;

#ifdef NDEBUG
		(void) dt;
#else
		std::string title = "Wulkan | FPS: " + std::to_string(1 / dt);
		m_window.setTitle(title);
#endif // NDEBUG

		App::draw();
		glfwPollEvents();
	}
	vkDeviceWaitIdle(m_logical_device);
}

void App::cleanupSwapChain()
{
	vkDestroyImageView(m_logical_device, m_color_image_view, NULL);
	vkDestroyImage(m_logical_device, m_color_image, NULL);
	vkFreeMemory(m_logical_device, m_color_image_memory, NULL);

	vkDestroyImageView(m_logical_device, m_depth_image_view, NULL);
	vkDestroyImage(m_logical_device, m_depth_image, NULL);
	vkFreeMemory(m_logical_device, m_depth_image_memory, NULL);

	for (auto &framebuffer: m_swap_chain_framebuffers) {
		vkDestroyFramebuffer(m_logical_device, framebuffer, NULL);
	}
	// NOTE(__LUNA__): Doing this by reference. Is this correct?
	for (auto &image_view: m_swap_chain_image_views) {
		vkDestroyImageView(m_logical_device, image_view, NULL);
	}

	vkDestroySwapchainKHR(m_logical_device, m_swap_chain, NULL);
}

void App::cleanup()
{
	App::cleanupSwapChain();
	vkDestroySampler(m_logical_device, m_texture_sampler, NULL);
	vkDestroyImageView(m_logical_device, m_texture_image_view, NULL);
	vkDestroyImage(m_logical_device, m_texture_image, NULL);
	vkFreeMemory(m_logical_device, m_texture_image_memory, NULL);
	for (size_t i = 0; i < FRAMES; i++) {
		vkDestroyBuffer(m_logical_device, m_uniform_buffers[i], NULL);
		vkFreeMemory(m_logical_device, m_uniform_buffers_memory[i], NULL);
	}
	vkDestroyDescriptorPool(m_logical_device, m_descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(m_logical_device, m_descriptor_layout, NULL);
	vkDestroyBuffer(m_logical_device, m_index_buffer, NULL);
	vkFreeMemory(m_logical_device, m_index_buffer_memory, NULL);
	vkDestroyBuffer(m_logical_device, m_vertex_buffer, NULL);
	vkFreeMemory(m_logical_device, m_vertex_buffer_memory, NULL);
	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		vkDestroySemaphore(m_logical_device, m_image_semaphores[i], NULL);
		vkDestroySemaphore(m_logical_device, m_render_semaphores[i], NULL);
		vkDestroyFence(m_logical_device, m_fences[i], NULL);
	}
	vkDestroyCommandPool(m_logical_device, m_command_pool, NULL);
	vkDestroyPipeline(m_logical_device, m_graphics_pipeline, NULL);
	vkDestroyRenderPass(m_logical_device, m_render_pass, NULL);
	vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, NULL);
	vkDestroyDevice(m_logical_device, NULL);
	vkDestroySurfaceKHR(m_instance, m_surface, NULL);
	if (m_validate) destroyDebugMessenger(m_instance, m_messenger, NULL);
	vkDestroyInstance(m_instance, NULL);
	glfwTerminate();
}
