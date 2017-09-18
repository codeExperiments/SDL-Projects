#include <math.h>
#include "View.h"

#define PI 3.14159265358979323846f

void initView(View *view, ViewRect *viewRect, int fov)
{
	view->rect.left = viewRect->left;
	view->rect.right = viewRect->right;
	view->rect.top = viewRect->top;
	view->rect.bot = viewRect->bot;
	view->eyeRow = IntToReal((view->rect.top + view->rect.bot) / 2);
	int width = view->rect.right - view->rect.left + 1;
	view->fov = fov; // degrees
	view->distPP = IntToReal((int)((width / 2) / tanf(((float)fov * PI / 180)  / 2)));
}

void resetView(View *view) 
{
}

int getViewWidth(View *view)
{
	return view->rect.right - view->rect.left + 1;
}

int getViewFov(View *view)
{
	return view->fov;
}
