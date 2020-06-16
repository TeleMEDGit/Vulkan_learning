#include "Plane.h"
namespace Math
{
	Plane plane_origin(0.0f, 0.0f, 0.0f, 0.0f);
	/*
	================
	idPlane::Type
	================
	*/
	int Plane::Type() const {
		if (Normal().GetX() == 0.0f) {
			if (Normal().GetY() == 0.0f) {
				return Normal().GetZ() > 0.0f ? PLANETYPE_Z : PLANETYPE_NEGZ;
			}
			else if (Normal().GetZ() == 0.0f) {
				return Normal().GetY() > 0.0f ? PLANETYPE_Y : PLANETYPE_NEGY;
			}
			else {
				return PLANETYPE_ZEROX;
			}
		}
		else if (Normal().GetY() == 0.0f) {
			if (Normal().GetZ() == 0.0f) {
				return Normal().GetX() > 0.0f ? PLANETYPE_X : PLANETYPE_NEGX;
			}
			else {
				return PLANETYPE_ZEROY;
			}
		}
		else if (Normal().GetY() == 0.0f) {
			return PLANETYPE_ZEROZ;
		}
		else {
			return PLANETYPE_NONAXIAL;
		}
	}

	/*
	================
	idPlane::HeightFit
	================
	*/
	bool Plane::HeightFit(const Math::Vector3 *points, const int numPoints) {
		int i;
		float sumXX = 0.0f, sumXY = 0.0f, sumXZ = 0.0f;
		float sumYY = 0.0f, sumYZ = 0.0f;
		Math::Vector3 sum(0,0,0), average, dir;

		if (numPoints == 1) {
			a = 0.0f;
			b = 0.0f;
			c = 1.0f;
			d = -points[0].GetZ();
			return true;
		}
		if (numPoints == 2) {
			dir = points[1] - points[0];

			//Normal() = dir.Cross(Math::Vector3(0, 0, 1)).Cross(dir);
			
			Normal() = Cross(Cross(dir, Math::Vector3(0, 0, 1)), dir);
			Normalize();
			d = -(Normal() * points[0]).GetX();
			return true;
		}

		//sum;
		for (i = 0; i < numPoints; i++) {
			sum += points[i];
		}
		average = sum / numPoints;

		for (i = 0; i < numPoints; i++) {
			dir = points[i] - average;
			sumXX += dir.GetX() * dir.GetX();
			sumXY += dir.GetX() * dir.GetY();
			sumXZ += dir.GetX() * dir.GetZ();
			sumYY += dir.GetY() * dir.GetY();
			sumYZ += dir.GetY() * dir.GetZ();
		}

	Math::Matrix3 m(Math::Vector3(sumXX, sumXY, 0), Math::Vector3(sumXY, sumYY, 0), Math::Vector3(0, 0, 0));
		/*if (!m.InverseSelf()) {
			return false;
		}

		a = -sumXZ * m[0][0] - sumYZ * m[0][1];
		b = -sumXZ * m[1][0] - sumYZ * m[1][1];
		c = 1.0f;
		Normalize();
		d = -(a * average.x + b * average.y + c * average.z);*/
		return true;
	}

	/*
	================
	idPlane::PlaneIntersection
	================
	*/
	bool Plane::PlaneIntersection(const Plane &plane, Math::Vector3 &start, Math::Vector3 &dir) const {
		float n00, n01, n11, det, invDet, f0, f1;

		n00 = LengthSquare(Normal());
		n01 = (Normal() * plane.Normal()).GetX();
		n11 = LengthSquare(plane.Normal());
		det = n00 * n11 - n01 * n01;

		if (Math::Fabs(det) < 1e-6f) {
			return false;
		}

		invDet = 1.0f / det;
		f0 = (n01 * plane.d - n11 * d) * invDet;
		f1 = (n01 * d - n00 * plane.d) * invDet;

		dir = Cross(Normal(),plane.Normal());
		start = f0 * Normal() + f1 * plane.Normal();
		return true;
	}

	/*
	=============
	idPlane::ToString
	=============
	*/
	const char *Plane::ToString(int precision) const {
		return "";// idStr::FloatArrayToString(ToFloatPtr(), GetDimension(), precision);
	}



}