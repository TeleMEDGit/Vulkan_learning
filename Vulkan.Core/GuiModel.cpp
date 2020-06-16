#include "GuiModel.h"




namespace Graphics
{

	void R_LinkDrawSurfToView(drawSurface * drawSurf, viewDefinition * viewDef);


	/*
	=================
	R_LinkDrawSurfToView

	Als called directly by GuiModel
	=================
	*/
	void R_LinkDrawSurfToView(drawSurface * drawSurf, viewDefinition * viewDef) {
		// if it doesn't fit, resize the list
		if (viewDef->numDrawSurfs == viewDef->maxDrawSurfs) {
			drawSurface **old = viewDef->drawSurfs;
			size_t  count;

			if (viewDef->maxDrawSurfs == 0) {
				viewDef->maxDrawSurfs = INITIAL_DRAWSURFS;
				count = 0;
			}
			else {
				count = viewDef->maxDrawSurfs * sizeof(viewDef->drawSurfs[0]);
				viewDef->maxDrawSurfs *= 2;
			}
			viewDef->drawSurfs = (drawSurface **)Graphics::render_system.FrameAlloc((int)viewDef->maxDrawSurfs * sizeof(viewDef->drawSurfs[0]), FRAME_ALLOC_DRAW_SURFACE_POINTER);
			memcpy(viewDef->drawSurfs, old, count);
		}

		viewDef->drawSurfs[viewDef->numDrawSurfs] = drawSurf;
		viewDef->numDrawSurfs++;
	}
	//VertexCache	vertex_cache;

	/*
	================
	idGuiModel::idGuiModel
	================
	*/
	idGuiModel::idGuiModel() :
		m_surf(NULL),
		m_vertexBlock(0),
		m_indexBlock(0),
		m_vertexPointer(NULL),
		m_indexPointer(NULL),
		m_numVerts(0),
		m_numIndexes(0) {

		m_surfaces.resize(16);

		// identity color for drawsurf register evaluation
		for (int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++) {
			m_shaderParms[i] = 1.0f;
		}
	}

	/*
	================
	idGuiModel::Clear

	Begins collecting draw commands into surfaces
	================
	*/
	void idGuiModel::Clear() {
		m_surfaces.resize(0);
		AdvanceSurf();
	}

	/*
	================
	idGuiModel::BeginFrame
	================
	*/
	void idGuiModel::BeginFrame() {
		m_vertexBlock = vertex_cache.AllocVertex(NULL, MAX_VERTS);
		m_indexBlock = vertex_cache.AllocIndex(NULL, MAX_INDEXES);
		m_vertexPointer = (Math::DrawVertex *)vertex_cache.MappedVertexBuffer(m_vertexBlock);
		m_indexPointer = (polyIndex *)vertex_cache.MappedIndexBuffer(m_indexBlock);
		m_numVerts = 0;
		m_numIndexes = 0;
		Clear();
	}

	/*
	================
	EmitSurfaces

	For full screen GUIs, we can add in per-surface stereoscopic depth effects
	================
	*/

