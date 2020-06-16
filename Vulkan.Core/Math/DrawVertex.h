#pragma once

#include "Common.h"
#include "..\Utility.h"

#include "VectorMath.h"



namespace Math
{

// The hardware converts a byte to a float by division with 255 and in the
// vertex programs we convert the floating-point value in the range [0, 1]
// to the range [-1, 1] by multiplying with 2 and subtracting 1.
#define VERTEX_BYTE_TO_FLOAT( x )		( (x) * ( 2.0f / 255.0f ) - 1.0f )
#define VERTEX_FLOAT_TO_BYTE( x )		idMath::Ftob( ( (x) + 1.0f ) * ( 255.0f / 2.0f ) + 0.5f )

// The hardware converts a byte to a float by division with 255 and in the
// fragment programs we convert the floating-point value in the range [0, 1]
// to the range [-1, 1] by multiplying with 2 and subtracting 1.
// This is the conventional OpenGL mapping which specifies an exact
// representation for -1 and +1 but not 0. The DirectX 10 mapping is
// in the comments which specifies a non-linear mapping with an exact
// representation of -1, 0 and +1 but -1 is represented twice.
#define NORMALMAP_BYTE_TO_FLOAT( x )	VERTEX_BYTE_TO_FLOAT( x )	//( (x) - 128.0f ) * ( 1.0f / 127.0f )
#define NORMALMAP_FLOAT_TO_BYTE( x )	VERTEX_FLOAT_TO_BYTE( x )	//idMath::Ftob( 128.0f + 127.0f * (x) + 0.5f )

/*
================================================
halfFloat_t
================================================
*/
typedef unsigned short halfFloat_t;

// GPU half-float bit patterns
#define HF_MANTISSA(x)	(x&1023)
#define HF_EXP(x)		((x&32767)>>10)
#define HF_SIGN(x)		((x&32768)?-1:1)

/*
========================
F16toF32
========================
*/
inline float F16toF32(halfFloat_t x) {
	int e = HF_EXP(x);
	int m = HF_MANTISSA(x);
	int s = HF_SIGN(x);

	if (0 < e && e < 31) {
		return s * powf(2.0f, (e - 15.0f)) * (1 + m / 1024.0f);
	}
	else if (m == 0) {
		return s * 0.0f;
	}
	return s * powf(2.0f, -14.0f) * (m / 1024.0f);
}


template< typename T >
inline T Lerp(const T from, const T to, float f) {
	return from + ((to - from) * f);
}
/*
========================
F32toF16
========================
*/
inline halfFloat_t F32toF16(float a) {
	unsigned int f = *(unsigned *)(&a);
	unsigned int signbit = (f & 0x80000000) >> 16;
	int exponent = ((f & 0x7F800000) >> 23) - 112;
	unsigned int mantissa = (f & 0x007FFFFF);

	if (exponent <= 0) {
		return 0;
	}
	if (exponent > 30) {
		return (halfFloat_t)(signbit | 0x7BFF);
	}

	return (halfFloat_t)(signbit | (exponent << 10) | (mantissa >> 13));
}



class DrawVertex {
	public:
		Vector3				xyz;			// 12 bytes
		halfFloat_t			st[2];			// 4 bytes
		unsigned char		normal[4];		// 4 bytes
		unsigned char		tangent[4];		// 4 bytes -- [3] is texture polarity sign
		unsigned char		color[4];		// 4 bytes
		unsigned char		color2[4];		// 4 bytes -- weights for skinning

		float				operator[](const int index) const;
		float &				operator[](const int index);

		void				Clear();

		const Vector3		GetNormal() const;
		const Vector3		GetNormalRaw() const;		// not re-normalized for renderbump

														// must be normalized already!
		void				SetNormal(float x, float y, float z);
		void				SetNormal(const Vector3 & n);

		const Vector3		GetTangent() const;
		const Vector3		GetTangentRaw() const;		// not re-normalized for renderbump

