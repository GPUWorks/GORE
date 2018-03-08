#include "gore_debug.h"

inline void DEBUGDeallocateTreeNode(debug_state* State, debug_tree_node* Entry) {
	Entry->PrevBro->NextBro = Entry->NextBro;
	Entry->NextBro->PrevBro = Entry->PrevBro;

	Entry->NextBro = State->FreeBlockSentinel->NextBro;
	Entry->PrevBro = State->FreeBlockSentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline debug_tree_node* DEBUGAllocateTreeNode(debug_state* State) {
	debug_tree_node* Result = 0;

	if (State->FreeBlockSentinel->NextBro != State->FreeBlockSentinel) {
		Result = State->FreeBlockSentinel->NextBro;

		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->DebugMemory, debug_tree_node);
	}

	return(Result);
}

inline void DEBUGInsertToList(debug_tree_node* Sentinel, debug_tree_node* Entry) {
	Entry->NextBro = Sentinel->NextBro;
	Entry->PrevBro = Sentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline debug_tree_node* DEBUGAllocateSentinelTreeNode(debug_state* State) {
	debug_tree_node* Sentinel = DEBUGAllocateTreeNode(State);

	Sentinel->NextBro = Sentinel;
	Sentinel->PrevBro = Sentinel;

	return(Sentinel);
}

inline debug_statistic* DEBUGAllocateStatistic(debug_state* State) {
	debug_statistic* Result = 0;

	if (State->FreeStatisticSentinel->NextBro != State->FreeStatisticSentinel) {
		Result = State->FreeStatisticSentinel->NextBro;
		
		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->DebugMemory, debug_statistic);
	}

	return(Result);
}

inline void DEBUGDeallocateStatistic(debug_state* State, debug_statistic* Statistic) {
	Statistic->NextBro->PrevBro = Statistic->PrevBro;
	Statistic->PrevBro->NextBro = Statistic->NextBro;

	Statistic->PrevBro = State->FreeStatisticSentinel;
	Statistic->NextBro = State->FreeStatisticSentinel->NextBro;

	Statistic->PrevBro->NextBro = Statistic;
	Statistic->NextBro->PrevBro = Statistic;

	Statistic->NextInHash = 0;
}

inline debug_statistic* DEBUGAllocateSentinelStatistic(debug_state* State) {
	debug_statistic* Sentinel = DEBUGAllocateStatistic(State);

	Sentinel->NextBro = Sentinel;
	Sentinel->PrevBro = Sentinel;

	return(Sentinel);
}

