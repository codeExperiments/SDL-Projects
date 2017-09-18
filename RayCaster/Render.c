#include <stdio.h> // for printf...
#include <SDL.h>
#include "Render.h"

#define XSIDE 0
#define YSIDE 1

// Render Implementation 
typedef struct TexSlice TexSlice;
typedef struct Slice Slice;
typedef struct SliceMem SliceMem;
typedef struct SliceMemPtrs SliceMemPtrs;

struct TexSlice {
	int texID;
	int mipMapLevel;
	int light;
	int texCol;
	Real texColInc;
	Real texColOffset;
};

struct Slice {
	int top, bot;
	int side; // used only for walls
	Real dist;
	TexSlice tex;
	Slice *next;
};

struct RenderImpl
{
	View view;
	AngleData angleData;
	Slice *wallSlices;
	Slice **overlaySlicesLists; // for transparent walls and sprite slices
	SliceMemPtrs *sliceAllocs; // slice allocators for each column
};

/*
			Top:
			|			   +x
-----------------------------
			|+y

			|270
180			|				0
-----------------------------
			|
			|90
			*/


/****************************************************************************************/
// Memory managament for transparent walls/object slices

#define SLICE_BLOCK_SIZE 8
struct SliceMem
{
	Slice *block;
	SliceMem *next;
};

struct SliceMemPtrs
{
	SliceMem *sliceMemList;
	SliceMem **sliceMemListPtr;
	SliceMem **sliceBlock;
	int blockOffset;
};

void initSliceMemPtrs(SliceMemPtrs *p);
void allocateNewSlicesBlock(SliceMemPtrs *p);
Slice *allocateNewSlice(SliceMemPtrs *p);
void freeSlices(SliceMemPtrs *p);
void deleteSlicesMem(SliceMemPtrs *p);

static inline void initSliceMemPtrs(SliceMemPtrs *p)
{
	p->sliceMemList = NULL;
	p->sliceMemListPtr = &p->sliceMemList;
	p->sliceBlock = &p->sliceMemList;
	p->blockOffset = 0;
	allocateNewSlicesBlock(p);
}

static inline void allocateNewSlicesBlock(SliceMemPtrs *p)
{
	/* allocate new block of slices */
	SliceMem **ptrBlockPtr = p->sliceMemListPtr;
	*ptrBlockPtr = (SliceMem*)malloc(sizeof(SliceMem));
	(*ptrBlockPtr)->next = NULL;
	(*ptrBlockPtr)->block = (Slice*)malloc(sizeof(Slice) * SLICE_BLOCK_SIZE);
	p->sliceMemListPtr = &(*ptrBlockPtr)->next;
}

static inline Slice *allocateNewSlice(SliceMemPtrs *p)
{
	if (p->blockOffset == SLICE_BLOCK_SIZE)
	{
		p->sliceBlock = &(*p->sliceBlock)->next;
		if (*(p->sliceBlock) == NULL)
		{
			allocateNewSlicesBlock(p);
		}
		p->blockOffset = 0;
	}

	Slice *newSlice = &(*p->sliceBlock)->block[p->blockOffset++];
	return newSlice;
}

static inline void freeSlices(SliceMemPtrs *p)
{
	p->sliceBlock = &p->sliceMemList;
	p->blockOffset = 0;
}

static inline void deleteSlicesMem(SliceMemPtrs *p)
{
	freeSlices(p);

	SliceMem *blockPtr = p->sliceMemList;
	while (blockPtr)
	{
		SliceMem *tmp = blockPtr;
		blockPtr = blockPtr->next;
		free(tmp->block);
		free(tmp);
	}

	p->sliceMemList = NULL;
	p->sliceMemListPtr = &p->sliceMemList;
}

/****************************************************************************************/

// Render Private Functions
void initImpl(Render *r, ViewRect *viewRect, int fov);
void releaseImpl(Render *r);
void initViewData(Render * r, ViewRect *viewRect, int fov);
void releaseViewData(Render * r);
void initOverlaySlicesList(Render *r, int col);
void initTransparentSlice(const Render *render, const int col, const Real dist, const Real colCos,
	const Real dpp, const Real eyeRow, const Real plHeight, const ViewRect *rect,
	const int texCol, int texID, int side);
void initSlice(const Real dist, const Real colCos,
	const Real dpp, const Real eyeRow, const Real plHeight, const ViewRect *rect,
	const int texCol, int texID, int side, Slice *slice);
//Door *checkDoorInCell(const World *w, int cellPos);

static inline void initImpl(Render *r, ViewRect *viewRect, int fov)
{
	initViewData(r, viewRect, fov);
}

