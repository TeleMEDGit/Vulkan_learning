#pragma once
#include "VectorMath.h"
#include "3DMath.h"


namespace Math
{
#define	ON_EPSILON					0.1f
#define DEGENERATE_DIST_EPSILON		1e-4f

#define	SIDE_FRONT					0
#define	SIDE_BACK					1
#define	SIDE_ON						2
#define	SIDE_CROSS					3

	// plane sides
#define PLANESIDE_FRONT				0
#define PLANESIDE_BACK				1
#define PLANESIDE_ON				2
#define PLANESIDE_CROSS				3

	// plane types
#define PLANETYPE_X					0
#define PLANETYPE_Y					1
#define PLANETYPE_Z					2
#define PLANETYPE_NEGX				3
#define PLANETYPE_NEGY				4
#define PLANETYPE_NEGZ				5
#define PLANETYPE_TRUEAXIAL			6	// all types < 6 are true axial planes
#define PLANETYPE_ZEROX				6
#define PLANETYPE_ZEROY				7
#define PLANETYPE_ZEROZ				8
#define PLANETYPE_NONAXIAL			9


	class Plane
	{
	public:
		Plane();
		explicit Plane(float a, float b, float c, float d);
		explicit Plane(const Math::Vector3 &normal, const float dist);
		explicit Plane(const Math::Vector3 & v0, const Math::Vector3 & v1, const Math::Vector3 & v2, bool fixDegenerate = false);

		float	operator[](int index) const;
		float &	operator[](int index);
	    Plane	operator-() const;						// flips plane
		Plane &	operator=(const Math::Vector3 &v);			// sets normal and sets Plane::d to zero
		Plane	operator+(const Plane &p) const;	// add plane equations
		Plane	operator-(const Plane &p) const;	// subtract plane equations
		Plane	operator*(const float s) const;		// scale plane
		Plane &	operator*=(const Math::Matrix3 &m);			// Normal() *= m

		bool			Compare(const Plane &p) const;						// exact compare, no epsilon
		bool			Compare(const Plane &p, const float epsilon) const;	// compare with epsilon
		bool			Compare(const Plane &p, const float normalEps, const float distEps) const;	// compare with epsilon
		bool			operator==(const Plane &p) const;					// exact compare, no epsilon
		bool			operator!=(const Plane &p) const;					// exact compare, no epsilon


		void			Zero();							// zero plane
		void			SetNormal(const Math::Vector3 &normal);		// sets the normal
		const Math::Vector3 &	Normal() const;					// reference to const normal
		Math::Vector3 &		Normal();							// reference to normal
		float			Normalize(bool fixDegenerate = true);	// only normalizes the plane normal, does not adjust d
		bool			FixDegenerateNormal();			// fix degenerate normal
		bool			FixDegeneracies(float distEpsilon);	// fix degenerate normal and dist
		float			Dist() const;						// returns: -d
		void			SetDist(const float dist);			// sets: d = -dist
		int				Type() const;						// returns plane type

		bool			FromPoints(const Math::Vector3 &p1, const Math::Vector3 &p2, const Math::Vector3 &p3, bool fixDegenerate = true);
		bool			FromVecs(const Math::Vector3 &dir1, const Math::Vector3 &dir2, const Math::Vector3 &p, bool fixDegenerate = true);
		void			FitThroughPoint(const Math::Vector3 &p);	// assumes normal is valid
		bool			HeightFit(const Math::Vector3 *points, const int numPoints);
		Plane			Translate(const Math::Vector3 &translation) const;
		Plane &		TranslateSelf(const Math::Vector3 &translation);
		Plane			Rotate(const Math::Vector3 &origin, const Math::Matrix3 &axis) const;
		Plane &		RotateSelf(const Math::Vector3 &origin, const Math::Matrix3 &axis);

		float			Distance(const Math::Vector3 &v) const;
		int				Side(const Math::Vector3 &v, const float epsilon = 0.0f) const;

		bool			LineIntersection(const Math::Vector3 &start, const Math::Vector3 &end) const;
		// intersection point is start + dir * scale
		bool			RayIntersection(const Math::Vector3 &start, const Math::Vector3 &dir, float &scale) const;
		bool			PlaneIntersection(const Plane &plane, Math::Vector3 &start, Math::Vector3 &dir) const;

