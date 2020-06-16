#pragma once
#include "BufferObject.h"
#include "..\Utility.h"
#include "..\Math\DrawVertex.h"



namespace Graphics
{
	const int VERTCACHE_INDEX_MEMORY_PER_FRAME = 31 * 1024 * 1024;
	const int VERTCACHE_VERTEX_MEMORY_PER_FRAME = 31 * 1024 * 1024;
	const int VERTCACHE_JOINT_MEMORY_PER_FRAME = 256 * 1024;

	// there are a lot more static indexes than vertexes, because interactions are just new
	// index lists that reference existing vertexes
	const int STATIC_INDEX_MEMORY = 31 * 1024 * 1024;
	const int STATIC_VERTEX_MEMORY = 31 * 1024 * 1024;	// make sure it fits in VERTCACHE_OFFSET_MASK!

														// vertCacheHandle_t packs size, offset, and frame number into 64 bits
	const int VERTCACHE_STATIC = 1;					// in the static set, not the per-frame set
	const int VERTCACHE_SIZE_SHIFT = 1;
	const int VERTCACHE_SIZE_MASK = 0x7fffff;		// 8 megs 
	const int VERTCACHE_OFFSET_SHIFT = 24;
	const int VERTCACHE_OFFSET_MASK = 0x1ffffff;	// 32 megs 
	const int VERTCACHE_FRAME_SHIFT = 49;
	const int VERTCACHE_FRAME_MASK = 0x7fff;		// 15 bits = 32k frames to wrap around

	const int VERTEX_CACHE_ALIGN = 32;
	const int INDEX_CACHE_ALIGN = 16;
	const int JOINT_CACHE_ALIGN = 16;

	enum cacheType_t {
		CACHE_VERTEX,
		CACHE_INDEX,
		CACHE_JOINT
	};

	struct GeometryBufferSet {
		 IndexBuffer			indexBuffer;
		 VertexBuffer			vertexBuffer;
		 UniformBuffer			jointBuffer;
		byte *					mappedVertexBase;
		byte *					mappedIndexBase;
		byte *					mappedJointBase;
		idSysInterlockedInteger					indexMemUsed;    //use to be some fancy type called interlock
		idSysInterlockedInteger					vertexMemUsed;
		idSysInterlockedInteger					jointMemUsed;
		int						allocations;	// number of index and vertex allocations combined
	};


	class VertexCache {
	public:
		void			Initialize(int uniformBufferOffsetAlignment);
		void			Shutdown();
		void			PurgeAll();

		// call on loading a new map

		void			FreeStaticData();

	 

		// this data is only valid for one frame of rendering
		vertCacheHandle	  AllocVertex(const void * data, int num, size_t size = sizeof(Math::DrawVertex*));
		vertCacheHandle	AllocIndex(const void * data, int num, size_t size = sizeof(polyIndex));
		//vertCacheHandle	AllocJoint(const void * data, int num, size_t size = sizeof(idJointMat));

		// this data is valid until the next map load
		vertCacheHandle	AllocStaticVertex(const void * data, int bytes);
		vertCacheHandle	AllocStaticIndex(const void * data, int bytes);

		byte *			MappedVertexBuffer(vertCacheHandle handle);
		byte *			MappedIndexBuffer(vertCacheHandle handle);


		// Returns false if it's been purged
		// This can only be called by the front end, the back end should only be looking at
		// vertCacheHandle_t that are already validated.
		bool			CacheIsCurrent(const vertCacheHandle handle);
		static bool		CacheIsStatic(const vertCacheHandle handle) { return (handle & VERTCACHE_STATIC) != 0; }

		// vb/ib is a temporary reference -- don't store it
		bool			GetVertexBuffer(vertCacheHandle handle, VertexBuffer * vb);
		bool			GetIndexBuffer(vertCacheHandle handle, IndexBuffer * ib);
		bool			GetJointBuffer(vertCacheHandle handle, UniformBuffer * jb);

		void			BeginBackEnd();

	public:
		int				m_currentFrame;	// for determining the active buffers
		int				m_listNum;		// currentFrame % NUM_FRAME_DATA
		int				m_drawListNum;	// (currentFrame-1) % NUM_FRAME_DATA

		GeometryBufferSet	m_staticData;
		GeometryBufferSet	m_frameData[NUM_FRAME_DATA];

		int				m_uniformBufferOffsetAlignment;

		// High water marks for the per-frame buffers
		int				m_mostUsedVertex;
		int				m_mostUsedIndex;
		int				m_mostUsedJoint;

		// Try to make room for <bytes> bytes
		vertCacheHandle	ActuallyAlloc(GeometryBufferSet & vcs, const void * data, int bytes, cacheType_t type);



	};


	// platform specific code to memcpy into vertex buffers efficiently
	// 16 byte alignment is guaranteed
	void CopyBuffer(byte * dst, const byte * src, int numBytes);

	extern	VertexCache	vertex_cache;


}

