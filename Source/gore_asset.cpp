#include "gore_asset.h"
#include "gore_voxshared.h"

#include <stdio.h>

struct asset_load_font_asset_work {

};

inline game_asset_tag* FindTagInAsset(game_asset* Asset, u32 TagType) {
	game_asset_tag* Result = 0;

	for (int TagIndex = 0;
		TagIndex < Asset->TagCount;
		TagIndex++)
	{
		game_asset_tag* Tag = Asset->Tags + TagIndex;
		if (Tag->Type == TagType) {
			Result = Tag;
			break;
		}
	}

	return(Result);
}

static game_asset* GetAssetByBestFloatTagInternal(asset_system* System, u32 GroupID, u32 TagType, float TagValue) {
	FUNCTION_TIMING();

	game_asset* Result = 0;

	game_asset_group* Group = &System->AssetGroups[GroupID];

	//NOTE(dima): Should be added with Group first asset index
	int BestMatchAssetIndex = 0;
	float BestMatchDiff = 1000000000.0f;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			float Diff = Tag->Value_Float - TagValue;
			//NOTE(dima): Getting absolute value of the diff
			if (Diff < 0.0f) {
				Diff = -Diff;
			}

			if (Diff < BestMatchDiff) {
				BestMatchAssetIndex = AssetIndex;
				BestMatchDiff = Diff;
			}
		}
	}

	int ResultAssetIndex = Group->FirstAssetIndex + BestMatchAssetIndex;
	Result = System->Assets + ResultAssetIndex;

	return(Result);
}

static game_asset* GetAssetByBestIntTagInternal(asset_system* System, u32 GroupID, u32 TagType, int TagValue) {
	FUNCTION_TIMING();

	game_asset* Result = 0;

	game_asset_group* Group = &System->AssetGroups[GroupID];

	//NOTE(dima): Should be added with Group first asset index
	int BestMatchAssetIndex = 0;
	int BestMatchDiff = 0x7FFFFFFF;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			int Diff = Tag->Value_Int - TagValue;
			//NOTE(dima): Getting absolute value of the diff
			if (Diff < 0) {
				Diff = -Diff;
			}

			if (Diff < BestMatchDiff) {
				BestMatchAssetIndex = AssetIndex;
				BestMatchDiff = Diff;
			}
		}
	}

	int ResultAssetIndex = Group->FirstAssetIndex + BestMatchAssetIndex;
	Result = System->Assets + ResultAssetIndex;

	return(Result);
}

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestFloatTagInternal(System, GroupID, TagType, TagValue);

	Assert(Asset->Type == AssetType);
	u32 Result = Asset->ID;

	return(Result);
}

u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestIntTagInternal(System, GroupID, TagType, TagValue);

	Assert(Asset->Type == AssetType);
	u32 Result = Asset->ID;

	return(Result);
}

u32 GetAssetByTag(asset_system* System, u32 GroupID, u32 TagType, u32 AssetType) {
	u32 ResultAssetIndex = 0;

	game_asset_group* Group = System->AssetGroups + GroupID;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			ResultAssetIndex = ExactAssetIndex;
			break;
		}
	}

	game_asset* ResultAsset = System->Assets + ResultAssetIndex;
	Assert(ResultAsset->Type == AssetType);

	return(ResultAssetIndex);
}

inline game_asset* GetFirstAssetInternal(asset_system* System, u32 GroupID) {
	u32 TargetAssetIndex = System->AssetGroups[GroupID].FirstAssetIndex;
	game_asset* Result = &System->Assets[TargetAssetIndex];

	return(Result);
}

u32 GetFirstAsset(asset_system* System, u32 GroupID, u32 AssetType) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType);

	u32 Result = FirstAsset->ID;

	return(Result);
}

bitmap_id GetFirstBitmap(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Bitmap);

	u32 Result = FirstAsset->ID;

	return(Result);
}

sound_id GetFirstSound(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Sound);

	u32 Result = FirstAsset->ID;

	return(Result);
}

font_id GetFirstFont(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Font);

	u32 Result = FirstAsset->ID;

	return(Result);
}

model_id GetFirstModel(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Model);

	u32 Result = FirstAsset->ID;

	return(Result);
}

mesh_id GetFirstMesh(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Mesh);

	u32 Result = FirstAsset->ID;

	return(Result);
}

static game_asset* AddGameAsset(asset_system* System) {
	game_asset* Result = System->Assets + System->AssetCount;

	Result->ID = System->AssetCount++;

	return(Result);
}