		int				GetDimension() const;

		const Math::Vector4 &	ToVec4() const;
		Math::Vector4 &		ToVec4();
		const float *	ToFloatPtr() const;
		float *			ToFloatPtr();
		const char *	ToString(int precision = 2) const;

	private:
		float			a;
		float			b;
		float			c;
		float			d;

	};

	extern Plane plane_origin;
#define plane_zero plane_origin

	INLINE Plane::Plane() {
	}

	INLINE Plane::Plane(float a, float b, float c, float d) {
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
	}

	INLINE Plane::Plane(const Math::Vector3 &normal, const float dist) {
		this->a = normal.GetX();
		this->b = normal.GetY();
		this->c = normal.GetZ();
		this->d = -dist;
	}

	INLINE Plane::Plane(const Math::Vector3 & v0, const Math::Vector3 & v1, const Math::Vector3 & v2, bool fixDegenerate) {
		FromPoints(v0, v1, v2, fixDegenerate);
	}

	INLINE float Plane::operator[](int index) const {
		return (&a)[index];
	}

	INLINE float& Plane::operator[](int index) {
		return (&a)[index];
	}

	INLINE Plane Plane::operator-() const {
		return Plane(-a, -b, -c, -d);
	}

	INLINE Plane &Plane::operator=(const Math::Vector3 &v) {
		a = v.GetX();
		b = v.GetY();
		c = v.GetZ();
		d = 0;
		return *this;
	}

	INLINE Plane Plane::operator+(const Plane &p) const {
		return Plane(a + p.a, b + p.b, c + p.c, d + p.d);
	}

	INLINE Plane Plane::operator-(const Plane &p) const {
		return Plane(a - p.a, b - p.b, c - p.c, d - p.d);
	}

	INLINE Plane Plane::operator*(const float s) const {
		return Plane(a * s, b * s, c * s, d * s);
	}

	INLINE Plane &Plane::operator*=(const Math::Matrix3 &m) {		
			Normal() *= m;
		return *this;
	}

	INLINE bool Plane::Compare(const Plane &p) const {
		return (a == p.a && b == p.b && c == p.c && d == p.d);
	}

	INLINE bool Plane::Compare(const Plane &p, const float epsilon) const {
		if (Math::Fabs(a - p.a) > epsilon) {
			return false;
		}

		if (Math::Fabs(b - p.b) > epsilon) {
			return false;
		}

		if (Math::Fabs(c - p.c) > epsilon) {
			return false;
		}

		if (Math::Fabs(d - p.d) > epsilon) {
			return false;
		}

		return true;
	}

	INLINE bool Plane::Compare(const Plane &p, const float normalEps, const float distEps) const {
		if (Math::Fabs(d - p.d) > distEps) {
			return false;
		}
		if (!Normal().Compare(p.Normal(), normalEps)) {
			return false;
		}
		return true;
	}

	INLINE bool Plane::operator==(const Plane &p) const {
		return Compare(p);
	}

	INLINE bool Plane::operator!=(const Plane &p) const {
		return !Compare(p);
	}

	INLINE void Plane::Zero() {
		a = b = c = d = 0.0f;
	}

	INLINE void Plane::SetNormal(const Math::Vector3 &normal) {
		a = normal.GetX();
		b = normal.GetY();
		c = normal.GetZ();
	}

	INLINE const Math::Vector3 &Plane::Normal() const {
		return *reinterpret_cast<const Math::Vector3 *>(&a);
	}

	INLINE Math::Vector3 &Plane::Normal() {
		return *reinterpret_cast<Math::Vector3 *>(&a);
	}

	INLINE float Plane::Normalize(bool fixDegenerate) {
		float length = reinterpret_cast<Math::Vector3 *>(&a)->GetLength();

		if (fixDegenerate) {
			FixDegenerateNormal();
		}
		return length;
	}

	INLINE bool Plane::FixDegenerateNormal() {
		return Normal().FixDegenerateNormal();
	}

