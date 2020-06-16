#pragma once
#include "Common.h"
//#include "3DMath.h"

class idScreenRect {
public:
	// Inclusive pixel bounds inside viewport
	float		x1;
	float		y1;
	float		x2;
	float		y2;

	// for depth bounds test
	float       zmin;
	float		zmax;

	// clear to backwards values
	void		Clear();
	bool		IsEmpty() const;
	float		GetWidth() const { return x2 - x1 + 1; }
	float		GetHeight() const { return y2 - y1 + 1; }
	float		GetArea() const { return (x2 - x1 + 1) * (y2 - y1 + 1); }

	// expand by one pixel each way to fix roundoffs
	void		Expand();

	// adds a point
	void		AddPoint(float x, float y);

	void		Intersect(const idScreenRect &rect);
	void		Union(const idScreenRect &rect);
	bool		Equals(const idScreenRect &rect) const;
};
