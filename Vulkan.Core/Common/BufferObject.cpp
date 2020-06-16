
#include "BufferObject.h"
#include "..\Utility.h"


#include "..\VulkanFoundation.h"
#include "VulkanStagingBuffer.h"
#include "..\Graphics.h"

namespace  Graphics
{
	int r_showBuffers = 0;

	


	void UnbindBufferObjects()
	{

	}

	bool IsWriteCombined(void * base)
	{
		MEMORY_BASIC_INFORMATION info;
		SIZE_T size = VirtualQueryEx(GetCurrentProcess(), base, &info, sizeof(info));
		if (size == 0) {
			DWORD error = GetLastError();
			error = error;
			return false;
		}
		bool isWriteCombined = ((info.AllocationProtect & PAGE_WRITECOMBINE) != 0);
		return isWriteCombined;
	}

#ifdef ID_WIN_X86_SSE2_INTRIN

	void CopyBuffer(byte * dst, const byte * src, int numBytes)
	{
		assert_16_byte_aligned(dst);
		assert_16_byte_aligned(src);

		int i = 0;
		for (; i + 128 <= numBytes; i += 128) {
			__m128i d0 = _mm_load_si128((__m128i *)&src[i + 0 * 16]);
			__m128i d1 = _mm_load_si128((__m128i *)&src[i + 1 * 16]);
			__m128i d2 = _mm_load_si128((__m128i *)&src[i + 2 * 16]);
			__m128i d3 = _mm_load_si128((__m128i *)&src[i + 3 * 16]);
			__m128i d4 = _mm_load_si128((__m128i *)&src[i + 4 * 16]);
			__m128i d5 = _mm_load_si128((__m128i *)&src[i + 5 * 16]);
			__m128i d6 = _mm_load_si128((__m128i *)&src[i + 6 * 16]);
			__m128i d7 = _mm_load_si128((__m128i *)&src[i + 7 * 16]);
			_mm_stream_si128((__m128i *)&dst[i + 0 * 16], d0);
			_mm_stream_si128((__m128i *)&dst[i + 1 * 16], d1);
			_mm_stream_si128((__m128i *)&dst[i + 2 * 16], d2);
			_mm_stream_si128((__m128i *)&dst[i + 3 * 16], d3);
			_mm_stream_si128((__m128i *)&dst[i + 4 * 16], d4);
			_mm_stream_si128((__m128i *)&dst[i + 5 * 16], d5);
			_mm_stream_si128((__m128i *)&dst[i + 6 * 16], d6);
			_mm_stream_si128((__m128i *)&dst[i + 7 * 16], d7);
		}
		for (; i + 16 <= numBytes; i += 16) {
			__m128i d = _mm_load_si128((__m128i *)&src[i]);
			_mm_stream_si128((__m128i *)&dst[i], d);
		}
		for (; i + 4 <= numBytes; i += 4) {
			*(uint32 *)&dst[i] = *(const uint32 *)&src[i];
		}
		for (; i < numBytes; i++) {
			dst[i] = src[i];
		}
		_mm_sfence();
	}

#else

	/*
	========================
	CopyBuffer
	========================
	*/
	void CopyBuffer(byte * dst, const byte * src, int numBytes) {
		assert_16_byte_aligned(dst);
		assert_16_byte_aligned(src);
		memcpy(dst, src, numBytes);
	}

#endif


	BufferObject::BufferObject()
	{
		m_size = 0;
		m_offsetInOtherBuffer = OWNS_BUFFER_FLAG;
		m_usage = BUFFER_USAGE_STATIC;

#if defined( VULKAN )
		m_apiObject = VK_NULL_HANDLE;
#if defined( ID_USE_AMD_ALLOCATOR )
		m_vmaAllocation = NULL;
#endif
#else
		m_apiObject = 0xFFFF;
		m_buffer = NULL;
#endif
	}

	IndexBuffer::IndexBuffer()
	{
		SetUnmapped();
	}

	IndexBuffer::~IndexBuffer()
	{
		FreeBufferObject();
	}

