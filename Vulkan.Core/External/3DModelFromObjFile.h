#pragma once



#include <fstream>
#include <iostream>
#include <cmath>

#include <string>

#include "../Math/VectorMath.h"

#include "../Math/DrawVertex.h"



//#include "stb_image.h"




namespace FileUtility
{
	


	bool GetBinaryFileContents(std::string const          & filename,
		std::vector<unsigned char> & contents);




	struct Mesh {
		std::vector<Math::DrawVertex>  Data; //TDO change this float to Math::DrawVertex in order to map Vertex Description

		struct Part {
			uint32_t  VertexOffset;
			uint32_t  VertexCount;
		};

		std::vector<Part>   Parts;
	};

	bool Load3DModelFromObjFileObj(char const * filename,
		bool         load_normals,
		bool         load_texcoords,
		bool         generate_tangent_space_vectors,
		bool         unify,
		Mesh       & mesh,
		uint32_t   * vertex_stride = nullptr);

}





