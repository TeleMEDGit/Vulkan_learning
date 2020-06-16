#pragma once
#include "Common/Common.h"





namespace Graphics
{

	struct Platform
	{

		std::vector<const char*> g_validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
		VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;
		//std::vector<std::string> g_instanceExtensions

		std::vector<const char*> g_instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME,VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

		std::vector<const char*> g_debugInstanceExtensions = { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };

		std::vector<const char*> g_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		const int NUM_FRAME_DATA = 2;
		const int NUM_TIMESTAMP_QUERIES = 16;
		const int r_swapInterval = 1;

	};

	//struct window_intance
	//{
	//	HWND m_hWnd;			//Window hook to 3D API
	//	HINSTANCE m_hInst;   //Instance hook to 3D API
	//};




	//



	//static const int g_numInstanceExtensions = 2;
	//static const char * g_instanceExtensions[g_numInstanceExtensions] = {
	//	VK_KHR_SURFACE_EXTENSION_NAME,
	//	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	//};

	/*static const int g_numDebugInstanceExtensions = 1;
	static const char * g_debugInstanceExtensions[g_numDebugInstanceExtensions] = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	static const int g_numDeviceExtensions = 1;
	static const char * g_deviceExtensions[g_numDeviceExtensions] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};*/

	//std::vector<char> g_deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, };
	//
	//static const int g_numValidationLayers = 1;
	//static const char * g_validationLayers[g_numValidationLayers] = {
	//	"VK_LAYER_LUNARG_standard_validation"
	//};



	void AddRequiredPlatformInstanceExtensions(std::vector<const char *> *instance_extensions);



}
