#include "Common.h"
#include "VulkanAllocateMemory.h"
#include "../Graphics.h"



namespace Graphics
{
	const int r_vkDeviceLocalMemoryMB = 128;
	const int r_vkHostVisibleMemoryMB = 64;


	class VulkanAllocator;

	/*
	========================
	SwapValues
	========================
	*/


	static const char * memoryUsageStrings[VULKAN_MEMORY_USAGES] = {
		"VULKAN_MEMORY_USAGE_UNKNOWN",
		"VULKAN_MEMORY_USAGE_GPU_ONLY",
		"VULKAN_MEMORY_USAGE_CPU_ONLY",
		"VULKAN_MEMORY_USAGE_CPU_TO_GPU",
		"VULKAN_MEMORY_USAGE_GPU_TO_CPU",
	};

	static const char * allocationTypeStrings[VULKAN_ALLOCATION_TYPES] = {
		"VULKAN_ALLOCATION_TYPE_FREE",
		"VULKAN_ALLOCATION_TYPE_BUFFER",
		"VULKAN_ALLOCATION_TYPE_IMAGE",
		"VULKAN_ALLOCATION_TYPE_IMAGE_LINEAR",
		"VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL",
	};

	/*
	=============
	IsOnSamePage

	Algorithm comes from the Vulkan 1.0.39 spec. "Buffer-Image Granularity"
	Also known as "Linear-Optimal Granularity"
	=============
	*/
	static bool IsOnSamePage(
		VkDeviceSize rAOffset, VkDeviceSize rASize,
		VkDeviceSize rBOffset, VkDeviceSize pageSize) {

		assert(rAOffset + rASize <= rBOffset && rASize > 0 && pageSize > 0);

		VkDeviceSize rAEnd = rAOffset + rASize - 1;
		VkDeviceSize rAEndPage = rAEnd & ~(pageSize - 1);
		VkDeviceSize rBStart = rBOffset;
		VkDeviceSize rBStartPage = rBStart & ~(pageSize - 1);

		return rAEndPage == rBStartPage;
	}

	/*
	=============
	HasGranularityConflict

	Check that allocation types obey buffer image granularity.
	=============
	*/
	static bool HasGranularityConflict(vulkanAllocationType_t type1, vulkanAllocationType_t type2) {
		if (type1 > type2) {
			SwapValues(type1, type2);
		}

		switch (type1) {
		case VULKAN_ALLOCATION_TYPE_FREE:
			return false;
		case VULKAN_ALLOCATION_TYPE_BUFFER:
			return	type2 == VULKAN_ALLOCATION_TYPE_IMAGE ||
				type2 == VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL;
		case VULKAN_ALLOCATION_TYPE_IMAGE:
			return  type2 == VULKAN_ALLOCATION_TYPE_IMAGE ||
				type2 == VULKAN_ALLOCATION_TYPE_IMAGE_LINEAR ||
				type2 == VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL;
		case VULKAN_ALLOCATION_TYPE_IMAGE_LINEAR:
			return type2 == VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL;
		case VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL:
			return false;
		default:
			assert(false);
			return true;
		}
	}

	uint32 FindMemoryTypeIndex(const uint32 memoryTypeBits, const vulkanMemoryUsage_t usage)
	{
		VkPhysicalDeviceMemoryProperties & physicalMemoryProperties = m_device_context.gpu.device_memory_props;

		VkMemoryPropertyFlags required = 0;
		VkMemoryPropertyFlags preferred = 0;

		switch (usage) {
		case VULKAN_MEMORY_USAGE_GPU_ONLY:
			preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case VULKAN_MEMORY_USAGE_CPU_ONLY:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		case VULKAN_MEMORY_USAGE_CPU_TO_GPU:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case VULKAN_MEMORY_USAGE_GPU_TO_CPU:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			break;
		default:
			ERROR("idVulkanAllocator::AllocateFromPools: Unknown memory usage.");
		}

		for (uint32 i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
			if (((memoryTypeBits >> i) & 1) == 0) {
				continue;
			}

			const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[i].propertyFlags;
			if ((properties & required) != required) {
				continue;
			}

			if ((properties & preferred) != preferred) {
				continue;
			}

			return i;
		}

		for (uint32 i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
			if (((memoryTypeBits >> i) & 1) == 0) {
				continue;
			}

			const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[i].propertyFlags;
			if ((properties & required) != required) {
				continue;
			}

			return i;
		}

		return UINT32_MAX;
	}




