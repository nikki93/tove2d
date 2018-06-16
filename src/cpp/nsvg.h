/*
 * TÖVE - Animated vector graphics for LÖVE.
 * https://github.com/poke1024/tove2d
 *
 * Copyright (c) 2018, Bernhard Liebl
 *
 * Distributed under the MIT license. See LICENSE file for details.
 *
 * All rights reserved.
 */

#ifndef __TOVE_NSVG
#define __TOVE_NSVG 1

#include "common.h"

namespace nsvg {

uint32_t makeColor(float r, float g, float b, float a);
uint32_t applyOpacity(uint32_t color, float opacity);

int maxDashes();

void curveBounds(float *bounds, float *curve);

void xformInverse(float *a, float *b);
void xformIdentity(float *m);

float *pathArcTo(float *cpx, float *cpy, float *args, int &npts);
NSVGimage *parsePath(const char *d);

struct CachedPaint {
	char type;
	char spread;
	float xform[6];

	uint32_t * const colors;
	const int rowBytes;
	const int numColors;

	inline CachedPaint(void *colors, int rowBytes, int numColors) :
	 	colors(reinterpret_cast<uint32_t*>(colors)),
		rowBytes(rowBytes),
		numColors(numColors) {
	}

	void init(const NSVGpaint &paint, float opacity);
};

bool shapeStrokeBounds(float *bounds, const NSVGshape *shape,
	float scale, const ToveTesselationQuality *quality);

uint8_t *rasterize(NSVGimage *image, float tx, float ty, float scale,
	int width, int height, const ToveTesselationQuality *quality);

class Transform {
private:
	float matrix[6];
	bool identity;
	bool scaleLineWidth;

public:
	Transform();
	Transform(
		float tx, float ty,
		float r,
		float sx, float sy,
		float ox, float oy,
		float kx, float ky);

	void multiply(const Transform &t);

	void transformGradient(NSVGgradient* grad) const;
	void transformPoints(float *pts, const float *srcpts, int npts) const;

	float getScale() const;

	inline bool isIdentity() const {
		return identity;
	}

	inline float wantsScaleLineWidth() const {
		return scaleLineWidth;
	}
	inline void setWantsScaleLineWidth(bool scale) {
		scaleLineWidth = scale;
	}
};


}

#endif // __TOVE_NSVG
