#pragma once

#include "Common/Common.h"
#include "Platform.h"
#include "Common/Image.h"

namespace Graphics
{



	struct GpuInfo
	{
		VkPhysicalDevice physical_device;
		VkPhysicalDeviceProperties device_props;
		VkPhysicalDeviceMemoryProperties device_memory_props;
		VkPhysicalDeviceFeatures device_features;
		VkSurfaceCapabilitiesKHR surface_caps;
		std::vector<VkSurfaceFormatKHR> surface_formats;
		std::vector<VkPresentModeKHR> present_modes;
		std::vector<VkQueueFamilyProperties> queue_family_props;
		std::vector<VkExtensionProperties> extension_props;



	};




	struct DeviceContext
	{
		//vertCacheHandle_t				jointCacheHandle;
		GpuInfo gpu;
		VkDevice device;
		int graphic_family_index;
		int present_family_index;
		VkQueue queue_graphics;
		VkQueue queue_presents;
		VkFormat						depth_format;
		VkRenderPass					render_pass;
		VkPipelineCache					pipeline_cache;
		VkSampleCountFlagBits			sample_count;
		bool							super_sampling;

		std::array<Image *, MAX_IMAGE_PARMS> image_prams;

	};
	extern DeviceContext m_device_context;


	extern uint32_t g_Width;
	extern uint32_t g_Height;

	extern FrameTimeTick m_frameTiming;

	//std::vector<char const *>  desired_extensions;




	void Initialize();
	void Terminate();

	void Resize(uint32_t width, uint32_t height);
	void Shutdown(void);
	void Present(void);

	void SetCurrentTick();

	// Returns the number of elapsed frames since application start
	uint64_t GetFrameCount(void);

	// The amount of time elapsed during the last completed frame.  The CPU and/or
	// GPU may be idle during parts of the frame.  The frame time measures the time
	// between calls to present each frame.
	float GetFrameTime(void);

	// The total number of frames per second
	float GetFrameRate(void);




	//extern DeviceContext m_device_context;
	//extern Bootup g_vBootup;







}

