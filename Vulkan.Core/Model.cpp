#include "Model.h"
#include "System\Heap.h"
//#include "Math\DrawVertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

namespace Graphics
{
	surfacePolygons *	R_AllocStaticTriSurf();
	void				R_AllocStaticTriSurfVerts(surfacePolygons *poly, int numVerts);
	void				R_AllocStaticTriSurfIndexes(surfacePolygons *poly, int numIndexes);
	void				R_FreeStaticTriSurf(surfacePolygons *poly);
	void				R_FreeStaticTriSurfVertexCaches(surfacePolygons * poly);
	int					R_TriSurfMemory(const surfacePolygons *poly);

	/*
	================
	AddCubeFace
	================
	*/
	static void AddCubeFace(surfacePolygons *tri, Math::Vector3 v1, Math::Vector3 v2, Math::Vector3 v3, Math::Vector3 v4) {
		tri->verts[tri->numVerts + 0].Clear();
		tri->verts[tri->numVerts + 0].xyz = v1 * 8;
		tri->verts[tri->numVerts + 0].SetTexCoord(0, 0);

		tri->verts[tri->numVerts + 1].Clear();
		tri->verts[tri->numVerts + 1].xyz = v2 * 8;
		tri->verts[tri->numVerts + 1].SetTexCoord(1, 0);

		tri->verts[tri->numVerts + 2].Clear();
		tri->verts[tri->numVerts + 2].xyz = v3 * 8;
		tri->verts[tri->numVerts + 2].SetTexCoord(1, 1);

		tri->verts[tri->numVerts + 3].Clear();
		tri->verts[tri->numVerts + 3].xyz = v4 * 8;
		tri->verts[tri->numVerts + 3].SetTexCoord(0, 1);

		tri->indexes[tri->numIndexes + 0] = tri->numVerts + 0;
		tri->indexes[tri->numIndexes + 1] = tri->numVerts + 1;
		tri->indexes[tri->numIndexes + 2] = tri->numVerts + 2;
		tri->indexes[tri->numIndexes + 3] = tri->numVerts + 0;
		tri->indexes[tri->numIndexes + 4] = tri->numVerts + 2;
		tri->indexes[tri->numIndexes + 5] = tri->numVerts + 3;

		tri->numVerts += 4;
		tri->numIndexes += 6;
	}



	void RenderModel::AllocStaticTriSurfVerts(surfacePolygons *poly, int numVerts)
	{
		assert(poly->verts == NULL);
		poly->verts = (Math::DrawVertex *)Mem_Alloc16(numVerts * sizeof(Math::DrawVertex), TAG_TRI_VERTS);
	}

	void RenderModel::AllocStaticTriSurfIndexes(surfacePolygons *poly, int numIndexes)
	{
		assert(poly->indexes == NULL);
		poly->indexes = (polyIndex *)Mem_Alloc16(numIndexes * sizeof(Math::DrawVertex), TAG_TRI_INDEXES);
	}

	surfacePolygons * RenderModel::AllocateStaticTriSurf()
	{
		return R_AllocStaticTriSurf();
	}

	void RenderModel::InitFromFile(const std::string fileName)
	{

		//TDO call object file loader below and change signature

		bool loaded = false;

		if (!loaded) {
			//::Warning("Couldn't load model: '%s'", name.c_str());
			MakeDefaultModel();
			return;
		}
	}

	bool RenderModel::LoadBinaryModel(FILE * file, const SYSTEM_TIME_T sourceTimeStamp)
	{
		if (file == NULL) {
			return false;
		}
		return false;
	}

	void RenderModel::InitEmpty(const char * name)
	{
	}

	void RenderModel::AddSurface(modelSurface surface)
	{
		surfaces.push_back(surface);
		if (surface.geometry) {
			bounds = surface.geometry->bounds;  //they were adding like  bounds += surface.geometry->bounds;
		}
	}

	void RenderModel::FinishSurfaces()
	{
	}

	void RenderModel::PurgeModel()
	{
		for (int i = 0; i < surfaces.size(); i++) {
			modelSurface * surf = &surfaces[i];

			if (surf->geometry) {
				R_FreeStaticTriSurf(surf->geometry);
			}
		}
		surfaces.clear();
	}

	void RenderModel::LoadModel()
	{
		PurgeModel();
		InitFromFile(name);
	}

	void RenderModel::TouchData()
	{
		//for (int i = 0; i < surfaces.Num(); i++) {
		//	const modelSurface_t	*surf = &surfaces[i];

		//	// re-find the material to make sure it gets added to the
		//	// level keep list
		//	declManager->FindMaterial(surf->shader->GetName());
		//}
	}

