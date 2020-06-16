#include "RenderSystem.h"
#include "Graphics.h"
//#include "Model.cpp"
#include "GuiModel.h"
#include "Common/Common.h"
#include "Math/3DMath.h"
//#include "External/3DModelFromObjFile.cpp"





namespace Graphics
{
	//this function should be part of world render code


/*
=============
R_SetEntityDefViewEntity

If the entityDef is not already on the viewEntity list, create
a viewEntity and add it to the list with an empty scissor rect.
=============
*/
	viewEntity *R_SetEntityDefViewEntity(RenderEntity *def) {
		//if (def->viewCount == tr.viewCount) {
		//	// already set up for this frame
		//	return def->viewEntity;
		//}
		def->viewCount = 1; // just to tryI DONT KNOW WHAT VIEWCOUNT IS  tr.viewCount;

		//TDO TDO def should contain

		viewEntity * vModel = (viewEntity *)render_system.ClearedFrameAlloc(sizeof(*vModel), FRAME_ALLOC_VIEW_ENTITY);
		vModel->entityDef = def;

		// the scissorRect will be expanded as the model bounds is accepted into visible portal chains
		// It will remain clear if the model is only needed for shadows.
		vModel->scissorRect.Clear();

		/*vModel->next = tr.m_viewDef->viewEntitys;
		tr.m_viewDef->viewEntitys = vModel;*/

		def->viewEntity = vModel;  //I am not sure this line

		return vModel;
	}



	//funtions to be moved
	/*
	======================
	R_SetupViewMatrix

	Sets up the world to view matrix for a given viewParm
	======================
	*/
	void R_SetupViewMatrix(viewDefinition *viewDef) {
		Math::Matrix4 s_flipMatrix = {
			Math::Vector4(0,0,-1,0),
			Math::Vector4(-1,0,0,0),
			Math::Vector4(0,1,0,0),
			Math::Vector4(0,0,0,1)
		};

		viewEntity *world = &viewDef->worldSpace;
		memset(world, 0, sizeof(*world));

		// the model matrix is an identity
		world->modelMatrix.SetX(Math::Vector4(1, 0, 0, 0));
		world->modelMatrix.SetY(Math::Vector4(0, 1, 0, 0));
		world->modelMatrix.SetZ(Math::Vector4(0, 0, 1, 0));
		world->modelMatrix.SetW(Math::Vector4(0, 0, 0, 0));

		//transform by the camera placement

		const Math::Vector3 & origin = viewDef->renderView.vieworg;
		
		const Math::Matrix3 & axis = viewDef->renderView.viewaxis;
		
		Math::Matrix4 viewerMatrix = {
			Math::Vector4(axis.GetX().GetX(),axis.GetY().GetX(),axis.GetZ().GetX(),0),
			Math::Vector4(axis.GetX().GetY(),axis.GetY().GetY(),axis.GetZ().GetY(),0),
			Math::Vector4(axis.GetX().GetZ(),axis.GetY().GetZ(),axis.GetZ().GetZ(),0),
			Math::Vector4(origin.GetX() *axis.GetX().GetX() - origin.GetY() * axis.GetX().GetY() - origin.GetZ() * axis.GetX().GetZ(),
						  origin.GetX() *axis.GetY().GetX() - origin.GetY() * axis.GetY().GetY() - origin.GetZ() * axis.GetY().GetZ(),
						  origin.GetX() *axis.GetZ().GetX() - origin.GetY() * axis.GetZ().GetY() - origin.GetZ() * axis.GetZ().GetZ()
						  ,1)
		};

		// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
		//R_MatrixMultiply(viewerMatrix, s_flipMatrix, world->modelViewMatrix);

		world->modelViewMatrix = (Math::Matrix4)Math::XMMatrixMultiply(viewerMatrix, s_flipMatrix);





	}