														// must be normalized already!
		void				SetTangent(float x, float y, float z);
		void				SetTangent(const Vector3 & t);

		// derived from normal, tangent, and tangent flag
		const Vector3 		GetBiTangent() const;
		const Vector3 		GetBiTangentRaw() const;	// not re-normalized for renderbump

		void				SetBiTangent(float x, float y, float z);
		inline void		SetBiTangent(const Vector3 & t);

		float				GetBiTangentSign() const;
		unsigned char		GetBiTangentSignBit() const;

		void				SetTexCoordNative(const halfFloat_t s, const halfFloat_t t);
		void				SetTexCoord(const Vector3 & st);
		void				SetTexCoord(float s, float t);
		void				SetTexCoordS(float s);
		void				SetTexCoordT(float t);
		const Vector3		GetTexCoord() const;
		const halfFloat_t	GetTexCoordNativeS() const;
		const halfFloat_t	GetTexCoordNativeT() const;

		// either 1.0f or -1.0f
		inline void		SetBiTangentSign(float sign);
		inline void		SetBiTangentSignBit(byte bit);

		void				Lerp(const DrawVertex &a, const DrawVertex &b, const float f);
		void				LerpAll(const DrawVertex &a, const DrawVertex &b, const float f);

		void				SetColor(unsigned short color);
		void				SetNativeOrderColor(unsigned short color);
		unsigned short				GetColor() const;

		void				SetColor2(unsigned short color);
		void				SetNativeOrderColor2(unsigned short color);
		void				ClearColor2();
		unsigned short		GetColor2() const;

		/*static idDrawVert	GetSkinnedDrawVert(const DrawVertex & vert, const idJointMat * joints);
		static idVec3		GetSkinnedDrawVertPosition(const DrawVertex & vert, const idJointMat * joints);*/

		//DrawVertex * _draw_vertex;

	};

#define DRAWVERT_SIZE				32
#define DRAWVERT_XYZ_OFFSET			(0*4)
#define DRAWVERT_ST_OFFSET			(3*4)
#define DRAWVERT_NORMAL_OFFSET		(4*4)
#define DRAWVERT_TANGENT_OFFSET		(5*4)
#define DRAWVERT_COLOR_OFFSET		(6*4)
#define DRAWVERT_COLOR2_OFFSET		(7*4)

	/*assert_offsetof(DrawVertex, xyz, DRAWVERT_XYZ_OFFSET);
	assert_offsetof(DrawVertex, normal, DRAWVERT_NORMAL_OFFSET);
	assert_offsetof(DrawVertex, tangent, DRAWVERT_TANGENT_OFFSET);*/

	/*
	========================
	VertexFloatToByte

	Assumes input is in the range [-1, 1]
	========================
	*/
	inline void VertexFloatToByte(const float & x, const float & y, const float & z, byte * bval) {
		assert_4_byte_aligned(bval);	// for __stvebx

#ifdef ID_WIN_X86_SSE2_INTRIN

		const __m128 vector_float_one = { 1.0f, 1.0f, 1.0f, 1.0f };
		const __m128 vector_float_half = { 0.5f, 0.5f, 0.5f, 0.5f };
		const __m128 vector_float_255_over_2 = { 255.0f / 2.0f, 255.0f / 2.0f, 255.0f / 2.0f, 255.0f / 2.0f };

		const __m128 xyz = _mm_unpacklo_ps(_mm_unpacklo_ps(_mm_load_ss(&x), _mm_load_ss(&z)), _mm_load_ss(&y));
		const __m128 xyzScaled = _mm_madd_ps(_mm_add_ps(xyz, vector_float_one), vector_float_255_over_2, vector_float_half);
		const __m128i xyzInt = _mm_cvtps_epi32(xyzScaled);
		const __m128i xyzShort = _mm_packs_epi32(xyzInt, xyzInt);
		const __m128i xyzChar = _mm_packus_epi16(xyzShort, xyzShort);
		const __m128i xyz16 = _mm_unpacklo_epi8(xyzChar, _mm_setzero_si128());

		bval[0] = (byte)_mm_extract_epi16(xyz16, 0);	// cannot use _mm_extract_epi8 because it is an SSE4 instruction
		bval[1] = (byte)_mm_extract_epi16(xyz16, 1);
		bval[2] = (byte)_mm_extract_epi16(xyz16, 2);

#else

		bval[0] = VERTEX_FLOAT_TO_BYTE(x);
		bval[1] = VERTEX_FLOAT_TO_BYTE(y);
		bval[2] = VERTEX_FLOAT_TO_BYTE(z);

#endif
	}

