#pragma once

//#include "Common\Common.h"
//#include "Common\RenderCommon.h"
#include "Math\DrawVertex.h"
#include "Math\BoundingPlane.h"


/*
===============================================================================

Render Model

===============================================================================
*/

namespace Graphics
{

//	// shared between the renderer, game, and Maya export DLL
//#define MD5_VERSION_STRING		"MD5Version"
//#define MD5_MESH_EXT			"md5mesh"
//#define MD5_ANIM_EXT			"md5anim"
//#define MD5_CAMERA_EXT			"md5camera"
//#define MD5_VERSION				10

// this is used for calculating unsmoothed normals and tangents for deformed models
	/*struct dominantTri_t {
		triIndex_t					v2, v3;
		float						normalizationScale[3];
	};*/

	const int SHADOW_CAP_INFINITE = 64;

	//surface polygons
	// our only drawing geometry type
	struct surfacePolygons {
		surfacePolygons() {}

	
		Math::BoundingPlane				bounds;					// for culling

		bool						generateNormals;		// create normals from geometry, instead of using explicit ones
		bool						tangentsCalculated;		// set when the vertex tangents have been calculated
		bool						perfectHull;			// true if there aren't any dangling edges
		bool						referencedVerts;		// if true the 'verts' are referenced and should not be freed
		bool						referencedIndexes;		// if true, indexes, silIndexes, mirrorVerts, and silEdges are
															// pointers into the original surface, and should not be freed

		int							numVerts;				// number of vertices
		Math::DrawVertex *				verts;					// vertices, allocated with special allocator

		int							numIndexes;				// for shadows, this has both front and rear end caps and silhouette planes
		polyIndex *					indexes;				// indexes, allocated with special allocator

		//polyIndex *					silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords

		int							numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
		int *						mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->numVerts - tri->numMirroredVerts + 0

		int							numDupVerts;			// number of duplicate vertexes
		int *						dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex

		int							numSilEdges;			// number of silhouette edges
		//silEdge_t *					silEdges;				// silhouette edges

		//dominantTri_t *				dominantTris;			// [numVerts] for deformed surface fast tangent calculation

		int							numShadowIndexesNoFrontCaps;	// shadow volumes with front caps omitted
		int							numShadowIndexesNoCaps;			// shadow volumes with the front and rear caps omitted

		int							shadowCapPlaneBits;		// bits 0-5 are set when that plane of the interacting light has triangles
															// projected on it, which means that if the view is on the outside of that
															// plane, we need to draw the rear caps of the shadow volume
															// dynamic shadows will have SHADOW_CAP_INFINITE

		//idShadowVert *				preLightShadowVertexes;	// shadow vertices in CPU memory for pre-light shadow volumes
		//idShadowVert *				staticShadowVertexes;	// shadow vertices in CPU memory for static shadow volumes

		surfacePolygons *			ambientSurface;			// for light interactions, point back at the original surface that generated
															// the interaction, which we will get the ambientCache from

		surfacePolygons *			nextDeferredFree;		// chain of tris to free next frame

															// for deferred normal / tangent transformations by joints
															// the jointsInverted list / buffer object on md5WithJoints may be
															// shared by multiple srfTriangles_t
		//idRenderModelStatic *		staticModelWithJoints;

		// data in vertex object space, not directly readable by the CPU
		vertexCacheHandle			indexCache;				// GL_INDEX_TYPE
		vertexCacheHandle			drawVertCache;			// idDrawVert
		vertexCacheHandle			shadowCache;			// idVec4

		DISALLOW_COPY_AND_ASSIGN(surfacePolygons);
	};

	typedef std::vector<surfacePolygons> PolygonList;

	struct modelSurface {
		int							id;
		//const idMaterial *			shader;
		surfacePolygons *			geometry;
	};

	enum dynamicModel_t {
		DM_STATIC,		// never creates a dynamic model
		DM_CACHED,		// once created, stays constant until the entity is updated (animating characters)
		DM_CONTINUOUS	// must be recreated for every single view (time dependent things like particles)
	};


	// the init methods may be called again on an already created model when
	// a reloadModels is issued



	class RenderModel {
	public:

						~RenderModel() {};

		// Loads static models only, dynamic models must be loaded by the modelManager
	   void				InitFromFile(const std::string fileName);
	   // Supports reading/writing binary file formats
	   bool				LoadBinaryModel(FILE * file, const SYSTEM_TIME_T sourceTimeStamp);

	   // this is used for dynamically created surfaces, which are assumed to not be reloadable.
	   // It can be called again to clear out the surfaces of a dynamic model for regeneration.
	   void				InitEmpty(const char *name);