	void R_SetupProjectionMatrix(viewDefinition * viewDef)
	{
		const float zNear = (viewDef->renderView.cramZNear) ? (3 * 0.25f) : 3;

		float ymax = zNear * Math::Tan(DEG2RAD(viewDef->renderView.fov_y) * 0.5f);
		float ymin = -ymax;

		float xmax = zNear * Math::Tan(DEG2RAD(viewDef->renderView.fov_x) * 0.5f);
		float xmin = -xmax;

		const float width = xmax - xmin;
		const float height = ymax - ymin;

		const int viewWidth = viewDef->viewport.x2 - viewDef->viewport.x1 + 1;
		const int viewHeight = viewDef->viewport.y2 - viewDef->viewport.y1 + 1;

		viewDef->projectionMatrix.SetX(Math::Vector4(2.0f, 0.0f, 0.0f, 0.0f));
		//viewDef->projectionMatrix.SetY(Math::Vector4(0.0f, -2.0f * zNear / height, 0.0f, 0.0f));
		viewDef->projectionMatrix.SetZ(Math::Vector4(0.0f, 0.0f, -0.999f, -1.0f * zNear));
		viewDef->projectionMatrix.SetW(Math::Vector4(0.0f, 0.0f, -1.0f, 0.0f));

		if (viewDef->renderView.flipProjection) {
			viewDef->projectionMatrix.SetY(Math::Vector4(0.0f, 2.0f * zNear / height, 0.0f, 0.0f));
		}
		else
		{
			viewDef->projectionMatrix.SetY(Math::Vector4(0.0f, -2.0f * zNear / height, 0.0f, 0.0f));
		}


	}

	void R_AddSingleModel(viewEntity * vEntity)
	{
		// we will add all interaction surfs here, to be chained to the lights in later serial code
		vEntity->drawSurfs = NULL;
		//vEntity->staticShadowVolumes = NULL;
		//vEntity->dynamicShadowVolumes = NULL;

		//Fill missin items here
			// 1. EntityDef.param
			// 2. Then param.xxx
		RenderModel renderModelLoader;
		renderEntity  modelDef;
		//renderModelLoader.Load3DModelFromObjFile("", modelDef->hModel);   //this type of call contains Model.cpp but has issue calling in here
		//then EntityDef.param = modelDef

		modelDef.hModel = renderModelLoader.Load3DModelFromObjFile("");

		vEntity->entityDef->parms = modelDef;



		//End of missing items 



		// globals we really should pass in...
		const viewDefinition * viewDef = render_system.m_viewDef;
		RenderEntity * entityDef = vEntity->entityDef;

		const renderEntity * renderEntity = &entityDef->parms;
		//const RenderWorld * world = entityDef->world;

		//TODO contiinue and check what entityDef->parms has
		//TODO this is time to run and debug on the fly
	}

	RenderSystem::RenderSystem() :m_bInitialized(false), m_frameData(NULL)
	{
		

	}

	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::Initialize()
	{
		InitFrameData();			//Creates a memory object of the FrameData
		m_backend.Initialize();   //This pass comapring with idSystem


		// clear all our internal state
		viewCount = 1;		// so cleared structures never match viewCount
							// we used to memset tr, but now that it is a class, we can't, so
							// there may be other state we need to reset

		m_guiModel = new (TAG_RENDER) idGuiModel;
		m_guiModel->Clear();


		/* make sure the m_unitSquareTriangles data is current in the vertex / index cache
		if (m_unitSquareTriangles == NULL) {
			m_unitSquareTriangles = R_MakeFullScreenTris();
		}
		make sure the zeroOneCubeTriangles data is current in the vertex / index cache
		if (m_zeroOneCubeTriangles == NULL) {
			m_zeroOneCubeTriangles = R_MakeZeroOneCubeTris();
		}
		make sure the m_testImageTriangles data is current in the vertex / index cache
		if (m_testImageTriangles == NULL) {
			m_testImageTriangles = R_MakeTestImageTriangles();
		}*/

		m_bInitialized = true;

		//
		// make sure the command buffers are ready to accept the first screen update
		SwapCommandBuffers(NULL);
	}

	void RenderSystem::SwapCommandBuffers(FrameTimeTick * frameTiming)
	{
		SwapCommandBuffers_FinishRendering(frameTiming);
		SwapCommandBuffers_FinishCommandBuffers();
	}

	void Graphics::RenderSystem::SwapAndRenderCommandBuffers(FrameTimeTick * frameTiming)
	{
		SwapCommandBuffers(frameTiming);
		RenderCommandBuffers();
	}

	void Graphics::RenderSystem::SwapCommandBuffers_FinishRendering(FrameTimeTick * frameTiming)
	{
		if (!m_bInitialized) {
			return;
		}

		


		m_backend.BlockingSwapBuffers();
	}