	void RenderModel::FreeVertexCache()
	{
		for (int j = 0; j < surfaces.size(); j++) {
			surfacePolygons *tri = surfaces[j].geometry;
			if (tri == NULL) {
				continue;
			}
			R_FreeStaticTriSurfVertexCaches(tri);
		}
	}

	const std::string RenderModel::Name() const
	{
		return name;
	}

	size_t RenderModel::Memory() const
	{
		size_t	totalBytes = 0;

		totalBytes += sizeof(*this);
		totalBytes += name.size();  //may not be rigth call
		totalBytes += surfaces.size(); //may not be rigth call

		for (int j = 0; j < NumSurfaces(); j++) {
			const modelSurface	*surf = Surface(j);
			if (!surf->geometry) {
				continue;
			}
			totalBytes += R_TriSurfMemory(surf->geometry);
		}

		return totalBytes;
	}

	SYSTEM_TIME_T RenderModel::Timestamp() const
	{
		return timeStamp;
	}

	size_t RenderModel::NumSurfaces() const
	{
		return surfaces.size();
	}

	size_t RenderModel::NumBaseSurfaces() const
	{
		return surfaces.size() - overlaysAdded;
	}

	const modelSurface * RenderModel::Surface(int surfaceNum) const
	{
		return nullptr;
	}

	surfacePolygons * RenderModel::AllocSurfaceTriangles(int numVerts, int numIndexes) const
	{
		surfacePolygons *tri = R_AllocStaticTriSurf();
		R_AllocStaticTriSurfVerts(tri, numVerts);
		R_AllocStaticTriSurfIndexes(tri, numIndexes);
		return tri;
		
	}

	

	void RenderModel::FreeSurfaceTriangles(surfacePolygons * tris) const
	{
		R_FreeStaticTriSurf(tris);
	}

	bool RenderModel::IsStaticWorldModel() const
	{
		return isStaticWorldModel;
	}

	bool RenderModel::IsReloadable() const
	{
		return reloadable;
	}

	dynamicModel_t RenderModel::IsDynamicModel() const
	{
		return DM_STATIC;
	}

	bool RenderModel::IsDefaultModel() const
	{
		return defaulted;
	}

	

	/*Math::BoundingPlane RenderModel::Bounds(const surfacePolygons * ent) const
	{
		return Math::BoundingPlane();
	}
*/

	/*RenderModel * idRenderModel::InstantiateDynamicModel(const renderEntity_t * ent, const viewDef_t * view, idRenderModel * cachedModel)
	{
		if (cachedModel) {
			delete cachedModel;
			cachedModel = NULL;
		}
		ERROR("InstantiateDynamicModel called on static model '%s'", name.c_str());
		return NULL;
	}*/

	void RenderModel::MakeDefaultModel()
	{

		defaulted = true;

		// throw out any surfaces we already have
		PurgeModel();

		// create one new surface
		modelSurface	surf;

		surfacePolygons *tri = R_AllocStaticTriSurf();

		//surf.shader = tr.defaultMaterial;
		surf.geometry = tri;

		R_AllocStaticTriSurfVerts(tri, 24);
		R_AllocStaticTriSurfIndexes(tri, 36);

		AddCubeFace(tri, Math::Vector3(-1, 1, 1), Math::Vector3(1, 1, 1), Math::Vector3(1, -1, 1), Math::Vector3(-1, -1, 1));
		AddCubeFace(tri, Math::Vector3(-1, 1, -1), Math::Vector3(-1, -1, -1), Math::Vector3(1, -1, -1), Math::Vector3(1, 1, -1));

		AddCubeFace(tri, Math::Vector3(1, -1, 1), Math::Vector3(1, 1, 1), Math::Vector3(1, 1, -1), Math::Vector3(1, -1, -1));
		AddCubeFace(tri, Math::Vector3(-1, -1, 1), Math::Vector3(-1, -1, -1), Math::Vector3(-1, 1, -1), Math::Vector3(-1, 1, 1));

		AddCubeFace(tri, Math::Vector3(-1, -1, 1), Math::Vector3(1, -1, 1), Math::Vector3(1, -1, -1), Math::Vector3(-1, -1, -1));
		AddCubeFace(tri, Math::Vector3(-1, 1, 1), Math::Vector3(-1, 1, -1), Math::Vector3(1, 1, -1), Math::Vector3(1, 1, 1));

		tri->generateNormals = true;

		AddSurface(surf);
		FinishSurfaces();
	}