	/*
	========================
	idDrawVert::operator[]
	========================
	*/
	inline float DrawVertex::operator[](const int index) const {
		assert(index >= 0 && index < 5);
		return ((float *)(&xyz))[index];
	}

	/*
	========================
	idDrawVert::operator[]
	========================
	*/
	inline float	&DrawVertex::operator[](const int index) {
		assert(index >= 0 && index < 5);
		return ((float *)(&xyz))[index];
	}

	/*
	========================
	idDrawVert::Clear
	========================
	*/
	inline void DrawVertex::Clear() {
		*reinterpret_cast<dword *>(&this->xyz.GetX()) = 0;
		*reinterpret_cast<dword *>(&this->xyz.GetX()) = 0;
		*reinterpret_cast<dword *>(&this->xyz.GetZ()) = 0;
		*reinterpret_cast<dword *>(this->st) = 0;
		*reinterpret_cast<dword *>(this->normal) = 0x00FF8080;	// x=0, y=0, z=1
		*reinterpret_cast<dword *>(this->tangent) = 0xFF8080FF;	// x=1, y=0, z=0
		*reinterpret_cast<dword *>(this->color) = 0;
		*reinterpret_cast<dword *>(this->color2) = 0;
	}

	/*
	========================
	idDrawVert::GetNormal
	========================
	*/
	inline const Vector3 DrawVertex::GetNormal() const {
		Vector3 n(VERTEX_BYTE_TO_FLOAT(normal[0]),
			VERTEX_BYTE_TO_FLOAT(normal[1]),
			VERTEX_BYTE_TO_FLOAT(normal[2]));
		Normalize(n);	// after the normal has been compressed & uncompressed, it may not be normalized anymore
		return n;
	}

	/*
	========================
	idDrawVert::GetNormalRaw
	========================
	*/
	inline const Vector3 DrawVertex::GetNormalRaw() const {
		Vector3 n(VERTEX_BYTE_TO_FLOAT(normal[0]),
			VERTEX_BYTE_TO_FLOAT(normal[1]),
			VERTEX_BYTE_TO_FLOAT(normal[2]));
		// don't re-normalize just like we do in the vertex programs
		return n;
	}

	/*
	========================
	idDrawVert::SetNormal
	must be normalized already!
	========================
	*/
	inline void DrawVertex::SetNormal(const Vector3 & n) {
		VertexFloatToByte(n.GetX(), n.GetY(), n.GetZ(), normal);
	}

	/*
	========================
	idDrawVert::SetNormal
	========================
	*/
	inline void DrawVertex::SetNormal(float x, float y, float z) {
		VertexFloatToByte(x, y, z, normal);
	}


	/*
	========================
	&idDrawVert::GetTangent
	========================
	*/
	inline const Vector3 DrawVertex::GetTangent() const {
		Vector3 t(VERTEX_BYTE_TO_FLOAT(tangent[0]),
			VERTEX_BYTE_TO_FLOAT(tangent[1]),
			VERTEX_BYTE_TO_FLOAT(tangent[2]));

		return Normalize(t);
	}

