#pragma once
#include <stdint.h>
#include "Real.h"

typedef struct ViewRect ViewRect;
typedef struct View View;
typedef struct View View;

struct ViewRect {
	int left, right;	// viewport area
	int top, bot;		// viewport area
};

struct View {
	ViewRect rect;
	Real distPP;		// distance to projection plane
	Real eyeRow;		// vert central row
	int fov;
};

// fov in degrees
void initView(View *, ViewRect *viewRect, int fov);
void resetView(View *);
int getViewWidth(View *);
int getViewFov(View *);