	void Graphics::RenderSystem::SwapCommandBuffers_FinishCommandBuffers()
	{
		if (!m_bInitialized) {
			return;
		}


		EmitFullscreenGui();

		vertex_cache.BeginBackEnd();

		m_guiModel->BeginFrame();

		ToggleSmpFrame();

		frameCount++;

		m_guiRecursionLevel = 0;  //TDO what is this
	}

	void Graphics::RenderSystem::RenderCommandBuffers()
	{
		m_backend.Execute();
	}

	void * Graphics::RenderSystem::FrameAlloc(int bytes, frameAllocType_t type)
	{
		#if defined( TRACK_FRAME_ALLOCS )
			//m_frameData->frameMemoryUsed.Add(bytes);
			frameAllocTypeCount[type].Add(bytes);
		#endif
		
			bytes = (bytes + FRAME_ALLOC_ALIGNMENT - 1) & ~(FRAME_ALLOC_ALIGNMENT - 1);
		
			//TDO thread save code below is broken and overiden the old memeory
			//FIX FIX FIX IT OK
		
			// thread safe add
			int end = m_frameData->frameMemoryAllocated.Add(bytes);
			//int end = bytes;
			if (end > MAX_FRAME_MEMORY) {
				ERROR("idRenderSystemLocal::FrameAlloc ran out of memory. bytes = %d, end = %d, highWaterAllocated = %d\n", bytes, end, m_frameData->highWaterAllocated);
			}
		
			byte * ptr = m_frameData->frameMemory + end - bytes;
		
			// cache line clear the memory
			for (int offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
				ZeroCacheLine(ptr, offset);
			}

		return  ptr;
	}

	void * RenderSystem::ClearedFrameAlloc(int bytes, frameAllocType_t type)
	{
		// NOTE: every allocation is cache line cleared
		return FrameAlloc(bytes, type);
	}
	// this should recieve the Decrription of the world as RenderWorld instead of RenderModel
	// TODO Check the rlationaship between model data and renderView object
	// is it loaded with model data or it is create just before render it by knowing where the viewer is looking from
	void RenderSystem::RenderScene(RenderModel * world, const renderView * renderView)
	{
		if (!m_bInitialized) {
			return;
		}

		//TODO evaluate this line and not needed remove it
		EmitFullscreenGui();

		// setup viewDef for the intial view
		viewDefinition * parms = (viewDefinition *)ClearedFrameAlloc(sizeof(*parms), FRAME_ALLOC_VIEW_DEF);
		parms->renderView = *renderView;

		// NOTE: TDO Get the size of the screen here in order to set view port and also pass
		// the scissor bounds may be shrunk in subviews even if
		// the viewport stays the same
		// this scissor range is local inside the viewport

		CropRenderSize(GetWidth(), GetHeight());

		GetCroppedViewport(&parms->viewport);

		parms->scissor.x1 = 0;
		parms->scissor.y1 = 0;
		parms->scissor.x2 = parms->viewport.x2 - parms->viewport.x1;
		parms->scissor.y2 = parms->viewport.y2 - parms->viewport.y1;

		parms->isSubview = false;
		parms->initialViewAreaOrigin = renderView->vieworg;
		//parms->renderWorld = world;

		// see if the view needs to reverse the culling sense in mirrors
	// or environment cube sides

		Math::Vector3 cross;
		cross =  Math::Cross( parms->renderView.viewaxis.GetY(), parms->renderView.viewaxis.GetZ());

		
		/*if (cross * parms->renderView.viewaxis.GetX() > 0) {
			parms->isMirror = false;
		}
		else {
			parms->isMirror = true;
		}*/


		// rendering this view may cause other views to be rendered
	// for mirrors / portals / shadows / environment maps
	// this will also cause any necessary entities and lights to be
	// updated to the demo file
		RenderView(parms);



		// prepare for any 2D drawing after this
		m_guiModel->Clear();


	}

	void RenderSystem::CropRenderSize(int width, int height)
	{
		if (!m_bInitialized)
		{
			return;
		}

		EmitFullscreenGui();

		if (width < 1 || height < 1)
		{
			ERROR("CropRenderSize: bad size");
		}

		idScreenRect & previous = m_renderCrops[m_currentRenderCrop];

		m_currentRenderCrop++;

		idScreenRect & current = m_renderCrops[m_currentRenderCrop];

		current.x1 = previous.x1;
		current.x2 = previous.x1 + width - 1;
		current.y1 = previous.y2 - height + 1;
		current.y2 = previous.y2;

	}