	/*
	========================
	&idDrawVert::GetTangentRaw
	========================
	*/
	inline const Vector3 DrawVertex::GetTangentRaw() const {
		Vector3 t(VERTEX_BYTE_TO_FLOAT(tangent[0]),
			VERTEX_BYTE_TO_FLOAT(tangent[1]),
			VERTEX_BYTE_TO_FLOAT(tangent[2]));
		// don't re-normalize just like we do in the vertex programs
		return t;
	}


	/*
	========================
	idDrawVert::SetTangent
	========================
	*/
	inline void DrawVertex::SetTangent(float x, float y, float z) {
		VertexFloatToByte(x, y, z, tangent);
	}

	/*
	========================
	idDrawVert::SetTangent
	========================
	*/
	inline void DrawVertex::SetTangent(const Vector3 & t) {
		VertexFloatToByte(t.GetX(), t.GetY(), t.GetZ(), tangent);
	}

	/*
	========================
	idDrawVert::GetBiTangent
	========================
	*/
	inline const Vector3 DrawVertex::GetBiTangent() const {
		// derive from the normal, tangent, and bitangent direction flag
		Vector3 bitangent;

		//TDO  this need to be implimented in vector class
		/*bitangent.Cross(GetNormal(), GetTangent());
		bitangent *= GetBiTangentSign();*/
		return bitangent;
	}

	/*
	========================
	idDrawVert::GetBiTangentRaw
	========================
	*/
	inline const Vector3 DrawVertex::GetBiTangentRaw() const {
		// derive from the normal, tangent, and bitangent direction flag
		// don't re-normalize just like we do in the vertex programs

		//TDO  multipicaion assign on vector
		Vector3 bitangent = Cross(GetNormalRaw(), GetTangentRaw());
		//bitangent * GetBiTangentSign();  
		return bitangent;
	}

	/*
	========================
	idDrawVert::SetBiTangent
	========================
	*/
	inline void DrawVertex::SetBiTangent(float x, float y, float z) {
		SetBiTangent(Vector3(x, y, z));
	}

	/*
	========================
	idDrawVert::SetBiTangent
	========================
	*/
	inline void DrawVertex::SetBiTangent(const Vector3 &t) {
		Vector3 bitangent = Cross(GetNormal(), GetTangent());
		//SetBiTangentSign(bitangent * t);  //TDO  this wat
	}

	/*
	========================
	idDrawVert::GetBiTangentSign
	========================
	*/
	inline float DrawVertex::GetBiTangentSign() const {
		return (tangent[3] < 128) ? -1.0f : 1.0f;
	}

	/*
	========================
	idDrawVert::GetBiTangentSignBit
	========================
	*/
	inline byte DrawVertex::GetBiTangentSignBit() const {
		return (tangent[3] < 128) ? 1 : 0;
	}

	/*
	========================
	idDrawVert::SetBiTangentSign
	========================
	*/
	inline void DrawVertex::SetBiTangentSign(float sign) {
		tangent[3] = (sign < 0.0f) ? 0 : 255;
	}

	/*
	========================
	idDrawVert::SetBiTangentSignBit
	========================
	*/
	inline void DrawVertex::SetBiTangentSignBit(byte sign) {
		tangent[3] = sign ? 0 : 255;
	}

	/*
	========================
	idDrawVert::Lerp
	========================
	*/
	inline void DrawVertex::Lerp(const DrawVertex &a, const DrawVertex &b, const float f) {
		xyz = a.xyz + f * (b.xyz - a.xyz);
		//SetTexCoord(Lerp(a.GetTexCoord(), b.GetTexCoord(), f));  //TDO impliment  texture coordinate
	}

