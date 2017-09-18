#pragma once
#include <stdint.h>

#define WALL_HEIGHT 64
#define CELL_SIZE (WALL_HEIGHT)

typedef struct Wall Wall;
typedef struct Cell Cell;
typedef struct World World;
typedef struct Door Door;

/*
Per come e' rappresentata la mappa (top,left per cella) occorre aggiungere una colonna e una riga extra a destra e in basso alla mappa
inizializzando correttamente anche i loro left (colonna a dx) e top (riga in basso) e il campo flags e altri eventuali campi.
Nota: il left e il top di quei muri deve essere WALL (e non deve essere cambiato nemmeno se c'e' una porta vicino, che non dovrebbe cmq 
esserci cosi vicino al bordo della mappa) (<- quindi NO porte adiacenti ai bordi dell'area A in figura.)
------------
|		 |x|
|	A	 |x|
|		 |x|
|		 |x|
------------
|x||x||x||x|
*/



/*
Nota2: quando si genera una mappa, per ogni porta si rispetteranno i seguenti vincoli sulla mappa:
-orizzontale, dovrai settare i left wall della sua cella e di quella a dx con il valore WALL. Mentre il top wall della cella della porta 
e di quella sotto avranno top a NEXT_DOOR.

		next_door
	--------------
	|             |
wall|_____________|wall
	|             |
	|-------------|
		next_door

-per le porte verticali, procederai in modo analogo: i top wall saranno a WALL, i left a NEXT_DOOR.

Per ogni porta, ricorda di settare la flag corrispondente della cella ke ne indica la presenza.
*/

// wall states
#define NO_WALL 0
#define WALL 1
#define TRANSPARENT_WALL 2
#define NEXT_PUSH_WALL 3
#define NEXT_DOOR 4

// cell flags
#define CELL_HAS_DOOR 0x1

// door flags
#define DOOR_OPENING	0x1			/* On if door is currently opening */
#define DOOR_CLOSING	0x2			/* On if door is currently closing */
#define DOOR_LOCKED	    0x4			/* On if door is locked */
#define DOOR_TYPE_SLIDE	    0x8		/* On if door is of type slide */
#define DOOR_TYPE_SPLIT	    0x10	/* On if door is of type split */
//#define DOOR_OPEN 0x20				/* On if the door is opened */

struct Door {
	uint16_t cellPos; // offset on the cell map
	uint8_t colOffset;   // if the colOffset field is non-zero the door is active and either opening or closing.
	uint8_t speed;
	uint8_t texID;
	uint8_t flags;	// bit flags
	Door *nextActive;
};

struct Wall
{
	uint8_t state;
	uint8_t texID;
};

struct Cell {
	Wall leftWall;
	Wall topWall;
	uint16_t flags;
};

struct World
{
	Cell *cells;
	Door *doors;
	uint8_t *floors;
	uint8_t *ceilings;
	Door *activeDoorList;
	int rows, cols;
	int numDoors;
};

void initWorld(World *w, int rows, int cols);
void resetWorld(World *w);

//static inline Cell *getCellOffset(World *w, int offset)
//{
//	return &w->cell[offset];
//}
//
//static inline Cell *getCell(World *w, int x, int y)
//{
//	return getCellOffset(w, x + y*w->cols);
//}

//static inline FloorCeil *getFloorCeilOffset(World *w, int offset)
//{
//	return &w->floorCeil[offset];
//}
//
//static inline FloorCeil *getFloorCeil(World *w, int x, int y) 
//{
//	return getFloorCeilOffset(w, x + y*w->cols);
//}