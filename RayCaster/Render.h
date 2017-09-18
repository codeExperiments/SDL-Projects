#pragma once
#include <SDL.h>
#include "Player.h"
#include "View.h"
#include "Angle.h"
#include "World.h"

typedef struct Render Render;
typedef struct RenderImpl RenderImpl;

struct Render {
	SDL_Surface *image;
	Player *player;
	World *world;
	RenderImpl *impl;
};

void initRender(Render *r, SDL_Surface *image, ViewRect *viewRect, int fov);
void releaseRender(Render *r);

void setPlayerRender(Render *r, Player *player);
void setWorldRender(Render *r, World *w);

void setViewRectRender(Render *r, ViewRect *viewRect);
void setFovRender(Render *r, int fov);
void drawSceneRender(Render *r);

Angle degreesToAngle(Render *r, float degrees);