	bool IndexBuffer::AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage)
	{
		assert(m_apiObject == VK_NULL_HANDLE);
		assert_16_byte_aligned(data);

		if (allocSize <= 0) {
			ERROR("idIndexBuffer::AllocBufferObject: allocSize = %i", allocSize);
		}

		m_size = allocSize;
		m_usage = usage;

		bool allocationFailed = false;

		int numBytes = GetAllocedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (m_usage == BUFFER_USAGE_STATIC) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaMemoryRequirements vmaReq = {};
		if (m_usage == BU_STATIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (m_usage == BU_DYNAMIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			vmaReq.flags = VMA_MEMORY_REQUIREMENT_PERSISTENT_MAP_BIT;
		}

		ID_VK_CHECK(vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &vmaReq, &m_apiObject, &m_vmaAllocation, &m_allocation));

#else
		VkResult ret = vkCreateBuffer(m_device_context.device, &bufferCreateInfo, NULL, &m_apiObject);
		assert(ret == VK_SUCCESS);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_device_context.device, m_apiObject, &memoryRequirements);

		vulkanMemoryUsage_t memUsage = (m_usage == BUFFER_USAGE_STATIC) ? VULKAN_MEMORY_USAGE_GPU_ONLY : VULKAN_MEMORY_USAGE_CPU_TO_GPU;

		m_allocation = vulkan_allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VULKAN_ALLOCATION_TYPE_BUFFER);

		VK_CHECK(vkBindBufferMemory(m_device_context.device, m_apiObject, m_allocation.deviceMemory, m_allocation.offset));
#endif

		if (r_showBuffers) {
			Utility::Printf("index buffer alloc %p, (%i bytes)\n", this, GetSize());
		}

		// copy the data
		if (data != NULL) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void IndexBuffer::FreeBufferObject()
	{
		if (IsMapped()) {
			UnmapBuffer();
		}

		// if this is a sub-allocation inside a larger buffer, don't actually free anything.
		if (OwnsBuffer() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (m_apiObject == VK_NULL_HANDLE) {
			return;
		}

		if (r_showBuffers) {
			Utility::Printf("index buffer free %p, (%i bytes)\n", this, GetSize());
		}

		if (m_apiObject != VK_NULL_HANDLE) {
#if defined( ID_USE_AMD_ALLOCATOR )
			vmaDestroyBuffer(vmaAllocator, m_apiObject, m_vmaAllocation);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VmaAllocationInfo();
			m_vmaAllocation = NULL;
#else
			vulkan_allocator.Free(m_allocation);
			vkDestroyBuffer(m_device_context.device, m_apiObject, NULL);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VulkanAllocation();
#endif
		}

		ClearWithoutFreeing();
	}

	void IndexBuffer::Reference(const IndexBuffer & other)
	{
		assert(IsMapped() == false);
		//assert( other.IsMapped() == false );	// this happens when building idTriangles while at the same time setting up triIndex_t
		assert(other.GetSize() > 0);

		FreeBufferObject();
		m_size = other.GetSize();					// this strips the MAPPED_FLAG
		m_offsetInOtherBuffer = other.GetOffset();	// this strips the OWNS_BUFFER_FLAG
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void IndexBuffer::Reference(const IndexBuffer & other, int refOffset, int refSize)
	{
		assert(IsMapped() == false);
		//assert( other.IsMapped() == false );	// this happens when building idTriangles while at the same time setting up triIndex_t
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		m_size = refSize;
		m_offsetInOtherBuffer = other.GetOffset() + refOffset;
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void IndexBuffer::Update(const void * data, int size, int offset) const
	{
		assert(m_apiObject != VK_NULL_HANDLE);
		assert_16_byte_aligned(data);
		assert((GetOffset() & 15) == 0);

		if (size > GetSize()) {
			ERROR("idIndexBuffer::Update: size overrun, %i > %i\n", size, GetSize());
		}

		if (m_usage == BUFFER_USAGE_DYNAMIC) {
			CopyBuffer(
#if defined( ID_USE_AMD_ALLOCATOR )
				(byte *)m_allocation.pMappedData + GetOffset() + offset,
#else
				m_allocation.data + GetOffset() + offset,
#endif
				(const byte *)data, size);
		}
		else {
			VkBuffer stageBuffer;
			VkCommandBuffer commandBuffer;
			int stageOffset = 0;
			byte * stageData = staging_buffer.Stage(size, 1, commandBuffer, stageBuffer, stageOffset);

			memcpy(stageData, data, size);

			VkBufferCopy bufferCopy = {};
			bufferCopy.srcOffset = stageOffset;
			bufferCopy.dstOffset = GetOffset() + offset;
			bufferCopy.size = size;

			vkCmdCopyBuffer(commandBuffer, stageBuffer, m_apiObject, 1, &bufferCopy);
		}

	}

	void * IndexBuffer::MapBuffer(bufferMapType_t mapType)
	{
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idIndexBuffer::MapBuffer: Cannot map a buffer marked as BU_STATIC.");
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		void * buffer = (byte *)m_allocation.pMappedData + GetOffset();
#else
		void * buffer = m_allocation.data + GetOffset();
#endif

		SetMapped();

		if (buffer == NULL) {
			ERROR("idIndexBuffer::MapBuffer: failed");
		}
		return buffer;
	}

	void IndexBuffer::UnmapBuffer()
	{
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idIndexBuffer::UnmapBuffer: Cannot unmap a buffer marked as BU_STATIC.");
		}

		SetUnmapped();
	}

	void IndexBuffer::ClearWithoutFreeing()
	{
		m_size = 0;
		m_offsetInOtherBuffer = OWNS_BUFFER_FLAG;
		m_apiObject = VK_NULL_HANDLE;
#if defined( ID_USE_AMD_ALLOCATOR )
		m_allocation = VmaAllocationInfo();
		m_vmaAllocation = NULL;
#else
		m_allocation.deviceMemory = VK_NULL_HANDLE;
#endif
	}

	VertexBuffer::VertexBuffer()
	{
		SetUnmapped();
	}

	VertexBuffer::~VertexBuffer()
	{
		FreeBufferObject();
	}

	bool VertexBuffer::AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage)
	{
		assert(m_apiObject == VK_NULL_HANDLE);
		assert_16_byte_aligned(data);

		if (allocSize <= 0) {
			ERROR("idVertexBuffer::AllocBufferObject: allocSize = %i", allocSize);
		}

		m_size = allocSize;
		m_usage = usage;

		bool allocationFailed = false;

		int numBytes = GetAllocedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (m_usage == BUFFER_USAGE_STATIC) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaMemoryRequirements vmaReq = {};
		if (m_usage == BU_STATIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (m_usage == BU_DYNAMIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			vmaReq.flags = VMA_MEMORY_REQUIREMENT_PERSISTENT_MAP_BIT;
		}

		ID_VK_CHECK(vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &vmaReq, &m_apiObject, &m_vmaAllocation, &m_allocation));

#else
		VkResult ret = vkCreateBuffer(m_device_context.device, &bufferCreateInfo, NULL, &m_apiObject);
		assert(ret == VK_SUCCESS);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_device_context.device, m_apiObject, &memoryRequirements);

		vulkanMemoryUsage_t memUsage = (m_usage == BUFFER_USAGE_STATIC) ? VULKAN_MEMORY_USAGE_GPU_ONLY : VULKAN_MEMORY_USAGE_CPU_TO_GPU;

		m_allocation = vulkan_allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VULKAN_ALLOCATION_TYPE_BUFFER);

		VK_CHECK(vkBindBufferMemory(m_device_context.device, m_apiObject, m_allocation.deviceMemory, m_allocation.offset));