	INLINE bool Plane::FixDegeneracies(float distEpsilon) {
		bool fixedNormal = FixDegenerateNormal();
		// only fix dist if the normal was degenerate
		if (fixedNormal) {
			if (Math::Fabs(d - Math::Rint(d)) < distEpsilon) {
				d = Math::Rint(d);
			}
		}
		return fixedNormal;
	}

	INLINE float Plane::Dist() const {
		return -d;
	}

	INLINE void Plane::SetDist(const float dist) {
		d = -dist;
	}

	INLINE bool Plane::FromPoints(const Math::Vector3 &p1, const Math::Vector3 &p2, const Math::Vector3 &p3, bool fixDegenerate) {
		Normal() = Cross((p1 - p2), (p3 - p2));
		if (Normalize(fixDegenerate) == 0.0f) {
			return false;
		}
		
		d = -(Normal() * p2).GetX();
		return true;
	}

	INLINE bool Plane::FromVecs(const Math::Vector3 &dir1, const Math::Vector3 &dir2, const Math::Vector3 &p, bool fixDegenerate) {
		Normal() = Cross(dir1,dir2);
		if (Normalize(fixDegenerate) == 0.0f) {
			return false;
		}
		d = -(Normal() * p).GetX();
		return true;
	}

	INLINE void Plane::FitThroughPoint(const Math::Vector3 &p) {
		d = -(Normal() * p).GetX();
	}

	INLINE Plane Plane::Translate(const Math::Vector3 &translation) const {
		return Plane(a, b, c, d - (translation * Normal()).GetLength());
	}

	INLINE Plane &Plane::TranslateSelf(const Math::Vector3 &translation) {
		d -= (translation * Normal()).GetLength();
		return *this;
	}

	INLINE Plane Plane::Rotate(const Math::Vector3 &origin, const Math::Matrix3 &axis) const {
		Plane p;
		p.Normal() =  axis * Normal();
		p.d = d + (origin * Normal()).GetX() - (origin * p.Normal()).GetX();
		return p;
	}

	INLINE Plane &Plane::RotateSelf(const Math::Vector3 &origin, const Math::Matrix3 &axis) {
		d += (origin * Normal()).GetX();
		Normal() *= axis;
		d -= (origin * Normal()).GetX();
		return *this;
	}

	INLINE float Plane::Distance(const Math::Vector3 &v) const {
		return a * v.GetX() + b * v.GetY() + c * v.GetZ() + d;
	}

	INLINE int Plane::Side(const Math::Vector3 &v, const float epsilon) const {
		float dist = Distance(v);
		if (dist > epsilon) {
			return PLANESIDE_FRONT;
		}
		else if (dist < -epsilon) {
			return PLANESIDE_BACK;
		}
		else {
			return PLANESIDE_ON;
		}
	}

	INLINE bool Plane::LineIntersection(const Math::Vector3 &start, const Math::Vector3 &end) const {
		float d1, d2, fraction;

		d1 = (Normal() * start).GetX() + d;
		d2 = (Normal() * end).GetX() + d;
		if (d1 == d2) {
			return false;
		}
		if (d1 > 0.0f && d2 > 0.0f) {
			return false;
		}
		if (d1 < 0.0f && d2 < 0.0f) {
			return false;
		}
		fraction = (d1 / (d1 - d2));
		return (fraction >= 0.0f && fraction <= 1.0f);
	}

	INLINE bool Plane::RayIntersection(const Math::Vector3 &start, const Math::Vector3 &dir, float &scale) const {
		float d1, d2;

		d1 = (Normal() * start).GetX();
		d2 = (Normal() * dir).GetX();
		if (d2 == 0.0f) {
			return false;
		}
		scale = -(d1 / d2);
		return true;
	}

	INLINE int Plane::GetDimension() const {
		return 4;
	}

	INLINE const Math::Vector4 &Plane::ToVec4() const {
		return *reinterpret_cast<const Math::Vector4 *>(&a);
	}

	INLINE Math::Vector4 &Plane::ToVec4() {
		return *reinterpret_cast<Math::Vector4 *>(&a);
	}

	INLINE const float *Plane::ToFloatPtr() const {
		return reinterpret_cast<const float *>(&a);
	}

	INLINE float *Plane::ToFloatPtr() {
		return reinterpret_cast<float *>(&a);
	}

	

}