	VulkanBlock::VulkanBlock(const uint32 memoryTypeIndex, const VkDeviceSize size, vulkanMemoryUsage_t usage) : m_nextBlockId(0),
		m_size(size),
		m_allocated(0),
		m_memoryTypeIndex(memoryTypeIndex),
		m_usage(usage),
		m_deviceMemory(VK_NULL_HANDLE)
	{

	}

	VulkanBlock::~VulkanBlock()
	{

		Shutdown();

	}

	bool VulkanBlock::Initialize()
	{
		if (m_memoryTypeIndex == UINT64_MAX) {
			return false;
		}

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = m_size;
		memoryAllocateInfo.memoryTypeIndex = m_memoryTypeIndex;

		VK_CHECK(vkAllocateMemory(m_device_context.device, &memoryAllocateInfo, NULL, &m_deviceMemory))

			if (m_deviceMemory == VK_NULL_HANDLE) {
				return false;
			}

		if (IsHostVisible()) {
			VK_CHECK(vkMapMemory(m_device_context.device, m_deviceMemory, 0, m_size, 0, (void **)&m_data));
		}

		m_head = new chunk_t();
		m_head->id = m_nextBlockId++;
		m_head->size = m_size;
		m_head->offset = 0;
		m_head->prev = NULL;
		m_head->next = NULL;
		m_head->type = VULKAN_ALLOCATION_TYPE_FREE;

		return true;
	}

	void VulkanBlock::Shutdown()
	{
		// Unmap the memory
		if (IsHostVisible()) {
			vkUnmapMemory(m_device_context.device, m_deviceMemory);
		}

		// Free the memory
		vkFreeMemory(m_device_context.device, m_deviceMemory, NULL);
		m_deviceMemory = VK_NULL_HANDLE;

		chunk_t * prev = NULL;
		chunk_t * current = m_head;
		while (1) {
			if (current->next == NULL) {
				delete current;
				break;
			}
			else {
				prev = current;
				current = current->next;
				delete prev;
			}
		}

		m_head = NULL;
	}

	bool VulkanBlock::Allocate(const uint32 size, const uint32 align, const VkDeviceSize granularity, const vulkanAllocationType_t allocType, VulkanAllocation & allocation)
	{

		const VkDeviceSize freeSize = m_size - m_allocated;
		if (freeSize < size) {
			return false;
		}

		chunk_t * current = NULL;
		chunk_t * bestFit = NULL;
		chunk_t * previous = NULL;

		VkDeviceSize padding = 0;
		VkDeviceSize offset = 0;
		VkDeviceSize alignedSize = 0;

		for (current = m_head; current != NULL; previous = current, current = current->next) {
			if (current->type != VULKAN_ALLOCATION_TYPE_FREE) {
				continue;
			}

			if (size > (uint32_t)current->size) {
				continue;
			}

			offset = ALIGN(current->offset, align);

			// Check for linear/optimal granularity conflict with previous allocation
			if (previous != NULL && granularity > 1) {
				if (IsOnSamePage(previous->offset, previous->size, offset, granularity)) {
					if (HasGranularityConflict(previous->type, allocType)) {
						offset = ALIGN(offset, granularity);
					}
				}
			}

			padding = offset - current->offset;
			alignedSize = padding + size;

			if (alignedSize > current->size) {
				continue;
			}

			if (alignedSize + m_allocated >= m_size) {
				return false;
			}

			if (granularity > 1 && current->next != NULL) {
				chunk_t * next = current->next;
				if (IsOnSamePage(offset, size, next->offset, granularity)) {
					if (HasGranularityConflict(allocType, next->type)) {
						continue;
					}
				}
			}

			bestFit = current;
			break;
		}

		if (bestFit == NULL) {
			return false;
		}

		if (bestFit->size > size) {
			chunk_t * chunk = new chunk_t();
			chunk_t * next = bestFit->next;

			chunk->id = m_nextBlockId++;
			chunk->prev = bestFit;
			bestFit->next = chunk;

			chunk->next = next;
			if (next) {
				next->prev = chunk;
			}

			chunk->size = bestFit->size - alignedSize;
			chunk->offset = offset + size;
			chunk->type = VULKAN_ALLOCATION_TYPE_FREE;
		}

		bestFit->type = allocType;
		bestFit->size = size;

		m_allocated += alignedSize;

		allocation.size = bestFit->size;
		allocation.id = bestFit->id;
		allocation.deviceMemory = m_deviceMemory;
		if (IsHostVisible()) {
			allocation.data = m_data + offset;
		}
		allocation.offset = offset;
		allocation.block = this;

		return true;
	}