	void RenderSystem::GetCroppedViewport(idScreenRect* viewport)
	{
		*viewport = m_renderCrops[m_currentRenderCrop];
	}

	void Graphics::RenderSystem::SetColor(const Math::Vector4 & color)
	{
	}

	uint32 Graphics::RenderSystem::GetColor()
	{
		return uint32();
	}

	void Graphics::RenderSystem::SetGLState(const uint64 glState)
	{
	}

	void Graphics::RenderSystem::Restart()
	{
		m_backend.Restart();
	}

	int Graphics::RenderSystem::GetWidth() const
	{
		return Graphics::g_Width;

		//return Graphics::GetWidth();
	}

	int Graphics::RenderSystem::GetHeight() const
	{
		return Graphics::g_Height;

		//return Graphics::GetHeight();
	}

	void Graphics::RenderSystem::Clear()
	{
	}

	void RenderSystem::AddDrawViewCmd(viewDefinition * parms, bool guiOnly)
	{
		renderCommand & cmd = m_frameData->renderCommands[m_frameData->renderCommandIndex++];
		cmd.op = RC_DRAW_VIEW;
		cmd.viewDef = parms;

		pc.c_numViews++;
	}

	void RenderSystem::InitFrameData()
	{
		ShutdownFrameData();

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			m_smpFrameData[i].frameMemory = (byte *)Mem_Alloc16(MAX_FRAME_MEMORY, TAG_RENDER);
		}

		// must be set before ToggleSmpFrame()
		m_frameData = &m_smpFrameData[0];

