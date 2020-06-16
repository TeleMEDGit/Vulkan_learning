//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

//#include "Common/Common.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include "Utility.h"
#include <string>

#include "Math/VectorMath.h"
//
//#include "Math/DrawVertex.h"
//
//#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#include "External/tiny_obj_loader.h"
#include "External/stb_image.h"






//#include "External/stb_image.h"

//#include <string>

namespace Utility
{



	struct skippedAssertion_t {
		skippedAssertion_t() :
			file(NULL),
			line(-1) {
		}
		const char *	file;
		int				line;
	};
	static std::vector< skippedAssertion_t> skippedAssertions;

	/*
	========================
	AssertFailed
	========================
	*/
	bool AssertFailed(const char * file, int line, const char * expression) {
		// Set this to true to skip ALL assertions, including ones YOU CAUSE!
		static volatile bool skipAllAssertions = false;
		if (skipAllAssertions) {
			return false;
		}

		// Set this to true to skip ONLY this assertion
		static volatile bool skipThisAssertion = false;
		skipThisAssertion = false;

		for (int i = 0; i < skippedAssertions.size(); i++) {
			if (skippedAssertions[i].file == file && skippedAssertions[i].line == line) {
				skipThisAssertion = true;
				// Set breakpoint here to re-enable
				if (!skipThisAssertion) {
					skippedAssertions.erase(skippedAssertions.begin() + i);
				}
				return false;
			}
		}

		Utility::Printf("ASSERTION FAILED! %s(%d): '%s'", file, line, expression);

		if (IsDebuggerPresent() || true /*com_assertOutOfDebugger.GetBool(*/) {
			__debugbreak();
		}

		if (skipThisAssertion) {
			skippedAssertion_t * skipped = skippedAssertions.data();
			skipped->file = file;
			skipped->line = line;
		}

		return true;
	}