static inline void releaseImpl(Render *r)
{
	releaseViewData(r);
	// release other impl data...
	free(r->impl);
	r->impl = NULL;
}

static inline void initViewData(Render * r, ViewRect *viewRect, int fov)
{
	initView(&r->impl->view, viewRect, fov);
	int width = getViewWidth(&r->impl->view);
	initAngleData(&r->impl->angleData, width, fov);
	r->impl->wallSlices = (Slice*)malloc(sizeof(Slice) * width);
	r->impl->overlaySlicesLists = (Slice**)malloc(sizeof(Slice*) * width);
	r->impl->sliceAllocs = (SliceMemPtrs*)malloc(sizeof(SliceMemPtrs) * width);
	for (int i = 0; i != width; ++i) {
		initSliceMemPtrs(&r->impl->sliceAllocs[i]);
	}
}

static inline void releaseViewData(Render * r)
{
	free(r->impl->wallSlices);
	r->impl->wallSlices = NULL;
	free(r->impl->overlaySlicesLists);
	r->impl->overlaySlicesLists = NULL;
	// delete allocators memory
	int width = getViewWidth(&r->impl->view);
	for (int i = 0; i != width; ++i) {
		deleteSlicesMem(&r->impl->sliceAllocs[i]);
	}
	free(r->impl->sliceAllocs);
	r->impl->sliceAllocs = NULL;

	resetView(&r->impl->view);
	resetAngleData(&r->impl->angleData);
}

static inline void initOverlaySlicesList(Render *r, int col)
{
	RenderImpl *impl = r->impl;
	freeSlices(&impl->sliceAllocs[col]);
	impl->overlaySlicesLists[col] = NULL;
}

static inline void initTransparentSlice(const Render *render, const int col, const Real dist, const Real colCos,
	const Real dpp, const Real eyeRow, const Real plHeight, const ViewRect *rect,
	const int texCol, int texID, int side)
{
	Slice *slice = allocateNewSlice(&render->impl->sliceAllocs[col]);

	initSlice(dist, colCos, dpp, eyeRow, plHeight, rect, texCol, texID, side, slice);

	// insert the new slice at the front of the column overlay slices list
	Slice **slicePtr = &render->impl->overlaySlicesLists[col];
	slice->next = *slicePtr;
	*slicePtr = slice;
}


static inline void initSlice(const Real dist, const Real colCos,
	const Real dpp, const Real eyeRow, const Real plHeight, const ViewRect *rect,
	const int texCol, int texID, int side, Slice *slice)
{
	Real hitDist = mulReal(dist, colCos); // si dovrebbe moltiplicare anche per WALL_HEIGHT per ottenere il valore della distanza nella world map. Pero' non divido per semplificare una formula seguente, vedi sotto.

	Real dppOverD = divReal(dpp, hitDist); // dpp/d (d senza fattore WALL_HEIGHT)
	int yBot = RealToInt(eyeRow + mulReal(dppOverD, plHeight)); // yBot = vcp + dpp/d * playerHeight. (playerHeight e' pre-normalizzato sul WALL_HEIGHT)
	Real sliceHeight = dppOverD; // mulReal(dppOverD, IntToReal(WALL_HEIGHT)); // D avrebbe quindi WALL_HEIGHT che si semplifica con WALL_HEIGHT al num.
	int yTop = yBot - RealToInt(sliceHeight) + 1;

	Real texInc = divReal(IntToReal(WALL_HEIGHT), sliceHeight);
	Real texStart = IntToReal(0);

	if (yTop < rect->top) {
		texStart = mulReal(IntToReal(rect->top - yTop), texInc);
		yTop = rect->top;
	}

	if (yBot > rect->bot) {
		yBot = rect->bot;
	}

	slice->top = yTop;
	slice->bot = yBot;
	slice->dist = hitDist;
	slice->side = side;
	slice->tex.texID = texID;
	// slice->tex.light
	//slice->tex.mipMapLevel
	slice->tex.texCol = texCol; // col index [0,63]
	slice->tex.texColInc = texInc;
	slice->tex.texColOffset = texStart;
}

// Pre: Esiste una porta di posizione cellPos in w->doors. (test su cell.flags & CELL_HAS_DOOR)
static inline Door *findDoor(World *w, int cellPos)
{
	Door *d = w->doors;

	while (d->cellPos != cellPos) ++d;

	// move to front
	if (d != w->doors) {
		Door tmp = w->doors[0];
		w->doors[0] = *d;
		*d = tmp;
	}

	return &w->doors[0];
}