	/*
	========================
	idDrawVert::LerpAll
	========================
	*/
	inline void DrawVertex::LerpAll(const DrawVertex &a, const DrawVertex &b, const float f) {

		//TDO Lerp function is missing

		//xyz = ::Lerp(a.xyz, b.xyz, f);
		/*SetTexCoord(Lerp(a.GetTexCoord(), b.GetTexCoord(), f));

		Vector3 normal = Math::Lerp(a.GetNormal(), b.GetNormal(), f);
		Vector3 tangent = ::Lerp(a.GetTangent(), b.GetTangent(), f);
		Vector3 bitangent = ::Lerp(a.GetBiTangent(), b.GetBiTangent(), f);
		Normalize(normal);
		Normalize(tangent);
		Normalize(bitangent);
		SetNormal(normal);
		SetTangent(tangent);*/
		//SetBiTangent(bitangent);

		color[0] = (byte)(a.color[0] + f * (b.color[0] - a.color[0]));
		color[1] = (byte)(a.color[1] + f * (b.color[1] - a.color[1]));
		color[2] = (byte)(a.color[2] + f * (b.color[2] - a.color[2]));
		color[3] = (byte)(a.color[3] + f * (b.color[3] - a.color[3]));

		color2[0] = (byte)(a.color2[0] + f * (b.color2[0] - a.color2[0]));
		color2[1] = (byte)(a.color2[1] + f * (b.color2[1] - a.color2[1]));
		color2[2] = (byte)(a.color2[2] + f * (b.color2[2] - a.color2[2]));
		color2[3] = (byte)(a.color2[3] + f * (b.color2[3] - a.color2[3]));
	}

	/*
	========================
	idDrawVert::SetNativeOrderColor
	========================
	*/
	inline void DrawVertex::SetNativeOrderColor(unsigned short color) {
		*reinterpret_cast<dword *>(this->color) = color;
	}

	/*
	========================
	idDrawVert::SetColor
	========================
	*/

	inline void DrawVertex::SetColor(unsigned short color) {
		*reinterpret_cast<dword *>(this->color) = color;
	}

	/*
	========================
	idDrawVert::SetColor
	========================
	*/
	inline unsigned short DrawVertex::GetColor() const {
		return *reinterpret_cast<const dword *>(this->color);
	}


	/*
	========================
	idDrawVert::SetTexCoordNative
	========================
	*/
	inline void DrawVertex::SetTexCoordNative(const halfFloat_t s, const halfFloat_t t) {
		st[0] = s;
		st[1] = t;
	}

	/*
	========================
	idDrawVert::SetTexCoord
	========================
	*/
	inline void DrawVertex::SetTexCoord(const Vector3 & st) {
		SetTexCoordS(st.GetX());
		SetTexCoordT(st.GetY());
	}

	/*
	========================
	idDrawVert::SetTexCoord
	========================
	*/
	inline void DrawVertex::SetTexCoord(float s, float t) {
		SetTexCoordS(s);
		SetTexCoordT(t);
	}

	/*
	========================
	idDrawVert::SetTexCoordS
	========================
	*/
	inline void DrawVertex::SetTexCoordS(float s) {
		st[0] = F32toF16(s);
	}

	/*
	========================
	idDrawVert::SetTexCoordT
	========================
	*/
	inline void DrawVertex::SetTexCoordT(float t) {
		st[1] = F32toF16(t);
	}

	/*
	========================
	idDrawVert::GetTexCoord
	========================
	*/
	inline const Vector3 DrawVertex::GetTexCoord() const {
		return Vector3(F16toF32(st[0]), F16toF32(st[1]), 1);
	}

	/*
	========================
	idDrawVert::GetTexCoordNativeS
	========================
	*/
	inline const halfFloat_t DrawVertex::GetTexCoordNativeS() const {
		return st[0];
	}

	/*
	========================
	idDrawVert::GetTexCoordNativeT
	========================
	*/
	inline const halfFloat_t DrawVertex::GetTexCoordNativeT() const {
		return st[1];
	}

	/*
	========================
	idDrawVert::SetNativeOrderColor2
	========================
	*/
	inline void DrawVertex::SetNativeOrderColor2(unsigned short color2) {
		*reinterpret_cast<dword *>(this->color2) = color2;
	}


	/*
	========================
	idDrawVert::SetColor
	========================
	*/
	inline void DrawVertex::SetColor2(unsigned short color2) {
		*reinterpret_cast<dword *>(this->color2) = color2;
	}