inline debug_tree_node* DEBUGInitLayerTreeNode(debug_state* State, debug_tree_node* CurrentBlock, char* Name, u32 Type) {
	debug_tree_node* NewBlock = 0;

	u32 BlockHashID = StringHashFNV(Name);

	debug_tree_node* At = CurrentBlock->ChildrenSentinel->PrevBro;
	for (At; At != CurrentBlock->ChildrenSentinel; At = At->PrevBro) {
#if 0
		if (StringsAreEqual(Record->UniqueName, At->Name)) {
#else
		if (BlockHashID == At->ID) {
#endif
			NewBlock = At;
			break;
		}
	}

	if (NewBlock == 0) {
		//NOTE(dima): Debug node not exist. Should be allocated.
		NewBlock = DEBUGAllocateTreeNode(State);

		*NewBlock = {};
		NewBlock->TreeNodeType = Type;
		NewBlock->Parent = CurrentBlock;
		NewBlock->UniqueName = PushString(&State->DebugMemory, Name);
		CopyStrings(NewBlock->UniqueName, Name);
		NewBlock->ID = StringHashFNV(NewBlock->UniqueName);

		if (Type == DebugTreeNode_Section ||
			Type == DebugTreeNode_Timing) 
		{
			NewBlock->ChildrenSentinel = DEBUGAllocateSentinelTreeNode(State);
		}

		DEBUGInsertToList(CurrentBlock, NewBlock);
	}

	return(NewBlock);
}

inline debug_statistic* DEBUGInitDebugStatistic(
	debug_state* State, 
	debug_statistic** StatisticArray,
	debug_statistic* StatisticSentinel, 
	char* UniqueName) 
{
	debug_statistic* Statistic = 0;

	//NOTE(dima): Finding needed element in the hash table
	u32 RecordHash = StringHashFNV(UniqueName);
	u32 TimingStatisticIndex = RecordHash & (DEBUG_TIMING_STATISTICS_COUNT - 1);
	Statistic = StatisticArray[TimingStatisticIndex];

	debug_statistic* PrevInHash = 0;

	while (Statistic) {
		if (Statistic->ID == RecordHash) {
			break;
		}

		PrevInHash = Statistic;
		Statistic = Statistic->NextInHash;
	}

	//NOTE(dima): If element was not found in hash table
	if (Statistic == 0) {
		Statistic = DEBUGAllocateStatistic(State);

		Statistic->NextBro = StatisticSentinel->NextBro;
		Statistic->PrevBro = StatisticSentinel;

		Statistic->NextBro->PrevBro = Statistic;
		Statistic->PrevBro->NextBro = Statistic;
		Statistic->NextInHash = 0;

		if (PrevInHash) {
			PrevInHash->NextInHash = Statistic;
		}

		Statistic->ID = RecordHash;
		Statistic->Timing = {};
	}


	return(Statistic);
}

static void DEBUGFreeFrameInfo(debug_state* State, debug_profiled_frame* Frame) {
	debug_statistic* TimingAt = Frame->TimingStatisticSentinel->NextBro;
	while (TimingAt != Frame->TimingStatisticSentinel) {
		debug_statistic* TempNext = TimingAt->NextBro;
		
		DEBUGDeallocateStatistic(State, TimingAt);

		TimingAt = TempNext;
	}

	for (int TimingStatisticIndex = 0;
		TimingStatisticIndex < DEBUG_TIMING_STATISTICS_COUNT;
		TimingStatisticIndex++)
	{
		Frame->TimingStatistics[TimingStatisticIndex] = 0;
	}
}

void DEBUGInit(debug_state* State, gui_state* GUIState) {
	State->DebugMemory = AllocateStackedMemory(MEGABYTES(16));

	State->GUIState = GUIState;
	
	State->FreeBlockSentinel = PushStruct(&State->DebugMemory, debug_tree_node);
	State->FreeBlockSentinel->NextBro = State->FreeBlockSentinel;
	State->FreeBlockSentinel->PrevBro = State->FreeBlockSentinel;

	State->FreeStatisticSentinel = PushStruct(&State->DebugMemory, debug_statistic);
	State->FreeStatisticSentinel->NextBro = State->FreeStatisticSentinel;
	State->FreeStatisticSentinel->PrevBro = State->FreeStatisticSentinel;

	State->RootSection= DEBUGAllocateSentinelTreeNode(State);
	State->CurrentSection = State->RootSection;

	for (int FrameIndex = 0;
		FrameIndex < DEBUG_FRAMES_COUNT;
		FrameIndex++)
	{
		debug_profiled_frame* Frame = &State->Frames[FrameIndex];

		Frame->TimingSentinel = DEBUGAllocateSentinelTreeNode(State);
		Frame->CurrentTiming = Frame->TimingSentinel;

		Frame->TimingStatisticSentinel = DEBUGAllocateSentinelStatistic(State);

		Frame->SectionSentinel = DEBUGAllocateSentinelTreeNode(State);
		Frame->CurrentSection = Frame->SectionSentinel;

		DEBUGFreeFrameInfo(State, Frame);
	}
}

inline debug_profiled_frame* DEBUGGetCollationFrame(debug_state* State) {
	debug_profiled_frame* Frame = &State->Frames[State->CollationFrameIndex];

	return(Frame);
}

inline void DEBUGIncrementFrameIndices(debug_state* State) {
	State->CollationFrameIndex = (State->CollationFrameIndex + 1) % DEBUG_FRAMES_COUNT;
}

void DEBUGProcessRecords(debug_state* State) {
	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;

	for (u32 CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value];

		debug_profiled_frame* Frame = DEBUGGetCollationFrame(State);

		switch (Record->RecordType) {
			case DebugRecord_BeginTiming: {
				debug_tree_node* CurrentBlock = Frame->CurrentTiming;

				debug_tree_node* TimingNode = DEBUGInitLayerTreeNode(State, CurrentBlock, Record->UniqueName, DebugTreeNode_Timing);
				debug_timing_snapshot* TimingSnapshot = &TimingNode->TimingSnapshot;

				TimingSnapshot->BeginClock = Record->Clocks;
				TimingSnapshot->ThreadID = Record->ThreadID;
				TimingSnapshot->ChildrenSumClocks = 0;
				TimingSnapshot->HitCount = 0;
				TimingSnapshot->ClocksElapsed = 0;

				Frame->CurrentTiming = TimingNode;
			}break;

			case DebugRecord_EndTiming: {

				debug_tree_node* CurrentBlock = Frame->CurrentTiming;
				Assert(CurrentBlock->TreeNodeType == DebugTreeNode_Section);
				debug_timing_snapshot* TimingSnapshot = &CurrentBlock->TimingSnapshot;

				//NOTE(dima): difference between start and end clocks
				TimingSnapshot->ClocksElapsed = Record->Clocks - TimingSnapshot->BeginClock;

				//NOTE(dima): Going througn children and summing all their total clocks
				TimingSnapshot->ChildrenSumClocks = 0;
				debug_tree_node* At = CurrentBlock->ChildrenSentinel->NextBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->NextBro) {
					TimingSnapshot->ChildrenSumClocks += At->TimingSnapshot.ClocksElapsed;
				}

				//NOTE(dima): Finding needed element in the hash table
				debug_statistic* TimingStatistic = DEBUGInitDebugStatistic(
					State, 
					Frame->TimingStatistics, 
					Frame->TimingStatisticSentinel, 
					Record->UniqueName);

				TimingStatistic->Type = DebugStatistic_Timing;
				TimingStatistic->Timing.TotalClocks += TimingSnapshot->ClocksElapsed;
				TimingStatistic->Timing.HitCount += 1;
				TimingStatistic->Timing.TotalClocksInChildren = TimingSnapshot->ChildrenSumClocks;

				Frame->CurrentTiming = CurrentBlock->Parent;
			}break;

			case DebugRecord_BeginSection: {
				debug_tree_node* CurrentSection = Frame->CurrentSection;

				debug_tree_node* NewBlock = DEBUGInitLayerTreeNode(State, CurrentSection, Record->UniqueName, DebugTreeNode_Section);

				//NOTE(dima): Setting frame's current section to newly pushed block
				Frame->CurrentSection = NewBlock;
			}break;

			case DebugRecord_EndSection: {
				debug_tree_node* CurrentSection = Frame->CurrentSection;
				Assert(CurrentSection->TreeNodeType == DebugTreeNode_Section);

				//NOTE(dima): Setting frame's current section to this block's parent
				Frame->CurrentSection = CurrentSection->Parent;
			}break;

			case DebugRecord_Value: {

			} break;
		}
	}

	DEBUGIncrementFrameIndices(State);

	//TODO(dima): Think about when to do this
	int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
	SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
	SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);

	//NOTE(dima): Section pointer must be equal to initial
	Assert(State->CurrentSection == State->RootSection);
}

