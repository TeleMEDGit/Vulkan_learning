#include "VulkanFoundation.h"
//#include "External/3DModelFromObjFile.h"
#include "Common/VulkanStagingBuffer.h"
#include "Common/VulkanRenderProgram.h"
#include "Graphics.h"
#include "Common/VulkanVertexCatch.h"

Graphics::Platform platform;
//Graphics::DeviceContext m_device_context;
const int r_multiSamples = VK_SAMPLE_COUNT_1_BIT; //number of multisamples  for now let be 0  TDO global vars






static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64 obj, size_t location, int32 code,
	const char * layerPrefix, const char * msg, void * userData) {

	/*Utility::Printf("VK_DEBUG::%s: %s flags=%d, objType=%d, obj=%llu, location=%lld, code=%d\n",
		layerPrefix, msg, flags, objType, obj, location, code);*/

	std::cout << "VK_DEBUG:: " << layerPrefix << " " << msg << " " << std::endl;

	return VK_FALSE;
}

/*
=============
CreateDebugReportCallback
=============
*/
static void CreateDebugReportCallback(VkInstance instance) {
	VkDebugReportCallbackCreateInfoEXT callbackInfo = {};
	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackInfo.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	callbackInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugCallback;

	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	VK_VALIDATE(func != NULL, "Could not find vkCreateDebugReportCallbackEXT");
	VK_CHECK(func(instance, &callbackInfo, NULL, &platform.debugReportCallback));
}

/*
=============
DestroyDebugReportCallback
=============
*/
static void DestroyDebugReportCallback(VkInstance instance) {
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	VK_VALIDATE(func != NULL, "Could not find vkDestroyDebugReportCallbackEXT");
	func(instance, platform.debugReportCallback, NULL);
}

/*
=============
ValidateValidationLayers
=============
*/


static void ValidateValidationLayers() {
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);




	std::vector< VkLayerProperties > instanceLayers(instanceLayerCount);

	VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data()));

	bool found = false;
	for (uint32 i = 0; i < platform.g_validationLayers.size(); ++i) {
		for (uint32 j = 0; j < instanceLayerCount; ++j) {

			if (strcmp(platform.g_validationLayers[i], instanceLayers[j].layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			Utility::Printf("Cannot find validation layer: %s.\n", platform.g_validationLayers[i]);


		}
	}
}

static bool CheckPhysicalDeviceExtensionSupport(Graphics::GpuInfo & gpu, std::vector< const char * > & requiredExt) {
	size_t required = requiredExt.size();
	int available = 0;

	for (int i = 0; i < requiredExt.size(); ++i) {
		for (int j = 0; j < gpu.extension_props.size(); ++j) {
			if (strcmp(requiredExt[i], gpu.extension_props[j].extensionName) == 0) {
				available++;
				break;
			}
		}
	}

	return available == required;
}

VkSurfaceFormatKHR ChooseSurfaceFormat(std::vector< VkSurfaceFormatKHR > & formats) {
	VkSurfaceFormatKHR result;

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		result.format = VK_FORMAT_D32_SFLOAT_S8_UINT;// VK_FORMAT_B8G8R8A8_UNORM;
		result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		return result;
	}

	for (int i = 0; i < formats.size(); ++i) {
		VkSurfaceFormatKHR & fmt = formats[i];
		if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return fmt;
		}
	}



	return formats[0];
}