static void castRays(Render * r)
{
	Player *player = r->player;
	World *world = r->world;
	Cell *cells = world->cells;
	AngleData *angleData = &r->impl->angleData;
	Slice *wallSlices = r->impl->wallSlices;
	View *view = &r->impl->view;
	int width = getViewWidth(view);
	Angle rayAngle = getFirstRayAngle(angleData, player->angle);
	Real px = divReal(IntToReal(player->x), IntToReal(CELL_SIZE));
	Real py = divReal(IntToReal(player->y), IntToReal(CELL_SIZE));
	// integer position on grid
	int ipx = RealToInt(px);
	int ipy = RealToInt(py);
	int worldCols = world->cols;
	int ipyRowOffset = ipy * worldCols;
	Real playerHeightOverWallHeight = divReal(IntToReal(player->height), IntToReal(WALL_HEIGHT));  // player height normalized over wallheight

																								   //rayAngle = degreesToAngle(r, 90); // for evaluation purposes

	for (int i = width, col = 0; i != 0;
		--i, ++col, rayAngle = incAngle(angleData, rayAngle, 1))
	{
		// intersections distance from px,py
		Real xdist, ydist;
		// Note: xdist (distance from a x-intersection (|), ydist (distance from a y-intersection (--))

		// distance increments on the grid
		Real xDistInc, yDistInc;

		// start cell position on grid
		int posx = ipx;
		int posy = ipy;

		// steps on grid
		int stepx, stepy, stepRow;

		Real cosRay = getCos(angleData, rayAngle);
		Real icosRay = getInvCos(angleData, rayAngle);
		xDistInc = absReal(icosRay);

		xdist = mulReal(IntToReal(posx) + ((cosRay >= 0) ? IntToReal(1) : IntToReal(0)) - px, icosRay);
		stepx = (cosRay >= 0) ? 1 : -1;

		//// set starting x,ydist and stepx,y
		//if (cosRay >= 0) {
		//	//xdist = (posx + 1 - px) * invCosRayAngle;
		//	xdist = mulReal(IntToReal(posx) + IntToReal(1) - px, icosRay);
		//	stepx = 1;
		//}
		//else {
		//	//xdist = (px - posx) * invCosRayAngle;
		//	xdist = mulReal(IntToReal(posx) - px, icosRay);
		//	stepx = -1;
		//}

		Real sinRay = getSin(angleData, rayAngle);
		Real isinRay = getInvSin(angleData, rayAngle);
		yDistInc = absReal(isinRay);

		ydist = mulReal(IntToReal(posy) + ((sinRay >= 0) ? IntToReal(1) : IntToReal(0)) - py, isinRay);
		stepy = (sinRay >= 0) ? 1 : -1;
		stepRow = (sinRay >= 0) ? worldCols : -worldCols;

		//if (sinRay >= 0) {
		//	//ydist = (posy + 1 - py) * invSinRayAngle;
		//	ydist = mulReal(IntToReal(posy) + IntToReal(1) - py, isinRay);
		//	stepy = 1;
		//	stepRow = worldCols;
		//}
		//else {
		//	//ydist = (py - posy) * invSinRayAngle;
		//	ydist = mulReal(IntToReal(posy) - py, isinRay);
		//	stepy = -1;
		//	stepRow = -worldCols;
		//}

		Cell *cellPos = cells + ipyRowOffset + posx; // start cell offset

													 // offset to help with the (left,top) walls data
		int colInc = (stepx > 0) ? 1 : 0;
		int rowInc = (stepy > 0) ? 1 : 0;
		int rowPitchInc = ((stepy > 0) ? worldCols : 0);

		int wallHit = 0;

		// reset the column overlay slices list
		initOverlaySlicesList(r, col);

		// ray casting loop
		while (!wallHit)
		{
			if (xdist < ydist)
			{
				// vertical intersection (vertical x line)

				//int colCell = posx + (stepx > 0) ? 1 : 0;
				//Cell *cell = &world->cell[cellRowOffset + colCell];
				//int colInc = (stepx > 0) ? 1 : 0;
				//Cell *cell = &cells[cellPos + colInc];
				Cell *cell = cellPos + colInc;

				switch (cell->leftWall.state) {
				case NO_WALL:
					// advance ray
				no_xwall_label:
					cellPos += stepx;
					posx += stepx;
					xdist += xDistInc;
					break;

				case WALL:
				{
					// x tex offset computation
					//Real dx = IntToReal(posx) - px + IntToReal(colInc);
					//Real yint = py + mulReal(getTan(angleData, rayAngle), dx);
					//texColOfs = mulReal(IntToReal(stepx), yint - IntToReal(posy)) + IntToReal(1 - colInc); 
					// <--try to use also (yint * 64) & 64
					//if ((int)iLastX < xPlayer)
					//BitmapColumn = 63 - BitmapColumn;


					//Real dx = IntToReal(posx) - px + IntToReal(colInc);
					//Real dy1 = mulReal(xdist, sinRay);
					//Real dy2 = mulReal(getTan(angleData, rayAngle), dx); // py + dy

					// find texCol
					Real dy = mulReal(xdist, sinRay);
					Real yint = py + dy;
					Real ydiff = yint - IntToReal(posy); // [0,1)
					int texCol = RealToInt(mulReal(ydiff, IntToReal(CELL_SIZE))); // [0 top, 63 bot]
					if (stepx < 0) { // flip texCol if ray is left oriented
						texCol = 63 - texCol;
					}

					initSlice(xdist, getColCos(angleData, col), view->distPP, view->eyeRow, playerHeightOverWallHeight,
						&view->rect, texCol, cell->leftWall.texID, XSIDE, &wallSlices[col]);

					wallHit = 1;
				}
				break;

				case TRANSPARENT_WALL:
				{
					// find texCol
					Real dy = mulReal(xdist, sinRay);
					Real yint = py + dy;
					Real ydiff = yint - IntToReal(posy); // [0,1)
					int texCol = RealToInt(mulReal(ydiff, IntToReal(CELL_SIZE))); // [0 top, 63 bot]
					if (stepx < 0) { // flip texCol if ray is left oriented
						texCol = 63 - texCol;
					}

					initTransparentSlice(r, col, xdist, getColCos(angleData, col), view->distPP, view->eyeRow,
						playerHeightOverWallHeight, &view->rect, texCol, cell->leftWall.texID, XSIDE);

					goto no_xwall_label;
				}
				break;

				case NEXT_DOOR:
				{
					if (!(cell[colInc - 1].flags & CELL_HAS_DOOR)) // -1 if ray is left oriented
					{
						goto no_xwall_label;
					}

					Real doorDist = xdist + mulReal(xDistInc, FloatToReal(0.5f)); // fix distance

					Real ytop = IntToReal(posy);
					Real ybot = ytop + IntToReal(1);

					Real dy = mulReal(doorDist, sinRay);
					Real yint = py + dy;

					if (yint < ytop || yint >= ybot) // ray does not intersect door wall
					{
						goto no_xwall_label;
					}

					Real ydiff = yint - ytop;
					int texCol = RealToInt(mulReal(ydiff, IntToReal(CELL_SIZE))); // texCol in [0 top,63 bot]
																				  /*if (stepx < 0) { // don't flip texCol for doors
																				  texCol = 63 - texCol;
																				  }*/

					Door *door = findDoor(world, (int)(&cell[colInc - 1] - cells));
					// door != NULL here
					int j = door->colOffset;

					if (j != 0) // door is opening or closing
					{
						int j = door->colOffset; // j has to be > 0 since the door is active

						if (door->flags & DOOR_TYPE_SLIDE) {
							//if (stepx > 0) j = -j;
							texCol -= j;
							if (texCol < 0) // door not hit
							{
								goto no_xwall_label;
							}
						}

					}

					// door not moving or door is moving and it is hit

					initSlice(doorDist,
						getColCos(angleData, col), view->distPP, view->eyeRow, playerHeightOverWallHeight,
						&view->rect,
						texCol,
						door->texID,
						XSIDE,
						&wallSlices[col]);

					wallHit = 1;
				}
				break;

				} // end switch

			}
			else // ydist <= xdist
			{
				// horizontal intersection (horizontal y line)

				//int cellRow = cellRowOffset + (stepy > 0) ? worldCols : 0;
				//Cell *cell = &world->cell[cellRowOffset + posx];
				//int rowInc = (stepy > 0) ? 1 : 0;
				//Cell *cell = &cells[cellPos + rowOfsInc];
				Cell *cell = cellPos + rowPitchInc;

				switch (cell->topWall.state) {
				case NO_WALL:
					// advance ray
				no_ywall_label:
					cellPos += stepRow;
					posy += stepy;
					ydist += yDistInc;
					break;

				case WALL:
				{

					// y tex offset computation
					//Real dy = IntToReal(posy) - py + IntToReal(rowInc);
					//Real xint = px + mulReal(getInvTan(angleData, rayAngle), dy);
					//texColOfs = mulReal(IntToReal(stepy), IntToReal(posx) - xint) + IntToReal(rowInc);


					//Real dy = IntToReal(posy) - py + IntToReal(rowInc);
					//Real dx = mulReal(getInvTan(angleData, rayAngle), dy);
					//Real xint = px + dx; // px + dx

					// find texCol
					Real dx = mulReal(ydist, cosRay);
					Real xint = px + dx;
					Real xdiff = xint - IntToReal(posx); // [0,1)
					int texCol = RealToInt(mulReal(xdiff, IntToReal(CELL_SIZE))); // [0 left, 63 right]
					if (stepy > 0) { // flip texCol if ray is bottom oriented
						texCol = 63 - texCol;
					}

					initSlice(ydist, getColCos(angleData, col), view->distPP, view->eyeRow, playerHeightOverWallHeight,
						&view->rect, texCol, cell->topWall.texID, YSIDE, &wallSlices[col]);

					wallHit = 1;
				}
				break;

				case TRANSPARENT_WALL:
				{
					// find texCol
					Real dx = mulReal(ydist, cosRay);
					Real xint = px + dx;
					Real xdiff = xint - IntToReal(posx); // [0,1)
					int texCol = RealToInt(mulReal(xdiff, IntToReal(CELL_SIZE))); // [0 left, 63 right]
					if (stepy > 0) { // flip texCol if ray is bottom oriented
						texCol = 63 - texCol;
					}

					initTransparentSlice(r, col, ydist, getColCos(angleData, col), view->distPP, view->eyeRow,
						playerHeightOverWallHeight, &view->rect, texCol, cell->topWall.texID, YSIDE);

					goto no_ywall_label;
				}
				break;

				case NEXT_DOOR:
				{
					if (!(cell[rowPitchInc - worldCols].flags & CELL_HAS_DOOR)) // -worldCols if ray is top oriented
					{
						goto no_ywall_label;
					}

					// door not moving or door is moving and it is hit
					Real doorDist = ydist + mulReal(yDistInc, FloatToReal(0.5f)); // fix distance

					Real xLeft = IntToReal(posx);
					Real xRight = xLeft + IntToReal(1);

					Real dx = mulReal(doorDist, cosRay);
					Real xint = px + dx;

					if (xint < xLeft || xint >= xRight) // ray does not intersect door wall
					{
						goto no_ywall_label;
					}

					Real xdiff = xint - IntToReal(posx); // [0,1)
					int texCol = RealToInt(mulReal(xdiff, IntToReal(CELL_SIZE))); // [0 left, 63 right]
																				  //if (stepy > 0) { // don't flip door texCol if ray is bottom oriented
																				  //	texCol = 63 - texCol;
																				  //}

					Door *door = findDoor(world, (int)(&cell[rowPitchInc - worldCols] - cells));
					// door != NULL here
					int j = door->colOffset;

					if (j != 0) // door is opening or closing
					{
						if (door->flags & DOOR_TYPE_SLIDE) {
							//if (stepx > 0) j = -j;
							texCol -= j;
							if (texCol < 0) // door not hit
							{
								goto no_ywall_label;
							}
						}
					}

					initSlice(doorDist,
						getColCos(angleData, col), view->distPP, view->eyeRow, playerHeightOverWallHeight,
						&view->rect,
						texCol,
						door->texID,
						XSIDE,
						&wallSlices[col]);

					wallHit = 1;

				}
				break;

				} // end switch


			}
		}
		// end ray casting loop


	}
}

