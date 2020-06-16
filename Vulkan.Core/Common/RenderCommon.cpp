#include "RenderCommon.h"

#include "..\RenderSystem.h"


/*
============================
idRenderEntity::idRenderEntity
============================
*/
RenderEntity::RenderEntity() {
	memset(&parms, 0, sizeof(parms));
	//memset(modelMatrix, 0, sizeof(modelMatrix));
	modelMatrix = Math::Matrix4();


	//world = NULL;
	index = 0;
	lastModifiedFrameNum = 0;
	archived = false;
	//dynamicModel = NULL;
	dynamicModelFrameCount = 0;
	//cachedDynamicModel = NULL;
	localReferenceBounds = Math::BoundingPlane(0, 0, 0, 0);
	globalReferenceBounds = Math::BoundingPlane(0, 0, 0, 0);
	viewCount = 0;
	viewEntity = NULL;
	//decals = NULL;
	//overlays = NULL;
	//entityRefs = NULL;
	//firstInteraction = NULL;
	//lastInteraction = NULL;
	//needsPortalSky = false;
}

/*
============================
idRenderEntity::IsDirectlyVisible
============================
*/
bool RenderEntity::IsVisible() const {
	if (viewCount != Graphics::render_system.viewCount) {
		return false;
	}
	if (viewEntity->scissorRect.IsEmpty()) {
		// a viewEntity was created for shadow generation, but the
		// model global reference bounds isn't directly visible
		return false;
	}
	return true;
}