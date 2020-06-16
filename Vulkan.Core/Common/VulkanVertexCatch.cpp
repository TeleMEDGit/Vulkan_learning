#include "VulkanVertexCatch.h"




#include <assert.h>



namespace Graphics
{

	VertexCache	vertex_cache;

	int r_showVertexCache = 1;

	int r_showVertexCacheTimings = 1;

	/*
	==============
	ClearGeoBufferSet
	==============
	*/
	static void ClearGeoBufferSet(GeometryBufferSet &gbs) {
		gbs.indexMemUsed.SetValue(0);
		gbs.vertexMemUsed.SetValue(0);
		gbs.jointMemUsed.SetValue(0);
		gbs.allocations = 0;
	}

	/*
	==============
	MapGeoBufferSet
	==============
	*/
	static void MapGeoBufferSet(GeometryBufferSet &gbs) {
		if (gbs.mappedVertexBase == NULL) {
			gbs.mappedVertexBase = (byte *)gbs.vertexBuffer.MapBuffer(BUFFER_MAP_WRITE);
		}
		if (gbs.mappedIndexBase == NULL) {
			gbs.mappedIndexBase = (byte *)gbs.indexBuffer.MapBuffer(BUFFER_MAP_WRITE);
		}
		if (gbs.mappedJointBase == NULL && gbs.jointBuffer.GetAllocedSize() != 0) {
			gbs.mappedJointBase = (byte *)gbs.jointBuffer.MapBuffer(BUFFER_MAP_WRITE);
		}
	}

	/*
	==============
	UnmapGeoBufferSet
	==============
	*/
	static void UnmapGeoBufferSet(GeometryBufferSet &gbs) {
		if (gbs.mappedVertexBase != NULL) {
			gbs.vertexBuffer.UnmapBuffer();
			gbs.mappedVertexBase = NULL;
		}
		if (gbs.mappedIndexBase != NULL) {
			gbs.indexBuffer.UnmapBuffer();
			gbs.mappedIndexBase = NULL;
		}
		if (gbs.mappedJointBase != NULL) {
			gbs.jointBuffer.UnmapBuffer();
			gbs.mappedJointBase = NULL;
		}
	}

	/*
	==============
	AllocGeoBufferSet
	==============
	*/
	static void AllocGeoBufferSet(GeometryBufferSet & gbs, const int vertexBytes, const int indexBytes, const int jointBytes, bufferUsageType_t usage) {
		gbs.vertexBuffer.AllocBufferObject(NULL, vertexBytes, usage);
		gbs.indexBuffer.AllocBufferObject(NULL, indexBytes, usage);
		if (jointBytes > 0) {
			gbs.jointBuffer.AllocBufferObject(NULL, jointBytes, usage);
		}

		ClearGeoBufferSet(gbs);
	}




