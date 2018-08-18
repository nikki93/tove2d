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

#ifndef __TOVE_GRAPHICS
#define __TOVE_GRAPHICS 1

#include "path.h"

BEGIN_TOVE_NAMESPACE

class AbstractMeshifier;

#ifdef NSVG_CLIP_PATHS
class Clip;
typedef std::shared_ptr<Clip> ClipRef;

class Clip : public PathOwner {
public:
	Clip(NSVGclipPath *path);
	Clip(const ClipRef &source, const nsvg::Transform &transform);

	inline void setNext(ClipRef clip) {
		nsvg.next = &clip->nsvg;
	}

	virtual void changed(ToveChangeFlags flags) {
	}

	void compute(const AbstractMeshifier &meshifier);

	NSVGclipPath nsvg;
	std::vector<PathRef> paths;
	ClipperLib::Paths computed;
};

class ClipSet { // clip sets are immutable.
private:
	std::vector<ClipRef> clips;
	void link();

public:
	ClipSet(const std::vector<ClipRef> &c) : clips(c) {
		link();
	}
	ClipSet(const ClipSet &source, const nsvg::Transform &t);

	NSVGclipPath *getHead() const {
		return clips.size() > 0 ? &clips[0]->nsvg : nullptr;
	}

	inline const ClipRef &get(int index) const {
		return clips.at(index);
	}

	const std::vector<ClipRef> &getClips() const {
		return clips;
	}
};

typedef std::shared_ptr<ClipSet> ClipSetRef;
#endif

class Graphics : public PathOwner {
private:
	std::vector<PathRef> paths;
#ifdef NSVG_CLIP_PATHS
	ClipSetRef clipSet;
#endif
	bool newPath;

	float bounds[4];
	float exactBounds[4];

	PaintRef fillColor;
	PaintRef strokeColor;

	float strokeWidth;
	std::vector<float> strokeDashes;
	float strokeDashOffset;
	int strokeLineJoin;
	int strokeLineCap;
	int miterLimit;
	int fillRule;

	ToveChangeFlags changes;

	inline const PathRef &current() const {
		return paths[paths.size() - 1];
	}

	void setNumPaths(int n);
	void _appendPath(const PathRef &path);

	PathRef beginPath();
	void closePath(bool closeCurves = false);

	void initialize(float width, float height);

	template<typename Get>
	void computeBounds(float *bounds, const Get &get) {
	    for (int i = 0; i < paths.size(); i++) {
	        const float *pathBounds = get(paths[i]);
	        if (i == 0) {
	            for (int j = 0; j < 4; j++) {
	                bounds[j] = pathBounds[j];
	            }
	        } else {
	            bounds[0] = std::min(bounds[0], pathBounds[0]);
	            bounds[1] = std::min(bounds[1], pathBounds[1]);
	            bounds[2] = std::max(bounds[2], pathBounds[2]);
	            bounds[3] = std::max(bounds[3], pathBounds[3]);
	        }
	    }
	}

public:
	NSVGimage nsvg;

	static GraphicsRef createFromSVG(
		const char *svg, const char *units, float dpi);

	Graphics();
	Graphics(const ClipSetRef &clipSet);
	Graphics(const NSVGimage *image);
	Graphics(const GraphicsRef &graphics);

	inline ~Graphics() {
		clear();
	}

	void clear();
	SubpathRef beginSubpath();
	void closeSubpath();
	void invertSubpath();

	inline void setFillColor(const PaintRef &color) {
		fillColor = color;
	}

	bool areColorsSolid() const;

	void setLineDash(const float *dashes, int count);

	inline void setLineDashOffset(float offset) {
		strokeDashOffset = offset;
	}

	inline void setLineWidth(float strokeWidth) {
 		this->strokeWidth = strokeWidth;
	}

	ToveLineJoin getLineJoin() const;
	void setLineJoin(ToveLineJoin join);

	inline void setMiterLimit(float limit) {
		this->miterLimit = limit;
	}

	inline void setLineColor(const PaintRef &color) {
		strokeColor = color;
	}

	void fill();
	void stroke();

	inline PathRef getCurrentPath() const {
		if (!paths.empty()) {
			return current();
		} else {
			return PathRef();
		}
	}

	void addPath(const PathRef &path);

	inline int getNumPaths() const {
		return paths.size();
	}

	inline PathRef getPath(int i) const {
		return paths.at(i);
	}

	PathRef getPathByName(const char *name) const;

	NSVGimage *getImage();

	const float *getBounds();
	const float *getExactBounds();

	void clean(float eps = 0.0);
	PathRef hit(float x, float y) const;

	void setOrientation(ToveOrientation orientation);

	void set(const GraphicsRef &source, const nsvg::Transform &transform);

	virtual void changed(ToveChangeFlags flags) {
		if (flags & (CHANGED_GEOMETRY | CHANGED_POINTS | CHANGED_BOUNDS)) {
			flags |= CHANGED_BOUNDS | CHANGED_EXACT_BOUNDS;
		}
		changes |= flags;
	}

	ToveChangeFlags fetchChanges(ToveChangeFlags flags, bool clearAll = false);
	void clearChanges(ToveChangeFlags flags);

	void animate(const GraphicsRef &a, const GraphicsRef &b, float t);

	void computeClipPaths(const AbstractMeshifier &meshifier) const;

#ifdef NSVG_CLIP_PATHS
	inline const ClipSetRef &getClipSet() const {
		return clipSet;
	}

	inline const ClipRef &getClipAtIndex(int index) const {
		return clipSet->get(index);
	}
#endif

	ToveMeshUpdateFlags tesselate(
		MeshRef mesh,
		float scale,
		const ToveTesselationQuality &quality,
		ToveHoles holes,
		ToveMeshUpdateFlags flags) const;

	void rasterize(
		uint8_t *pixels,
		int width, int height, int stride,
		float tx, float ty, float scale,
		const ToveTesselationQuality *quality);
};

END_TOVE_NAMESPACE

#endif // __TOVE_GRAPHICS
