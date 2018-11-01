#include "gore_engine.h"

engine_systems* EngineSystemsInit(input_system* InputSystem, game_settings* GameSettings) {
	ClearStackedMemory(&PlatformApi.EngineSystemsMemoryBlock);

	engine_systems* EngineSystems = (engine_systems*)PlatformApi.EngineSystemsMemoryBlock.BaseAddress;

	EngineSystems = PushStructUnaligned(&PlatformApi.EngineSystemsMemoryBlock, engine_systems);

	EngineSystems->SystemsStateMemory = InitStackedMemory(
		GetCurrentMemoryBase(&PlatformApi.EngineSystemsMemoryBlock),
		PlatformApi.EngineSystemsMemoryBlock.MaxSize - sizeof(engine_systems));

	stacked_memory* PermMem = &EngineSystems->SystemsStateMemory;

	EngineSystems->InputSystem = InputSystem;

	//NOTE(dima): Assets initialization
	EngineSystems->AssetSystem = PushStruct(PermMem, asset_system);
	EngineSystems->AssetSystemMemory = SplitStackedMemory(PermMem, MEGABYTES(50));
	ASSETSInit(EngineSystems->AssetSystem, &EngineSystems->AssetSystemMemory);

	//NOTE(dima): Render initialization
	EngineSystems->RenderState = PushStruct(PermMem, render_state);
	EngineSystems->RENDERMemory = SplitStackedMemory(PermMem, MEGABYTES(5));
	RenderInitState(
		EngineSystems->RenderState,
		&EngineSystems->RENDERMemory,
		InputSystem->WindowDim.x,
		InputSystem->WindowDim.y,
		EngineSystems->AssetSystem,
		EngineSystems->InputSystem);

	//NOTE(dima): Initialization of colors state
	EngineSystems->ColorsState = PushStruct(PermMem, color_state);
	EngineSystems->ColorsMemory = SplitStackedMemory(PermMem, KILOBYTES(20));
	InitColorsState(EngineSystems->ColorsState, &EngineSystems->ColorsMemory);

	//NOTE(dima): Initialization of game settings
	EngineSystems->GameSettings = GameSettings;

#if GORE_DEBUG_ENABLED
	//NOTE(dima): Initialization of debug state
	EngineSystems->DEBUGState = PushStruct(PermMem, debug_state);
	EngineSystems->DEBUGMem = SplitStackedMemory(PermMem, MEGABYTES(128));
	DEBUGInit(
		EngineSystems->DEBUGState, 
		&EngineSystems->DEBUGMem, 
		InputSystem, 
		EngineSystems->AssetSystem, 
		EngineSystems->RenderState);
#endif

	return(EngineSystems);
}
