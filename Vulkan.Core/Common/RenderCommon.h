#pragma once
#include "Common.h"

#include "..\Math\VectorMath.h"
#include "..\Math\ScreenRect.h"
#include "..\Math\BoundingPlane.h"

#include "..\idLib\Thread.h"

#include "..\Model.h"

// we may expand this to six for some subview issuess
const int MAX_CLIP_PLANES = 1;


struct viewDefinition;

struct viewEntity;
struct renderEntity;
struct renderView;

struct drawSurface
{
	size_t					numIndexes;
	vertexCacheHandle		indexCache;			// triIndex_t
	vertexCacheHandle		drawVertCache;		// idDrawVert

	const viewEntity * space;
	//const Graphics::idMaterial *		material;			// may be NULL for shadow volumes

	float					sort;

};

//class RenderModel;


struct renderEntity
{
	RenderModel *  hModel;

	int entityNumber;
	int bodyId;

	// Entities that are expensive to generate, like skeletal models, can be
	// deferred until their bounds are found to be in view, in the frustum
	// of a shadowing light that is in view, or contacted by a trace / overlay test.
	// This is also used to do visual cueing on items in the view
	// The renderView may be NULL if the callback is being issued for a non-view related
	// source.
	// The callback function should clear renderEntity->callback if it doesn't
	// want to be called again next time the entity is referenced (ie, if the
	// callback has now made the entity valid until the next updateEntity)
	Math::BoundingPlane		bounds;					// only needs to be set for deferred models and md5s

	//deferredEntityCallback_t	callback;

	void *					callbackData;

	// positioning
	// axis rotation vectors must be unit length for many
	// R_LocalToGlobal functions to work, so don't scale models!
	// axis vectors are [0] = forward, [1] = left, [2] = up
	Math::Vector3					origin;
	Math::Matrix3					axis;


};

class RenderEntity
{
public:
	RenderEntity();

	bool IsVisible() const;

	renderEntity parms;

	Math::Matrix4				modelMatrix;		// this is just a rearrangement of parms.axis and parms.origin
	Math::Matrix4				modelRenderMatrix;
	Math::Matrix4				inverseBaseModelProject;// transforms the unit cube to exactly cover the model in world space
	
	int						index;					// in world entityDefs

	int						lastModifiedFrameNum;	// to determine if it is constantly changing,
													// and should go in the dynamic frame memory, or kept
													// in the cached memory
	bool					archived;				// for demo writing

	//idRenderModel *			dynamicModel;			// if parms.model->IsDynamicModel(), this is the generated data
	int						dynamicModelFrameCount;	// continuously animating dynamic models will recreate
													// dynamicModel if this doesn't == tr.viewCount
	
	// the local bounds used to place entityRefs, either from parms for dynamic entities, or a model bounds
	Math::BoundingPlane				localReferenceBounds;

	// axis aligned bounding box in world space, derived from refernceBounds and
	// modelMatrix in idRenderWorld::CreateEntityRefs()
	Math::BoundingPlane				globalReferenceBounds;

	// a viewEntity_t is created whenever a idRenderEntity is considered for inclusion
	// in a given view, even if it turns out to not be visible
	int						viewCount;				// if tr.viewCount == viewCount, viewEntity is valid,
													// but the entity may still be off screen
	viewEntity *			viewEntity;				// in frame temporary memory

	
	

};


struct viewEntity
{
	viewEntity * next;
	// back end should NOT reference the entityDef, because it can change when running SMP
	RenderEntity	*		entityDef;

	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals, but was created for shadow casting.
	// a viewEntity can have a non-empty scissorRect, meaning that an area
	// that it is in is visible, and still not be visible.
	idScreenRect			scissorRect;

	bool					isGuiSurface;			// force two sided and vertex colors regardless of material setting

	Math::Matrix4		    modelMatrix;		// local coords to global coords
	Math::Matrix4		    modelViewMatrix;	// local coords to eye coords

	//idRenderMatrix			mvp;  //this may need to impliment future after 3D mater course is passes

	Math::Matrix4		mvp;

	drawSurface* drawSurfs;



};

struct renderView
{
	// player views will set this to a non-zero integer for model suppress / allow
	// subviews (mirrors, cameras, etc) will always clear it to zero
	int						viewID;

	float					fov_x, fov_y;		// in degrees
	Math::Vector3			vieworg;			// has already been adjusted for stereo world seperation
	Math::Vector3			vieworg_weapon;		// has already been adjusted for stereo world seperation
	Math::Matrix3			viewaxis;			// transformation matrix, view looks down the positive X axis

