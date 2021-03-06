#pragma once

#include "segautils\Vector.h"
#include "segashared\Strings.h"

typedef struct GLWindow_T GLWindow;
typedef struct GLFWmonitor GLFWmonitor;
typedef struct Keyboard_t Keyboard;
typedef struct Mouse_t Mouse;

GLWindow *glWindowCreate(Int2 winSize, StringView windowName, GLFWmonitor *monitor);
void glWindowDestroy(GLWindow *self);

void glWindowBeginRendering(GLWindow *self);
void glWindowPollEvents(GLWindow *self);
void glWindowSwapBuffers(GLWindow *self);
int glWindowShouldClose(GLWindow *self);

Int2 glWindowGetSize(GLWindow *self);
Keyboard *glWindowGetKeyboard(GLWindow *self);
Mouse *glWindowGetMouse(GLWindow *self);

int getSegaKey(int GLFWKey);
int getSegaAction(int GLFWAction);
int getSegaMouseButton(int GLFWAction);