		ToggleSmpFrame();

	}

	void RenderSystem::ShutdownFrameData()
	{
		m_frameData = NULL;
		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			Mem_Free16(m_smpFrameData[i].frameMemory);
			m_smpFrameData[i].frameMemory = NULL;
		}
	}

	void RenderSystem::ToggleSmpFrame()
	{
		// update the highwater mark
		if (m_frameData->frameMemoryAllocated.GetValue() > m_frameData->highWaterAllocated) {
			m_frameData->highWaterAllocated = m_frameData->frameMemoryAllocated.GetValue();
#if defined( TRACK_FRAME_ALLOCS )
			m_frameData->highWaterUsed = m_frameData->frameMemoryUsed.GetValue();
			for (int i = 0; i < FRAME_ALLOC_MAX; i++) {
				frameHighWaterTypeCount[i] = frameAllocTypeCount[i].GetValue();
			}
#endif
		}

		// switch to the next frame
		m_smpFrame++;
		m_frameData = &m_smpFrameData[m_smpFrame % NUM_FRAME_DATA];

		// reset the memory allocation
		const uint32 bytesNeededForAlignment = FRAME_ALLOC_ALIGNMENT - ((uint32)m_frameData->frameMemory & (FRAME_ALLOC_ALIGNMENT - 1));
		m_frameData->frameMemoryAllocated.SetValue(bytesNeededForAlignment);
		m_frameData->frameMemoryUsed.SetValue(0);

#if defined( TRACK_FRAME_ALLOCS )
		for (int i = 0; i < FRAME_ALLOC_MAX; i++) {
			frameAllocTypeCount[i].SetValue(0);
		}
#endif

		// clear the command chain
		m_frameData->renderCommandIndex = 0;

		//std::fill(std::begin(m_frameData->renderCommands), std::end(m_frameData->renderCommands), 0);
		//m_frameData->renderCommands.fill(0);
	
	}

	void RenderSystem::RenderView(viewDefinition * parms)
	{
		// save view in case we are a subview
		viewDefinition * oldView = m_viewDef;

		m_viewDef = parms;


		// setup the matrix for world space to eye space
		R_SetupViewMatrix( m_viewDef );

		// we need to set the projection matrix before doing
		// portal-to-screen scissor calculations
		R_SetupProjectionMatrix( m_viewDef );

		m_viewDef->projectionRenderMatrix = Math::Transpose(m_viewDef->projectionMatrix);

		Math::Matrix4 viewRenderMatrix = Math::Transpose(m_viewDef->worldSpace.modelViewMatrix);

		m_viewDef->worldSpace.mvp = (Math::Matrix4)Math::XMMatrixMultiply(m_viewDef->projectionRenderMatrix, viewRenderMatrix);

		Math::GetFrustumPlanes(m_viewDef->frustum, m_viewDef->worldSpace.mvp, false, true);

		// the DOOM 3 frustum planes point outside the frustum
		for (int i = 0; i < 6; i++) {
			m_viewDef->frustum[i] = -m_viewDef->frustum[i];
		}
		// remove the Z-near to avoid portals from being near clipped
		m_viewDef->frustum[4][3] -= 3.0;

		//TDO TDO add block of code that gets ViewLights and Entities
		//similar what this call is doing idRenderWorld::FindViewLightsAndEntities()


		
		// adds ambient surfaces and create any necessary interaction surfaces to add to the light lists
		AddModels();


		// add the rendering commands for this viewDef
		AddDrawViewCmd(parms, false);



 		m_viewDef = oldView;


	}

	void RenderSystem::EmitFullscreenGui()
	{
		viewDefinition * guiViewDef = m_guiModel->EmitFullScreen();
		if (guiViewDef) {
			// add the command to draw this view
			AddDrawViewCmd(guiViewDef, true);
		}
		m_guiModel->Clear();
	}

	void RenderSystem::AddModels()
	{
		//Note find a way to add serface information including Geo data
		//skip the rest of the detail
		//this should link to catched traingles or vertices  this should endup in framedata 

		//Just create an entity in order to view this model.  this is part of render world or where ever i can fit it

		//TDO TDO  add way initiate a render entity  like: 
		   //

		//viewEntity* vEntity = m_viewDef->viewEntitys;
		
		RenderEntity * entity = new (TAG_RENDER_ENTITY) RenderEntity;

		//TDO TDO fill def here and before the R_Set..

		//filling entity temp. here it should be loaded with map data or areal view
		{ 
			entity->modelMatrix.SetX(Math::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
			entity->modelMatrix.SetY(Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
			entity->modelMatrix.SetZ(Math::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
			entity->modelMatrix.SetW(Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

			entity->modelRenderMatrix.SetX(Math::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
			entity->modelRenderMatrix.SetY(Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
			entity->modelRenderMatrix.SetZ(Math::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
			entity->modelRenderMatrix.SetW(Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

			entity->inverseBaseModelProject.SetX(Math::Vector4(7136.0f, 0.0f, 0.0f, -5016.0f));
			entity->inverseBaseModelProject.SetY(Math::Vector4(0.0f, 2690.0f, 0.0f, -5821.0f));
			entity->inverseBaseModelProject.SetX(Math::Vector4(0.0f, 0.0f, 1504.0f, 672.0f));
			entity->inverseBaseModelProject.SetX(Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			entity->index = 0;
			entity->viewCount = 0;
			entity->localReferenceBounds = Math::BoundingPlane(Math::Vector3(-12152.00, -8512.0, -832.0), Math::Vector3(2120.0, -3131.0, 2176.0));
			entity->globalReferenceBounds = Math::BoundingPlane(Math::Vector3(-12152.00, -8512.0, -832.0), Math::Vector3(2120.0, -3131.0, 2176.0));
			entity->viewEntity = nullptr;
			entity->lastModifiedFrameNum = 0;
			entity->archived = false;
			entity->dynamicModelFrameCount = 0;
			

		}

		 //load a model here add 
		{

		}




		m_viewDef->viewEntitys = R_SetEntityDefViewEntity(entity);

		//TODO viewEntitys should have model info via renderEntity
		//thier system loading that info during world render so we should have a process of render model
		//

		for (viewEntity * vEntity = m_viewDef->viewEntitys; vEntity != NULL; vEntity = vEntity->next) {
			R_AddSingleModel(vEntity);
		}
	}

	void * R_StaticAlloc(int bytes, const MemoryTag tag)
	{
		render_system.pc.c_alloc++;

		void * buf = Mem_Alloc(bytes, tag);

		// don't exit on failure on zero length allocations since the old code didn't
		if (buf == NULL && bytes != 0) {
			ERROR("R_StaticAlloc failed on %i bytes", bytes);
		}
		return buf;
	}

	void * R_ClearedStaticAlloc(int bytes)
	{
		void * buf = R_StaticAlloc(bytes);
		memset(buf, 0, bytes);
		return buf;
	}

	void R_StaticFree(void * data)
	{
		render_system.pc.c_free++;
		Mem_Free(data);
	}

}
