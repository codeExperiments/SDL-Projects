#pragma once
#include <stdint.h>
#include "Angle.h"

typedef struct Player Player;

struct Player {
	int x, y;
	int height;
	Angle angle;
};
