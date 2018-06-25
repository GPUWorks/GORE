#ifndef GORE_VOXMESH_H_INCLUDED
#define GORE_VOXMESH_H_INCLUDED

#include "gore_asset.h"
#include "gore_voxshared.h"
#include "gore_render_state.h"
#include <mutex>

enum voxel_normal_type_index{
	VoxelNormalIndex_Up,
	VoxelNormalIndex_Down,
	VoxelNormalIndex_Left,
	VoxelNormalIndex_Right,
	VoxelNormalIndex_Front,
	VoxelNormalIndex_Back,

	VoxelNormalIndex_Count,
};

enum voxel_texture_vert_type{
	VoxelTextureVertType_UpLeft,
	VoxelTextureVertType_UpRight,
	VoxelTextureVertType_DownRight,
	VoxelTextureVertType_DownLeft,
};

struct voxworld_table_entry {
	voxworld_table_entry* NextInHash;

	voxworld_table_entry* NextBro;
	voxworld_table_entry* PrevBro;

	u32 Key;

	voxel_chunk_info* ValueChunk;
};

struct voxworld_generation_state {
	/*
		NOTE(dima): Work threadworks are used to 
		store loaded chunks data.
	*/
	int FreeWorkThreadworksCount;
	int TotalWorkThreadworksCount;

	std::mutex WorkMutex;
	voxworld_threadwork* WorkUseSentinel;
	voxworld_threadwork* WorkFreeSentinel;

	/*
		NOTE(dima): Gen threadworks are used to 
		store temp information while generating chunks or meshes;
	*/
	int FreeGenThreadworksCount;
	int TotalGenThreadworksCount;

	std::mutex GenMutex;
	voxworld_threadwork* GenUseSentinel;
	voxworld_threadwork* GenFreeSentinel;

	int ChunksSideCount;
	int ChunksCount;
	int ChunksViewDistance;

	int ChunksPushedToRender;

	platform_atomic_type_i32 MeshGenerationsStartedThisFrame;
	
	stacked_memory* TotalMemory;

#define VOXWORLD_TABLE_SIZE 2048
	voxworld_table_entry* HashTable[VOXWORLD_TABLE_SIZE];
	int HashTableCollisionCount;
	int HashTableTotalInsertedEntries;

	voxworld_table_entry* FreeEntrySentinel;

	voxel_atlas_info* VoxelAtlas;

	voxel_generation_statistics DEBUGStat;
};

void GenerateTestChunk(voxel_chunk_info* Chunk);
void VoxmeshGenerate(voxel_mesh_info* Result, voxel_chunk_info* Chunk, voxel_atlas_info* Atlas);

void VoxelChunksGenerationInit(
	voxworld_generation_state* Generation,
	stacked_memory* Memory,
	int ChunksViewDistanceCount);

void VoxelChunksGenerationUpdate(
	voxworld_generation_state* Generation,
	render_state* RenderState,
	v3 CameraPos);

#endif