static void AssetAllocateBitmap(
	bitmap_info* Bitmap, 
	void* BitmapDataMemory, 
	int Width, int Height) 
{
	Bitmap->Width = Width;
	Bitmap->Height = Height;
	Bitmap->WidthOverHeight = (float)Width / (float)Height;
	Bitmap->Pitch = Width * 4;
	Bitmap->Pixels = (u8*)BitmapDataMemory;
	Bitmap->TextureHandle = 0;
}

void* AssetRequestMemory(asset_system* System, u32 MemorySize) {
	void* Result = malloc(MemorySize);

	return(Result);
}

void ASSETSInit(asset_system* System, stacked_memory* AssetSystemMemory) {

	System->AssetSystemMemory = AssetSystemMemory;
	System->TempMemoryForFileLoading = SplitStackedMemory(System->AssetSystemMemory, KILOBYTES(500));

	System->Assets[0] = {};
	System->AssetCount = 1;

	//NOTE(dima): INitializing groups
	for (int GroupIndex = 0; GroupIndex < GameAsset_Count; GroupIndex++) {
		game_asset_group* Group = System->AssetGroups + GroupIndex;

		Group->FirstAssetIndex = 0;
		Group->GroupAssetCount = 0;
	}
	 
	//NOTE(dima): integrating file groups to asset system
	platform_file_group FileGroup = PlatformApi.OpenAllFilesOfTypeBegin("../Data/", FileType_Asset);
	for (platform_file_entry* File = FileGroup.FirstFileEntry;
		File != 0;
		File = File->Next) 
	{
		stacked_memory TempMemory = BeginTempStackedMemory(
			&System->TempMemoryForFileLoading,
			System->TempMemoryForFileLoading.MaxSize);

		u32 CurrentOffset;
		u32 StartOffset = sizeof(asset_file_header);

		//asset_file_header* FileHeader = (asset_file_header*)File->Data;
		asset_file_header FileHeader;

		PlatformApi.ReadDataFromFileEntry(File, &FileHeader, 0, sizeof(asset_file_header));

		b32 HeaderIsEqual =
			FileHeader.AssetFileHeader[0] == 'G' &&
			FileHeader.AssetFileHeader[1] == 'A' &&
			FileHeader.AssetFileHeader[2] == 'S' &&
			FileHeader.AssetFileHeader[3] == 'S';

		Assert(HeaderIsEqual);
		Assert(FileHeader.Version >= ASSET_FILE_VERSION);
		Assert(FileHeader.AssetGroupsCount == GameAsset_Count);

		//NOTE(dima): Reading file asset groups
		game_asset_group FileGroups[GameAsset_Count];
		asset_file_asset_group BufferForReadFileGroups[GameAsset_Count];

		PlatformApi.ReadDataFromFileEntry(
			File, 
			BufferForReadFileGroups, 
			StartOffset, 
			sizeof(BufferForReadFileGroups));

		for (int GroupIndex = 0;
			GroupIndex < FileHeader.AssetGroupsCount;
			GroupIndex++)
		{
			game_asset_group* Grp = &FileGroups[GroupIndex];
			asset_file_asset_group* RdGroup = &BufferForReadFileGroups[GroupIndex];

			Grp->FirstAssetIndex = RdGroup->FirstAssetIndex;
			Grp->GroupAssetCount = RdGroup->GroupAssetCount;
		}
		StartOffset += sizeof(asset_file_asset_group) * FileHeader.AssetGroupsCount;

		//NOTE(dima): Reading file asset lines offsets
		u32 FileAssetCount = FileHeader.AssetsCount;
		u32* AssetLinesOffsets = PushArray(&TempMemory, u32, FileAssetCount);

		PlatformApi.ReadDataFromFileEntry(
			File,
			AssetLinesOffsets,
			FileHeader.LinesOffsetsByteOffset,
			FileAssetCount * sizeof(u32));

		gass_tag* HelperTags = PushArray(&TempMemory, gass_tag, 64);

		for (int FileGroupIndex = 0;
			FileGroupIndex < FileHeader.AssetGroupsCount;
			FileGroupIndex++)
		{
			game_asset_group* FileGroup = FileGroups + FileGroupIndex;

			for (int ToGroupIndex = 0;
				ToGroupIndex < GameAsset_Count;
				ToGroupIndex++)
			{
				game_asset_group* ToGroup = System->AssetGroups + ToGroupIndex;

				ToGroup->FirstAssetIndex = System->AssetCount;

				if (ToGroupIndex == FileGroupIndex) {
					for (int FileAssetIndex = 0;
						FileAssetIndex < FileAssetCount;
						FileAssetIndex++)
					{
						//gass_header* GASS = (gass_header*)((u8*)File->Data + AssetLinesOffsets[FileAssetIndex]);
						gass_header GASS_;
						PlatformApi.ReadDataFromFileEntry(
							File,
							&GASS_,
							AssetLinesOffsets[FileAssetIndex],
							sizeof(gass_header));

						gass_header* GASS = &GASS_;

						game_asset* Asset = AddGameAsset(System);

						Asset->Type = GASS->AssetType;

						//NOTE(dima): Incrementing target group asset count
						ToGroup->GroupAssetCount++;

						//NOTE(dima): Allocating memory for storing asset
						u32 AssetRequestMemorySize = GASS->AssetTotalDataSize + GASS->AssetTotalTagsSize;
						void* AssetNeededMemory = AssetRequestMemory(
							System, 
							AssetRequestMemorySize);

						stacked_memory AssetMem = InitStackedMemory(
							AssetNeededMemory, 
							AssetRequestMemorySize);

						//NOTE(dima): Reading asset tags
						Asset->Tags = 0;
						Asset->TagCount = GASS->TagCount;
						if (Asset->TagCount) {
							Asset->Tags = PushArray(&AssetMem, game_asset_tag, Asset->TagCount);

							u32 TagMemoryRequired = Asset->TagCount * sizeof(game_asset_tag);
							Assert(GASS->AssetTotalTagsSize == TagMemoryRequired);

							PlatformApi.ReadDataFromFileEntry(
								File,
								HelperTags,
								AssetLinesOffsets[FileAssetIndex] + GASS->LineFirstTagOffset,
								GASS->AssetTotalTagsSize);

							for (int TagIndex = 0;
								TagIndex < Asset->TagCount;
								TagIndex++)
							{
								gass_tag* ReadTag = HelperTags + TagIndex;
								game_asset_tag* ToTag = Asset->Tags + TagIndex;

								ToTag->Type = ReadTag->Type;
								ToTag->Value_Int = ReadTag->Value_Int;
							}
						}

#if 0
						switch (Asset->Type) {
							case AssetType_Bitmap: {
								Asset->Bitmap = PushStruct(????);

								bitmap_info* TargetBitmap = Asset->Bitmap;

								void* PixelsData = PushSomeMemory(&AssetMem, GASS->AssetTotalDataSize);

								AssetAllocateBitmap(
									TargetBitmap,
									AssetNeededMemory,
									GASS->Bitmap.Width, 
									GASS->Bitmap.Height);
							}break;

							case AssetType_Font: {
								Asset->Font = PushStruct(???);

								font_info* Font = Asset->Font;

								Font->AscenderHeight = GASS->Font.AscenderHeight;
								Font->DescenderHeight = GASS->Font.DescenderHeight;
								Font->LineGap = GASS->Font.LineGap;
								
							}break;

							case AssetType_FontGlyph: {
								Asset->Glyph = PushStruct(???);

								glyph_info* Glyph = Asset->Glyph;

								Glyph->Codepoint = GASS->Glyph.Codepoint;

								Glyph->Advance = GASS->Glyph.Advance;
								Glyph->XOffset = GASS->Glyph.XOffset;
								Glyph->YOffset = GASS->Glyph.YOffset;
								Glyph->LeftBearingX = GASS->Glyph.LeftBearingX;
								Glyph->Width = GASS->Glyph.BitmapWidth;
								Glyph->Height = GASS->Glyph.BitmapHeight;

								AssetAllocateBitmap(
									&Glyph->Bitmap,
									AssetNeededMemory,
									GASS->Glyph.BitmapWidth,
									GASS->Glyph.BitmapHeight);

								Glyph->AtlasMinUV = V2(GASS->Glyph.AtlasMinUV_x, GASS->Glyph.AtlasMinUV_y);
								Glyph->AtlasMaxUV = V2(GASS->Glyph.AtlasMaxUV_x, GASS->Glyph.AtlasMaxUV_y);

							}break;
						}
#endif
					}
				}
			}
		}

		EndTempStackedMemory(&System->TempMemoryForFileLoading, &TempMemory);
	}
	PlatformApi.OpenAllFilesOfTypeEnd(&FileGroup);
}
