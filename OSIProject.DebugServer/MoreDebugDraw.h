#pragma once

#include "LOMNAPI.h"

#if GAME_EDITION == BETA
extern bool* GcDebugOptions__sWireframe;
extern bool* gCollisionBoxes;
extern bool* gTriggerPlanes;
extern bool* gTriggerBoxes;
#elif GAME_EDITION == ALPHA
extern bool* GcDebugOptions__sWireframe;
extern bool* gCollisionBoxes;
#endif


void MDDInitialize();
void MDDShutdown();