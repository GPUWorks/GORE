#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_types.h"
#include "gore_asset_identifiers.h"
#include "gore_game_settings.h"
#include "gore_file_formats.h"
#include "gore_asset_common.h"

enum game_asset_state {
	GameAssetState_Unloaded,

	GameAssetState_InProgress,
	GameAssetState_Loaded,
};

struct game_asset_group {
	u32 FirstAssetIndex;
	u32 GroupAssetCount;
};

struct game_asset_tag {
	u32 Type;

	union{
		float Value_Float;
		int Value_Int;
	};
};

enum asset_memory_entry_state {
	MemoryEntryState_None,
	MemoryEntryState_Used,
};

struct asset_memory_entry {
	asset_memory_entry* NextAllocEntry;
	asset_memory_entry* PrevAllocEntry;

	asset_memory_entry* NextMem;
	asset_memory_entry* PrevMem;

	void* Data;
	u32 DataSize;

	u32 State;
};

struct game_asset {
	u32 ID;

	u32 State;

	u32 Type;

	game_asset* Next;
	game_asset* Prev;

	game_asset_tag* Tags;
	int TagCount;

	u32 AssetDataOffsetInFile;
	char AssetFilePath[128];

	asset_memory_entry* DataMemoryEntry;

	union {
		bitmap_info Bitmap;
		font_info Font;
		glyph_info Glyph;
		sound_info Sound;
		model_info Model;
		mesh_info Mesh;
	};
};


struct game_asset_pool_block {
	game_asset_pool_block* NextBlock;
	game_asset_pool_block* PrevBlock;

	game_asset UseSentinel;
	game_asset FreeSentinel;

	u32 UseAssetsCount;

	u32 MaxBlockAssetCount;
	game_asset* BlockAssets;
};

#define TEMP_STORED_ASSET_COUNT 2048
struct asset_system {
	stacked_memory* AssetSystemMemory;

	stacked_memory TempMemoryForFileLoading;

	game_asset_pool_block AssetPoolBlockSentinel;
	asset_memory_entry FirstUseMemoryEntry;
	asset_memory_entry FirstFreeMemoryEntry;

	asset_memory_entry* SentinelAssetMem;

	u32 AssetCount;
	game_asset Assets[TEMP_STORED_ASSET_COUNT];
	game_asset_group AssetGroups[GameAsset_Count];
};

bitmap_id GetFirstBitmap(asset_system* System, u32 GroupID);
sound_id GetFirstSound(asset_system* System, u32 GroupID);
font_id GetFirstFont(asset_system* System, u32 GroupID);
model_id GetFirstModel(asset_system* System, u32 GroupID);
mesh_id GetFirstMesh(asset_system* System, u32 GroupID);

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType);
u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType);

void ASSETSInit(asset_system* System, stacked_memory* AssetSystemMemory);

inline game_asset* GetByAssetID(asset_system* System, u32 ID) {
	game_asset* Result = System->Assets + ID;

	return(Result);
}

/*
	NOTE(dima): Get asset from global asset structure
	with check if it has been loaded and with the check
	of the type of the asset
*/
inline game_asset* GetByAssetIDCheck(asset_system* System, u32 ID, u32 AssetType) {
	game_asset* Result = 0;

	game_asset* Asset = GetByAssetID(System, ID);
	if ((Asset->State == GameAssetState_Loaded) && 
		(Asset->Type == AssetType)) 
	{
		Result = Asset;
	}

	return(Result);
}

inline bitmap_info* GetBitmapFromID(asset_system* System, bitmap_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Bitmap);

	bitmap_info* Result = 0;
	if (Asset){
		Result = &Asset->Bitmap;
	}

	return(Result);
}

inline sound_info* GetSoundFromID(asset_system* System, sound_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Sound);

	sound_info* Result = 0;
	if (Asset) {
		Result = &Asset->Sound;
	}

	return(Result);
}

inline font_info* GetFontFromID(asset_system* System, font_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Font);

	font_info* Result = 0;
	if (Asset) {
		Result = &Asset->Font;
	}

	return(Result);
}

inline glyph_info* GetGlyphFromID(asset_system* System, font_glyph_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_FontGlyph);

	glyph_info* Result = 0;
	if (Asset) {
		Result = &Asset->Glyph;
	}

	return(Result);
}

inline mesh_info* GetMeshFromID(asset_system* System, mesh_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Mesh);

	mesh_info* Result = 0;
	if (Asset) {
		Result = &Asset->Mesh;
	}

	return(Result);
}

inline model_info* GetModelFromID(asset_system* System, model_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Model);

	model_info* Result = 0;
	if (Asset) {
		Result = &Asset->Model;
	}

	return(Result);
}
#endif