	//void R_LinkDrawSurfToView(drawSurf_t * drawSurf, viewDef_t * viewDef);
	void idGuiModel::EmitSurfaces(Math::Matrix4 modelMatrix, Math::Matrix4 modelViewMatrix, bool depthHack, bool linkAsEntity) {

		//TDO note:
		//all matrixes in this method are zeros
		//modelMatrix should be {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}
		//modelViewMatrix should be {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}

		//projectionRernderMatrix was calculate somewhere ans it should be  {0.003125,0.0,0.0,-1.0}, {0.0,0.004166,0,0}, {0.0,0.0,-1.0,0.0}, {0.0,0.0,0.0,0.0}




		viewEntity * guiSpace = (viewEntity *)Graphics::render_system.ClearedFrameAlloc(sizeof(*guiSpace), Graphics::FRAME_ALLOC_VIEW_ENTITY);
		memcpy(&guiSpace->modelMatrix, &modelMatrix, sizeof(guiSpace->modelMatrix));
		memcpy(&guiSpace->modelViewMatrix, &modelViewMatrix, sizeof(guiSpace->modelViewMatrix));
		//guiSpace->weaponDepthHack = depthHack;
		guiSpace->isGuiSurface = true;

		// If this is an in-game gui, we need to be able to find the matrix again for head mounted
		// display bypass matrix fixup.
		if (linkAsEntity) {
			guiSpace->next = Graphics::render_system.m_viewDef->viewEntitys;
			Graphics::render_system.m_viewDef->viewEntitys = guiSpace;
		}

		//---------------------------
		// make a tech5 renderMatrix
		//---------------------------
		Math::Matrix4 viewMat = Math::Transpose(modelViewMatrix);

		guiSpace->mvp = Graphics::render_system.m_viewDef->projectionRenderMatrix  * viewMat;


		auto whatt = m_surfaces.size();

		//if (depthHack) {
		//	idRenderMatrix::ApplyDepthHack(guiSpace->mvp);
		//}

		// add the surfaces to this view
		for (int i = 0; i < m_surfaces.size(); i++) {
			const guiModelSurface_t & guiSurf = m_surfaces[i];
			if (guiSurf.numIndexes == 0) {
				continue;
			}

			//const idMaterial * shader = guiSurf.material;
			drawSurface * drawSurf = (drawSurface *)Graphics::render_system.FrameAlloc(sizeof(*drawSurf), Graphics::FRAME_ALLOC_DRAW_SURFACE);

			drawSurf->numIndexes = guiSurf.numIndexes;
			//drawSurf->ambientCache = m_vertexBlock;
			// build a vertCacheHandle_t that points inside the allocated block
			drawSurf->indexCache = m_indexBlock + ((int64)(guiSurf.firstIndex * sizeof(polyIndex)) << Graphics::VERTCACHE_OFFSET_SHIFT);
			//drawSurf->shadowCache = 0;
			//drawSurf->jointCache = 0;
			//drawSurf->frontEndGeo = NULL;
			drawSurf->space = guiSpace;
		    //drawSurf->material = shader;
			//drawSurf->extraGLState = guiSurf.glState;
			//drawSurf->scissorRect = Graphics::render_system.m_viewDef->scissor;
			//drawSurf->sort = shader->GetSort();
			//drawSurf->renderZFail = 0;
			// process the shader expressions for conditionals / color / texcoords


			

			//const float	*constRegs = shader->ConstantRegisters();
			//if (constRegs) {
			//	// shader only uses constant values
			//	drawSurf->shaderRegisters = constRegs;
			//}
			//else {
			//	float *regs = (float *)Graphics::render_system.FrameAlloc(shader->GetNumRegisters() * sizeof(float), FRAME_ALLOC_SHADER_REGISTER);
			//	drawSurf->shaderRegisters = regs;
			//	shader->EvaluateRegisters(regs, m_shaderParms, Graphics::render_system.m_viewDef->renderView.shaderParms, Graphics::render_system.m_viewDef->renderView.time[1] * 0.001f/* NULL*/);
			//}
			R_LinkDrawSurfToView(drawSurf, Graphics::render_system.m_viewDef);
		}
	}

	/*
	====================
	EmitToCurrentView
	====================
	*/
	void idGuiModel::EmitToCurrentView(Math::Matrix4 modelMatrix, bool depthHack) {
		Math::Matrix4	modelViewMatrix = modelMatrix * Graphics::render_system.m_viewDef->worldSpace.modelViewMatrix;



		EmitSurfaces(modelMatrix, modelViewMatrix, depthHack, true /* link as entity */);
	}