	bool					cramZNear;			// for cinematics, we want to set ZNear much lower
	bool					flipProjection;
	bool					forceUpdate;		// for an update 

												// time in milliseconds for shader effects and other time dependent rendering issues
	int						time[2];
	float					shaderParms[MAX_GLOBAL_SHADER_PARMS];		// can be used in any way by shader
	//const idMaterial		*globalMaterial;							// used to override everything draw

};


struct renderCommand
{
	renderCommand() :
		op(Graphics::RC_NOP),
		viewDef(),
		x(0),
		y(0),
		imageWidth(0),
		imageHeight(0),
		cubeFace(0),
		clearColorAfterCopy(false) {

	}

	
	Graphics::RenderOption	op;
	viewDefinition *	viewDef;

	int			x;
	int			y;
	int			imageWidth;
	int			imageHeight;
	//Graphics::Image *	image;
	int			cubeFace; // when copying to a cubeMap
	bool		clearColorAfterCopy;

};

struct viewDefinition
{
	// specified in the call to DrawScene()
	renderView		renderView;



	Math::Matrix4				projectionMatrix;
	Math::Matrix4		projectionRenderMatrix;	// tech5 version of projectionMatrix
	viewEntity		worldSpace;

	///idRenderWorld *renderWorld;

	Math::Vector3				initialViewAreaOrigin;
	// Used to find the portalArea that view flooding will take place from.
	// for a normal view, the initialViewOrigin will be renderView.viewOrg,
	// but a mirror may put the projection origin outside
	// of any valid area, or in an unconnected area of the map, so the view
	// area must be based on a point just off the surface of the mirror / subview.
	// It may be possible to get a failed portal pass if the plane of the
	// mirror intersects a portal, and the initialViewAreaOrigin is on
	// a different side than the renderView.viewOrg is.

	bool				isSubview;				// true if this view is not the main view
	bool				isMirror;				// the portal is a mirror, invert the face culling
	bool				isXraySubview;

	bool				isEditor;
	bool				is2Dgui;

	int					numClipPlanes;			// mirrors will often use a single clip plane

	//TDO this should be a plane class
	//I am trying the is vector
	Math::Vector4		clipPlanes[MAX_CLIP_PLANES];		// in world space, the positive side
															// of the plane is the visible side
	idScreenRect		viewport;				// in real pixels and proper Y flip

	idScreenRect		scissor;
	// for scissor clipping, local inside renderView viewport
	// subviews may only be rendering part of the main view
	// these are real physical pixel values, possibly scaled and offset from the
	// renderView x/y/width/height

	//viewDef_t *			superView;				// never go into an infinite subview loop 
	//const drawSurf_t *	subviewSurface;

	// drawSurfs are the visible surfaces of the viewEntities, sorted
	// by the material sort parameter
	drawSurface **		drawSurfs;				// we don't use an idList for this, because
	size_t						numDrawSurfs;			// it is allocated in frame temporary memory
	size_t					maxDrawSurfs;			// may be resized

	//viewLight_t	*		viewLights;			// chain of all viewLights effecting view
	viewEntity *		viewEntitys;			// chain of all viewEntities effecting view, including off screen ones casting shadows
												// we use viewEntities as a check to see if a given view consists solely
												// of 2D rendering, which we can optimize in certain ways.  A 2D view will
												// not have any viewEntities


	//TDO this should be a plane class
	//I am trying the is vector
	Math::Plane		frustum[6];				// positive sides face outward, [4] is the front clip plane

	int					areaNum;				// -1 = not in a valid area

												// An array in frame temporary memory that lists if an area can be reached without
												// crossing a closed door.  This is used to avoid drawing interactions
												// when the light is behind a closed door.
	bool *				connectedAreas;

};

/*
===========================================================================

idFrameData

all of the information needed by the back end must be
contained in a idFrameData.  This entire structure is
duplicated so the front and back end can run in parallel
on an SMP machine.

===========================================================================
*/

class FrameData {
public:
	FrameData() :
		frameMemory(NULL),
		highWaterAllocated(0),
		highWaterUsed(0),
		renderCommandIndex(0)
	{

		frameMemoryAllocated.SetValue(0);
		frameMemoryUsed.SetValue(0);

		//renderCommands.empty();
	}

	idSysInterlockedInteger	frameMemoryAllocated;
	idSysInterlockedInteger	frameMemoryUsed;
	byte *					frameMemory;

	int						highWaterAllocated;	// max used on any frame
	int						highWaterUsed;

	int						renderCommandIndex;
	//std::vector< renderCommand_t> renderCommands;
	std::array< renderCommand, 16> renderCommands;
};
