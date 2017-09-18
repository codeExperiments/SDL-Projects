#pragma once
#include "Engine.h"
#include "Render.h"
#include "World.h"

#define DOOR_FULLY_OPEN 0xA0

struct EngineImpl
{
	World *world;
};

void checkDoors(Engine *engine);


void initEngine()
{

}

void checkDoors(Engine * engine)
{
	Door *door = engine->engineImpl->world->activeDoorList;
	Door *prevDoor = NULL;

	while (door) {

		door->colOffset += door->speed;

		if (door->speed < 0 && door->colOffset < 64)
		{

		}

		if (door->colOffset > DOOR_FULLY_OPEN)
		{
			door->speed = -door->speed;
			door->flags &= ~DOOR_OPENING;
			door->flags |= DOOR_CLOSING;
		}

		prevDoor = door;
		door = door->nextActive;
	}
}
