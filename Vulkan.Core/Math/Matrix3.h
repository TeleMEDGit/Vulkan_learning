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

#pragma once

#include "Quaternion.h"

namespace Math
{
	// Represents a 3x3 matrix while occuping a 4x4 memory footprint.  The unused row and column are undefined but implicitly
	// (0, 0, 0, 1).  Constructing a Matrix4 will make those values explicit.
	__declspec(align(16)) class Matrix3
	{
	public:
		INLINE Matrix3() {}
		INLINE Matrix3(Vector3 x, Vector3 y, Vector3 z) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
		INLINE Matrix3(const Matrix3& m) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
		INLINE Matrix3(Quaternion q) { *this = Matrix3(XMMatrixRotationQuaternion(q)); }
		INLINE explicit Matrix3(const XMMATRIX& m) { m_mat[0] = Vector3(m.c[0]); m_mat[1] = Vector3(m.c[1]); m_mat[2] = Vector3(m.c[2]); }
		INLINE explicit Matrix3(EIdentityTag) { m_mat[0] = Vector3(kXUnitVector); m_mat[1] = Vector3(kYUnitVector); m_mat[2] = Vector3(kZUnitVector); }
		INLINE explicit Matrix3(EZeroTag) { m_mat[0] = m_mat[1] = m_mat[2] = Vector3(kZero); }

		INLINE void SetX(Vector3 x) { m_mat[0] = x; }
		INLINE void SetY(Vector3 y) { m_mat[1] = y; }
		INLINE void SetZ(Vector3 z) { m_mat[2] = z; }

		INLINE Vector3 GetX() const { return m_mat[0]; }
		INLINE Vector3 GetY() const { return m_mat[1]; }
		INLINE Vector3 GetZ() const { return m_mat[2]; }

		static INLINE Matrix3 MakeXRotation(float angle) { return Matrix3(XMMatrixRotationX(angle)); }
		static INLINE Matrix3 MakeYRotation(float angle) { return Matrix3(XMMatrixRotationY(angle)); }
		static INLINE Matrix3 MakeZRotation(float angle) { return Matrix3(XMMatrixRotationZ(angle)); }
		static INLINE Matrix3 MakeScale(float scale) { return Matrix3(XMMatrixScaling(scale, scale, scale)); }
		static INLINE Matrix3 MakeScale(float sx, float sy, float sz) { return Matrix3(XMMatrixScaling(sx, sy, sz)); }
		static INLINE Matrix3 MakeScale(Vector3 scale) { return Matrix3(XMMatrixScalingFromVector(scale)); }

		INLINE operator XMMATRIX() const { return (const XMMATRIX&)m_mat; }

		INLINE Vector3 operator* (Vector3 vec) const { return Vector3(XMVector3TransformNormal(vec, *this)); }
		INLINE Matrix3 operator* (const Matrix3& mat) const { return Matrix3(*this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ()); }
		INLINE friend Vector3 operator*=(Vector3 &vec, const Matrix3 &mat) {
			float x = mat.GetX().GetX() + vec.GetX() + mat.GetY().GetX() * vec.GetY() + mat.GetZ().GetX() * vec.GetZ();
			float y = mat.GetX().GetY() + vec.GetX() + mat.GetY().GetY() * vec.GetY() + mat.GetZ().GetY() * vec.GetZ();
			vec.SetZ( mat.GetX().GetZ() * vec.GetX() + mat.GetY().GetZ() * vec.GetY() + mat.GetZ().GetZ() * vec.GetZ());

			vec.SetX(x);
			vec.SetY(y);

			return vec;

		}

	private:
		Vector3 m_mat[3];
	};

} // namespace Math