	void VulkanBlock::Free(VulkanAllocation & allocation)
	{
		chunk_t * current = NULL;


		for (current = m_head; current != NULL; current = current->next) {
			if (current->id == allocation.id) {
				break;
			}
		}

		if (current == NULL) {
			ERROR("idVulkanBlock::Free: Tried to free an unknown allocation. %p - %lu", this, allocation.id);
			return;
		}

		current->type = VULKAN_ALLOCATION_TYPE_FREE;

		if (current->prev && current->prev->type == VULKAN_ALLOCATION_TYPE_FREE) {
			chunk_t * prev = current->prev;

			prev->next = current->next;
			if (current->next) {
				current->next->prev = prev;
			}

			prev->size += current->size;

			delete current;
			current = prev;
		}

		if (current->next && current->next->type == VULKAN_ALLOCATION_TYPE_FREE) {
			chunk_t * next = current->next;

			if (next->next) {
				next->next->prev = current;
			}

			current->next = next->next;

			current->size += next->size;

			delete next;
		}

		m_allocated -= allocation.size;

	}

	void VulkanBlock::Print()
	{
		/*int count = 0;
		for (chunk_t * current = m_head; current != NULL; current = current->next) { count++; }

		Utility::Printf("Type Index: %lu\n", m_memoryTypeIndex);
		Utility::Printf("Usage:      %s\n", memoryUsageStrings[m_usage]);
		Utility::Printf("Count:      %d\n", count);
		Utility::Printf("Size:       %llu\n", m_size);
		Utility::Printf("Allocated:  %llu\n", m_allocated);
		Utility::Printf("Next Block: %lu\n", m_nextBlockId);
		Utility::Printf("------------------------\n");

		for (chunk_t * current = m_head; current != NULL; current = current->next) {
			Utility::Printf("{\n");

			Utility::Printf("\tId:     %lu\n", current->id);
			Utility::Printf("\tSize:   %llu\n", current->size);
			Utility::Printf("\tOffset: %llu\n", current->offset);
			Utility::Printf("\tType:   %s\n", allocationTypeStrings[current->type]);

			Utility::Printf("}\n");
		}

		Utility::Printf("\n");*/
	}

#if defined( ID_USE_AMD_ALLOCATOR )
	VmaAllocator vmaAllocator;
#else
	VulkanAllocator vulkan_allocator;
#endif


	VulkanAllocator::VulkanAllocator() :
		m_garbageIndex(0),
		m_deviceLocalMemoryBytes(0),
		m_hostVisibleMemoryBytes(0),
		m_bufferImageGranularity(0) {

	}

	void VulkanAllocator::Initialize()
	{
		m_deviceLocalMemoryBytes = r_vkDeviceLocalMemoryMB * 1024 * 1024;
		m_hostVisibleMemoryBytes = r_vkHostVisibleMemoryMB * 1024 * 1024;
		m_bufferImageGranularity = m_device_context.gpu.device_props.limits.bufferImageGranularity;
	}