void DEBUGOutputSectionChildrenToGUI(debug_state* State, debug_tree_node* TreeNode) {
	debug_tree_node* At = TreeNode->ChildrenSentinel->PrevBro;
	
	for (At; At != TreeNode->ChildrenSentinel; At = At->PrevBro) {
		switch (At->TreeNodeType) {
			case DebugTreeNode_Section: {
				GUITreeBegin(State->GUIState, TreeNode->UniqueName);
				DEBUGOutputSectionChildrenToGUI(State, At);
				GUITreeEnd(State->GUIState);
			}break;

			case DebugTreeNode_Value: {

			}break;
		}
	}
}

inline void DEBUGPushFrameColumn(gui_state* GUIState, u32 FrameIndex, v2 InitP, v2 Dim, v4 Color) {
	rect2 ColumnRect = Rect2MinDim(V2(InitP.x + Dim.x * FrameIndex, InitP.y), Dim);

	RENDERPushRect(GUIState->RenderStack, ColumnRect, Color);
}

void DEBUGFramesSlider(debug_state* State) {
	gui_state* GUIState = State->GUIState;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "FrameSlider", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		v4 OutlineColor = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);

		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		v2 GraphMin = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);
		v2 GraphDim = V2((float)GUIState->ScreenWidth * 0.8f, AscByScale * 3);

		float OneColumnWidth = GraphDim.x / (float)DEBUG_FRAMES_COUNT;

		v2 ColumnDim = V2(OneColumnWidth, GraphDim.y);
		rect2 ColumnRect = Rect2MinDim(GraphMin, ColumnDim);