	RenderModel* RenderModel::Load3DModelFromObjFile(char const* filename)
	{
		RenderModel render_model;

		const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

		Assimp::Importer importer;
		const aiScene* pScene;

		std::string file = "D:\\Development\\C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\Workbench_Low.fbx";

		pScene = importer.ReadFile(file.c_str(), defaultFlags);

		if (pScene)
		{
			int meshCount = pScene->mNumMeshes;

			render_model.surfaces.resize(meshCount);

			for (unsigned int i = 0; i < render_model.surfaces.size(); i++)
			{
				render_model.surfaces[i].id = i;
				render_model.surfaces[i].geometry = nullptr;

				const aiMesh* paiMesh = pScene->mMeshes[i];

				if (paiMesh != nullptr)
				{
					render_model.surfaces[i].geometry = render_model.AllocateStaticTriSurf();

					surfacePolygons& tri = *render_model.surfaces[i].geometry;

					int ambientViewCount = 0;	// FIXME: remove

					//ambientViewCount = pScene->mMeshes[i].

					tri.generateNormals = pScene->mMeshes[i]->HasNormals();
					tri.tangentsCalculated = pScene->mMeshes[i]->HasTangentsAndBitangents();

					tri.numVerts = pScene->mMeshes[i]->mNumVertices;

					tri.verts = nullptr;

					render_model.AllocStaticTriSurfVerts(&tri, tri.numVerts);

					assert(tri.verts != NULL);

					for (int j = 0; j < tri.numVerts; j++) {

						if (pScene->mMeshes[i]->mVertices) {
							tri.verts[j].xyz = Math::Vector3(pScene->mMeshes[i]->mVertices[j].x, pScene->mMeshes[i]->mVertices[j].y, pScene->mMeshes[i]->mVertices[j].z);

							//TDO calculte bounding box as simple two vector3 
							//smalled and biggest vector
						}
						else
						{
							assert(0);
						}

						if (pScene->mMeshes[i]->mTextureCoords[0])
						{
							tri.verts[j].SetTexCoord(pScene->mMeshes[i]->mTextureCoords[0][j].x, pScene->mMeshes[i]->mTextureCoords[0][j].x);
						}
						else
						{
							tri.verts[j].SetTexCoord(0, 0);
						}

						if (pScene->mMeshes[i]->mNormals)
						{
							tri.verts[j].SetNormal(pScene->mMeshes[i]->mNormals[j].x, pScene->mMeshes[i]->mNormals[j].y, pScene->mMeshes[i]->mNormals[j].z);
						}

						if (pScene->mMeshes[i]->mTangents)
						{
							tri.verts[j].SetTangent(pScene->mMeshes[i]->mTangents[j].x, pScene->mMeshes[i]->mTangents[j].y, pScene->mMeshes[i]->mTangents[j].z);
						}
						else
						{
							tri.verts[j].SetTangent(1, 0, 0);
						}

						if (pScene->mMeshes[i]->mBitangents)
						{
							tri.verts[j].SetBiTangent(pScene->mMeshes[i]->mBitangents[j].x, pScene->mMeshes[i]->mBitangents[j].y, pScene->mMeshes[i]->mBitangents[j].z);
						}
						else
						{
							tri.verts[j].SetBiTangent(1, 0, 0);
						}
					}

					tri.numIndexes = pScene->mMeshes[i]->mNumFaces * 3;

					tri.indexes = nullptr;

					if (tri.numIndexes > 0)
					{
						render_model.AllocStaticTriSurfIndexes(&tri, tri.numIndexes);


						for (unsigned int f = 0; f < pScene->mMeshes[i]->mNumFaces; f++)
						{


							assert(pScene->mMeshes[i]->mFaces[f].mNumIndices == 3);
							*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[0];
							*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[1];
							*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[2];


						}

					}


				}

			}

		}	

		return &render_model;
	}

	//bool RenderModel::Load3DModelFromObjFile(char const * filename, RenderModel * renderModel)
	//{

	//	const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

	//	Assimp::Importer importer;
	//	const aiScene* pScene;
	//	RenderModel render_model;
	//	//std::string file = "D:\\Development C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\stylized_levi.fbx";

	//	std::string file = "D:\\Development\\C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\Workbench_Low.fbx";

	//	pScene = importer.ReadFile(file.c_str(), defaultFlags);

	//	if (pScene)
	//	{
	//		int meshCount = pScene->mNumMeshes;

