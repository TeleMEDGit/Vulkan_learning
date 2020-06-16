#pragma once
//#include "Common.h"
//#include "..\Utility.h"
#include "VulkanAllocateMemory.h"




namespace Graphics
{
	
	struct StagingBuffer
	{
		StagingBuffer() :
			submitted(false),
			commandBuffer(VK_NULL_HANDLE),
			buffer(VK_NULL_HANDLE),
			fence(VK_NULL_HANDLE),
			offset(0),
			data(NULL) {}


		bool				submitted;
		VkCommandBuffer		commandBuffer;
		VkBuffer			buffer;
		VkFence				fence;
		VkDeviceSize		offset;
		byte *				data;
	};
	class VulkanStagingBuffer
	{
	public:
		 friend class VulkanFoundation;
		VulkanStagingBuffer();
		~VulkanStagingBuffer();

		void Initialize();
		void			Shutdown();
		byte *			Stage(const int size, const int alignment, VkCommandBuffer & commandBuffer, VkBuffer & buffer, int & bufferOffset);
		void			Flush();

	private:
		void			Wait(StagingBuffer & stage);
		int				m_maxBufferSize;
		int				m_currentBuffer;
		byte *			m_mappedData;
		VkDeviceMemory	m_memory;
		VkCommandPool	m_commandPool;

		StagingBuffer m_buffers[NUM_FRAME_DATA];   //this needs common place it same as NUM_FRAME_DATA

		
	};
	
	extern VulkanStagingBuffer staging_buffer;
}
