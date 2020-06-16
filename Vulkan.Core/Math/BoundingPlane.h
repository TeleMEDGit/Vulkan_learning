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

#include "VectorMath.h"

namespace Math
{
	class BoundingPlane
	{
	public:

		BoundingPlane() {}
		BoundingPlane(Vector3 normalToPlane, float distanceFromOrigin) : m_repr(normalToPlane, distanceFromOrigin) {}
		BoundingPlane(Vector3 pointOnPlane, Vector3 normalToPlane);
		BoundingPlane(float A, float B, float C, float D) : m_repr(A, B, C, D) {}
		BoundingPlane(const BoundingPlane& plane) : m_repr(plane.m_repr) {}
		explicit BoundingPlane(Vector4 plane) : m_repr(plane) {}

		INLINE operator Vector4() const { return m_repr; }

		Vector3 GetNormal(void) const { return Vector3(m_repr); }

		Scalar DistanceFromPoint(Vector3 point) const
		{
			//return Dot(point, GetNormal()) + m_repr.GetW();
			return Dot(Vector4(point, 1.0f), m_repr);
		}

		friend BoundingPlane operator* (const OrthogonalTransform& xform, BoundingPlane plane)
		{
			return BoundingPlane(xform.GetTranslation() - plane.GetNormal() * plane.m_repr.GetW(), xform.GetRotation() * plane.GetNormal());
		}

		friend BoundingPlane operator* (const Matrix4& mat, BoundingPlane plane)
		{
			return BoundingPlane(Transpose(Invert(mat)) * plane.m_repr);
		}

	private:

		Vector4 m_repr;
	};

	//=======================================================================================================
	// Inline implementations
	//
	inline BoundingPlane::BoundingPlane(Vector3 pointOnPlane, Vector3 normalToPlane)
	{
		// Guarantee a normal.  This constructor isn't meant to be called frequently, but if it is, we can change this.
		normalToPlane = Normalize(normalToPlane);
		m_repr = Vector4(normalToPlane, -Dot(pointOnPlane, normalToPlane));
	}

	//=======================================================================================================
	// Functions operating on planes
	//
	inline BoundingPlane PlaneFromPointsCCW(Vector3 A, Vector3 B, Vector3 C)
	{
		return BoundingPlane(A, Cross(B - A, C - A));
	}

	inline void GetFrustumPlanes(Plane planes[6], const Math::Matrix4 & frustum, bool zeroToOne, bool normalize)
	{
		// FIXME:	need to know whether or not this is a D3D MVP.
	//			We cannot just assume that it's an D3D MVP matrix when
	//			zeroToOne = false and CLIP_SPACE_ZERO_TO_ONE is defined because
	//			this code may be called for non-MVP matrices.
		const bool isZeroOneZ = false;

		if (zeroToOne) {

			// left: inside(p) = p * frustum[0] > 0
			planes[0][0] = frustum.GetX().GetX();
			planes[0][1] = frustum.GetX().GetY();
			planes[0][2] = frustum.GetX().GetZ();
			planes[0][3] = frustum.GetX().GetW();

			// bottom: inside(p) = p * frustum[1] > 0
			planes[2][0] = frustum.GetY().GetX();
			planes[2][1] = frustum.GetY().GetY();
			planes[2][2] = frustum.GetY().GetZ();
			planes[2][3] = frustum.GetY().GetW();

			// near: inside(p) = p * frustum[2] > 0
			planes[4][0] = frustum.GetZ().GetX();
			planes[4][1] = frustum.GetZ().GetY();
			planes[4][2] = frustum.GetZ().GetZ();
			planes[4][3] = frustum.GetZ().GetW();



		}
		else {
			// left: inside(p) = p * frustum[0] > - ( p * frustum[3] )

			planes[0][0] = frustum.GetZ().GetX() + frustum.GetX().GetX();
			planes[0][1] = frustum.GetZ().GetY() + frustum.GetX().GetY();
			planes[0][2] = frustum.GetZ().GetZ() + frustum.GetX().GetZ();
			planes[0][3] = frustum.GetZ().GetW() + frustum.GetX().GetW();

			// bottom: inside(p) = p * frustum[1] > -( p * frustum[3] )

			planes[2][0] = frustum.GetZ().GetX() + frustum.GetY().GetX();
			planes[2][1] = frustum.GetZ().GetY() + frustum.GetY().GetY();
			planes[2][2] = frustum.GetZ().GetZ() + frustum.GetY().GetZ();
			planes[2][3] = frustum.GetZ().GetW() + frustum.GetY().GetW();

			// near: inside(p) = p * frustum[2] > -( p * frustum[3] )

			planes[4][0] = isZeroOneZ ? (frustum.GetY().GetX()) : (frustum.GetZ().GetX() + frustum.GetY().GetX());
			planes[4][1] = isZeroOneZ ? (frustum.GetY().GetY()) : (frustum.GetZ().GetY() + frustum.GetY().GetY());
			planes[4][2] = isZeroOneZ ? (frustum.GetY().GetZ()) : (frustum.GetZ().GetZ() + frustum.GetY().GetZ());
			planes[4][3] = isZeroOneZ ? (frustum.GetY().GetW()) : (frustum.GetZ().GetW() + frustum.GetY().GetW());

		}

		// right: inside(p) = p * frustum[0] < p * frustum[3]

		planes[1][0] = frustum.GetZ().GetX() - frustum.GetX().GetX();
		planes[1][1] = frustum.GetZ().GetY() - frustum.GetX().GetY();
		planes[1][2] = frustum.GetZ().GetZ() - frustum.GetX().GetZ();
		planes[1][3] = frustum.GetZ().GetW() - frustum.GetX().GetW();

		// top: inside(p) = p * frustum[1] < p * frustum[3]

		planes[3][0] = frustum.GetZ().GetX() - frustum.GetY().GetX();
		planes[3][1] = frustum.GetZ().GetY() - frustum.GetY().GetY();
		planes[3][2] = frustum.GetZ().GetZ() - frustum.GetY().GetZ();
		planes[3][3] = frustum.GetZ().GetW() - frustum.GetY().GetW();

		// far: inside(p) = p * frustum[2] < p * frustum[3]

		planes[5][0] = frustum.GetZ().GetX() - frustum.GetZ().GetX();
		planes[5][1] = frustum.GetZ().GetY() - frustum.GetZ().GetY();
		planes[5][2] = frustum.GetZ().GetZ() - frustum.GetZ().GetZ();
		planes[5][3] = frustum.GetZ().GetW() - frustum.GetZ().GetW();

		if (normalize)
		{
			for (int i = 0; i < 6; i++) {
				float s = Math::InvSqrt(Math::LengthSquare(planes[i].Normal()));
				planes[i][0] *= s;
				planes[i][1] *= s;
				planes[i][2] *= s;
				planes[i][3] *= s;
			}

		}

	}




} // namespace Math

