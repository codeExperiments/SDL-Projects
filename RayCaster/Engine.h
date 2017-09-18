#pragma once

typedef struct Engine Engine;
typedef struct EngineImpl EngineImpl;

struct Engine {
	EngineImpl *engineImpl;
};

void initEngine();
