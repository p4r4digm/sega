#pragma once

#include "WorldView.h"

typedef struct MapEditor_t MapEditor;
typedef struct Frame_t Frame;

MapEditor *mapEditorCreate(WorldView *view);
void mapEditorDestroy(MapEditor *self);

void mapEditorInitialize(MapEditor *self);
void mapEditorSetEnabled(MapEditor *self, bool enabled);

void mapEditorRender(MapEditor *self, Frame *frame);

void mapEditorUpdateStats(MapEditor *self, Int2 mouseGridPos);