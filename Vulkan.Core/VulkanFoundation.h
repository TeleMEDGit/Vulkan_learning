#pragma once

//this foundation object is implimented as per Vulkan 1.1 Spec
//While reading and trying to make sence out of the content
//Also I refer the Chart created by Dustin Land for his "The First 1,000 Loc" presentation
//Written By Ahmed Omer
//Dated: Aptil 01, 2018


//#include "Common/VulkanAllocateMemory.h"
#include "Math/VectorMath.h"
//#include "Utility.h"
#include "Common/VulkanAllocateMemory.h"
#include "External/3DModelFromObjFile.h"
//#include "Common/Image.h"




namespace Graphics
{


	

	//struct GpuInfo
	//{
	//	VkPhysicalDevice physical_device;
	//	VkPhysicalDeviceProperties device_props;
	//	VkPhysicalDeviceMemoryProperties device_memory_props;
	//	VkPhysicalDeviceFeatures device_features;
	//	VkSurfaceCapabilitiesKHR surface_caps;
	//	std::vector<VkSurfaceFormatKHR> surface_formats;
	//	std::vector<VkPresentModeKHR> present_modes;
	//	std::vector<VkQueueFamilyProperties> queue_family_props;
	//	std::vector<VkExtensionProperties> extension_props;



	//};




	//struct DeviceContext
	//{
	//	//vertCacheHandle_t				jointCacheHandle;
	//	GpuInfo gpu;
	//	VkDevice device;
	//	int graphic_family_index;
	//	int present_family_index;
	//	VkQueue queue_graphics;
	//	VkQueue queue_presents;
	//	VkFormat						depth_format;
	//	VkRenderPass					render_pass;
	//	VkPipelineCache					pipeline_cache;
	//	VkSampleCountFlagBits			sample_count;
	//	bool							super_sampling;

	//	//std::array<Image *, MAX_IMAGE_PARMS> image_prams;

	//};
	//DeviceContext m_device_context;

	/*struct SwapChain
	{
		VkSwapchainKHR					swapchain;
	};

	extern SwapChain g_swapchain;*/





	class VulkanFoundation
	{
	public:
		VulkanFoundation();
		~VulkanFoundation();
		void Initialize();
		void Shutdown();
		//void Execute(const int numCmds, const std::array< renderCommand_t, 16> & renderCommands);
		void Execute();

		void Restart();

		void CreateInstance();
		void BlockingSwapBuffers();

		void SelectSuitablePhysicalDevice();
		void CreateSurface();
		void CreateLogicalDeviceAndQueues();
		void CreateSemaphores();
		void CreateQueryPool();
		void CreateCommandPool();
		void CreateCommandBuffer();
		void CreateSwapChain();
		void DestroySwapChain();
		void CreateRenderTargets();
		void DestroyRenderTargets();
		void CreateRenderPass();


		static void CreatePipelineCache();
		void				CreateFrameBuffers();

		void				DestroyFrameBuffers();


		//HWND m_hWindow = nullptr;//Graphics::m_hWnd;			//Window hook to 3D API
		//HINSTANCE m_hInstance = nullptr;//Graphics::m_hInst;

		//void				DrawView(const renderCommand_t & cmd);
		void				DrawView();

	private:
		

		
		void				DrawInteractions();
		
	private:

		void				Clear();
		void				GL_StartFrame();
		void				GL_EndFrame();
		void				GL_State(uint64 stateBits, bool forceGlState = false);
		void				GL_SetDefaultState();
		void				GL_Scissor(int x /* left*/, int y /* bottom */, int w, int h);
		void				GL_Viewport(int x /* left */, int y /* bottom */, int w, int h);

		void				GL_Clear(bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a);
		
		void				GL_PolygonOffset(float scale, float bias);

		

		void				GL_Color(Math::Vector4 color);
		ID_INLINE void		GL_Color(Math::Vector3 color) { GL_Color(Math::Vector4(color, 1.0f)); }
		void				GL_Color(float * color);

		




	public:
		// surfaces used for code-based drawing
		
	private:
		uint64				m_glStateBits;
		uint64							m_counter;
		uint32							m_currentFrameData;

		VkInstance m_instance;
		VkPhysicalDevice	m_physicalDevice;

		bool				m_currentRenderCopied;	// true if any material has already referenced _currentRender

		Math::Matrix4		m_prevMVP;				// world MVP from previous frame for motion blur

		unsigned short		m_gammaTable[256];	// brightness / gamma modify this

		std::vector< const char * >			m_instanceExtensions;
		std::vector< const char * >			m_deviceExtensions;
		std::vector< const char * >			m_validationLayers;

		VkSurfaceKHR					m_surface;

		VkPresentModeKHR				m_presentMode;
		int								m_fullscreen;
		VkSwapchainKHR					m_swapchain;
		VkFormat						m_swapchainFormat;
		VkExtent2D						m_swapchainExtent;
		uint32							m_currentSwapIndex;
		VkImage							m_msaaImage;
		VkImageView						m_msaaImageView;

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaAllocation					m_msaaVmaAllocation;
		VmaAllocationInfo				m_msaaAllocation;
#else
		VulkanAllocation				m_msaaAllocation;
#endif
		VkCommandPool					m_commandPool;


		std::vector< VkImage>			m_swapchainImages;  //size of NUM_FRAME_DATA
		std::vector< VkImageView>		m_swapchainViews;
		std::vector< VkFramebuffer>	    m_frameBuffers;

		std::vector< bool>				m_commandBufferRecorded;
		std::vector<VkSemaphore> m_acquireSemaphores;
		std::vector<VkSemaphore> m_renderCompleteSemaphores;

		std::vector< uint32>			m_queryIndex;
		std::vector<VkQueryPool>		m_queryPools;
		std::array< std::array< uint64, NUM_TIMESTAMP_QUERIES >, NUM_FRAME_DATA > m_queryResults;


		std::vector< VkCommandBuffer>	m_commandBuffers;
		std::vector< VkFence>			m_commandBufferFences;


	};


}


