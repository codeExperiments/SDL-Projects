#include <stdlib.h>
#include "World.h"



static void allocMem(World *w)
{
	w->cells = (Cell*)malloc(sizeof(Cell) * w->rows * w->cols);
	w->floors = (uint8_t*)malloc(sizeof(uint8_t) * w->rows * w->cols);
	w->ceilings = (uint8_t*)malloc(sizeof(uint8_t) * w->rows * w->cols);
	w->doors = (Door*)malloc(sizeof(Door) * w->numDoors);
}

static void freeMem(World *w)
{
	free(w->cells);
	free(w->floors);
	free(w->ceilings);
	free(w->doors);
}

void initWorld(World *w, int rows, int cols)
{
	w->rows = rows;
	w->cols = cols;
	w->numDoors = 10;
	w->activeDoorList = NULL;
	//w->ceilings ...
	allocMem(w);
	for (int i = 0; i != w->numDoors; ++i) {
		w->doors[i].cellPos = -1;
		w->doors[i].colOffset = 0;
		w->doors[i].speed = 0;
		w->doors[i].flags = 0;
		w->doors[i].flags |= DOOR_TYPE_SLIDE;
		w->doors[i].nextActive = NULL;
		//...
	}
}

void resetWorld(World * w)
{
	w->rows = w->cols = 0;
	freeMem(w);
}