	void VulkanAllocator::Shutdown()
	{
		EmptyGarbage();
		/*for each (auto block in m_blocks)
		{
			block.clear();

		}*/

		for (int i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
			auto & blocks = m_blocks[i];
			const size_t numBlocks = blocks.size();
			for (size_t j = 0; j < numBlocks; ++j) {
				//delete blocks[j] //= nullptr;  delete is missing 

				delete blocks[j];

				//TDO delete blocks
			}

			blocks.clear();
		}
	}

	VulkanAllocation VulkanAllocator::Allocate(const uint32 size, const uint32 align, const uint32 memoryTypeBits, const vulkanMemoryUsage_t usage, const vulkanAllocationType_t allocType)
	{
		VulkanAllocation allocation;

		uint32 memoryTypeIndex = FindMemoryTypeIndex(memoryTypeBits, usage);
		if (memoryTypeIndex == UINT32_MAX) {
			ERROR("idVulkanAllocator::Allocate: Unable to find a memoryTypeIndex for allocation request.");
		}

		std::vector< VulkanBlock * > & blocks(m_blocks[memoryTypeIndex]);
		const size_t numBlocks = blocks.size();
		for each (auto block in blocks)
		{
			if (block->m_memoryTypeIndex != memoryTypeIndex) {
				continue;
			}
			if (block->Allocate(size, align, m_bufferImageGranularity, allocType, allocation)) {
				return allocation;
			}
		}

		/*const int numBlocks = blocks.size();
		for (int i = 0; i < numBlocks; ++i) {
			VulkanBlock * block = blocks[i];

			if (block->m_memoryTypeIndex != memoryTypeIndex) {
				continue;
			}

			if (block->Allocate(size, align, m_bufferImageGranularity, allocType, allocation)) {
				return allocation;
			}
		}
*/
		VkDeviceSize blockSize = (usage == VULKAN_MEMORY_USAGE_GPU_ONLY) ? m_deviceLocalMemoryBytes : m_hostVisibleMemoryBytes;

		VulkanBlock * block = new VulkanBlock(memoryTypeIndex, blockSize, usage);
		if (block->Initialize()) {
			blocks.push_back(block);
		}
		else {
			ERROR("idVulkanAllocator::Allocate: Could not allocate new memory block.");
		}

		block->Allocate(size, align, m_bufferImageGranularity, allocType, allocation);

		return allocation;
	}

	void VulkanAllocator::Free(const VulkanAllocation allocation)
	{
		m_garbage[m_garbageIndex].push_back(allocation);
	}

	void VulkanAllocator::EmptyGarbage()
	{
		m_garbageIndex = (m_garbageIndex + 1) % NUM_FRAME_DATA;

		std::list< VulkanAllocation > & garbage = m_garbage[m_garbageIndex];

		for each (auto allocation in garbage)
		{
			if (allocation.block != nullptr)
			{
				allocation.block->Free(allocation);
				if (allocation.block->m_allocated == 0) {
					m_blocks[allocation.block->m_memoryTypeIndex];
					delete allocation.block;
					allocation.block = NULL;
				}
			}
		}



		/*const int numAllocations = garbage.size();
		for (int i = 0; i < numAllocations; ++i) {
			VulkanAllocation allocation = garbage[i];

			allocation.block->Free(allocation);

			if (allocation.block->m_allocated == 0) {
				m_blocks[allocation.block->m_memoryTypeIndex].remove(allocation.block);
				delete allocation.block;
				allocation.block = NULL;
			}
		}*/

		garbage.clear();
	}

	void VulkanAllocator::Print()
	{
		/*Utility::Printf("Device Local MB: %d\n", int(m_deviceLocalMemoryBytes / 1024 * 1024));
		Utility::Printf("Host Visible MB: %d\n", int(m_hostVisibleMemoryBytes / 1024 * 1024));
		Utility::Printf("Buffer Granularity: %llu\n", m_bufferImageGranularity);
		Utility::Printf("\n");

		for (int i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
			std::list< VulkanBlock * > & blocksByType = m_blocks[i];

			const int numBlocks = blocksByType.size();
			for (int j = 0; j < numBlocks; ++j) {
				blocksByType[j]->Print();
			}
		}*/
	}

}