#endif

		if (r_showBuffers) {
			Utility::Printf("vertex buffer alloc %p, (%i bytes)\n", this, GetSize());
		}

		// copy the data
		if (data != NULL) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void VertexBuffer::FreeBufferObject()
	{
		if (IsMapped()) {
			UnmapBuffer();
		}

		// if this is a sub-allocation inside a larger buffer, don't actually free anything.
		if (OwnsBuffer() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (m_apiObject == VK_NULL_HANDLE) {
			return;
		}

		if (r_showBuffers) {
			Utility::Printf("vertex buffer free %p, (%i bytes)\n", this, GetSize());
		}

		if (m_apiObject != VK_NULL_HANDLE) {
#if defined( ID_USE_AMD_ALLOCATOR )
			vmaDestroyBuffer(vmaAllocator, m_apiObject, m_vmaAllocation);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VmaAllocationInfo();
			m_vmaAllocation = NULL;
#else
			vulkan_allocator.Free(m_allocation);
			vkDestroyBuffer(m_device_context.device, m_apiObject, NULL);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VulkanAllocation();
#endif
		}

		ClearWithoutFreeing();
	}

	void VertexBuffer::Reference(const VertexBuffer & other)
	{
		assert(IsMapped() == false);
		//assert( other.IsMapped() == false );	// this happens when building idTriangles while at the same time setting up idDrawVerts
		assert(other.GetSize() > 0);

		FreeBufferObject();
		m_size = other.GetSize();					// this strips the MAPPED_FLAG
		m_offsetInOtherBuffer = other.GetOffset();	// this strips the OWNS_BUFFER_FLAG
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void VertexBuffer::Reference(const VertexBuffer & other, int refOffset, int refSize)
	{
		assert(IsMapped() == false);
		//assert( other.IsMapped() == false );	// this happens when building idTriangles while at the same time setting up idDrawVerts
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		m_size = refSize;
		m_offsetInOtherBuffer = other.GetOffset() + refOffset;
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void VertexBuffer::Update(const void * data, int size, int offset) const
	{
		assert(m_apiObject != VK_NULL_HANDLE);
		assert_16_byte_aligned(data);
		assert((GetOffset() & 15) == 0);

		if (size > GetSize()) {
			ERROR("idVertexBuffer::Update: size overrun, %i > %i\n", size, GetSize());
		}

		if (m_usage == BUFFER_USAGE_DYNAMIC) {
			CopyBuffer(
#if defined( ID_USE_AMD_ALLOCATOR )
				(byte *)m_allocation.pMappedData + GetOffset() + offset,
#else
				m_allocation.data + GetOffset() + offset,
#endif
				(const byte *)data, size);
		}
		else {
			VkBuffer stageBuffer;
			VkCommandBuffer commandBuffer;
			int stageOffset = 0;
			byte * stageData = staging_buffer.Stage(size, 1, commandBuffer, stageBuffer, stageOffset);

			memcpy(stageData, data, size);

			VkBufferCopy bufferCopy = {};
			bufferCopy.srcOffset = stageOffset;
			bufferCopy.dstOffset = GetOffset() + offset;
			bufferCopy.size = size;

			vkCmdCopyBuffer(commandBuffer, stageBuffer, m_apiObject, 1, &bufferCopy);
		}
	}

	void * VertexBuffer::MapBuffer(bufferMapType_t mapType)
	{
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idVertexBuffer::MapBuffer: Cannot map a buffer marked as BU_STATIC.");
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		void * buffer = (byte *)m_allocation.pMappedData + GetOffset();
#else
		void * buffer = m_allocation.data + GetOffset();
#endif

		SetMapped();

		if (buffer == NULL) {
			ERROR("idVertexBuffer::MapBuffer: failed");
		}
		return buffer;
	}

	void VertexBuffer::UnmapBuffer()
	{
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idVertexBuffer::UnmapBuffer: Cannot unmap a buffer marked as BU_STATIC.");
		}

		SetUnmapped();
	}

	void VertexBuffer::ClearWithoutFreeing()
	{
		m_size = 0;
		m_offsetInOtherBuffer = OWNS_BUFFER_FLAG;
		m_apiObject = VK_NULL_HANDLE;
#if defined( ID_USE_AMD_ALLOCATOR )
		m_allocation = VmaAllocationInfo();
		m_vmaAllocation = NULL;
#else
		m_allocation.deviceMemory = VK_NULL_HANDLE;
#endif
	}

	UniformBuffer::UniformBuffer()
	{
		m_usage = BUFFER_USAGE_DYNAMIC;
		SetUnmapped();
	}

	UniformBuffer::~UniformBuffer()
	{
		FreeBufferObject();
	}

	bool UniformBuffer::AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage)
	{
		assert(m_apiObject == VK_NULL_HANDLE);
		assert_16_byte_aligned(data);

		if (allocSize <= 0) {
			ERROR("idUniformBuffer::AllocBufferObject: allocSize = %i", allocSize);
		}

		m_size = allocSize;
		m_usage = usage;

		bool allocationFailed = false;

		const int numBytes = GetAllocedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (m_usage == BUFFER_USAGE_STATIC) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaMemoryRequirements vmaReq = {};
		if (m_usage == BU_STATIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (m_usage == BU_DYNAMIC) {
			vmaReq.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			vmaReq.flags = VMA_MEMORY_REQUIREMENT_PERSISTENT_MAP_BIT;
		}

		ID_VK_CHECK(vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &vmaReq, &m_apiObject, &m_vmaAllocation, &m_allocation));

#else
		VkResult ret = vkCreateBuffer(m_device_context.device, &bufferCreateInfo, NULL, &m_apiObject);
		assert(ret == VK_SUCCESS);

		VkMemoryRequirements memoryRequirements = {};
		vkGetBufferMemoryRequirements(m_device_context.device, m_apiObject, &memoryRequirements);

		vulkanMemoryUsage_t memUsage = (m_usage == BUFFER_USAGE_STATIC) ? VULKAN_MEMORY_USAGE_GPU_ONLY : VULKAN_MEMORY_USAGE_CPU_TO_GPU;

		m_allocation = vulkan_allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VULKAN_ALLOCATION_TYPE_BUFFER);

		VK_CHECK(vkBindBufferMemory(m_device_context.device, m_apiObject, m_allocation.deviceMemory, m_allocation.offset));
#endif

		if (r_showBuffers) {
			ERROR("joint buffer alloc %p, (%i bytes)\n", this, GetSize());
		}

		// copy the data
		if (data != NULL) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void UniformBuffer::FreeBufferObject()
	{
		if (IsMapped()) {
			UnmapBuffer();
		}

		// if this is a sub-allocation inside a larger buffer, don't actually free anything.
		if (OwnsBuffer() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (m_apiObject == VK_NULL_HANDLE) {
			return;
		}

		if (r_showBuffers) {
			Utility::Printf("joint buffer free %p, (%i bytes)\n", this, GetSize());
		}

		if (m_apiObject != VK_NULL_HANDLE) {
#if defined( ID_USE_AMD_ALLOCATOR )
			vmaDestroyBuffer(vmaAllocator, m_apiObject, m_vmaAllocation);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VmaAllocationInfo();
			m_vmaAllocation = NULL;
#else
			vulkan_allocator.Free(m_allocation);
			vkDestroyBuffer(m_device_context.device, m_apiObject, NULL);
			m_apiObject = VK_NULL_HANDLE;
			m_allocation = VulkanAllocation();
#endif
		}

		ClearWithoutFreeing();
	}

	void UniformBuffer::Reference(const UniformBuffer & other)
	{
		assert(IsMapped() == false);
		assert(other.IsMapped() == false);
		assert(other.GetSize() > 0);

		FreeBufferObject();
		m_size = other.GetSize();					// this strips the MAPPED_FLAG
		m_offsetInOtherBuffer = other.GetOffset();	// this strips the OWNS_BUFFER_FLAG
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void UniformBuffer::Reference(const UniformBuffer & other, size_t refOffset, size_t refSize)
	{
		assert(IsMapped() == false);
		assert(other.IsMapped() == false);
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		m_size = (int)refSize;
		m_offsetInOtherBuffer = other.GetOffset() + (int)refOffset;
		m_usage = other.m_usage;
		m_apiObject = other.m_apiObject;
#if defined( VULKAN )
		m_allocation = other.m_allocation;
#endif
		assert(OwnsBuffer() == false);
	}

	void UniformBuffer::Update(const void * data, size_t size, size_t offset) const
	{
		assert(m_apiObject != VK_NULL_HANDLE);
		assert_16_byte_aligned(data);
		assert((GetOffset() & 15) == 0);

		if (size > GetSize()) {
			ERROR("idUniformBuffer::Update: size overrun, %i > %i\n", size, m_size);
		}

		if (m_usage == BUFFER_USAGE_DYNAMIC) {
			CopyBuffer(
#if defined( ID_USE_AMD_ALLOCATOR )
				(byte *)m_allocation.pMappedData + GetOffset() + offset,
#else
				m_allocation.data + GetOffset() + offset,
#endif
				(const byte *)data, (int)size);
		}
		else {
			VkBuffer stageBuffer;
			VkCommandBuffer commandBuffer;
			int stageOffset = 0;
			byte * stageData = staging_buffer.Stage((int)size, 1, commandBuffer, stageBuffer, stageOffset);

			memcpy(stageData, data, size);

			VkBufferCopy bufferCopy = {};
			bufferCopy.srcOffset = stageOffset;
			bufferCopy.dstOffset = GetOffset() + offset;
			bufferCopy.size = size;

			vkCmdCopyBuffer(commandBuffer, stageBuffer, m_apiObject, 1, &bufferCopy);
		}
	}

	void * UniformBuffer::MapBuffer(bufferMapType_t mapType)
	{
		assert(mapType == BUFFER_MAP_WRITE);
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idUniformBuffer::MapBuffer: Cannot map a buffer marked as BU_STATIC.");
		}

#if defined( ID_USE_AMD_ALLOCATOR )
		void * buffer = (byte *)m_allocation.pMappedData + GetOffset();
#else
		void * buffer = m_allocation.data + GetOffset();
#endif

		SetMapped();

		if (buffer == NULL) {
			ERROR("idUniformBuffer::MapBuffer: failed");
		}
		return buffer;
	}

	void UniformBuffer::UnmapBuffer()
	{
		assert(m_apiObject != VK_NULL_HANDLE);

		if (m_usage == BUFFER_USAGE_STATIC) {
			ERROR("idUniformBuffer::UnmapBuffer: Cannot unmap a buffer marked as BU_STATIC.");
		}

		SetUnmapped();
	}

	void UniformBuffer::ClearWithoutFreeing()
	{
		m_size = 0;
		m_offsetInOtherBuffer = OWNS_BUFFER_FLAG;
		m_apiObject = VK_NULL_HANDLE;
#if defined( ID_USE_AMD_ALLOCATOR )
		m_allocation = VmaAllocationInfo();
		m_vmaAllocation = NULL;
#else
		m_allocation.deviceMemory = VK_NULL_HANDLE;
#endif
	}

}