/****************************************************************************************/

// Render Public Functions

void initRender(Render *r, SDL_Surface *image, ViewRect *viewRect, int fov)
{
	r->image = image;
	r->impl = (RenderImpl*)malloc(sizeof(RenderImpl));
	initImpl(r, viewRect, fov);
}

void releaseRender(Render *r)
{
	releaseImpl(r);
}

void setPlayerRender(Render *r, Player *player)
{
	r->player = player;
}

void setWorldRender(Render * r, World * w)
{
	r->world = w;
}

void setViewRectRender(Render *r, ViewRect *viewRect)
{
	int fov = getViewFov(&r->impl->view);
	releaseViewData(r);
	initViewData(r, viewRect, fov);
}

void setFovRender(Render *r, int fov)
{
	ViewRect rect = r->impl->view.rect;
	releaseViewData(r);
	initViewData(r, &rect, fov);
}

Angle degreesToAngle(Render *r, float degrees) {
	Angle angle = convertFromDegrees(&r->impl->angleData, FloatToReal(degrees));
	Angle _2piAngle = getAngle2PI(&r->impl->angleData);
	while (angle < 0) {
		angle += _2piAngle;
	}
	while (angle >= _2piAngle) {
		angle -= _2piAngle;
	}
	return angle;
}

void drawSceneRender(Render * r)
{
	castRays(r);
}