	// A faster version of memcopy that uses SSE instructions.  TODO:  Write an ARM variant if necessary.
	void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords)
	{
		//ASSERT(Math::IsAligned(_Dest, 16));
		//ASSERT(Math::IsAligned(_Source, 16));

		__m128i* __restrict Dest = (__m128i* __restrict)_Dest;
		const __m128i* __restrict Source = (const __m128i* __restrict)_Source;

		// Discover how many quadwords precede a cache line boundary.  Copy them separately.
		size_t InitialQuadwordCount = (4 - ((size_t)Source >> 4) & 3) & 3;
		if (InitialQuadwordCount > NumQuadwords)
			InitialQuadwordCount = NumQuadwords;

		switch (InitialQuadwordCount)
		{
		case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
		case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
		case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
		default:
			break;
		}

		if (NumQuadwords == InitialQuadwordCount)
			return;

		Dest += InitialQuadwordCount;
		Source += InitialQuadwordCount;
		NumQuadwords -= InitialQuadwordCount;

		size_t CacheLines = NumQuadwords >> 2;

		switch (CacheLines)
		{
		default:
		case 10: _mm_prefetch((char*)(Source + 36), _MM_HINT_NTA);	// Fall through
		case 9:  _mm_prefetch((char*)(Source + 32), _MM_HINT_NTA);	// Fall through
		case 8:  _mm_prefetch((char*)(Source + 28), _MM_HINT_NTA);	// Fall through
		case 7:  _mm_prefetch((char*)(Source + 24), _MM_HINT_NTA);	// Fall through
		case 6:  _mm_prefetch((char*)(Source + 20), _MM_HINT_NTA);	// Fall through
		case 5:  _mm_prefetch((char*)(Source + 16), _MM_HINT_NTA);	// Fall through
		case 4:  _mm_prefetch((char*)(Source + 12), _MM_HINT_NTA);	// Fall through
		case 3:  _mm_prefetch((char*)(Source + 8), _MM_HINT_NTA);	// Fall through
		case 2:  _mm_prefetch((char*)(Source + 4), _MM_HINT_NTA);	// Fall through
		case 1:  _mm_prefetch((char*)(Source + 0), _MM_HINT_NTA);	// Fall through

																	// Do four quadwords per loop to minimize stalls.
			for (size_t i = CacheLines; i > 0; --i)
			{
				// If this is a large copy, start prefetching future cache lines.  This also prefetches the
				// trailing quadwords that are not part of a whole cache line.
				if (i >= 10)
					_mm_prefetch((char*)(Source + 40), _MM_HINT_NTA);

				_mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
				_mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
				_mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
				_mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

				Dest += 4;
				Source += 4;
			}

		case 0:	// No whole cache lines to read
			break;
		}

		// Copy the remaining quadwords
		switch (NumQuadwords & 3)
		{
		case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
		case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
		case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
		default:
			break;
		}

		_mm_sfence();
	}

	void SIMDMemFill(void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords)
	{
		/*ASSERT(Math::IsAligned(_Dest, 16));*/

		register const __m128i Source = _mm_castps_si128(FillVector);
		__m128i* __restrict Dest = (__m128i* __restrict)_Dest;

		switch (((size_t)Dest >> 4) & 3)
		{
		case 1: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
		case 2: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
		case 3: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
		default:
			break;
		}

		size_t WholeCacheLines = NumQuadwords >> 2;

		// Do four quadwords per loop to minimize stalls.
		while (WholeCacheLines--)
		{
			_mm_stream_si128(Dest++, Source);
			_mm_stream_si128(Dest++, Source);
			_mm_stream_si128(Dest++, Source);
			_mm_stream_si128(Dest++, Source);
		}

		// Copy the remaining quadwords
		switch (NumQuadwords & 3)
		{
		case 3: _mm_stream_si128(Dest++, Source);	 // Fall through
		case 2: _mm_stream_si128(Dest++, Source);	 // Fall through
		case 1: _mm_stream_si128(Dest++, Source);	 // Fall through
		default:
			break;
		}

		_mm_sfence();
	}

	std::wstring MakeWStr(const std::string& str)
	{
		return std::wstring(str.begin(), str.end());
	}

	//bool GetBinaryFileContents(std::string const          & filename,
	//	std::vector<unsigned char> & contents) {
	//	contents.clear();

	//	std::ifstream file(filename, std::ios::binary);
	//	if (file.fail()) {
	//		std::cout << "Could not open '" << filename << "' file." << std::endl;
	//		return false;
	//	}

	//	std::streampos begin;
	//	std::streampos end;
	//	begin = file.tellg();
	//	file.seekg(0, std::ios::end);
	//	end = file.tellg();

	//	if ((end - begin) == 0) {
	//		std::cout << "The '" << filename << "' file is empty." << std::endl;
	//		return false;
	//	}
	//	contents.resize(static_cast<size_t>(end - begin));
	//	file.seekg(0, std::ios::beg);
	//	file.read(reinterpret_cast<char*>(contents.data()), end - begin);
	//	file.close();

	//	return true;
	//}

	//namespace {

	//	void GenerateTangentSpaceVectors(Mesh & mesh);
	//	//void GenerateTangentSpaceVectors2(Mesh2 & mesh);

	//}

	//bool Load3DModelFromObjFile(char const * filename,
	//	bool         load_normals,
	//	bool         load_texcoords,
	//	bool         generate_tangent_space_vectors,
	//	bool         unify,
	//	Mesh       & mesh,
	//	uint32_t   * vertex_stride) {
	//	// Load model
	//	tinyobj::attrib_t                attribs;
	//	std::vector<tinyobj::shape_t>    shapes;
	//	std::vector<tinyobj::material_t> materials;
	//	std::string                      error;

	//	bool add_color = false;
	//	


	//	bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &error, filename);
	//	if (!result) {
	//		std::cout << "Could not open the '" << filename << "' file.";
	//		if (0 < error.size()) {
	//			std::cout << " " << error;
	//		}
	//		std::cout << std::endl;
	//		return false;
	//	}

	//	// Normal vectors and texture coordinates are required to generate tangent and bitangent vectors
	//	if (!load_normals || !load_texcoords) {
	//		generate_tangent_space_vectors = false;
	//	}

	//	// Load model data and unify (normalize) its size and position
	//	float min_x = attribs.vertices[0];
	//	float max_x = attribs.vertices[0];
	//	float min_y = attribs.vertices[1];
	//	float max_y = attribs.vertices[1];
	//	float min_z = attribs.vertices[2];
	//	float max_z = attribs.vertices[2];

	//	mesh = {};

	//	//Mesh2 _mesh2;
	//	
	//	uint32_t offset = 0;
	//	for (auto & shape : shapes) {
	//		uint32_t part_offset = offset;

	//		for (auto & index : shape.mesh.indices) {
	//			/*mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
	//			mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
	//			mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);*/

	//			Math::DrawVertex vertices;
	//			vertices.xyz = Math::Vector3(attribs.vertices[3 * index.vertex_index + 0], attribs.vertices[3 * index.vertex_index + 1], attribs.vertices[3 * index.vertex_index + 2]);



	//			++offset;

	//			if (load_normals) {
	//				if (attribs.normals.size() == 0) {
	//					std::cout << "Could not load normal vectors data in the '" << filename << "' file.";
	//					return false;
	//				}
	//				else {
	//					/*mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 0]);
	//					mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 1]);
	//					mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 2]);

	//					auto nm1 = attribs.normals[3 * index.normal_index + 0];
	//					auto nm2 = attribs.normals[3 * index.normal_index + 1];
	//					auto nm3 = attribs.normals[3 * index.normal_index + 2];*/

	//					vertices.SetNormal(attribs.normals[3 * index.normal_index + 0], attribs.normals[3 * index.normal_index + 1], attribs.normals[3 * index.normal_index + 2]);
	//				}
	//			}

	//			if (load_texcoords) {
	//				if (attribs.texcoords.size() == 0) {
	//					std::cout << "Could not load texture coordinates data in the '" << filename << "' file.";
	//					return false;
	//				}
	//				else {
	//					/*mesh.Data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 0]);
	//					mesh.Data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 1]);*/

	//					vertices.SetTexCoord(attribs.texcoords[2 * index.texcoord_index + 0], attribs.texcoords[2 * index.texcoord_index + 1]);
	//				}
	//			}

	//			if (generate_tangent_space_vectors) {
	//				// Insert temporary tangent space vectors data
	//				/*for (int i = 0; i < 3; ++i) {
	//					mesh.Data.emplace_back(0.0f);
	//				}*/

	//				vertices.SetBiTangent(Math::Vector3(0, 0, 0));
	//			}

	//			if (add_color)
	//			{
	//				// Insert temporary color vectors data
	//				for (int i = 0; i < 6; ++i) {
	//					//mesh.Data.emplace_back(0.0f);

	//					vertices.color[i] = 0;

	//					vertices.color2[i] = 0;
	//				}
	//			}

	//			



	//			if (unify) {
	//				if (attribs.vertices[3 * index.vertex_index + 0] < min_x) {
	//					min_x = attribs.vertices[3 * index.vertex_index + 0];
	//				}
	//				if (attribs.vertices[3 * index.vertex_index + 0] > max_x) {
	//					max_x = attribs.vertices[3 * index.vertex_index + 0];
	//				}
	//				if (attribs.vertices[3 * index.vertex_index + 1] < min_y) {
	//					min_y = attribs.vertices[3 * index.vertex_index + 1];
	//				}
	//				if (attribs.vertices[3 * index.vertex_index + 1] > max_y) {
	//					max_y = attribs.vertices[3 * index.vertex_index + 1];
	//				}
	//				if (attribs.vertices[3 * index.vertex_index + 2] < min_z) {
	//					min_z = attribs.vertices[3 * index.vertex_index + 2];
	//				}
	//				if (attribs.vertices[3 * index.vertex_index + 2] > max_z) {
	//					max_z = attribs.vertices[3 * index.vertex_index + 2];
	//				}
	//			}

	//			mesh.Data.emplace_back(vertices);

	//		}

	//		uint32_t part_vertex_count = offset - part_offset;
	//		if (0 < part_vertex_count) {
	//			mesh.Parts.push_back({ part_offset, part_vertex_count });
	//			//_mesh2.Parts.push_back({ part_offset, part_vertex_count });
	//		}
	//	}

	//	/*uint32_t stride = 3 + (load_normals ? 3 : 0) + (load_texcoords ? 2 : 0) + (generate_tangent_space_vectors ? 3 : 0) + (add_color ? 6 : 0);
	//	if (vertex_stride) {
	//		*vertex_stride = stride * sizeof(float);
	//	}*/

	//	//only tanget calculation
	//	if (generate_tangent_space_vectors) {
	//		GenerateTangentSpaceVectors ( mesh);
	//		//GenerateTangentSpaceVectors2(_mesh2);
	//	}

	//	if (unify) {
	//		/*float offset_x = 0.5f * (min_x + max_x);
	//		float offset_y = 0.5f * (min_y + max_y);
	//		float offset_z = 0.5f * (min_z + max_z);
	//		float scale_x = abs(min_x - offset_x) > abs(max_x - offset_x) ? abs(min_x - offset_x) : abs(max_x - offset_x);
	//		float scale_y = abs(min_y - offset_y) > abs(max_y - offset_y) ? abs(min_y - offset_y) : abs(max_y - offset_y);
	//		float scale_z = abs(min_z - offset_z) > abs(max_z - offset_z) ? abs(min_z - offset_z) : abs(max_z - offset_z);
	//		float scale = scale_x > scale_y ? scale_x : scale_y;
	//		scale = scale_z > scale ? 1.0f / scale_z : 1.0f / scale;

	//		for (size_t i = 0; i < mesh.Data.size() - 2; i += stride) {
	//			mesh.Data[i + 0] = scale * (mesh.Data[i + 0] - offset_x);
	//			mesh.Data[i + 1] = scale * (mesh.Data[i + 1] - offset_y);
	//			mesh.Data[i + 2] = scale * (mesh.Data[i + 2] - offset_z);
	//		}*/
	//	}

	//	return true;
	//}


	//namespace {


	//	void CalculateTangent(Math::DrawVertex  *data,
	//		Math::Vector3 const & face_tangent)
	//	{

	//		// Gram-Schmidt orthogonalize
	//		Math::Vector3 const normal = data->GetNormal();
	//		Math::Vector3 const tangent = Math::Normalize(face_tangent - normal * Math::Dot(normal, face_tangent));

	//		// Calculate handedness
	//		//float handedness = (Math::Dot(Math::Cross(normal, tangent), face_bitangent) < 0.0f) ? -1.0f : 1.0f;

	//		//Math::Vector3 const bitangent = handedness * Math::Cross(normal, tangent);

	//		data->SetTangent(tangent);
	//		



	//	}

	//	//void CalculateTangent(float const   * normal_data,
	//	//	Math::Vector3 const & face_tangent,			
	//	//	float         * tangent_data
	//	//	) {

	//	//	// Gram-Schmidt orthogonalize
	//	//	Math::Vector3 const normal = { normal_data[0], normal_data[1], normal_data[2] };
	//	//	Math::Vector3 const tangent = Math::Normalize(face_tangent - normal * Math::Dot(normal, face_tangent));

	//	//	// Calculate handedness
	//	//	//float handedness = (Math::Dot(Math::Cross(normal, tangent), face_bitangent) < 0.0f) ? -1.0f : 1.0f;

	//	//	//Math::Vector3 const bitangent = handedness * Math::Cross(normal, tangent);

	//	//	tangent_data[0] = tangent.GetX();
	//	//	tangent_data[1] = tangent.GetY();
	//	//	tangent_data[2] = tangent.GetZ();

	//	//	

	//	//}

	//	// Based on:
	//	// Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary Mesh". Terathon Software 3D Graphics Library, 2001.
	//	// http://www.terathon.com/code/tangent.html

	//	//void CalculateTangentAndBitangent(float const   * normal_data,
	//	//	Math::Vector3 const & face_tangent,
	//	//	Math::Vector3 const & face_bitangent,
	//	//	float         * tangent_data,
	//	//	float         * bitangent_data) {
	//	//	// Gram-Schmidt orthogonalize
	//	//	Math::Vector3 const normal = { normal_data[0], normal_data[1], normal_data[2] };
	//	//	Math::Vector3 const tangent = Math::Normalize(face_tangent - normal * Math::Dot(normal, face_tangent));

	//	//	// Calculate handedness
	//	//	float handedness = (Math::Dot(Math::Cross(normal, tangent), face_bitangent) < 0.0f) ? -1.0f : 1.0f;

	//	//	Math::Vector3 const bitangent = handedness * Math::Cross(normal, tangent);

	//	//	tangent_data[0] = tangent.GetX();
	//	//	tangent_data[1] = tangent.GetY();
	//	//	tangent_data[2] = tangent.GetZ();

	//	//	bitangent_data[0] = bitangent.GetX();
	//	//	bitangent_data[1] = bitangent.GetY();
	//	//	bitangent_data[2] = bitangent.GetZ();
	//	//}


	//	void GenerateTangentSpaceVectors(Mesh & mesh)
	//	{
	//		for (auto & part : mesh.Parts) {
	//			for (size_t i = 0; i < mesh.Data.size(); i += 3) {

	//				//Getting first three locations
	//				Math::Vector3 const v1 = mesh.Data[i].xyz;
	//				Math::Vector3 const v2 = mesh.Data[i + 1].xyz;
	//				Math::Vector3 const v3 = mesh.Data[i + 2].xyz;


	//				std::array<float, 2> const w1 = { mesh.Data[i].GetTexCoord().GetX(), mesh.Data[i].GetTexCoord().GetY() };
	//				std::array<float, 2> const w2 = { mesh.Data[i + 1].GetTexCoord().GetX(), mesh.Data[i + 1].GetTexCoord().GetY() };
	//				std::array<float, 2> const w3 = { mesh.Data[i + 2].GetTexCoord().GetX(), mesh.Data[i + 2].GetTexCoord().GetY() };


	//				
	//				float x1 = v2.GetX() - v1.GetX();
	//				float x2 = v3.GetX() - v1.GetX();
	//				float y1 = v2.GetY() - v1.GetY();
	//				float y2 = v3.GetY() - v1.GetY();
	//				float z1 = v2.GetZ() - v1.GetZ();
	//				float z2 = v3.GetZ() - v1.GetZ();

	//				float s1 = w2[0] - w1[0];
	//				float s2 = w3[0] - w1[0];
	//				float t1 = w2[1] - w1[1];
	//				float t2 = w3[1] - w1[1];

	//				float r = 1.0f / (s1 * t2 - s2 * t1);
	//				Math::Vector3 face_tangent = { (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r };
	//				Math::Vector3 face_bitangent = { (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r };


	//				CalculateTangent(&mesh.Data[i], face_tangent);
	//				CalculateTangent(&mesh.Data[i+1], face_tangent);
	//				CalculateTangent(&mesh.Data[i+2], face_tangent);
	//			
	//			
	//			}
	//		}
	//	}
	//	

	//	//void GenerateTangentSpaceVectors(Mesh & mesh) {
	//	//	size_t const normal_offset = 3;
	//	//	size_t const texcoord_offset = 6;
	//	//	size_t const tangent_offset = 8;
	//	//	//size_t const bitangent_offset = 11;
	//	//	//size_t const color_offset = 11;
	//	//	size_t const stride = tangent_offset + 3;
	//	//	
	//	//	for (auto & part : mesh.Parts) {
	//	//		for (size_t i = 0; i < mesh.Data.size(); i += stride * 3) {
	//	//			size_t i1 = i;
	//	//			size_t i2 = i1 + stride;
	//	//			size_t i3 = i2 + stride;


	//	//			//Getting first three locations
	//	//			Math::Vector3 const v1 = { mesh.Data[i1], mesh.Data[i1 + 1], mesh.Data[i1 + 2] };
	//	//			Math::Vector3 const v2 = { mesh.Data[i2], mesh.Data[i2 + 1], mesh.Data[i2 + 2] };
	//	//			Math::Vector3 const v3 = { mesh.Data[i3], mesh.Data[i3 + 1], mesh.Data[i3 + 2] };



	//	//			//first three texcoords

	//	//			std::array<float, 2> const w1 = { mesh.Data[i1 + texcoord_offset], mesh.Data[i1 + texcoord_offset + 1] };
	//	//			std::array<float, 2> const w2 = { mesh.Data[i2 + texcoord_offset], mesh.Data[i2 + texcoord_offset + 1] };
	//	//			std::array<float, 2> const w3 = { mesh.Data[i3 + texcoord_offset], mesh.Data[i3 + texcoord_offset + 1] };

	//	//			float x1 = v2.GetX() - v1.GetX();
	//	//			float x2 = v3.GetX() - v1.GetX();
	//	//			float y1 = v2.GetY() - v1.GetY();
	//	//			float y2 = v3.GetY() - v1.GetY();
	//	//			float z1 = v2.GetZ() - v1.GetZ();
	//	//			float z2 = v3.GetZ() - v1.GetZ();

	//	//			float s1 = w2[0] - w1[0];
	//	//			float s2 = w3[0] - w1[0];
	//	//			float t1 = w2[1] - w1[1];
	//	//			float t2 = w3[1] - w1[1];

	//	//			float r = 1.0f / (s1 * t2 - s2 * t1);
	//	//			Math::Vector3 face_tangent = { (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r };
	//	//			Math::Vector3 face_bitangent = { (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r };

	//	//			/*CalculateTangentAndBitangent(&mesh.Data[i1 + normal_offset], face_tangent, face_bitangent, &mesh.Data[i1 + tangent_offset], &mesh.Data[i1 + bitangent_offset]);
	//	//			CalculateTangentAndBitangent(&mesh.Data[i2 + normal_offset], face_tangent, face_bitangent, &mesh.Data[i2 + tangent_offset], &mesh.Data[i2 + bitangent_offset]);
	//	//			CalculateTangentAndBitangent(&mesh.Data[i3 + normal_offset], face_tangent, face_bitangent, &mesh.Data[i3 + tangent_offset], &mesh.Data[i3 + bitangent_offset]);*/

	//	//			CalculateTangent(&mesh.Data[i1 + normal_offset], face_tangent,  &mesh.Data[i1 + tangent_offset]);
	//	//			CalculateTangent(&mesh.Data[i2 + normal_offset], face_tangent,  &mesh.Data[i2 + tangent_offset]);
	//	//			CalculateTangent(&mesh.Data[i3 + normal_offset], face_tangent, &mesh.Data[i3 + tangent_offset]);



	//	//		}
	//	//	}
	//	//}
	//}


	bool LoadTextureDataFromFile(char const                 * filename,
		int                          num_requested_components,
		std::vector<unsigned char> & image_data,
		int                        * image_width,
		int                        * image_height,
		int                        * image_num_components,
		int                        * image_data_size) {
		int width = 0;
		int height = 0;
		int num_components = 0;
		std::unique_ptr<unsigned char, void(*)(void*)> stbi_data(stbi_load(filename, &width, &height, &num_components, num_requested_components), stbi_image_free);

		if ((!stbi_data) ||
			(0 >= width) ||
			(0 >= height) ||
			(0 >= num_components)) {
			std::cout << "Could not read image!" << std::endl;
			return false;
		}

		int data_size = width * height * (0 < num_requested_components ? num_requested_components : num_components);
		if (image_data_size) {
			*image_data_size = data_size;
		}
		if (image_width) {
			*image_width = width;
		}
		if (image_height) {
			*image_height = height;
		}
		if (image_num_components) {
			*image_num_components = num_components;
		}

		image_data.resize(data_size);
		std::memcpy(image_data.data(), stbi_data.get(), data_size);
		return true;
	}

	//bool Load3DModelFromObjFile(char const * filename, RenderModel & renderModel)
	//{

	//	const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

	//	Assimp::Importer importer;
	//	const aiScene* pScene;
	//	RenderModel render_model;
	//	//std::string file = "D:\\Development C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\stylized_levi.fbx";

	//	std::string file = "D:\\Development C++\\VULKAN PROJECTS\\Vulkan.April2019\\Vulkan.Model.Loader\\AssetFolder\\Levi\\Workbench_Low.fbx";

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

	//	return false;
	//}

}