	void VertexCache::Initialize(int uniformBufferOffsetAlignment)
	{
		m_currentFrame = 0;
		m_listNum = 0;

		m_uniformBufferOffsetAlignment = uniformBufferOffsetAlignment;

		m_mostUsedVertex = 0;
		m_mostUsedIndex = 0;
		m_mostUsedJoint = 0;

		for (int i = 0; i < NUM_FRAME_DATA; i++) {
			AllocGeoBufferSet(m_frameData[i], VERTCACHE_VERTEX_MEMORY_PER_FRAME, VERTCACHE_INDEX_MEMORY_PER_FRAME, VERTCACHE_JOINT_MEMORY_PER_FRAME, BUFFER_USAGE_DYNAMIC);
		}
#if 1
		AllocGeoBufferSet(m_staticData, STATIC_VERTEX_MEMORY, STATIC_INDEX_MEMORY, 0, BUFFER_USAGE_STATIC);
#else
		AllocGeoBufferSet(m_staticData, STATIC_VERTEX_MEMORY, STATIC_INDEX_MEMORY, 0, BUFFER_USAGE_DYNAMIC);
#endif

		MapGeoBufferSet(m_frameData[m_listNum]);
	}
	void VertexCache::Shutdown()
	{
		for (int i = 0; i < NUM_FRAME_DATA; i++) {
			m_frameData[i].vertexBuffer.FreeBufferObject();
			m_frameData[i].indexBuffer.FreeBufferObject();
			m_frameData[i].jointBuffer.FreeBufferObject();
		}
	}
	void VertexCache::PurgeAll()
	{
		Shutdown();
		Initialize(m_uniformBufferOffsetAlignment);
	}
	void VertexCache::FreeStaticData()
	{
		ClearGeoBufferSet(m_staticData);
		m_mostUsedVertex = 0;
		m_mostUsedIndex = 0;
		m_mostUsedJoint = 0;
	}
	vertCacheHandle VertexCache::AllocVertex(const void * data, int num, size_t size)
	{
		return ActuallyAlloc(m_frameData[m_listNum], data, ALIGN(num * size, VERTEX_CACHE_ALIGN), CACHE_VERTEX);
	}
	vertCacheHandle VertexCache::AllocIndex(const void * data, int num, size_t size)
	{
		return ActuallyAlloc(m_frameData[m_listNum], data, ALIGN(num * size, INDEX_CACHE_ALIGN), CACHE_INDEX);
	}
	vertCacheHandle VertexCache::AllocStaticVertex(const void * data, int bytes)
	{
		if (m_staticData.vertexMemUsed.GetValue() + bytes > STATIC_VERTEX_MEMORY) {     //NOTE this may case since it is manualy icreased
			ERROR("AllocStaticVertex failed, increase STATIC_VERTEX_MEMORY");
		}
		return ActuallyAlloc(m_staticData, data, bytes, CACHE_VERTEX);
	}
	vertCacheHandle VertexCache::AllocStaticIndex(const void * data, int bytes)
	{
		if (m_staticData.indexMemUsed.GetValue() + bytes > STATIC_INDEX_MEMORY) {
			ERROR("AllocStaticIndex failed, increase STATIC_INDEX_MEMORY");
		}
		return ActuallyAlloc(m_staticData, data, bytes, CACHE_INDEX);
	}
	byte * VertexCache::MappedVertexBuffer(vertCacheHandle handle)
	{
		//if(!CacheIsStatic(handle)) ERROR("VertexCache::MappedVertexBuffer  failed");
		release_assert(!CacheIsStatic(handle));
		const uint64 offset = (int)(handle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;
		const uint64 frameNum = (int)(handle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
		//if (frameNum == (m_currentFrame & VERTCACHE_FRAME_MASK))  ERROR("VertexCache::MappedVertexBuffer  mask failed");
		release_assert(frameNum == (m_currentFrame & VERTCACHE_FRAME_MASK));


		return m_frameData[m_listNum].mappedVertexBase + offset;
	}
	byte * VertexCache::MappedIndexBuffer(vertCacheHandle handle)
	{
        //if(!CacheIsStatic(handle))  ERROR("VertexCache::MappedIndexBuffer  failed");
		release_assert(!CacheIsStatic(handle));
		const uint64 offset = (int)(handle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;
		const uint64 frameNum = (int)(handle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
		//if (frameNum == (m_currentFrame & VERTCACHE_FRAME_MASK))ERROR("VertexCache::MappedIndexBuffer  mask failed");
		release_assert(frameNum == (m_currentFrame & VERTCACHE_FRAME_MASK));
		return m_frameData[m_listNum].mappedIndexBase + offset;
	}
	bool VertexCache::CacheIsCurrent(const vertCacheHandle handle)
	{
		const int isStatic = handle & VERTCACHE_STATIC;
		if (isStatic) {
			return true;
		}
		const uint64 frameNum = (int)(handle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
		if (frameNum != (m_currentFrame & VERTCACHE_FRAME_MASK)) {
			return false;
		}
		return true;
	}
	bool VertexCache::GetVertexBuffer(vertCacheHandle handle, VertexBuffer * vb)
	{
		const int isStatic = handle & VERTCACHE_STATIC;
		const uint64 size = (int)(handle >> VERTCACHE_SIZE_SHIFT) & VERTCACHE_SIZE_MASK;
		const uint64 offset = (int)(handle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;
		const uint64 frameNum = (int)(handle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
		if (isStatic) {
			vb->Reference(m_staticData.vertexBuffer, offset, size);
			return true;
		}
		if (frameNum != ((m_currentFrame - 1) & VERTCACHE_FRAME_MASK)) {
			return false;
		}
		vb->Reference(m_frameData[m_drawListNum].vertexBuffer, offset, size);
		return true;
	}
	bool VertexCache::GetIndexBuffer(vertCacheHandle handle, IndexBuffer * ib)
	{
		const int isStatic = handle & VERTCACHE_STATIC;
		const uint64 size = (int)(handle >> VERTCACHE_SIZE_SHIFT) & VERTCACHE_SIZE_MASK;
		const uint64 offset = (int)(handle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;
		const uint64 frameNum = (int)(handle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
		if (isStatic) {
			ib->Reference(m_staticData.indexBuffer, offset, size);
			return true;
		}
		if (frameNum != ((m_currentFrame - 1) & VERTCACHE_FRAME_MASK)) {
			return false;
		}
		ib->Reference(m_frameData[m_drawListNum].indexBuffer, offset, size);
		return true;
	}
	bool VertexCache::GetJointBuffer(vertCacheHandle handle, UniformBuffer * jb)
	{
		return false;
	}
	void VertexCache::BeginBackEnd()
	{

	
		m_mostUsedVertex = Max(m_mostUsedVertex, m_frameData[m_listNum].vertexMemUsed.GetValue());
		m_mostUsedIndex = Max(m_mostUsedIndex, m_frameData[m_listNum].indexMemUsed.GetValue());
		m_mostUsedJoint = Max(m_mostUsedJoint, m_frameData[m_listNum].jointMemUsed.GetValue());

		/*if (r_showVertexCache) {
			Utility::Printf("%08d: %d allocations, %dkB vertex, %dkB index, %kB joint : %dkB vertex, %dkB index, %kB joint\n",
				m_currentFrame, m_frameData[m_listNum].allocations,
				m_frameData[m_listNum].vertexMemUsed / 1024,
				m_frameData[m_listNum].indexMemUsed / 1024,
				m_frameData[m_listNum].jointMemUsed / 1024,
				m_mostUsedVertex / 1024,
				m_mostUsedIndex / 1024,
				m_mostUsedJoint / 1024);
		}*/

		// unmap the current frame so the GPU can read it
		//const int startUnmap = Sys_Milliseconds();
		UnmapGeoBufferSet(m_frameData[m_listNum]);
		UnmapGeoBufferSet(m_staticData);
		/*const int endUnmap = Sys_Milliseconds();
		if (endUnmap - startUnmap > 1) {
			Utility::PrintfIf(r_showVertexCacheTimings, "idVertexCache::unmap took %i msec\n", endUnmap - startUnmap);
		}*/
		m_drawListNum = m_listNum;

		// prepare the next frame for writing to by the CPU
		m_currentFrame++;

		m_listNum = m_currentFrame % NUM_FRAME_DATA;
		//const int startMap = Sys_Milliseconds();
		MapGeoBufferSet(m_frameData[m_listNum]);
		/*const int endMap = Sys_Milliseconds();
		if (endMap - startMap > 1) {
			idLib::PrintfIf(r_showVertexCacheTimings.GetBool(), "idVertexCache::map took %i msec\n", endMap - startMap);
		}*/

		ClearGeoBufferSet(m_frameData[m_listNum]);
	}
	vertCacheHandle VertexCache::ActuallyAlloc(GeometryBufferSet & vcs, const void * data, int bytes, cacheType_t type)
	{
		if (bytes == 0) {
			return (vertCacheHandle)0;
		}

		assert((((UINT_PTR)(data)) & 15) == 0);
		assert((bytes & 15) == 0);

		int	endPos = 0;
		int offset = 0;

		switch (type) {
		case CACHE_INDEX: {
			endPos = vcs.indexMemUsed.Add (bytes);
			if (endPos > vcs.indexBuffer.GetAllocedSize()) {
				ERROR("Out of index cache");
			}

			offset = endPos - bytes;

			if (data != NULL) {
				if (vcs.indexBuffer.GetUsage() == BUFFER_USAGE_DYNAMIC) {
					MapGeoBufferSet(vcs);
				}
				vcs.indexBuffer.Update(data, bytes, offset);
			}

			break;
		}
		case CACHE_VERTEX: {
			endPos = vcs.vertexMemUsed.Add(bytes);
			if (endPos > vcs.vertexBuffer.GetAllocedSize()) {
				ERROR("Out of vertex cache");
			}

			offset = endPos - bytes;

			if (data != NULL) {
				if (vcs.vertexBuffer.GetUsage() == BUFFER_USAGE_DYNAMIC) {
					MapGeoBufferSet(vcs);
				}
				vcs.vertexBuffer.Update(data, bytes, offset);
			}

			break;
		}
		case CACHE_JOINT: {
			/*endPos = vcs.jointMemUsed.Add(bytes);
			if (endPos > vcs.jointBuffer.GetAllocedSize()) {
				idLib::Error("Out of joint buffer cache");
			}

			offset = endPos - bytes;

			if (data != NULL) {
				if (vcs.jointBuffer.GetUsage() == BU_DYNAMIC) {
					MapGeoBufferSet(vcs);
				}
				vcs.jointBuffer.Update(data, bytes, offset);
			}
*/
			break;
		}
		default:
			ERROR("VertexCache::ActuallyAlloc  failled");
		}

		vcs.allocations++;

		vertCacheHandle handle = ((uint64)(m_currentFrame & VERTCACHE_FRAME_MASK) << VERTCACHE_FRAME_SHIFT) |
			((uint64)(offset & VERTCACHE_OFFSET_MASK) << VERTCACHE_OFFSET_SHIFT) |
			((uint64)(bytes & VERTCACHE_SIZE_MASK) << VERTCACHE_SIZE_SHIFT);
		if (&vcs == &m_staticData) {
			handle |= VERTCACHE_STATIC;
		}
		return handle;
	}
}