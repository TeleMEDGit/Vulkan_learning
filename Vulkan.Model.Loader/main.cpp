#include <iostream>

#include "../Vulkan.Core/Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/cimport.h>
#include <assimp/postprocess.h>


int main(int argc, char * args)
{

	static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals ;

	 Assimp::Importer importer;
	 const aiScene* pScene;
	 RenderModel render_model;
	 //std::string file = "D:\\Development C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\stylized_levi.fbx";

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

				surfacePolygons & tri = *render_model.surfaces[i].geometry;

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

   // remove unused data
	 /* importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, 
	  aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_CAMERAS);*/

  //  // max triangles and vertices per mesh, splits above this threshold
  //  importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, INT_MAX);
  //  importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 0xfffe); // avoid the primitive restart index

  //  // remove points and lines
  //  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	//const aiScene model = aiImportFile("file.ci",ai);
	
	return 0;
}