#if 0
		for (int ColumnIndex = 0;
			ColumnIndex < DEBUG_FRAMES_COUNT;
			ColumnIndex++)
		{

			RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, 123));

			ColumnRect.Min.x += OneColumnWidth;
			ColumnRect.Max.x += OneColumnWidth;
		}
#endif

		//NOTE(dima): Collation frame column
		DEBUGPushFrameColumn(GUIState, State->CollationFrameIndex, GraphMin, ColumnDim, GUIGetColor(GUIState, GUIColor_Orange));

		v2 BarDim = V2(1.0f, GraphDim.y);

		rect2 BarRect = Rect2MinDim(GraphMin + V2(OneColumnWidth, 0.0f), V2(1.0f, GraphDim.y));
		for (int BarIndex = 0;
			BarIndex < DEBUG_FRAMES_COUNT - 1;
			BarIndex++)
		{
			RENDERPushRect(GUIState->RenderStack, BarRect, OutlineColor);

			BarRect.Min.x += OneColumnWidth;
			BarRect.Max.x += OneColumnWidth;

		}

		rect2 GraphRect = Rect2MinDim(GraphMin, GraphDim);
		RENDERPushRectOutline(GUIState->RenderStack, GraphRect, 2, OutlineColor);

		GUIDescribeElement(GUIState, GraphDim, GraphMin);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void DEBUGFramesGraph(debug_state* State) {
	gui_state* GUIState = State->GUIState;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "ProfileFrameGraph", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		v4 OutlineColor = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);

		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		v2 GraphMin = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);
		v2 GraphDim = V2((float)GUIState->ScreenWidth * 0.8f, (float)GUIState->ScreenHeight * 0.15f);

		float OneColumnWidth = GraphDim.x / (float)DEBUG_FRAMES_COUNT;

		v2 ColumnDim = V2(OneColumnWidth, GraphDim.y);
		rect2 ColumnRect = Rect2MinDim(GraphMin, ColumnDim);

		for (int ColumnIndex = 0;
			ColumnIndex < DEBUG_FRAMES_COUNT;
			ColumnIndex++)
		{

			RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, 123));

			ColumnRect.Min.x += OneColumnWidth;
			ColumnRect.Max.x += OneColumnWidth;
		}

		v2 BarDim = V2(1.0f, GraphDim.y);

		rect2 BarRect = Rect2MinDim(GraphMin + V2(OneColumnWidth, 0.0f), V2(1.0f, GraphDim.y));
		for (int BarIndex = 0;
			BarIndex < DEBUG_FRAMES_COUNT - 1;
			BarIndex++)
		{
			RENDERPushRect(GUIState->RenderStack, BarRect, OutlineColor);

			BarRect.Min.x += OneColumnWidth;
			BarRect.Max.x += OneColumnWidth;

		}

		rect2 GraphRect = Rect2MinDim(GraphMin, GraphDim);
		RENDERPushRectOutline(GUIState->RenderStack, GraphRect, 3, OutlineColor);

#if 0
		gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, GraphDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(5, 5), &ResizeInteraction);
#endif

		GUIDescribeElement(GUIState, GraphDim, GraphMin);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}



void DEBUGOverlayToOutput(debug_state* State, render_stack* RenderStack) {
	GUIBeginFrame(State->GUIState, RenderStack);

	GUIBeginLayout(State->GUIState, "Root", GUILayout_Simple);
	DEBUGOutputSectionChildrenToGUI(State, State->RootSection);
	GUIEndLayout(State->GUIState, GUILayout_Simple);

	GUIEndFrame(State->GUIState);
}

void DEBUGUpdate(debug_state* State) {
	DEBUGProcessRecords(State);

	DEBUGOverlayToOutput(State, State->GUIState->RenderStack);
}