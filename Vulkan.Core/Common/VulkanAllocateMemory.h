#pragma once

#include "Common.h"
#include "../Utility.h"

namespace Graphics
{
	uint32 FindMemoryTypeIndex(const uint32 memoryTypeBits, const vulkanMemoryUsage_t usage);

	class VulkanBlock;

	struct VulkanAllocation {
		VulkanAllocation() :
			block(NULL),
			id(0),
			deviceMemory(VK_NULL_HANDLE),
			offset(0),
			size(0),
			data(NULL) {
		}

		VulkanBlock * block;
		uint32			id;
		VkDeviceMemory	deviceMemory;
		VkDeviceSize	offset;
		VkDeviceSize	size;
		byte *			data;
	};

	class VulkanBlock
	{
		friend class VulkanAllocator;

	public:
		VulkanBlock(const uint32 memoryTypeIndex, const VkDeviceSize size, vulkanMemoryUsage_t usage);
		~VulkanBlock();
		bool				Initialize();
		void				Shutdown();

		bool				IsHostVisible() const { return m_usage != VULKAN_MEMORY_USAGE_GPU_ONLY; }

		bool				Allocate(
			const uint32 size,
			const uint32 align,
			const VkDeviceSize granularity,
			const vulkanAllocationType_t allocType,
			VulkanAllocation & allocation);
		void				Free(VulkanAllocation & allocation);

		void				Print();

	private:
		struct chunk_t {
			uint32					id;
			VkDeviceSize			size;
			VkDeviceSize			offset;
			chunk_t *				prev;
			chunk_t *				next;
			vulkanAllocationType_t	type;
		};
		chunk_t *			m_head;

		uint32				m_nextBlockId;
		uint32				m_memoryTypeIndex;
		vulkanMemoryUsage_t	m_usage;
		VkDeviceMemory		m_deviceMemory;
		VkDeviceSize		m_size;
		VkDeviceSize		m_allocated;
		byte *				m_data;
	};


	typedef std::array<std::vector<VulkanBlock *>, VK_MAX_MEMORY_TYPES> vulkan_blocks;

	class VulkanAllocator
	{
	public:
		VulkanAllocator();
		void					Initialize();
		void					Shutdown();

		VulkanAllocation			Allocate(
			const uint32 size,
			const uint32 align,
			const uint32 memoryTypeBits,
			const vulkanMemoryUsage_t usage,
			const vulkanAllocationType_t allocType);
		void					Free(const VulkanAllocation allocation);
		void					EmptyGarbage();

		void					Print();

	private:
		int							m_garbageIndex;

		int							m_deviceLocalMemoryBytes;
		int							m_hostVisibleMemoryBytes;
		VkDeviceSize				m_bufferImageGranularity;

		vulkan_blocks			m_blocks;
		std::list<VulkanAllocation>	m_garbage[NUM_FRAME_DATA];
	};

#if defined( ID_USE_AMD_ALLOCATOR )
	extern VmaAllocator vmaAllocator;
#else
	extern VulkanAllocator vulkan_allocator;
#endif

}
