#ifndef GORE_GEOMETRIKA_H_INCLUDED
#define GORE_GEOMETRIKA_H_INCLUDED

#include "gore_platform.h"
#include "gore_game_common.h"
#include "gore_input.h"
#include "gore_render_stack.h"

struct geometrika_state {
	b32 IsInitialized;

	game_camera Camera;
};

extern void GEOMKAUpdateAndRender(
	geometrika_state* State, 
	render_stack* RenderStack, 
	input_system* Input);

#endif