VkPresentModeKHR ChoosePresentMode(std::vector< VkPresentModeKHR > & modes, int swapInterval) {
	VkPresentModeKHR desiredMode = VK_PRESENT_MODE_FIFO_KHR;

	if (swapInterval < 1) {
		for (int i = 0; i < modes.size(); i++) {
			if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
			if ((modes[i] != VK_PRESENT_MODE_MAILBOX_KHR) && (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
				return VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	for (int i = 0; i < modes.size(); ++i) {
		if (modes[i] == desiredMode) {
			return desiredMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSurfaceExtent(VkSurfaceCapabilitiesKHR & caps) {
	VkExtent2D extent;

	if (caps.currentExtent.width == -1) {
		extent.width = Graphics::g_Width; //800;//Graphics:: win32.nativeScreenWidth;
		extent.height = Graphics::g_Height; //600;//win32.nativeScreenHeight;
	}
	else {
		extent = caps.currentExtent;
	}

	return extent;
}

static VkFormat ChooseSupportedFormat(VkPhysicalDevice physicalDevice, VkFormat * formats, int numFormats, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (int i = 0; i < numFormats; ++i) {
		VkFormat format = formats[i];

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	Utility::Print("Failed to find a supported format.");

	return VK_FORMAT_UNDEFINED;
}



namespace AppCore
{
	extern HWND g_hWnd;
	extern HINSTANCE g_hInst;
	extern uint32_t g_Width;
	extern uint32_t g_Height;
}



namespace Graphics
{
	static void ClearContext() {
		//m_device_context.jointCacheHandle = 0;
		m_device_context.gpu = GpuInfo();
		m_device_context.device = VK_NULL_HANDLE;
		m_device_context.graphic_family_index = -1;
		m_device_context.present_family_index = -1;
		m_device_context.queue_graphics = VK_NULL_HANDLE;
		m_device_context.queue_presents = VK_NULL_HANDLE;
		m_device_context.depth_format = VK_FORMAT_UNDEFINED;
		m_device_context.render_pass = VK_NULL_HANDLE;
		m_device_context.pipeline_cache = VK_NULL_HANDLE;
		m_device_context.sample_count = VK_SAMPLE_COUNT_1_BIT;
		m_device_context.super_sampling = false;
		//m_device_context.image_prams;
	}
	void VulkanFoundation::Initialize()
	{
		CreateInstance();

		CreateSurface();
		SelectSuitablePhysicalDevice();
		CreateLogicalDeviceAndQueues();
		CreateSemaphores();
		CreateQueryPool();
		CreateCommandPool();
		CreateCommandBuffer();

		// Setup the allocator
#if defined( ID_USE_AMD_ALLOCATOR )
		extern idCVar r_vkHostVisibleMemoryMB;
		extern idCVar r_vkDeviceLocalMemoryMB;

		VmaAllocatorCreateInfo createInfo = {};
		createInfo.physicalDevice = m_physicalDevice;
		createInfo.device = vkcontext.device;
		createInfo.preferredSmallHeapBlockSize = r_vkHostVisibleMemoryMB.GetInteger() * 1024 * 1024;
		createInfo.preferredLargeHeapBlockSize = r_vkDeviceLocalMemoryMB.GetInteger() * 1024 * 1024;

		vmaCreateAllocator(&createInfo, &vmaAllocator);
#else
		vulkan_allocator.Initialize();
#endif

		staging_buffer.Initialize();  //this called is doing what createBuffer in niagra is doing one plus more

		CreateSwapChain();

		CreateRenderTargets();

		CreateRenderPass();



		CreatePipelineCache();

		CreateFrameBuffers();



		render_program_manager.Initiliaze();

		//global_images->Initialize();    //this should find the appropraite position to call

		

		vertex_cache.Initialize(m_device_context.gpu.device_props.limits.minUniformBufferOffsetAlignment);

		FileUtility::Mesh _model;

		if (!Load3DModelFromObjFileObj("D:\\Development\\3rdParty\\3DModel\\cube.obj", true, true, true, false, _model)) {
			ERROR("Loading 3D model failled");
		}

		
	   vertex_cache.AllocStaticVertex((void*)&_model.Data[0], ALIGN(_model.Data.size() * sizeof(Math::DrawVertex), VERTEX_CACHE_ALIGN));  //upto uploadbuffer


	   //I think after this it shoud go to a render loop
	   //cross fingers
	   //also before crossing finger  I think here where data injected in framecames from.  
	   //	check how the data allacated above ties to the data in each frame 
	   //	may be this is the ahaha moment for endovor


	   //TDO it looks like we need GuiModel class which has 



	}
	
	void VulkanFoundation::CreateInstance()
	{
		VkApplicationInfo  application_info = {};

		//pApplicationInfo.apiVersion = 


		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName = "Expert Plus Khromo Spec";
		application_info.applicationVersion = 1;
		application_info.pEngineName = "iGasle 0.5";
		application_info.engineVersion = 1;
		application_info.apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);

		VkInstanceCreateInfo   create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &application_info;

		const bool enable_layers = true;  //create a global vars that can be loaded from disk or whatever

		m_instanceExtensions.clear();
		m_deviceExtensions.clear();
		m_validationLayers.clear();

		for (int i = 0; i < platform.g_instanceExtensions.size(); ++i) {
			m_instanceExtensions.push_back(platform.g_instanceExtensions[i]);
		}

		for (int i = 0; i < platform.g_deviceExtensions.size(); ++i) {
			m_deviceExtensions.push_back(platform.g_deviceExtensions[i]);
		}

		if (enable_layers) {
			for (int i = 0; i < platform.g_debugInstanceExtensions.size(); ++i) {
				m_instanceExtensions.push_back(platform.g_debugInstanceExtensions[i]);
			}

			for (int i = 0; i < platform.g_validationLayers.size(); ++i) {
				m_validationLayers.push_back(platform.g_validationLayers[i]);
			}

			ValidateValidationLayers();
		}

		create_info.enabledExtensionCount = (uint32_t)m_instanceExtensions.size();
		create_info.ppEnabledExtensionNames = m_instanceExtensions.data();
		create_info.enabledLayerCount = 0;	//m_validationLayers.size();
		create_info.ppEnabledLayerNames = nullptr;//m_validationLayers.data();


		VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_instance));

		if (enable_layers) {
			CreateDebugReportCallback(m_instance);
		}
	}

	VulkanFoundation::VulkanFoundation()
	{
		m_commandBufferRecorded.resize(NUM_FRAME_DATA);
		m_queryIndex.resize(NUM_FRAME_DATA);
	}

	VulkanFoundation::~VulkanFoundation()
	{
		auto afa = -1;
	}

	

	void VulkanFoundation::Shutdown()
	{
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			//Image::EmptyGarbage();
		}

		// Detroy Frame Buffers
		DestroyFrameBuffers();

		// Destroy Pipeline Cache
		vkDestroyPipelineCache(m_device_context.device, m_device_context.pipeline_cache, NULL);

		// Destroy Render Pass
		vkDestroyRenderPass(m_device_context.device, m_device_context.render_pass, NULL);

		// Destroy Render Targets
		DestroyRenderTargets();

		// Destroy Swap Chain
		DestroySwapChain();

		// Stop the Staging Manager
		//staging_buffer.Shutdown();

		// Destroy Command Buffer
		vkFreeCommandBuffers(m_device_context.device, m_commandPool, NUM_FRAME_DATA, m_commandBuffers.data());
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			vkDestroyFence(m_device_context.device, m_commandBufferFences[i], NULL);
		}

		// Destroy Command Pool
		vkDestroyCommandPool(m_device_context.device, m_commandPool, NULL);


		// Destroy Query Pools
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			vkDestroyQueryPool(m_device_context.device, m_queryPools[i], NULL);
		}

		// Destroy Semaphores
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			vkDestroySemaphore(m_device_context.device, m_acquireSemaphores[i], NULL);
			vkDestroySemaphore(m_device_context.device, m_renderCompleteSemaphores[i], NULL);
		}

		// Destroy Debug Callback
		// for let us assume it is there
		if (true) {
			DestroyDebugReportCallback(m_instance);
		}

		// Dump all our memory
#if defined( ID_USE_AMD_ALLOCATOR )
		vmaDestroyAllocator(vmaAllocator);
#else
		vulkan_allocator.Shutdown();
#endif

		// Destroy Logical Device
		vkDestroyDevice(m_device_context.device, NULL);

		// Destroy Surface
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);

		// Destroy the Instance
		vkDestroyInstance(m_instance, NULL);

		ClearContext();

		Clear();
	}

	void VulkanFoundation::Execute()
	{
		GL_StartFrame();

		//TDO system time needed hereBinding
		uint64 backEndStartTime = SystemTime::GetCurrentTick();

		// needed for editor rendering   TDO I do not why do i need
		GL_SetDefaultState();


		//Note Draw should be called here

		DrawView();

		GL_EndFrame();

		//TDO timing and loging 

		


	}

	void VulkanFoundation::Restart()
	{
		vkDeviceWaitIdle(m_device_context.device);

		//Image::EmptyGarbage();

		DestroyFrameBuffers();

		DestroyRenderTargets();

		DestroySwapChain();

		// Destroy Current Surface
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);

#if !defined( ID_USE_AMD_ALLOCATOR )
		vulkan_allocator.EmptyGarbage();
#endif

		CreateSurface();

		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_device_context.gpu.surface_caps));

		// Recheck presentation support
		VkBool32 supportsPresent = VK_FALSE;
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_device_context.present_family_index, m_surface, &supportsPresent));
		if (supportsPresent == VK_FALSE) {
			ERROR("idRenderBackend::ResizeImages: New surface does not support present?");
		}

		// Create New Swap Chain
		CreateSwapChain();

		// Create New Render Targets
		CreateRenderTargets();

		// Create New Frame Buffers
		CreateFrameBuffers();
	}

	
	void VulkanFoundation::BlockingSwapBuffers()
	{
		//Utility::Printf("***************** BlockingSwapBuffers *****************\n\n\n");
		m_counter++;
		m_currentFrameData = m_counter % NUM_FRAME_DATA;

		if (m_commandBufferRecorded[m_currentFrameData] == false) {
			return;
		}

		VK_CHECK(vkWaitForFences(m_device_context.device, 1, &m_commandBufferFences[m_currentFrameData], VK_TRUE, UINT64_MAX));

		VK_CHECK(vkResetFences(m_device_context.device, 1, &m_commandBufferFences[m_currentFrameData]));
		m_commandBufferRecorded[m_currentFrameData] = false;
	}

	void VulkanFoundation::SelectSuitablePhysicalDevice()
	{
		uint32 numDevices = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &numDevices, NULL));
		VK_VALIDATE(numDevices > 0, "vkEnumeratePhysicalDevices returned zero devices.");

		std::vector< VkPhysicalDevice > devices(numDevices);


		VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &numDevices, devices.data()));
		VK_VALIDATE(numDevices > 0, "vkEnumeratePhysicalDevices returned zero devices.");

		std::vector< GpuInfo> gpus(numDevices);


		for (uint32 i = 0; i < numDevices; ++i) {
			GpuInfo & gpu = gpus[i];
			gpu.physical_device = devices[i];

			{
				uint32 numQueues = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(gpu.physical_device, &numQueues, NULL);
				VK_VALIDATE(numQueues > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned zero queues.");

				gpu.queue_family_props.resize(numQueues);
				vkGetPhysicalDeviceQueueFamilyProperties(gpu.physical_device, &numQueues, gpu.queue_family_props.data());
				VK_VALIDATE(numQueues > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned zero queues.");
			}

			{
				uint32 numExtension;
				VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.physical_device, NULL, &numExtension, NULL));
				VK_VALIDATE(numExtension > 0, "vkEnumerateDeviceExtensionProperties returned zero extensions.");

				gpu.extension_props.resize(numExtension);
				VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.physical_device, NULL, &numExtension, gpu.extension_props.data()));
				VK_VALIDATE(numExtension > 0, "vkEnumerateDeviceExtensionProperties returned zero extensions.");
			}

			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.physical_device, m_surface, &gpu.surface_caps));

			{
				uint32 numFormats;
				VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical_device, m_surface, &numFormats, NULL));
				VK_VALIDATE(numFormats > 0, "vkGetPhysicalDeviceSurfaceFormatsKHR returned zero surface formats.");

				gpu.surface_formats.resize(numFormats);
				VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical_device, m_surface, &numFormats, gpu.surface_formats.data()));
				VK_VALIDATE(numFormats > 0, "vkGetPhysicalDeviceSurfaceFormatsKHR returned zero surface formats.");
			}

			{
				uint32 numPresentModes;
				VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physical_device, m_surface, &numPresentModes, NULL));
				VK_VALIDATE(numPresentModes > 0, "vkGetPhysicalDeviceSurfacePresentModesKHR returned zero present modes.");

				gpu.present_modes.resize(numPresentModes);
				VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physical_device, m_surface, &numPresentModes, gpu.present_modes.data()));
				VK_VALIDATE(numPresentModes > 0, "vkGetPhysicalDeviceSurfacePresentModesKHR returned zero present modes.");
			}

			vkGetPhysicalDeviceMemoryProperties(gpu.physical_device, &gpu.device_memory_props);
			vkGetPhysicalDeviceProperties(gpu.physical_device, &gpu.device_props);
			vkGetPhysicalDeviceFeatures(gpu.physical_device, &gpu.device_features);

			//gpus.push_back(gpu);
		}

		// Now try to select one
		for (int i = 0; i < gpus.size(); ++i) {
			GpuInfo  & gpu = gpus[i];

			int graphicsIdx = -1;
			int presentIdx = -1;

			if (!CheckPhysicalDeviceExtensionSupport(gpu, m_deviceExtensions)) {
				continue;
			}

			if (gpu.surface_formats.size() == 0) {
				continue;
			}

			if (gpu.present_modes.size() == 0) {
				continue;
			}

			// Find graphics queue family
			for (int j = 0; j < gpu.queue_family_props.size(); ++j) {
				VkQueueFamilyProperties & props = gpu.queue_family_props[j];

				if (props.queueCount == 0) {
					continue;
				}

				if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					graphicsIdx = j;
					break;
				}
			}

			// Find present queue family
			for (int j = 0; j < gpu.queue_family_props.size(); ++j) {
				VkQueueFamilyProperties & props = gpu.queue_family_props[j];

				if (props.queueCount == 0) {
					continue;
				}

				VkBool32 supportsPresent = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physical_device, j, m_surface, &supportsPresent);
				if (supportsPresent) {
					presentIdx = j;
					break;
				}
			}

			// Did we find a device supporting both graphics and present.
			if (graphicsIdx >= 0 && presentIdx >= 0) {
				m_device_context.graphic_family_index = graphicsIdx;
				m_device_context.present_family_index = presentIdx;
				m_physicalDevice = gpu.physical_device;
				m_device_context.gpu = gpu;

				return;
			}
		}

		// If we can't render or present, just bail.
		ERROR("Could not find a physical device which fits our desired profile");
	}

	void VulkanFoundation::CreateSurface()
	{
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

		//Graphics::m_window_instance.m_hWnd = g_hWnd;
		//Graphics::m_window_instance.m_hInst = hInst;
		createInfo.hinstance = AppCore::g_hInst;   //Graphics::m_window_instance.m_hInst;
		createInfo.hwnd = AppCore::g_hWnd;//Graphics::m_window_instance.m_hWnd;



		VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, NULL, &m_surface));
	}

	void VulkanFoundation::CreateLogicalDeviceAndQueues()
	{
		std::vector< int > uniqueIdx;

		//uniqueIndx  should always has a data enteries
		uniqueIdx.push_back(m_device_context.graphic_family_index);
		if (m_device_context.graphic_family_index != m_device_context.present_family_index)
			uniqueIdx.push_back(m_device_context.present_family_index);

		std::vector< VkDeviceQueueCreateInfo > devqInfo = {};

		const float priority = 1.0f;
		for (int i = 0; i < uniqueIdx.size(); ++i) {
			VkDeviceQueueCreateInfo qinfo = {};
			qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			qinfo.queueFamilyIndex = uniqueIdx[i];
			qinfo.queueCount = 1;
			qinfo.pQueuePriorities = &priority;




			devqInfo.push_back(qinfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.textureCompressionBC = VK_TRUE;
		deviceFeatures.imageCubeArray = VK_TRUE;
		deviceFeatures.depthClamp = VK_TRUE;
		deviceFeatures.depthBiasClamp = VK_TRUE;
		deviceFeatures.depthBounds = m_device_context.gpu.device_features.depthBounds;
		deviceFeatures.fillModeNonSolid = VK_TRUE;

		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = (uint32_t)devqInfo.size();
		info.pQueueCreateInfos = devqInfo.data();
		info.pEnabledFeatures = &deviceFeatures;
		info.enabledExtensionCount = (uint32_t)m_deviceExtensions.size();
		info.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (true) {   //needs a global vars
			info.enabledLayerCount = (uint32_t)m_validationLayers.size();
			info.ppEnabledLayerNames = m_validationLayers.data();
		}
		else {
			info.enabledLayerCount = 0;
		}
		VK_CHECK(vkCreateDevice(m_physicalDevice, &info, NULL, &m_device_context.device));


		vkGetDeviceQueue(m_device_context.device, m_device_context.graphic_family_index, 0, &m_device_context.queue_graphics);
		vkGetDeviceQueue(m_device_context.device, m_device_context.present_family_index, 0, &m_device_context.queue_presents);

	}

	void VulkanFoundation::CreateSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_acquireSemaphores.resize(platform.NUM_FRAME_DATA);
		m_renderCompleteSemaphores.resize(platform.NUM_FRAME_DATA);

		for (int i = 0; i < platform.NUM_FRAME_DATA; ++i) {
			VK_CHECK(vkCreateSemaphore(m_device_context.device, &semaphoreCreateInfo, NULL, &m_acquireSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(m_device_context.device, &semaphoreCreateInfo, NULL, &m_renderCompleteSemaphores[i]));
		}
	}

	void VulkanFoundation::CreateQueryPool()
	{
		VkQueryPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		createInfo.queryCount = platform.NUM_TIMESTAMP_QUERIES;

		m_queryPools.resize(platform.NUM_FRAME_DATA);

		for (int i = 0; i < platform.NUM_FRAME_DATA; ++i) {
			VK_CHECK(vkCreateQueryPool(m_device_context.device, &createInfo, NULL, &m_queryPools[i]));
		}
	}

	void VulkanFoundation::CreateCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = m_device_context.graphic_family_index;

		VK_CHECK(vkCreateCommandPool(m_device_context.device, &commandPoolCreateInfo, NULL, &m_commandPool));

	}

	void VulkanFoundation::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.commandBufferCount = platform.NUM_FRAME_DATA;


		m_commandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);
		m_commandBufferFences.resize(commandBufferAllocateInfo.commandBufferCount);

		VK_CHECK(vkAllocateCommandBuffers(m_device_context.device, &commandBufferAllocateInfo, m_commandBuffers.data()));

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		for (int i = 0; i < platform.NUM_FRAME_DATA; ++i) {
			VK_CHECK(vkCreateFence(m_device_context.device, &fenceCreateInfo, NULL, &m_commandBufferFences[i]));
		}
	}

	void VulkanFoundation::CreateSwapChain()
	{
		//TDO 
		//save old swapChain
		//read current screen size at ChooseSurfaceExtent call
		//at end if old swapChain is VK_NULL_HANDLE then vkDestroySwapchainKHR

		//VkSwapchainKHR oldSwapChain = m_swapchain;
		GpuInfo & gpu = m_device_context.gpu;

		VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(gpu.surface_formats);
		VkPresentModeKHR presentMode = ChoosePresentMode(gpu.present_modes, platform.r_swapInterval);   //TDO r_swapInterval should come from system vars that can be load from disk
		VkExtent2D extent = ChooseSurfaceExtent(gpu.surface_caps);

		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = m_surface;
		info.minImageCount = platform.NUM_FRAME_DATA;
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent = extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		//.oldSwapchain = oldSwapChain;


		if (m_device_context.graphic_family_index != m_device_context.present_family_index) {
			uint32 indices[] = { (uint32)m_device_context.graphic_family_index, (uint32)m_device_context.present_family_index };

			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = indices;
		}
		else {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = presentMode;
		info.clipped = VK_TRUE;

		VK_CHECK(vkCreateSwapchainKHR(m_device_context.device, &info, NULL, &m_swapchain));

		m_swapchainFormat = surfaceFormat.format;
		m_presentMode = presentMode;
		m_swapchainExtent = extent;
		m_fullscreen = 1; // win32.isFullscreen;  1 is place holder it should come from the AppCore

		uint32 numImages = 0;
		VK_CHECK(vkGetSwapchainImagesKHR(m_device_context.device, m_swapchain, &numImages, NULL));
		VK_VALIDATE(numImages > 0, "vkGetSwapchainImagesKHR returned a zero image count.");

		m_swapchainImages.resize(numImages);

		VK_CHECK(vkGetSwapchainImagesKHR(m_device_context.device, m_swapchain, &numImages, m_swapchainImages.data()));
		VK_VALIDATE(numImages > 0, "vkGetSwapchainImagesKHR returned a zero image count.");

		m_swapchainViews.resize(platform.NUM_FRAME_DATA);

		for (int i = 0; i < platform.NUM_FRAME_DATA; ++i) {
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = m_swapchainImages[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = m_swapchainFormat;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.flags = 0;

			VK_CHECK(vkCreateImageView(m_device_context.device, &imageViewCreateInfo, NULL, &m_swapchainViews[i]));
		}

		/*if (oldSwapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_device_context.device, oldSwapChain, nullptr);
		}*/
	}

	void VulkanFoundation::DestroySwapChain()
	{
		for (uint32 i = 0; i < NUM_FRAME_DATA; ++i) {
			vkDestroyImageView(m_device_context.device, m_swapchainViews[i], NULL);
		}
		m_swapchainViews.clear();

		vkDestroySwapchainKHR(m_device_context.device, m_swapchain, NULL);
	}

	void VulkanFoundation::CreateRenderTargets()
	{
		// Determine samples before creating depth
		VkImageFormatProperties fmtProps = {};
		vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, m_swapchainFormat,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &fmtProps);

		const int samples = r_multiSamples;

		m_device_context.sample_count = VK_SAMPLE_COUNT_1_BIT;
		

		if (samples >= 16 && (fmtProps.sampleCounts & VK_SAMPLE_COUNT_16_BIT)) {
			m_device_context.sample_count = VK_SAMPLE_COUNT_16_BIT;
		}
		else if (samples >= 8 && (fmtProps.sampleCounts & VK_SAMPLE_COUNT_8_BIT)) {
			m_device_context.sample_count = VK_SAMPLE_COUNT_8_BIT;
		}
		else if (samples >= 4 && (fmtProps.sampleCounts & VK_SAMPLE_COUNT_4_BIT)) {
			m_device_context.sample_count = VK_SAMPLE_COUNT_4_BIT;
		}
		else if (samples >= 2 && (fmtProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
			m_device_context.sample_count = VK_SAMPLE_COUNT_2_BIT;
		}

		// Select Depth Format
		{
			VkFormat formats[] = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT
			};
			m_device_context.depth_format = ChooseSupportedFormat(
				m_physicalDevice,
				formats, 3,
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}

		//NOTE:  this commented section suppose to delete and image with name of "_viewDepth"
		//and created/alocate as TEXTURE_FORMAT_DEPTH in the depthOptions and include to golbalImages
		//for this current RenderTargets

		ImageOperations depthOptions;
		depthOptions.format = TEXTURE_FORMAT_DEPTH;
		depthOptions.width = Graphics::g_Width;
		depthOptions.height = Graphics::g_Height;
		depthOptions.numLevels = 1;
		depthOptions.samples = static_cast< TextureSamples >(m_device_context.sample_count);

		global_images->m_scratch_depth=    global_images->ScratchImage("_viewDepth", depthOptions);

		if (m_device_context.sample_count > VK_SAMPLE_COUNT_1_BIT) {
			m_device_context.super_sampling = m_device_context.gpu.device_features.sampleRateShading == VK_TRUE;

			VkImageCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			createInfo.imageType = VK_IMAGE_TYPE_2D;
			createInfo.format = m_swapchainFormat;
			createInfo.extent.width = m_swapchainExtent.width;
			createInfo.extent.height = m_swapchainExtent.height;
			createInfo.extent.depth = 1;
			createInfo.mipLevels = 1;
			createInfo.arrayLayers = 1;
			createInfo.samples = m_device_context.sample_count;
			createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VK_CHECK(vkCreateImage(m_device_context.device, &createInfo, NULL, &m_msaaImage));

#if defined( ID_USE_AMD_ALLOCATOR )
			VmaMemoryRequirements vmaReq = {};
			vmaReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			ID_VK_CHECK(vmaCreateImage(vmaAllocator, &createInfo, &vmaReq, &m_msaaImage, &m_msaaVmaAllocation, &m_msaaAllocation));
#else
			VkMemoryRequirements memoryRequirements = {};
			vkGetImageMemoryRequirements(m_device_context.device, m_msaaImage, &memoryRequirements);

			m_msaaAllocation = vulkan_allocator.Allocate(
				memoryRequirements.size,
				memoryRequirements.alignment,
				memoryRequirements.memoryTypeBits,
				VULKAN_MEMORY_USAGE_GPU_ONLY,
				VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL);

			VK_CHECK(vkBindImageMemory(m_device_context.device, m_msaaImage, m_msaaAllocation.deviceMemory, m_msaaAllocation.offset));
#endif

			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.format = m_swapchainFormat;
			viewInfo.image = m_msaaImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VK_CHECK(vkCreateImageView(m_device_context.device, &viewInfo, NULL, &m_msaaImageView));
		}
	}

	void VulkanFoundation::DestroyRenderTargets()
	{
		vkDestroyImageView(m_device_context.device, m_msaaImageView, NULL);
#if defined( ID_USE_AMD_ALLOCATOR )
		vmaDestroyImage(vmaAllocator, m_msaaImage, m_msaaVmaAllocation);
		m_msaaAllocation = VmaAllocationInfo();
		m_msaaVmaAllocation = NULL;

#else
		vkDestroyImage(m_device_context.device, m_msaaImage, NULL);
		vulkan_allocator.Free(m_msaaAllocation);
		m_msaaAllocation = VulkanAllocation();
#endif

		m_msaaImage = VK_NULL_HANDLE;
		m_msaaImageView = VK_NULL_HANDLE;
	}

	void VulkanFoundation::CreateRenderPass()
	{
		VkAttachmentDescription attachments[3];
		memset(attachments, 0, sizeof(attachments));

		const bool resolve = m_device_context.sample_count > VK_SAMPLE_COUNT_1_BIT;

		VkAttachmentDescription & colorAttachment = attachments[0];
		colorAttachment.format = m_swapchainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkAttachmentDescription & depthAttachment = attachments[1];
		depthAttachment.format = m_device_context.depth_format;
		depthAttachment.samples = m_device_context.sample_count;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription & resolveAttachment = attachments[2];
		resolveAttachment.format = m_swapchainFormat;
		resolveAttachment.samples = m_device_context.sample_count;
		resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkAttachmentReference colorRef = {};
		colorRef.attachment = resolve ? 2 : 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthRef = {};
		depthRef.attachment = 1;
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference resolveRef = {};
		resolveRef.attachment = 0;
		resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pDepthStencilAttachment = &depthRef;
		if (resolve) {
			subpass.pResolveAttachments = &resolveRef;
		}

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = resolve ? 3 : 2;
		renderPassCreateInfo.pAttachments = attachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 0;

		VK_CHECK(vkCreateRenderPass(m_device_context.device, &renderPassCreateInfo, NULL, &m_device_context.render_pass));
	}

	void VulkanFoundation::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK(vkCreatePipelineCache(m_device_context.device, &pipelineCacheCreateInfo, NULL, &m_device_context.pipeline_cache));
	}

	void VulkanFoundation::CreateFrameBuffers()
	{
		VkImageView attachments[3] = {};

		// depth attachment is the same
		Image * depthImg = global_images->m_scratch_depth;  //global_images->GetImage("_viewDepth");
		if (depthImg == NULL) {
			ERROR("CreateFrameBuffers: No _viewDepth image.");
		}
		else {
			attachments[1] = depthImg->GetView();
		}

		const bool resolve = m_device_context.sample_count > VK_SAMPLE_COUNT_1_BIT;
		if (resolve) {
			attachments[2] = m_msaaImageView;
		}

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_device_context.render_pass;
		frameBufferCreateInfo.attachmentCount = resolve ? 3 : 2;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = Graphics::g_Width;//AppCore::g_Width;// //renderSystem->GetWidth();
		frameBufferCreateInfo.height = Graphics::g_Height;//AppCore::g_Height; // renderSystem->GetHeight();   just to complie
		frameBufferCreateInfo.layers = 1;

		//The spec valid usage text states 'If attachmentCount is not 0, 
		//	pAttachments must be a valid pointer to an array of attachmentCount valid VkImageView handles' 
		// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VUID-VkFramebufferCreateInfo-pAttachments-parameter

		m_frameBuffers.resize(NUM_FRAME_DATA);
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			attachments[0] = m_swapchainViews[i];
			VK_CHECK(vkCreateFramebuffer(m_device_context.device, &frameBufferCreateInfo, NULL, &m_frameBuffers[i]));
		}
	}

	void VulkanFoundation::DestroyFrameBuffers()
	{
		for (int i = 0; i < NUM_FRAME_DATA; ++i)
		{
			vkDestroyFramebuffer(m_device_context.device, m_frameBuffers[i], nullptr);
		}

		m_frameBuffers.clear();
	}

	void VulkanFoundation::DrawView()
	{
		//-------------------------------------------------
			// RB_BeginDrawingView
			//
			// Any mirrored or portaled views have already been drawn, so prepare
			// to actually render the visible surfaces for this view
			//
			// clear the z buffer, set the projection matrix, etc
			//-------------------------------------------------

			// set the window clipping
		//GL_Viewport(m_viewDef->viewport.x1,
		//	m_viewDef->viewport.y1,
		//	m_viewDef->viewport.x2 + 1 - m_viewDef->viewport.x1,
		//	m_viewDef->viewport.y2 + 1 - m_viewDef->viewport.y1);

		//// the scissor may be smaller than the viewport for subviews
		//GL_Scissor(m_viewDef->viewport.x1 + m_viewDef->scissor.x1,
		//	m_viewDef->viewport.y1 + m_viewDef->scissor.y1,
		//	m_viewDef->scissor.x2 + 1 - m_viewDef->scissor.x1,
		//	m_viewDef->scissor.y2 + 1 - m_viewDef->scissor.y1);
		//m_currentScissor = m_viewDef->scissor;

		// ensures that depth writes are enabled for the depth clear
		GL_State(GLS_DEFAULT | GLS_CULL_FRONTSIDED, true);

		// Clear the depth buffer and clear the stencil to 128 for stencil shadows as well as gui masking
		GL_Clear(true, true, true, STENCIL_SHADOW_TEST_VALUE, 0.13f, 0.31f, 0.81f, 0.0f);
	}

	void VulkanFoundation::DrawInteractions()
	{
		if (false /*r_skipInteractions.GetBool()*/) {
			return;
		}
		//TDO impliment when needed
	}

	void VulkanFoundation::Clear()
	{
		//m_counter = 0;
		//m_currentFrameData = 0;

		m_instance = VK_NULL_HANDLE;
		m_physicalDevice = VK_NULL_HANDLE;

		//debugReportCallback = VK_NULL_HANDLE;
		m_instanceExtensions.clear();
		m_deviceExtensions.clear();
		m_validationLayers.clear();

		m_surface = VK_NULL_HANDLE;
		m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

		m_fullscreen = 0;
		m_swapchain = VK_NULL_HANDLE;
		m_swapchainFormat = VK_FORMAT_UNDEFINED;
		m_currentSwapIndex = 0;
		m_msaaImage = VK_NULL_HANDLE;
		m_msaaImageView = VK_NULL_HANDLE;
		m_commandPool = VK_NULL_HANDLE;

		m_swapchainImages.clear();
		m_swapchainViews.clear();
		m_frameBuffers.clear();

		m_commandBuffers.clear();
		m_commandBufferFences.clear();
		m_commandBufferRecorded.clear();
		m_acquireSemaphores.clear();
		m_renderCompleteSemaphores.capacity();

		m_queryIndex.clear();
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			m_queryResults[i].fill(0);   //TDO this needs proper call per item
		}
		m_queryPools.clear();
	}

	void VulkanFoundation::GL_StartFrame()
	{
		VK_CHECK(vkAcquireNextImageKHR(m_device_context.device, m_swapchain, UINT64_MAX, m_acquireSemaphores[m_currentFrameData], VK_NULL_HANDLE, &m_currentSwapIndex));

		/*Image::EmptyGarbage();
#if !defined( ID_USE_AMD_ALLOCATOR )
		vulkan_allocator.EmptyGarbage();
#endif
		staging_buffer.Flush();
		render_program_manager.StartFrame();*/

		VkQueryPool queryPool = m_queryPools[m_currentFrameData];
		std::array< uint64, NUM_TIMESTAMP_QUERIES > & results = m_queryResults[m_currentFrameData];
		if (m_queryIndex[m_currentFrameData] > 0) {
			vkGetQueryPoolResults(m_device_context.device, queryPool, 0, 2,
				results.size(), results.data(), sizeof(uint64), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

			const uint64 gpuStart = results[0];
			const uint64 gpuEnd = results[1];
			const uint64 tick = (1000 * 1000 * 1000) / m_device_context.gpu.device_props.limits.timestampPeriod;

			//TDO replace this with proper counters
			//m_pc.gpuMicroSec = ((gpuEnd - gpuStart) * 1000 * 1000) / tick;

			m_queryIndex[m_currentFrameData] = 0;
		}


		VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrameData];

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		vkCmdResetQueryPool(commandBuffer, queryPool, 0, NUM_TIMESTAMP_QUERIES);
		std::vector<VkClearValue> vkClear = { { 0.2f, 0.5f, 0.8f, 1.0f }, {1.0f, 0} };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_device_context.render_pass;
		renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentSwapIndex];
		renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
		renderPassBeginInfo.pClearValues = vkClear.data();
		renderPassBeginInfo.clearValueCount = (uint32_t)vkClear.size();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, m_queryIndex[m_currentFrameData]++);

	}

	void VulkanFoundation::GL_EndFrame()
	{
		VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrameData];

		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPools[m_currentFrameData], m_queryIndex[m_currentFrameData]++);

		vkCmdEndRenderPass(commandBuffer);

		// Transition our swap image to present.
		// Do this instead of having the renderpass do the transition
		// so we can take advantage of the general layout to avoid 
		// additional image barriers.
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_swapchainImages[m_currentSwapIndex];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, NULL, 0, NULL, 1, &barrier);

		VK_CHECK(vkEndCommandBuffer(commandBuffer))
			m_commandBufferRecorded[m_currentFrameData] = true;

		VkSemaphore * acquire = &m_acquireSemaphores[m_currentFrameData];
		VkSemaphore * finished = &m_renderCompleteSemaphores[m_currentFrameData];

		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = acquire;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = finished;
		submitInfo.pWaitDstStageMask = &dstStageMask;

		VK_CHECK(vkQueueSubmit(m_device_context.queue_graphics, 1, &submitInfo, m_commandBufferFences[m_currentFrameData]));

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = finished;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &m_currentSwapIndex;

		VK_CHECK(vkQueuePresentKHR(m_device_context.queue_presents, &presentInfo));

		m_counter++;
		m_currentFrameData = m_counter % NUM_FRAME_DATA;
	}

	void VulkanFoundation::GL_State(uint64 stateBits, bool forceGlState)
	{
		m_glStateBits = stateBits | (m_glStateBits & GLS_KEEP);
		/*if (m_viewDef != NULL && m_viewDef->isMirror) {
			m_glStateBits |= GLS_MIRROR_VIEW;
		}*/
	}

	void VulkanFoundation::GL_SetDefaultState()
	{
		//Utility::Printf("--- GL_SetDefaultState ---\n");

		m_glStateBits = 0;

		GL_State(0, true);

		GL_Scissor(0, 0, Graphics::g_Width, Graphics::g_Height /*renderSystem->GetWidth(), renderSystem->GetHeight()*/);

		
	}

	void VulkanFoundation::GL_Scissor(int x, int y, int w, int h)
	{
		VkRect2D scissor;
		scissor.offset.x = x;
		scissor.offset.y = y;
		scissor.extent.width = w;
		scissor.extent.height = h;
		vkCmdSetScissor(m_commandBuffers[m_currentFrameData], 0, 1, &scissor);
	}

	void VulkanFoundation::GL_Viewport(int x, int y, int w, int h)
	{
		VkViewport viewport;
		viewport.x = x;
		viewport.y = y;
		viewport.width = w;
		viewport.height = h;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_commandBuffers[m_currentFrameData], 0, 1, &viewport);
	}

	void VulkanFoundation::GL_Clear(bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a)
	{
		/*Utility::Printf("GL_Clear( color=%d, depth=%d, stencil=%d, stencil=%d, r=%f, g=%f, b=%f, a=%f )\n",
			color, depth, stencil, stencilValue, r, g, b, a);*/

		uint32 numAttachments = 0;
		VkClearAttachment attachments[2];
		memset(attachments, 0, sizeof(attachments));

		if (color) {
			VkClearAttachment & attachment = attachments[numAttachments++];
			attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachment.colorAttachment = 0;
			VkClearColorValue & color = attachment.clearValue.color;
			color.float32[0] = r;
			color.float32[1] = g;
			color.float32[2] = b;
			color.float32[3] = a;
		}

		if (depth || stencil) {
			VkClearAttachment & attachment = attachments[numAttachments++];
			if (depth) {
				attachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			if (stencil) {
				attachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			attachment.clearValue.depthStencil.depth = 1.0f;
			attachment.clearValue.depthStencil.stencil = stencilValue;
		}

		VkClearRect clearRect = {};
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;
		clearRect.rect.extent = m_swapchainExtent;

		vkCmdClearAttachments(m_commandBuffers[m_currentFrameData], numAttachments, attachments, 1, &clearRect);
	}

	void VulkanFoundation::GL_PolygonOffset(float scale, float bias)
	{
		vkCmdSetDepthBias(m_commandBuffers[m_currentFrameData], bias, 0.0f, scale);
	}

	void VulkanFoundation::GL_Color(Math::Vector4 color)
	{
	}

	void VulkanFoundation::GL_Color(float * color)
	{
	}

}