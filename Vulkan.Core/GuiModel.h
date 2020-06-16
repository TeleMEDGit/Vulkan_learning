/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2016-2017 Dustin Land

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "Common\Common.h"
//#include "..\Math\VectorMath.h"
#include "Common\RenderCommon.h"
#include "Math\DrawVertex.h"
#include "RenderSystem.h"
#include "Common\VulkanVertexCatch.h"


namespace Graphics
{

	struct guiModelSurface_t {
		//const idMaterial *	material;
		unsigned long long				glState;
		size_t					firstIndex;
		size_t				numIndexes;
	};



	class idGuiModel {
	public:
		idGuiModel();

		void		Clear();

		// allocates memory for verts and indexes in frame-temporary buffer memory
		void		BeginFrame();

		void		EmitToCurrentView(Math::Matrix4 modelMatrix, bool depthHack);
		viewDefinition *	EmitFullScreen();

		// the returned pointer will be in write-combined memory, so only make contiguous
		// 32 bit writes and never read from it.
		Math::DrawVertex * AllocTris(int numVerts, const polyIndex * indexes, int numIndexes, const void * material, const uint64 glState);

		//---------------------------
	private:
		void		AdvanceSurf();
		void		EmitSurfaces(Math::Matrix4 modelMatrix, Math::Matrix4 modelViewMatrix, bool depthHack, bool linkAsEntity);

		guiModelSurface_t *			m_surf;

		float						m_shaderParms[MAX_ENTITY_SHADER_PARMS];

		// if we exceed these limits we stop rendering GUI surfaces
		static const int MAX_INDEXES = (20000 * 6);
		static const int MAX_VERTS = (20000 * 4);

		vertexCacheHandle			m_vertexBlock;
		vertexCacheHandle			m_indexBlock;
		Math::DrawVertex *		    m_vertexPointer;
		polyIndex *				m_indexPointer;

		int							m_numVerts;
		int							m_numIndexes;

		std::vector< guiModelSurface_t> m_surfaces;
	};
}


