// TODO(__LUNA__): Make a program that bundles all the TODO/NOTE/FIXME comments
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
// NOTE(__LUNA__): Unnecessary for latest glm... used for legacy glm
#define GLM_FORCE_RADIANS
// NOTE(__LUNA__): Doesn't fully solve alignment issues
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// NOTE(__LUNA__): Convert from OpenGL -1.0f, 1.0f range to 0.0f, 1.0f range
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// NOTE(__LUNA__): Anything in gtx folder is experimental
#define GLM_ENABLE_EXPIREMENTAL
#include <glm/gtx/hash.hpp>

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
	#define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN32

#include <iostream>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>

#include "Window.h"
#include "Input.h"

#define FRAMES 2

// NOTE(__LUNA__): Why does typedef not work here.. | Wat is typedef
typedef struct Queue {
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	bool complete() {
		return graphics.has_value() && present.has_value();
	}
} Queue;

typedef struct Swap_Chain {
	// NOTE(__LUNA__): Only supporting one format and one present_mode for now
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
} Swap_Chain;

typedef struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texture_coord;

	// NOTE(__LUNA__): Bind vertex data from gpu to vertex shader
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription description = {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		return description;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> description = {};
		// NOTE(__LUNA__): Vertex coords
		description[0].binding = 0;
		description[0].location = 0;
		description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		description[0].offset = offsetof(Vertex, pos);

		// NOTE(__LUNA__): Vertex colors
		description[1].binding = 0;
		description[1].location = 1;
		description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		description[1].offset = offsetof(Vertex, color);

		// NOTE(__LUNA__): Texture coords
		description[2].binding = 0;
		description[2].location = 2;
		description[2].format = VK_FORMAT_R32G32_SFLOAT;
		description[2].offset = offsetof(Vertex, texture_coord);

		return description;
	}
	bool operator==(const Vertex &other) const
	{
		return pos == other.pos && color == other.color && texture_coord == other.texture_coord;
	}
} Vertex;

typedef struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
} UBO;

class App {
public:
	void run();
private:
#ifdef NDEBUG
	const bool m_validate = false;
	const std::vector<const char *> m_validation_layers = {};
#else
	const bool m_validate = true;
	// NOTE(__LUNA__): Check debug validation layers | Device extensions
	const std::vector<const char *> m_validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};
#endif // NDEBUG
	VkDebugUtilsMessengerEXT m_messenger;

	// TODO(__LUNA__): Maybe refactor code and use snake_case for functions and camelCase for variables...?
	Window m_window;
	Input m_input;
	VkInstance m_instance;
	VkSurfaceKHR m_surface;

	std::vector<const char *> m_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkPhysicalDevice m_physical_device;
	VkDevice m_logical_device;
	VkSampleCountFlagBits m_ms_aa_samples = VK_SAMPLE_COUNT_1_BIT;

	VkSwapchainKHR m_swap_chain;
	std::vector<VkImage> m_swap_chain_images;
	std::vector<VkImageView> m_swap_chain_image_views;
	VkImageView m_texture_image_view;
	VkSampler m_texture_sampler;
	VkFormat m_format;
	VkExtent2D m_extent;

	VkRenderPass m_render_pass;
	VkDescriptorSetLayout m_descriptor_layout;
	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;

	std::vector<VkFramebuffer> m_swap_chain_framebuffers;
	VkCommandPool m_command_pool;
	VkDescriptorPool m_descriptor_pool;
	std::vector<VkDescriptorSet> m_descriptor_sets;

	VkBuffer m_vertex_buffer;
	VkDeviceMemory m_vertex_buffer_memory;
	uint32_t m_mip_levels;

	VkImage m_texture_image;
	VkDeviceMemory m_texture_image_memory;
	VkBuffer m_index_buffer;
	VkDeviceMemory m_index_buffer_memory;
	VkImage m_depth_image;
	VkDeviceMemory m_depth_image_memory;
	VkImageView m_depth_image_view;
	VkImage m_color_image;
	VkDeviceMemory m_color_image_memory;
	VkImageView m_color_image_view;

	std::vector<VkBuffer> m_uniform_buffers;
	std::vector<VkDeviceMemory> m_uniform_buffers_memory;

	std::vector<VkCommandBuffer> m_command_buffers;

	std::vector<VkSemaphore> m_image_semaphores, m_render_semaphores;
	std::vector<VkFence> m_fences;
	uint32_t m_current_frame = 0;
	// NOTE(__LUNA__): Do transfer queue challenge
	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

	// NOTE(__LUNA__): Triangle vertices with attached color values | Does openGL share same coord sys as Vulkan?
//	const std::vector<Vertex> m_vertices {
//		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//	};

	// NOTE(__LUNA__): Rectangle | Without use of index buffer
//	const std::vector<Vertex> m_vertices {
//		{{0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
//		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//
//		{{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//		{{0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
//		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//	};
	// NOTE(__LUNA__): Rectangle | With use of index buffer
//	const std::vector<Vertex> m_vertices = {
//		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
//	};
//	const std::vector<uint16_t> m_indices = {
//		0, 1, 2, 2, 3, 0
//	};
	// NOTE(__LUNA__): Rectangle | With use of texture coords
//	const std::vector<Vertex> m_vertices = {
//		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//	};
//	const std::vector<uint16_t> m_indices = {
//		0, 1, 2, 2, 3, 0
//	};
	// NOTE(__LUNA__): 3D rectangle
//	const std::vector<Vertex> m_vertices = {
//		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
//
//		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//	};
//	const std::vector<uint16_t> m_indices = {
//		0, 1, 2, 2, 3, 0,
//		4, 5, 6, 6, 7, 4
//	};
	// NOTE(__LUNA__): To be filled with .obj information
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
private:
	bool validateLayers();
	void createInstance();
	void createMessenger();
	void createSurface();

	void initVk();
	Queue findQueue(VkPhysicalDevice device);
	bool supportedDeviceExtension(VkPhysicalDevice device);
	Swap_Chain supportedSwapChain(VkPhysicalDevice device);
	bool supportedDevice(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxSamples();
	VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags flags);
	uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	void createPhysicalDevice();
	void createLogicalDevice();

	VkExtent2D swapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);
	void createSwapChain();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mip_levels);
	void createImageViews();
	void createRenderPass();
	VkShaderModule createShaderModule(const std::vector<char> &bytecode);

	void createDescriptorLayout();
	void createGraphicsPipeline();

	void createCommandPool();
	VkFormat findDepthFormat();
	void createColorResources();
	void createDepthResources();
	void createFramebuffers();
	void createImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags flags, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &memory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mip_levels);
	void createTextureImage(const char *file_path);
	void createTextureImageView();
	void createTextureSampler();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer &buffer, VkDeviceMemory &memory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer buffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
	void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
	template<typename T>
	void createBuffers(const std::vector<T>  &input, VkBuffer &buffer, VkDeviceMemory &memory, VkBufferUsageFlags flags);
	void loadModels(const char *file_path);
	void createVertexBuffers();
	void createIndexBuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateUniformBuffer(uint32_t frame);
	void createCommandBuffers();
	void createSyncObjects();
	void recordCommandBuffer(uint32_t index);

	void recreateSwapChain();
	void draw();
	void mainLoop();

	void cleanupSwapChain();
	void cleanup();
};