	//		render_model.surfaces.resize(meshCount);

	//		for (unsigned int i = 0; i < render_model.surfaces.size(); i++)
	//		{
	//			render_model.surfaces[i].id = i;
	//			render_model.surfaces[i].geometry = nullptr;

	//			const aiMesh* paiMesh = pScene->mMeshes[i];

	//			if (paiMesh != nullptr)
	//			{
	//				render_model.surfaces[i].geometry = render_model.AllocateStaticTriSurf();

	//				surfacePolygons & tri = *render_model.surfaces[i].geometry;

	//				int ambientViewCount = 0;	// FIXME: remove

	//				//ambientViewCount = pScene->mMeshes[i].

	//				tri.generateNormals = pScene->mMeshes[i]->HasNormals();
	//				tri.tangentsCalculated = pScene->mMeshes[i]->HasTangentsAndBitangents();

	//				tri.numVerts = pScene->mMeshes[i]->mNumVertices;

	//				tri.verts = nullptr;

	//				render_model.AllocStaticTriSurfVerts(&tri, tri.numVerts);

	//				assert(tri.verts != NULL);

	//				for (int j = 0; j < tri.numVerts; j++) {

	//					if (pScene->mMeshes[i]->mVertices) {
	//						tri.verts[j].xyz = Math::Vector3(pScene->mMeshes[i]->mVertices[j].x, pScene->mMeshes[i]->mVertices[j].y, pScene->mMeshes[i]->mVertices[j].z);

	//						//TDO calculte bounding box as simple two vector3 
	//						//smalled and biggest vector
	//					}
	//					else
	//					{
	//						assert(0);
	//					}

	//					if (pScene->mMeshes[i]->mTextureCoords[0])
	//					{
	//						tri.verts[j].SetTexCoord(pScene->mMeshes[i]->mTextureCoords[0][j].x, pScene->mMeshes[i]->mTextureCoords[0][j].x);
	//					}
	//					else
	//					{
	//						tri.verts[j].SetTexCoord(0, 0);
	//					}

	//					if (pScene->mMeshes[i]->mNormals)
	//					{
	//						tri.verts[j].SetNormal(pScene->mMeshes[i]->mNormals[j].x, pScene->mMeshes[i]->mNormals[j].y, pScene->mMeshes[i]->mNormals[j].z);
	//					}

	//					if (pScene->mMeshes[i]->mTangents)
	//					{
	//						tri.verts[j].SetTangent(pScene->mMeshes[i]->mTangents[j].x, pScene->mMeshes[i]->mTangents[j].y, pScene->mMeshes[i]->mTangents[j].z);
	//					}
	//					else
	//					{
	//						tri.verts[j].SetTangent(1, 0, 0);
	//					}

	//					if (pScene->mMeshes[i]->mBitangents)
	//					{
	//						tri.verts[j].SetBiTangent(pScene->mMeshes[i]->mBitangents[j].x, pScene->mMeshes[i]->mBitangents[j].y, pScene->mMeshes[i]->mBitangents[j].z);
	//					}
	//					else
	//					{
	//						tri.verts[j].SetBiTangent(1, 0, 0);
	//					}
	//				}

	//				tri.numIndexes = pScene->mMeshes[i]->mNumFaces * 3;

	//				tri.indexes = nullptr;

	//				if (tri.numIndexes > 0)
	//				{
	//					render_model.AllocStaticTriSurfIndexes(&tri, tri.numIndexes);


	//					for (unsigned int f = 0; f < pScene->mMeshes[i]->mNumFaces; f++)
	//					{


	//						assert(pScene->mMeshes[i]->mFaces[f].mNumIndices == 3);
	//						*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[0];
	//						*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[1];
	//						*tri.indexes++ = pScene->mMeshes[i]->mFaces[f].mIndices[2];


	//					}

	//				}


	//			}

	//		}

	//	}

	//	renderModel = &render_model;

	//	return false;
	//}

	


	/*
	==============
	R_AllocStaticTriSurf
	==============
	*/
	surfacePolygons *R_AllocStaticTriSurf() {
		surfacePolygons *tris = (surfacePolygons *)Mem_ClearedAlloc(sizeof(surfacePolygons), TAG_SRFTRIS);
		return tris;
	}

	/*
	=================
	R_AllocStaticTriSurfVerts
	=================
	*/
	void R_AllocStaticTriSurfVerts(surfacePolygons *tri, int numVerts) {
		assert(tri->verts == NULL);
		tri->verts = (Math::DrawVertex *)Mem_Alloc16(numVerts * sizeof(Math::DrawVertex), TAG_TRI_VERTS);
	}

