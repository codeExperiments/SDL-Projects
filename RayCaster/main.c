#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <SDL.h>
#include "Real.h"
#include "Angle.h"
#include "View.h"
#include "Player.h"
#include "Render.h"
#include "World.h"

#define PI 3.14159265358979323846f
#define _2PI (2*PI)

extern void buildCosSinTable();


int main(int arc, char* args[])
{
#ifdef USE_TRIG_LUT
	buildCosSinTable();
#endif

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("SDL!!!",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_MINIMIZED);
	SDL_Surface* backbuffer = SDL_GetWindowSurface(window);
	/*Game Code*/

	SDL_Surface* image = SDL_CreateRGBSurface(0, 320, 240, 8, 0,0,0,0);
	ViewRect rect = { 0, image->w-1, 0, image->h-1 };
	int fov = 60;
	Render r;
	initRender(&r, image, &rect, fov);
	int siz = sizeof(Cell);

	Player p;
	p.x = 64*2 + 32;
	p.y = 96;
	p.height = 32;
	p.angle = degreesToAngle(&r, 0);
	setPlayerRender(&r, &p);

	World w;
	int dim = 5;
	initWorld(&w, dim+1, dim+1);
	for (int i = 0; i != dim+1; ++i) {
		for (int j = 0; j != dim+1; ++j) {
			w.cells[i * w.cols + j].leftWall.state = NO_WALL;
			w.cells[i * w.cols + j].topWall.state = NO_WALL;
			w.cells[i * w.cols + j].leftWall.texID = w.cells[i * w.cols + j].topWall.texID = 0;
			w.cells[i * w.cols + j].flags = 0;
		}
	}

	for (int j = 0; j != dim; ++j) w.cells[0 * w.cols + j].topWall.state = WALL;
	for (int j = 0; j != dim; ++j) w.cells[dim * w.cols + j].topWall.state = WALL;
	for (int i = 0; i != dim; ++i) w.cells[i * w.cols + 0].leftWall.state = w.cells[i * w.cols + dim].leftWall.state = WALL;
	
	//w.cells[1 * w.cols + 3].leftWall.state = TRANSPARENT_WALL;
	//w.cells[1 * w.cols + 3].topWall.state = TRANSPARENT_WALL;

	//// vertical door[1] at (1,3)
	//w.cells[1 * w.cols + 3].flags |= CELL_HAS_DOOR;
	//w.cells[1 * w.cols + 3].leftWall.state = NEXT_DOOR;
	//w.cells[1 * w.cols + 4].leftWall.state = NEXT_DOOR;
	//w.doors[1].cellPos = 1 * w.cols + 3;
	//w.doors[1].colOffset = 40;
	//w.cells[1 * w.cols + 3].topWall.state = WALL;
	//w.cells[2 * w.cols + 3].topWall.state = WALL;

	//// vertical door[2] at (1,1)
	//w.cells[1 * w.cols + 1].flags |= CELL_HAS_DOOR;
	//w.cells[1 * w.cols + 1].leftWall.state = NEXT_DOOR;
	//w.cells[1 * w.cols + 2].leftWall.state = NEXT_DOOR;
	//w.doors[2].cellPos = 1 * w.cols + 1;
	//w.doors[2].colOffset = 10;
	//w.cells[1 * w.cols + 1].topWall.state = WALL;
	//w.cells[2 * w.cols + 1].topWall.state = WALL;

	//horizontal door[3] at (2, 2)
	w.cells[2 * w.cols + 2].flags |= CELL_HAS_DOOR;
	w.cells[2 * w.cols + 2].leftWall.state = WALL;
	w.cells[2 * w.cols + 3].leftWall.state = WALL;
	w.doors[3].cellPos = 2 * w.cols + 2;
	w.doors[3].colOffset = 40;
	w.cells[2 * w.cols + 2].topWall.state = NEXT_DOOR;
	w.cells[3 * w.cols + 2].topWall.state = NEXT_DOOR;


	setWorldRender(&r, &w);
	drawSceneRender(&r);

	//DWORD start = GetTickCount();
	//for (int i = 0; i != 10000; ++i) {
	//	//p.angle = incAngle(&r.impl->angleData, p.angle, 5);
	//	castRays(&r);
	//}
	//DWORD end = GetTickCount();
	//printf("%d\n", end - start);
	
	//SDL_Surface* image = SDL_CreateRGBSurface(0,320, 240, 8, 0,0,0,0);
	//Render r;
	//ViewRect rect = { 0, image->w-1, 0, image->h-1 };
	//initRender(&r, image, &rect, 60);
	//setFovRender(&r, 70);
	//rect.left = rect.top = 10;
	//rect.right = image->w - 10 - 1;
	//rect.bot = image->h - 10 - 1;
	//setViewRectRender(&r, &rect);

	SDL_DestroyWindow(window);
	SDL_Quit();

	//Real a, b, c;
	//a = FloatToReal(4.57f);
	//b = FloatToReal(7.88f);
	//c = a + b;
	//float r = RealToFloat(c);


	//int width = 320;
	//float fovd = 60.f;
	//AngleData *adata = createAngleData(width, fovd);
	//Angle rayAngle = getFirstRayAngle(adata, 0);
	//float sa = 330.f * _2PI / 360;
	//float inc = (fovd * _2PI / 360) / width;
	//for (int i = 0; i != width; ++i) {
	//	float ca = sa;
	//	float rca = RealToFloat(convertFromAngles(adata, rayAngle));
	//	float sca = sinf(ca);
	//	float cca = cosf(ca);
	//	float rsca = RealToFloat(getSin(adata, rayAngle));
	//	//float rcca = RealToFloat(getCos(adata, rayAngle));
	//	float rcca = RealToFloat(getColCos(adata, i));
	//	printf("%f, %f, %f\n", fabsf(ca - rca), fabsf(sca - rsca), fabsf(cca - rcca));
	//	rayAngle = incAngle(adata, rayAngle, 1);
	//	sa = sa + inc * 1;
	//}

	//const int JMAX = 1000;
	//const int IMAX = 100000;
	//int width = 320;
	//float fovd = 60.f;
	//AngleData *adata = createAngleData(width, fovd);
	//Angle rayAngle = 0;
	//Real smax = FloatToReal(-1.f);

	//DWORD start = GetTickCount();
	//for (int j = 0; j != JMAX; ++j) {
	//	for (int i = 0; i != IMAX; ++i) {
	//		Real sn = getSin(adata, rayAngle);
	//		Real cn = getCos(adata, rayAngle);
	//		if (sn > smax) smax = sn;
	//		if (cn > smax) smax = cn;
	//		rayAngle = incAngle(adata, rayAngle, 1);
	//	}
	//	//printf("%i\n", j);
	//}
	//DWORD end = GetTickCount();
	//printf("%f\n", RealToFloat(smax));
	//printf("%d\n", end - start);

	//system("pause");
	return EXIT_SUCCESS;
}

