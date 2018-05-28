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

 #ifndef __TOVE_TRAJECTORY
 #define __TOVE_TRAJECTORY 1

#include "common.h"
#include "claimable.h"
#include "primitive.h"
#include "shader/curvedata.h"
#include "nsvg.h"
#include "utils.h"
#include "intersect.h"

class Trajectory : public Claimable<Path>, public std::enable_shared_from_this<Trajectory> {
private:
	struct Command {
		uint8_t type; // ToveCommandType
		bool dirty;
		uint16_t index; // point index
		int8_t direction; // cw vs. ccw

		union {
            LinePrimitive line;
			RectPrimitive rect;
			EllipsePrimitive ellipse;
		};
	};

    enum {
        DIRTY_BOUNDS = 1,
        DIRTY_COMMANDS = 2,
        DIRTY_COEFFICIENTS = 4,
        DIRTY_CURVE_BOUNDS = 8
    };

	mutable std::vector<Command> commands;
    mutable std::vector<CurveData> curves;
	mutable uint8_t dirty;

	float *addPoints(int n);

	inline void addPoint(float x, float y) {
		float *p = addPoints(1);
		p[0] = x;
		p[1] = y;
	}

	void setNumPoints(int npts);

	int addCommand(ToveCommandType type, int index);

	inline float *getMutablePoints(int index) const {
		return nsvg.pts + 2 * index;
	}

	inline void commit() const {
		if (dirty & DIRTY_COMMANDS) {
			updateCommands();
		}
	}

	void updateCommands() const;

    bool isLoop() const;

    inline void ensureCurveData(uint8_t flags) const {
        if (dirty & flags) {
            updateCurveData(flags);
        }
    }

    void updateCurveData(uint8_t flags) const;

public:
	NSVGpath nsvg;

	Trajectory();
	Trajectory(const NSVGpath *path);
	Trajectory(const TrajectoryRef &t);

	inline ~Trajectory() {
		free(nsvg.pts);
	}

	int moveTo(float x, float y);
	int lineTo(float x, float y);
	int curveTo(float cpx1, float cpy1, float cpx2, float cpy2, float x, float y);
	int arc(float x, float y, float r, float startAngle, float endAngle, bool counterclockwise);
	int drawRect(float x, float y, float w, float h, float rx, float ry);
	int drawEllipse(float cx, float cy, float rx, float ry);

	inline float getValue(int index) const {
		if (index >= 0 && index < nsvg.npts * 2) {
			commit();
			return nsvg.pts[index];
		} else {
			return 0.0;
		}
	}

	inline void setValue(int index, float value) {
		if (index >= 0 && index < nsvg.npts * 2) {
			commit();
			nsvg.pts[index] = value;
			changed(CHANGED_POINTS);
		}
	}

    /* for the point interface to love, we hide the last
       duplicated point on closed curves. */

    inline int getLoveNumPoints() const {
        const int n = getNumPoints();
        return isClosed() && n > 0 ? n - 1 : n;
    }

    float getLovePointValue(int index, int dim);
    void setLovePointValue(int index, int dim, float value);

	void setLovePoints(const float *pts, int npts);

	inline float getCommandPoint(const Command &command, int what) {
		const float *p = nsvg.pts + 2 * command.index;
		return p[command.direction * (what / 2) * 2 + (what & 1)];
	}

	void setCommandPoint(const Command &command, int what, float value);

	float getCommandValue(int commandIndex, int what);
	void setCommandValue(int commandIndex, int what, float value);
    void setCommandDirty(int commandIndex);

	void updateBounds();

	void transform(float sx, float sy, float tx, float ty);

	inline int getNumPoints() const {
		return nsvg.npts;
	}

	inline const float *getPoints() const {
		commit();
		return nsvg.pts;
	}

	inline bool isClosed() const {
		return nsvg.closed;
	}

	void setIsClosed(bool closed);

	inline int getNumCurves(bool countClose = true) const {
		const int npts = nsvg.npts;
		int n = ncurves(npts);

		if (countClose && n > 0) {
			// always count an additional "close" curve (might change due to mutable points).
			n += 1;
		}

		return n;
	}

	/*bool computeShaderCloseCurveData(
		ToveShaderGeometryData *shaderData,
		int target,
		ExtendedCurveData &extended);*/

	bool computeShaderCurveData(
		ToveShaderGeometryData *shaderData,
        int curveIndex,
		int target,
		ExCurveData &extended);

	void animate(const TrajectoryRef &a, const TrajectoryRef &b, float t);

	inline void setNext(const TrajectoryRef &trajectory) {
		nsvg.next = &trajectory->nsvg;
	}

	void updateNSVG();

	inline int fetchChanges() {
		// always emits a change; could be optimized. currently
		// only used in the curves renderer GeometryShaderLinkImpl.
		return CHANGED_POINTS;
	}

	void changed(ToveChangeFlags flags);

	inline void clearChanges(ToveChangeFlags flags) {
	}

	void invert();
	void clean(float eps = 0.0);

	ToveOrientation getOrientation() const;
	void setOrientation(ToveOrientation orientation);

    void testInside(float x, float y, AbstractInsideTest &test) const;
    void intersect(const AbstractRay &ray, Intersecter &intersecter) const;

    ToveVec2 getPosition(float globalt) const;
    ToveVec2 getNormal(float globalt) const;

    float closest(float x, float y, float dmin, float dmax) const;
};

#endif // __TOVE_TRAJECTORY