	/*
	================
	idGuiModel::EmitFullScreen

	Creates a view that covers the screen and emit the surfaces
	================
	*/
	viewDefinition * idGuiModel::EmitFullScreen() {

		if (m_surfaces[0].numIndexes == 0) {
			return NULL;
		}

		//SCOPED_PROFILE_EVENT("Gui::EmitFullScreen");

		viewDefinition * viewDef = (viewDefinition *)Graphics::render_system.ClearedFrameAlloc(sizeof(*viewDef), Graphics::FRAME_ALLOC_VIEW_DEF);
		viewDef->is2Dgui = true;
		//Graphics::render_system.GetCroppedViewport(&viewDef->viewport);

		viewDef->scissor.x1 = 0;
		viewDef->scissor.y1 = 0;
		viewDef->scissor.x2 = viewDef->viewport.x2 - viewDef->viewport.x1;
		viewDef->scissor.y2 = viewDef->viewport.y2 - viewDef->viewport.y1;

		viewDef->projectionMatrix.SetX(Math::Vector4(2.0f / 800, 0.0f, 0.0f, 0.0f));
		viewDef->projectionMatrix.SetY(Math::Vector4(0.0f, 2.0f / 600, 0.0f, 0.0f));
		viewDef->projectionMatrix.SetZ(Math::Vector4(0.0f, 0.0f, -1.0f, 0.0f));
		viewDef->projectionMatrix.SetW(Math::Vector4(-1.0f, -1.0f, 0.0f, 1.0f));



		//viewDef->projectionMatrix.SetY(Math::Vector4())        //[1 * 4 + 0] = 0.0f;
	//#if defined( ID_VULKAN )
	//	viewDef->projectionMatrix[1 * 4 + 1] = 2.0f / SCREEN_HEIGHT;
	//#else
	//	viewDef->projectionMatrix[1 * 4 + 1] = -2.0f / 600;//SCREEN_HEIGHT;
	//#endif
	//	viewDef->projectionMatrix[1 * 4 + 2] = 0.0f;
	//	viewDef->projectionMatrix[1 * 4 + 3] = 0.0f;
	//
	//	viewDef->projectionMatrix[2 * 4 + 0] = 0.0f;
	//	viewDef->projectionMatrix[2 * 4 + 1] = 0.0f;
	//	viewDef->projectionMatrix[2 * 4 + 2] = -1.0f;
	//	viewDef->projectionMatrix[2 * 4 + 3] = 0.0f;
	//
	//	viewDef->projectionMatrix[3 * 4 + 0] = -1.0f;
	//#if defined( ID_VULKAN)
	//	viewDef->projectionMatrix[3 * 4 + 1] = -1.0f;
	//#else
	//	viewDef->projectionMatrix[3 * 4 + 1] = 1.0f;
	//#endif
	//	viewDef->projectionMatrix[3 * 4 + 2] = 0.0f;
	//	viewDef->projectionMatrix[3 * 4 + 3] = 1.0f;

		// make a tech5 renderMatrix for faster culling


		//TDO this may be wrong at it should be
		//modelMatrix should be {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}
		//modelViewMatrix should be {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}


		viewDef->projectionRenderMatrix = Math::Transpose(viewDef->projectionMatrix);

		

		viewDef->worldSpace.modelMatrix.SetX(Math::Vector4(1.0f, 0.0f, 0.0f, 0.0f));						// [0 * 4 + 0] = 1.0f;
		viewDef->worldSpace.modelMatrix.SetY(Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f));						//[1 * 4 + 1] = 1.0f;
		viewDef->worldSpace.modelMatrix.SetZ(Math::Vector4(0.0f, 0.0f, 1.0f, 0.0f));						//[2 * 4 + 2] = 1.0f;
		viewDef->worldSpace.modelMatrix.SetW(Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f));						//[3 * 4 + 3] = 1.0f;

		viewDef->worldSpace.modelViewMatrix.SetX(Math::Vector4(1.0f, 0.0f, 0.0f, 0.0f));                    //[0 * 4 + 0] = 1.0f;
		viewDef->worldSpace.modelViewMatrix.SetY(Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f));					   //[1 * 4 + 1] = 1.0f;
		viewDef->worldSpace.modelViewMatrix.SetZ(Math::Vector4(0.0f, 0.0f, 1.0f, 0.0f));                    //[2 * 4 + 2] = 1.0f;
		viewDef->worldSpace.modelViewMatrix.SetW(Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f));                    //[3 * 4 + 3] = 1.0f;

		viewDef->maxDrawSurfs = m_surfaces.size();
		viewDef->drawSurfs = (drawSurface **)Graphics::render_system.FrameAlloc((int)viewDef->maxDrawSurfs * sizeof(viewDef->drawSurfs[0]), Graphics::FRAME_ALLOC_DRAW_SURFACE_POINTER);
		viewDef->numDrawSurfs = 0;

		viewDefinition * oldViewDef = Graphics::render_system.m_viewDef;
		Graphics::render_system.m_viewDef = viewDef;

		EmitSurfaces(viewDef->worldSpace.modelMatrix, viewDef->worldSpace.modelViewMatrix,
			false /* depthHack */, false /* link as entity */);

		Graphics::render_system.m_viewDef = oldViewDef;

		return viewDef;
	}

	/*
	=============
	AdvanceSurf
	=============
	*/
	void idGuiModel::AdvanceSurf() {
		guiModelSurface_t	s;

		if (m_surfaces.size()) {
			//s.material = m_surf->material;
			s.glState = m_surf->glState;
		}
		else {
			//s.material = tr.defaultMaterial;
			s.glState = 0;
		}

		// advance indexes so the pointer to each surface will be 16 byte aligned
		m_numIndexes = ALIGN(m_numIndexes, 8);

		s.numIndexes = 0;
		s.firstIndex = m_numIndexes;

		m_surfaces.push_back(s);
		m_surf = &m_surfaces[m_surfaces.size() - 1];
	}

	/*
	=============
	AllocTris
	=============
	*/
	Math::DrawVertex * idGuiModel::AllocTris(int vertCount, const polyIndex * tempIndexes, int indexCount, const void * material, const uint64 glState) {
		//TDO matrial may need all the time

		if (material == NULL) {
			return NULL;
		}
	

		if (m_numIndexes + indexCount > MAX_INDEXES) {
			static int warningFrame = 0;
			if (warningFrame != Graphics::render_system.frameCount) {
				warningFrame = Graphics::render_system.frameCount;
				//idLib::Warning("idGuiModel::AllocTris: MAX_INDEXES exceeded");
			}
			return NULL;
		}
		if (m_numVerts + vertCount > MAX_VERTS) {
			static int warningFrame = 0;
			if (warningFrame != Graphics::render_system.frameCount) {
				warningFrame = Graphics::render_system.frameCount;
				//idLib::Warning("idGuiModel::AllocTris: MAX_VERTS exceeded");
			}
			return NULL;
		}

		// break the current surface if we are changing to a new material or we can't
		// fit the data into our allocated block
		if (/*material != m_surf->material ||*/ glState != m_surf->glState) {
			if (m_surf->numIndexes) {
				AdvanceSurf();
			}
			//m_surf->material = material;
			m_surf->glState = glState;
		}

		int startVert = m_numVerts;
		int startIndex = m_numIndexes;

		m_numVerts += vertCount;
		m_numIndexes += indexCount;

		m_surf->numIndexes += indexCount;

		if ((startIndex & 1) || (indexCount & 1)) {
			// slow for write combined memory!
			// this should be very rare, since quads are always an even index count
			for (int i = 0; i < indexCount; i++) {
				m_indexPointer[startIndex + i] = startVert + tempIndexes[i];
			}
		}
		else {
			for (int i = 0; i < indexCount; i += 2) {
				WriteIndexPair(m_indexPointer + startIndex + i, startVert + tempIndexes[i], startVert + tempIndexes[i + 1]);
			}
		}

		return m_vertexPointer + startVert;
	}

}
