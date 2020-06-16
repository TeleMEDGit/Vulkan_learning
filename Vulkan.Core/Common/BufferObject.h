#pragma once

//#include "Common.h"

#include "VulkanAllocateMemory.h"
//#include "..\idLib\Thread.h"
//#include "..\idLib\sys_threading.h"


namespace  Graphics
{
	//class DrawVertex;

	enum bufferMapType_t {
		BUFFER_MAP_READ,			// map for reading
		BUFFER_MAP_WRITE			// map for writing
	};

	enum bufferUsageType_t {
		BUFFER_USAGE_STATIC,			// GPU R
		BUFFER_USAGE_DYNAMIC,			// GPU R, CPU R/W
	};

	// Returns all targets to virtual memory use instead of buffer object use.
	// Call this before doing any conventional buffer reads, like screenshots.
	void UnbindBufferObjects();
	bool IsWriteCombined(void * base);
	void CopyBuffer(byte * dst, const byte * src, int numBytes);



	class BufferObject {
	public:
		friend class VulkanFoundation;
		BufferObject();

		int					GetSize() const { return (m_size & ~MAPPED_FLAG); }
		int					GetAllocedSize() const { return ((m_size & ~MAPPED_FLAG) + 15) & ~15; }
		bufferUsageType_t	GetUsage() const { return m_usage; }
		VkBuffer			GetAPIObject() const { return m_apiObject; }
		int					GetOffset() const { return (m_offsetInOtherBuffer & ~OWNS_BUFFER_FLAG); }

		bool				IsMapped() const { return (m_size & MAPPED_FLAG) != 0; }

	protected:
		void				SetMapped() const { const_cast<int &>(m_size) |= MAPPED_FLAG; }
		void				SetUnmapped() const { const_cast<int &>(m_size) &= ~MAPPED_FLAG; }
		bool				OwnsBuffer() const { return ((m_offsetInOtherBuffer & OWNS_BUFFER_FLAG) != 0); }

	protected:
		int					m_size;					// size in bytes
		int					m_offsetInOtherBuffer;	// offset in bytes
		bufferUsageType_t	m_usage;

		VkBuffer			m_apiObject;
#if defined( ID_USE_AMD_ALLOCATOR )
		VmaAllocation		m_vmaAllocation;
		VmaAllocationInfo	m_allocation;
#else
		VulkanAllocation	m_allocation;

#endif

		// sizeof() confuses typeinfo...
		static const int	MAPPED_FLAG = 1 << (4 /* sizeof( int ) */ * 8 - 1);
		static const int	OWNS_BUFFER_FLAG = 1 << (4 /* sizeof( int ) */ * 8 - 1);
	};


	/*
	================================================================================================

	idIndexBuffer

	================================================================================================
	*/
	class IndexBuffer : public BufferObject {
	public:
		IndexBuffer();
		~IndexBuffer();

		// Allocate or free the buffer.
		bool				AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage);
		void				FreeBufferObject();

		// Make this buffer a reference to another buffer.
		void				Reference(const IndexBuffer & other);
		void				Reference(const IndexBuffer & other, int refOffset, int refSize);

		// Copies data to the buffer. 'size' may be less than the originally allocated size.
		void				Update(const void * data, int size, int offset = 0) const;

		void *				MapBuffer(bufferMapType_t mapType);
		//triIndex_t *		MapIndexBuffer(bufferMapType_t mapType) { return static_cast< triIndex_t * >(MapBuffer(mapType)); }
		void				UnmapBuffer();

	private:
		void				ClearWithoutFreeing();

		//DISALLOW_COPY_AND_ASSIGN(idIndexBuffer);
	};


	class VertexBuffer : public BufferObject {
	public:
		VertexBuffer();
		~VertexBuffer();

		// Allocate or free the buffer.
		bool				AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage);
		void				FreeBufferObject();

		// Make this buffer a reference to another buffer.
		void				Reference(const VertexBuffer & other);
		void				Reference(const VertexBuffer & other, int refOffset, int refSize);

		// Copies data to the buffer. 'size' may be less than the originally allocated size.
		void				Update(const void * data, int size, int offset = 0) const;

		void *				MapBuffer(bufferMapType_t mapType);
		// DrawVertex *		MapVertexBuffer(bufferMapType_t mapType) { return static_cast< idDrawVert * >(MapBuffer(mapType)); }
		void				UnmapBuffer();

	private:
		void				ClearWithoutFreeing();

		//DISALLOW_COPY_AND_ASSIGN(idVertexBuffer);  disallow as per C++
	};

	/*
	================================================================================================

	idUniformBuffer

	IMPORTANT NOTICE: on the PC, binding to an offset in uniform buffer objects
	is limited to GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, which is 256 on current nvidia cards,
	so joint offsets, which are multiples of 48 bytes, must be in multiples of 16 = 768 bytes.
	================================================================================================
	*/
	class UniformBuffer : public BufferObject {
	public:
		UniformBuffer();
		~UniformBuffer();

		// Allocate or free the buffer.
		bool				AllocBufferObject(const void * data, int allocSize, bufferUsageType_t usage);
		void				FreeBufferObject();

		// Make this buffer a reference to another buffer.
		void				Reference(const UniformBuffer & other);
		void				Reference(const UniformBuffer & other, size_t refOffset, size_t refSize);

		// Copies data to the buffer. 'size' may be less than the originally allocated size.
		void				Update(const void * data, size_t size, size_t offset = 0) const;

		void *				MapBuffer(bufferMapType_t mapType);
		void				UnmapBuffer();

	private:
		void				ClearWithoutFreeing();

		//DISALLOW_COPY_AND_ASSIGN(idUniformBuffer);
	};

}


