#pragma once
//#include "Common/Common.h"
#include "VulkanFoundation.h"
#include "Common/RenderCommon.h"
#include "DXT\sys_intrinsics.h"


namespace Graphics
{
	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();
		void Initialize();

		void			SwapCommandBuffers(FrameTimeTick * frameTiming);
		void			SwapAndRenderCommandBuffers(FrameTimeTick * frameTiming);
		void			SwapCommandBuffers_FinishRendering(FrameTimeTick * frameTiming);
		void			SwapCommandBuffers_FinishCommandBuffers();
		void			RenderCommandBuffers();
		void *			FrameAlloc(int bytes, frameAllocType_t type = FRAME_ALLOC_UNKNOWN);

		void *			ClearedFrameAlloc(int bytes, frameAllocType_t type);

		//TODO 
		//Create a RenderScene and RenderView
		//The rest should fan out from there
		
		//render model should be replace with renderworld afer this called is mastered
		void			RenderScene(RenderModel * world, const renderView *renderView);

		//View port and Scissor creation

		void			CropRenderSize(int width, int height);
		void			GetCroppedViewport(idScreenRect * viewport);


		void			SetColor(const Math::Vector4 & color);
		uint32			GetColor();
		void			SetGLState(const uint64 glState);
		void			Restart();
		int				GetWidth() const;
		int				GetHeight() const;

	

	public:
		int						frameCount;			// incremented every frame
		int						viewCount;			// incremented every view (twice a scene if subviewed)
													// and every R_MarkFragments call

		viewDefinition *		m_viewDef;
		struct performanceCounters_t {
			int		c_box_cull_in;
			int		c_box_cull_out;
			int		c_createInteractions;	// number of calls to idInteraction::CreateInteraction
			int		c_createShadowVolumes;
			int		c_generateMd5;
			int		c_entityDefCallbacks;
			int		c_alloc;				// counts for R_StaticAllc/R_StaticFree
			int		c_free;
			int		c_visibleViewEntities;
			int		c_shadowViewEntities;
			int		c_viewLights;
			int		c_numViews;				// number of total views rendered
			int		c_deformedSurfaces;		// idMD5Mesh::GenerateSurface
			int		c_deformedVerts;		// idMD5Mesh::GenerateSurface
			int		c_deformedIndexes;		// idMD5Mesh::GenerateSurface
			int		c_tangentIndexes;		// R_DeriveTangents()
			int		c_entityUpdates;
			int		c_lightUpdates;
			int		c_entityReferences;
			int		c_lightReferences;
			int		c_guiSurfs;
			int		frontEndMicroSec;		// sum of time in all RE_RenderScene's in a frame
		} pc;

	private:
		void					Clear();

		void AddDrawViewCmd(viewDefinition *parms, bool guiOnly);

		void					InitFrameData();
		void					ShutdownFrameData();
		void					ToggleSmpFrame();

		//Render
		void					RenderView(viewDefinition * parms); //this sets up render parameters before AddDrawViewCommand is added



		// Guis
		void					EmitFullscreenGui();

	private:

		// Models
		void					AddModels();
		

	private:
		bool					m_bInitialized;

		idScreenRect			m_renderCrops[MAX_RENDER_CROPS];
		
		
		// GUI drawing variables for surface creation
		class idGuiModel *		m_guiModel;
		// GUI drawing variables for surface creation
		int						m_guiRecursionLevel;	// to prevent infinite overruns
		uint32					m_currentColorNativeBytesOrder;
		uint64					m_currentGLState;
		//

		// these are allocated at buffer swap time, but
		// the back end should only use the ones in the backEnd stucture,
		// which are copied over from the frame that was just swapped.

		int						m_currentRenderCrop;

		Graphics::VulkanFoundation		m_backend;
		FrameData				m_smpFrameData[NUM_FRAME_DATA];
		FrameData *			m_frameData;
		uint32					m_smpFrame;




	};

	extern RenderSystem  render_system;

	/*
	====================================================================

	TR_FRONTEND_MAIN

	====================================================================
	*/

	void *	R_StaticAlloc(int bytes, const MemoryTag tag = TAG_RENDER_STATIC);		// just malloc with error checking
	void *	R_ClearedStaticAlloc(int bytes);	// with memset
	void	R_StaticFree(void *data);

	//functions that need to be moved and name properly
	void R_SetupViewMatrix(viewDefinition *viewDef);
	void R_SetupProjectionMatrix(viewDefinition *viewDef);
	void R_AddSingleModel(viewEntity * vEntity);

	viewEntity *R_SetEntityDefViewEntity(RenderEntity *def);

}