	/*
	========================
	idDrawVert::ClearColor2
	========================
	*/
	inline void DrawVertex::ClearColor2() {
		*reinterpret_cast<dword *>(this->color2) = 0x80808080;
	}

	/*
	========================
	idDrawVert::GetColor2
	========================
	*/
	inline unsigned short DrawVertex::GetColor2() const {
		return *reinterpret_cast<const dword *>(this->color2);
	}

	/*
		== == == == == == == == == == == ==
		WriteDrawVerts16

		Use 16 - byte in - order SIMD writes because the destVerts may live in write - combined memory
		== == == == == == == == == == == ==
		*/
	inline void WriteDrawVerts16(DrawVertex * destVerts, const DrawVertex * localVerts, int numVerts) {
		//assert_sizeof(DrawVertex, 32);
		assert_16_byte_aligned(destVerts);
		assert_16_byte_aligned(localVerts);

#ifdef ID_WIN_X86_SSE2_INTRIN

		for (int i = 0; i < numVerts; i++) {
			__m128i v0 = _mm_load_si128((const __m128i *)((byte *)(localVerts + i) + 0));
			__m128i v1 = _mm_load_si128((const __m128i *)((byte *)(localVerts + i) + 16));
			_mm_stream_si128((__m128i *)((byte *)(destVerts + i) + 0), v0);
			_mm_stream_si128((__m128i *)((byte *)(destVerts + i) + 16), v1);
		}

#else

		memcpy(destVerts, localVerts, numVerts * sizeof(idDrawVert));

#endif
	}

	/*
	=====================
	idDrawVert::GetSkinnedDrawVert
	=====================
	*/
	//TDO this funtions needs joint matrix


	/*inline DrawVertex DrawVertex::GetSkinnedDrawVert(const DrawVertex & vert, const idJointMat * joints) {
		if (joints == NULL) {
			return vert;
		}

		const idJointMat & j0 = joints[vert.color[0]];
		const idJointMat & j1 = joints[vert.color[1]];
		const idJointMat & j2 = joints[vert.color[2]];
		const idJointMat & j3 = joints[vert.color[3]];

		const float w0 = vert.color2[0] * (1.0f / 255.0f);
		const float w1 = vert.color2[1] * (1.0f / 255.0f);
		const float w2 = vert.color2[2] * (1.0f / 255.0f);
		const float w3 = vert.color2[3] * (1.0f / 255.0f);

		idJointMat accum;
		idJointMat::Mul(accum, j0, w0);
		idJointMat::Mad(accum, j1, w1);
		idJointMat::Mad(accum, j2, w2);
		idJointMat::Mad(accum, j3, w3);

		DrawVert outVert;
		outVert.xyz = accum *Vector4(vert.xyz.GetX, vert.xyz.GetY, vert.xyz.GetZ, 1.0f);
		outVert.SetTexCoordNative(vert.GetTexCoordNativeS(), vert.GetTexCoordNativeT());
		outVert.SetNormal(accum * vert.GetNormal());
		outVert.SetTangent(accum * vert.GetTangent());
		outVert.tangent[3] = vert.tangent[3];
		for (int i = 0; i < 4; i++) {
			outVert.color[i] = vert.color[i];
			outVert.color2[i] = vert.color2[i];
		}
		return outVert;
	}*/


	/*
	===============================================================================
	Shadow Vertex
	===============================================================================
	*/
	/*class ShadowVertex {
	public:
		Vector4			xyzw;

		void			Clear();
		static int		CreateShadowCache(ShadowVertex * vertexCache, const DrawVertex *verts, const int numVerts);
	};
#define SHADOWVERT_XYZW_OFFSET		(0)

	assert_offsetof(ShadowVertex, xyzw, SHADOWVERT_XYZW_OFFSET);

	inline void ShadowVertex::Clear() {
		xyzw = Vector4{ 0,0,0,0 };
	}*/


}
		

















