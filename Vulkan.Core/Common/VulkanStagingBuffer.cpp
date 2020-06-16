
#include "VulkanStagingBuffer.h"
#include "..\Graphics.h"




namespace Graphics
{

	const int r_vkUploadBufferSizeMB = 64;


	VulkanStagingBuffer staging_buffer;

	VulkanStagingBuffer::VulkanStagingBuffer() :
		m_maxBufferSize(0),
		m_currentBuffer(0),
		m_mappedData(NULL),
		m_memory(VK_NULL_HANDLE),
		m_commandPool(VK_NULL_HANDLE)
	{
	}


	VulkanStagingBuffer::~VulkanStagingBuffer()
	{
	}

	void VulkanStagingBuffer::Initialize()
	{
		m_maxBufferSize = (size_t)(r_vkUploadBufferSizeMB * 1024 * 1024);

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = m_maxBufferSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			m_buffers[i].offset = 0;

			VK_CHECK(vkCreateBuffer(m_device_context.device, &bufferCreateInfo, NULL, &m_buffers[i].buffer));
		}

		VkMemoryRequirements memoryRequirements;

		//I dont know why I loop here
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			vkGetBufferMemoryRequirements(m_device_context.device, m_buffers[i].buffer, &memoryRequirements);
		}

		const VkDeviceSize alignMod = memoryRequirements.size % memoryRequirements.alignment;
		const VkDeviceSize alignedSize = (alignMod == 0) ? memoryRequirements.size : (memoryRequirements.size + memoryRequirements.alignment - alignMod);

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = alignedSize * NUM_FRAME_DATA;
		memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, VULKAN_MEMORY_USAGE_CPU_TO_GPU);

		VK_CHECK(vkAllocateMemory(m_device_context.device, &memoryAllocateInfo, NULL, &m_memory));

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
		 VK_CHECK(vkBindBufferMemory(m_device_context.device, m_buffers[i].buffer, m_memory, i * alignedSize));
		}

		VK_CHECK(vkMapMemory(m_device_context.device, m_memory, 0, alignedSize * NUM_FRAME_DATA, 0, reinterpret_cast< void ** >(&m_mappedData)));

		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = m_device_context.graphic_family_index;
		VK_CHECK(vkCreateCommandPool(m_device_context.device, &commandPoolCreateInfo, NULL, &m_commandPool));

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			VK_CHECK(vkAllocateCommandBuffers(m_device_context.device, &commandBufferAllocateInfo, &m_buffers[i].commandBuffer));
			VK_CHECK(vkCreateFence(m_device_context.device, &fenceCreateInfo, NULL, &m_buffers[i].fence));
			VK_CHECK(vkBeginCommandBuffer(m_buffers[i].commandBuffer, &commandBufferBeginInfo));

			m_buffers[i].data = (byte *)m_mappedData + (i * alignedSize);
		}



	}
	void VulkanStagingBuffer::Shutdown()
	{
		vkUnmapMemory(m_device_context.device, m_memory);
		m_memory = VK_NULL_HANDLE;
		m_mappedData = NULL;

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			vkDestroyFence(m_device_context.device, m_buffers[i].fence, NULL);
			vkDestroyBuffer(m_device_context.device, m_buffers[i].buffer, NULL);
			vkFreeCommandBuffers(m_device_context.device, m_commandPool, 1, &m_buffers[i].commandBuffer);
		}
		memset(m_buffers, 0, sizeof(m_buffers));

		m_maxBufferSize = 0;
		m_currentBuffer = 0;
	}
	byte * VulkanStagingBuffer::Stage(const int size, const int alignment, VkCommandBuffer & commandBuffer, VkBuffer & buffer, int & bufferOffset)
	{
		if (size > m_maxBufferSize) {
			ERROR("Can't allocate %d MB in gpu transfer buffer", (int)(size / 1024 / 1024));
		}

		StagingBuffer * stage = &m_buffers[m_currentBuffer];
		const int alignMod = stage->offset % alignment;
		stage->offset = ((stage->offset % alignment) == 0) ? stage->offset : (stage->offset + alignment - alignMod);

		if ((stage->offset + size) >= (m_maxBufferSize) && !stage->submitted) {
			Flush();
		}

		stage = &m_buffers[m_currentBuffer];
		if (stage->submitted) {
			Wait(*stage);
		}

		commandBuffer = stage->commandBuffer;
		buffer = stage->buffer;
		bufferOffset = stage->offset;

		byte * data = stage->data + stage->offset;
		stage->offset += size;

		return data;
		return nullptr;
	}
	void VulkanStagingBuffer::Flush()
	{
		StagingBuffer & stage = m_buffers[m_currentBuffer];
		if (stage.submitted || stage.offset == 0) {
			return;
		}

		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
		vkCmdPipelineBarrier(
			stage.commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0, 1, &barrier, 0, NULL, 0, NULL);

		vkEndCommandBuffer(stage.commandBuffer);

		VkMappedMemoryRange memoryRange = {};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = m_memory;
		memoryRange.size = VK_WHOLE_SIZE;
		vkFlushMappedMemoryRanges(m_device_context.device, 1, &memoryRange);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &stage.commandBuffer;

		vkQueueSubmit(m_device_context.queue_graphics, 1, &submitInfo, stage.fence);

		stage.submitted = true;

		m_currentBuffer = (m_currentBuffer + 1) % NUM_FRAME_DATA;
	}
	void VulkanStagingBuffer::Wait(StagingBuffer & stage)
	{
		if (stage.submitted == false) {
			return;
		}

		VK_CHECK(vkWaitForFences(m_device_context.device, 1, &stage.fence, VK_TRUE, UINT64_MAX));
		VK_CHECK(vkResetFences(m_device_context.device, 1, &stage.fence));

		stage.offset = 0;
		stage.submitted = false;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CHECK(vkBeginCommandBuffer(stage.commandBuffer, &commandBufferBeginInfo));
	}
}