	   // dynamic model instantiations will be created with this
	   // the geometry data will be owned by the model, and freed when it is freed
	   // the geoemtry should be raw triangles, with no extra processing
	   void				AddSurface(modelSurface surface);

	   // cleans all the geometry and performs cross-surface processing
	   // like shadow hulls
	   // Creates the duplicated back side geometry for two sided, alpha tested, lit materials
	   // This does not need to be called if none of the surfaces added with AddSurface require
	   // light interaction, and all the triangles are already well formed.
	   void				FinishSurfaces();

	   // frees all the data, but leaves the class around for dangling references,
	   // which can regenerate the data with LoadModel()
	   void				PurgeModel();

	   // resets any model information that needs to be reset on a same level load etc.. 
	   // currently only implemented for liquids
	   void				Reset() {};

	   // used for initial loads, reloadModel, and reloading the data of purged models
	   // Upon exit, the model will absolutely be valid, but possibly as a default model
	   void				LoadModel();

	   // models that are already loaded at level start time
	   // will still touch their data to make sure they
	   // are kept loaded
	   void				TouchData();

	   // dump any ambient caches on the model surfaces
	   void				FreeVertexCache();

	   // returns the name of the model
	   const std::string		Name() const;

	   // reports the amount of memory (roughly) consumed by the model
	   size_t					Memory() const;

	   // for reloadModels
	   SYSTEM_TIME_T			Timestamp() const;

	   // returns the number of surfaces
	   size_t					NumSurfaces() const;

	   // NumBaseSurfaces will not count any overlays added to dynamic models
	   size_t					NumBaseSurfaces() const;

	   // get a pointer to a surface
	   const modelSurface *Surface(int surfaceNum) const;

	   // Allocates surface triangles.
	   // Allocates memory for srfTriangles_t::verts and srfTriangles_t::indexes
	   // The allocated memory is not initialized.
	   // srfTriangles_t::numVerts and srfTriangles_t::numIndexes are set to zero.
	   surfacePolygons *	AllocSurfaceTriangles(int numVerts, int numIndexes) const;

	   void AllocStaticTriSurfVerts(surfacePolygons *poly, int numVerts);
	   void AllocStaticTriSurfIndexes(surfacePolygons *tri, int numIndexes);

	   // Frees surfaces triangles.
	   void				FreeSurfaceTriangles(surfacePolygons *tris) const ;

	   // models of the form "_area*" may have a prelight shadow model associated with it
	   bool				IsStaticWorldModel() const;

	   // models parsed from inside map files or dynamically created cannot be reloaded by
	   // reloadmodels
	   bool				IsReloadable() const;

	   // md3, md5, particles, etc
	   dynamicModel_t		IsDynamicModel() const;

	   // if the load failed for any reason, this will return true
	   bool				IsDefaultModel() const;

	   surfacePolygons * AllocateStaticTriSurf();

	   // dynamic models should return a fast, conservative approximation
	   // static models should usually return the exact value
	  // Math::BoundingPlane				Bounds(const struct renderEntity_t *ent = NULL) const;

	   // returns a static model based on the definition and view
	   // currently, this will be regenerated for every view, even though
	   // some models, like character meshes, could be used for multiple (mirror)
	   // views in a frame, or may stay static for multiple frames (corpses)
	   // The renderer will delete the returned dynamic model the next view
	   // This isn't const, because it may need to reload a purged model if it
	   // wasn't precached correctly.
	   //idRenderModel *		InstantiateDynamicModel(const struct renderEntity_t *ent, const viewDef_t *view, idRenderModel *cachedModel);

	   void						MakeDefaultModel();

	   bool Load3DModelFromObjFile(char const * filename,
		   RenderModel       * renderModel);

	   RenderModel * Load3DModelFromObjFile(char const* filename);

	   public:
		   std::vector<modelSurface>	surfaces;
		   Math::BoundingPlane			bounds;
		   int							overlaysAdded;

		   // when an md5 is instantiated, the inverted joints array is stored to allow GPU skinning
		   int							numInvertedJoints;
		   //idJointMat *				jointsInverted;
		   vertexCacheHandle			jointsInvertedBuffer;

protected:
	int							lastModifiedFrame;
	int							lastArchivedFrame;

	std::string						name;
	bool						isStaticWorldModel;
	bool						defaulted;
	bool						purged;					// eventually we will have dynamic reloading
	bool						fastLoad;				// don't generate tangents and shadow data
	bool						reloadable;				// if not, reloadModels won't check timestamp
	bool						levelLoadReferenced;	// for determining if it needs to be freed
	bool						hasDrawingSurfaces;
	bool						hasInteractingSurfaces;
	bool						hasShadowCastingSurfaces;
	SYSTEM_TIME_T					timeStamp;




	};




}