	/*
	=================
	R_AllocStaticTriSurfIndexes
	=================
	*/
	void R_AllocStaticTriSurfIndexes(surfacePolygons *tri, int numIndexes) {
		assert(tri->indexes == NULL);
		tri->indexes = (polyIndex *)Mem_Alloc16(numIndexes * sizeof(polyIndex), TAG_TRI_INDEXES);
	}

	/*
	==============
	R_FreeStaticTriSurf
	==============
	*/
	void R_FreeStaticTriSurf(surfacePolygons *tri) {
		if (!tri) {
			return;
		}

		R_FreeStaticTriSurfVertexCaches(tri); 

		if (!tri->referencedVerts) {
			if (tri->verts != NULL) {
				// R_CreateLightTris points tri->verts at the verts of the ambient surface
				if (tri->ambientSurface == NULL || tri->verts != tri->ambientSurface->verts) {
					Mem_Free(tri->verts);
				}
			}
		}

		if (!tri->referencedIndexes) {
			if (tri->indexes != NULL) {
				// if a surface is completely inside a light volume R_CreateLightTris points tri->indexes at the indexes of the ambient surface
				if (tri->ambientSurface == NULL || tri->indexes != tri->ambientSurface->indexes) {
					Mem_Free(tri->indexes);
				}
			}
			/*if (tri->silIndexes != NULL) {
				Mem_Free(tri->silIndexes);
			}*/
			/*if (tri->silEdges != NULL) {
				Mem_Free(tri->silEdges);
			}*/
			/*if (tri->dominantTris != NULL) {
				Mem_Free(tri->dominantTris);
			}*/
			if (tri->mirroredVerts != NULL) {
				Mem_Free(tri->mirroredVerts);
			}
			if (tri->dupVerts != NULL) {
				Mem_Free(tri->dupVerts);
			}
		}

		/*if (tri->preLightShadowVertexes != NULL) {
			Mem_Free(tri->preLightShadowVertexes);
		}*/
		/*if (tri->staticShadowVertexes != NULL) {
			Mem_Free(tri->staticShadowVertexes);
		}*/

		// clear the tri out so we don't retain stale data
		memset(tri, 0, sizeof(surfacePolygons));

		Mem_Free(tri);
	}

	/*
	==============
	R_FreeStaticTriSurfVertexCaches
	==============
	*/
	void R_FreeStaticTriSurfVertexCaches(surfacePolygons * tri) {
		// we don't support reclaiming static geometry memory
		// without a level change
		tri->drawVertCache = 0;
		tri->indexCache = 0;
		tri->shadowCache = 0;
	}

	/*
	=================
	R_TriSurfMemory

	For memory profiling
	=================
	*/
	int R_TriSurfMemory(const surfacePolygons *tri) {
		int total = 0;

		if (tri == NULL) {
			return total;
		}

		//if (tri->preLightShadowVertexes != NULL) {
		//	total += tri->numVerts * 2 * sizeof(tri->preLightShadowVertexes[0]);
		//}
		//if (tri->staticShadowVertexes != NULL) {
		//	total += tri->numVerts * 2 * sizeof(tri->staticShadowVertexes[0]);
		//}
		if (tri->verts != NULL) {
			if (tri->ambientSurface == NULL || tri->verts != tri->ambientSurface->verts) {
				total += tri->numVerts * sizeof(tri->verts[0]);
			}
		}
		if (tri->indexes != NULL) {
			if (tri->ambientSurface == NULL || tri->indexes != tri->ambientSurface->indexes) {
				total += tri->numIndexes * sizeof(tri->indexes[0]);
			}
		}
		/*if (tri->silIndexes != NULL) {
			total += tri->numIndexes * sizeof(tri->silIndexes[0]);
		}*/
		/*if (tri->silEdges != NULL) {
			total += tri->numSilEdges * sizeof(tri->silEdges[0]);
		}*/
		/*if (tri->dominantTris != NULL) {
			total += tri->numVerts * sizeof(tri->dominantTris[0]);
		}*/
		if (tri->mirroredVerts != NULL) {
			total += tri->numMirroredVerts * sizeof(tri->mirroredVerts[0]);
		}
		if (tri->dupVerts != NULL) {
			total += tri->numDupVerts * sizeof(tri->dupVerts[0]);
		}

		total += sizeof(*tri);

		return total;
	}




}
