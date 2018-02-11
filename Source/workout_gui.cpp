#include "workout_gui.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

inline v4 GUIColor255(int R, int G, int B) {
	float OneOver255 = 1.0f / 255.0f;
	v4 Result = V4(R, G, B, 1.0f);
	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

inline int GUIIntFromHexChar(char C) {
	int Result = 0;

	if (C >= 'a' && C <= 'f') {
		C += 'A' - 'a';
	}

	if (C >= '0' && C <= '9') {
		Result = C - '0';
	}

	if (C >= 'A' && C <= 'F') {
		Result = C + 10 - 'A';
	}

	return(Result);
}

inline v4 GUIColorHex(char* Str) {
	float OneOver255 = 1.0f / 255.0f;
	
	v4 Result;

	Assert(Str[0] == '#');

	int R, G, B;
	R = GUIIntFromHexChar(Str[1]) * 16 + GUIIntFromHexChar(Str[2]);
	G = GUIIntFromHexChar(Str[3]) * 16 + GUIIntFromHexChar(Str[4]);
	B = GUIIntFromHexChar(Str[5]) * 16 + GUIIntFromHexChar(Str[6]);

	Result = V4(R, G, B, 1.0f);

	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

inline void GUIResetView(gui_view* View) {

}

void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height){
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->FontScale = 1.0f;

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

	GUIState->PlusMinusSymbol = 0;

	GUIState->GUIMem = AllocateStackedMemory(KILOBYTES(100));

#if 0
	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
#endif

	//NOTE(Dima): Initialization of the root node;
	GUIState->RootNode = PushStruct(&GUIState->GUIMem, gui_element);
	gui_element* RootNode = GUIState->RootNode;

	RootNode->PrevBro = RootNode;
	RootNode->NextBro = RootNode;

	CopyStrings(RootNode->Name, "MainRoot");
	CopyStrings(RootNode->Text, "MainRoot");

	RootNode->ID = GUIStringHashFNV(RootNode->Name);
	RootNode->Expanded = 1;
	RootNode->Depth = 0;

	RootNode->Parent = 0;
	RootNode->Type = GUIElement_None;

	RootNode->ChildrenSentinel = PushStruct(&GUIState->GUIMem, gui_element);

	RootNode->ChildrenSentinel->NextBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->PrevBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->Parent = RootNode;

	RootNode->NextBro = RootNode;
	RootNode->PrevBro = RootNode;

	RootNode->TempParent = 0;
	RootNode->TempParentTree = 0;

	GUIState->WalkaroundEnabled = false;
	GUIState->WalkaroundIsHot = false;
	GUIState->WalkaroundElement = RootNode;

	GUIState->CurrentNode = RootNode;
	GUIState->CurrentTreeParent = RootNode;

	/*
		NOTE(Dima): 
			Initialization of the "Free store" of the gui elements;
			Used for freeing and allocating static gui elements;
			Static gui elements has no cache and can be freed every frame;
	*/
	GUIState->FreeElementsSentinel = PushStruct(&GUIState->GUIMem, gui_element);
	GUIState->FreeElementsSentinel->NextBro = GUIState->FreeElementsSentinel;
	GUIState->FreeElementsSentinel->PrevBro = GUIState->FreeElementsSentinel;

	//NOTE(Dima): Initialization of view sentinel
	GUIState->ViewSentinel = PushStruct(&GUIState->GUIMem, gui_view);
	GUIState->ViewSentinel->NextBro = GUIState->ViewSentinel;
	GUIState->ViewSentinel->PrevBro = GUIState->ViewSentinel;

	//NOTE(Dima): Initialization of view free list sentinel element
	GUIState->FreeViewSentinel = PushStruct(&GUIState->GUIMem, gui_view);
	GUIState->FreeViewSentinel->NextBro = GUIState->FreeViewSentinel;
	GUIState->FreeViewSentinel->PrevBro = GUIState->FreeViewSentinel;

	//NOTE(DIMA): Allocating and initializing default view
	gui_view* DefaultView = PushStruct(&GUIState->GUIMem, gui_view);
	*DefaultView = {};
	DefaultView->NextBro = GUIState->ViewSentinel->NextBro;
	DefaultView->PrevBro = GUIState->ViewSentinel;

	DefaultView->NextBro->PrevBro = DefaultView;
	DefaultView->PrevBro->NextBro = DefaultView;

	DefaultView->ID = GUIStringHashFNV("DefaultView");
	CopyStrings(DefaultView->Name, "DefaultView");

	DefaultView->ViewType = GUIView_Tree;

	DefaultView->CurrentX = 0;
	DefaultView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;

	GUIState->DefaultView = DefaultView;
	GUIState->CurrentView = DefaultView;

	//NOTE(Dima): Initialization of the color table
	GUIState->ColorTable[GUIColor_Black] = V4(0.0f, 0.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_White] = V4(1.0f, 1.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Red] = V4(1.0f, 0.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Green] = V4(0.0f, 1.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Blue] = V4(0.0f, 0.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Yellow] = V4(1.0f, 1.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Magenta] = V4(1.0f, 0.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Cyan] = V4(0.0f, 1.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_PrettyBlue] = V4(0.0f, 0.5f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_PrettyGreen] = V4(0.5f, 1.0f, 0.0f, 1.0f);

	GUIState->ColorTable[GUIColor_Purple] = GUIColor255(85, 26, 139);
	GUIState->ColorTable[GUIColor_DarkRed] = GUIColorHex("#cd0000");
	GUIState->ColorTable[GUIColor_Orange] = GUIColorHex("#ffa500");
	GUIState->ColorTable[GUIColor_OrangeRed] = GUIColorHex("#ff4500");
	GUIState->ColorTable[GUIColor_RoyalBlue] = GUIColorHex("#436eee");
	GUIState->ColorTable[GUIColor_PrettyPink] = GUIColorHex("#ee30a7");

	GUIState->ColorTable[GUIColor_BluishGray] = GUIColorHex("#778899");

	GUIState->ColorTable[GUIColor_Burlywood] = GUIColorHex("#deb887");
	GUIState->ColorTable[GUIColor_DarkGoldenrod] = GUIColorHex("#b8860b");
	GUIState->ColorTable[GUIColor_OliveDrab] = GUIColorHex("#6b8e23");

	GUIState->ColorTable[GUIColor_Black_x20] = GUIColorHex("#202020");

	//NOTE(DIMA): Initialization of the color theme
	GUIState->ColorTheme = GUIDefaultColorTheme();
}

void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack) {
	GUIState->TempRenderStack = GUIState->RenderStack;
	GUIState->RenderStack = Stack;
}
 
void GUIEndTempRenderStack(gui_state* GUIState) {
	GUIState->RenderStack = GUIState->TempRenderStack;
}

void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack) {
	GUIState->RenderStack = RenderStack;
}

inline b32 GUIElementIsValidForWalkaround(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;

		if (Parent) {
			if (Parent->ChildrenSentinel != Element) {
				Result =
					(Element->Type != GUIElement_StaticItem) &&
					(Element->Type != GUIElement_Row) &&
					(Element->Type != GUIElement_None);
			}
		}
		else {
			//NOTE(DIMA): This is the root element
			Assert(Element->Type == GUIElement_None);
			Result = 0;
		}
	}

	return(Result);
}

inline b32 GUIElementIsSentinelOfRow(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;
		if (Element == Parent->ChildrenSentinel &&
			Parent->Type == GUIElement_Row) 
		{
			Result = 1;
		}
	}

	return(Result);
}

enum gui_walkaround_type {
	GUIWalkaround_None,

	GUIWalkaround_Next,
	GUIWalkaround_Prev,
};

inline gui_element* GUIWalkaroundStep(gui_element* Elem, u32 WalkaroundType) {
	gui_element* Result = 0;

	if (WalkaroundType == GUIWalkaround_Next) {
		Result = Elem->NextBro;
	}
	else if (WalkaroundType == GUIWalkaround_Prev){
		Result = Elem->PrevBro;
	}
	else {
		Assert(!"Invalid walkaround type");
	}

	return(Result);
}

inline gui_element* GUIFindNextForWalkaroundInRow(gui_element* Row, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Row->ChildrenSentinel, WalkaroundType);

	while (At != Row->ChildrenSentinel) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					return(Result);
				}
			}
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

inline gui_element* GUIFindForWalkaround(gui_element* Element, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Element, WalkaroundType);
	while (At != Element) {

		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
#if 1
			if (GUIElementIsSentinelOfRow(At)) {
				At = At->Parent;
				At = GUIWalkaroundStep(At, WalkaroundType);
				continue;
			}

			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					break;
				}
			}
#endif
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

static gui_element* GUIFindNextForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Next);

	return(Result);
}

static gui_element* GUIFindPrevForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Prev);

	return(Result);
}

inline gui_element* GUIFindElementForWalkaroundBFS(gui_element* CurrentElement) {
	gui_element* Result = 0;

	u32 WalkaroundType = GUIWalkaround_Prev;

	if (CurrentElement->ChildrenSentinel) {
		gui_element* At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);
		
		while (At != CurrentElement->ChildrenSentinel) {

			if (GUIElementIsValidForWalkaround(At)) {
				Result = At;
				return(Result);
			}
			else {
#if 1
				if (GUIElementIsSentinelOfRow(At)) {
					At = At->Parent;
					At = GUIWalkaroundStep(At, WalkaroundType);
					continue;
				}

				if (At->Type == GUIElement_Row) {
					//Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
					Result = GUIFindElementForWalkaroundBFS(At);
					if (Result) {
						return(Result);
					}
				}
#endif
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}

		At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);
		while (At != CurrentElement->ChildrenSentinel) {
			Result = GUIFindElementForWalkaroundBFS(At);
			if (Result) {
				return(Result);
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}
	}

	return(Result);
}

inline gui_element* GUIFindTrueParent(gui_element* Elem) {
	gui_element* Result = 0;

	gui_element* At = Elem->Parent;
	while (At) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}

		At = At->Parent;
	}

	if (!Result) {
		Result = Elem;
	}

	return(Result);
}

inline b32 GUIWalkaroundIsOnElement(gui_state* State, gui_element* Element) {
	b32 Result = 0;

	if (State->WalkaroundEnabled) {
		if (State->WalkaroundElement == Element) {
			Result = 1;
		}
	}

	return(Result);
}

inline b32 GUIWalkaroundIsHere(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	b32 Result = GUIWalkaroundIsOnElement(State, State->CurrentNode);

	return(Result);
}

void GUIEndFrame(gui_state* GUIState) {

	//NOTE(DIMA): Processing walkaround
	if (ButtonWentDown(GUIState->Input, KeyType_Backquote)) {
		GUIState->WalkaroundEnabled = !GUIState->WalkaroundEnabled;
	}

	if (GUIState->WalkaroundEnabled) {
		gui_element** Walk = &GUIState->WalkaroundElement;

		if (!GUIElementIsValidForWalkaround(*Walk)) {
			*Walk = GUIFindElementForWalkaroundBFS(*Walk);
		}

		if (!GUIState->WalkaroundIsHot) {

			if (ButtonWentDown(GUIState->Input, KeyType_Up)) {
				gui_element* PrevElement = GUIFindNextForWalkaround(*Walk);

				if (PrevElement) {
					*Walk = PrevElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Down)) {
				gui_element* NextElement = GUIFindPrevForWalkaround(*Walk);

				if (NextElement) {
					*Walk = NextElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Right)) {
				if ((*Walk)->Expanded) {
					gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);

					if (FirstChildren) {
						*Walk = FirstChildren;
					}
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Left)) {
				*Walk = GUIFindTrueParent(*Walk);
			}
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
			if ((*Walk)->Type == GUIElement_TreeNode) {
				(*Walk)->Expanded = !(*Walk)->Expanded;
			}

#if 0
			if ((*Walk)->Type == GUIElement_InteractibleItem) {
				GUIState->WalkaroundIsHot = !GUIState->WalkaroundIsHot;
			}
#endif
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Backspace)) {
			*Walk = GUIFindTrueParent(*Walk);
			if ((*Walk)->Expanded) {
				(*Walk)->Expanded = 0;
			}
			
			GUIState->WalkaroundIsHot = 0;
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Tab)) {
			if (!(*Walk)->Expanded) {
				(*Walk)->Expanded = true;
			}
			gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);
			if (FirstChildren) {
				*Walk = FirstChildren;
			}
		}
	}

	//NOTE(DIMA): Resetting default view;
	gui_view* DefView = GUIState->DefaultView;
	DefView->CurrentX = 0;
	DefView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;
}

inline gui_view* GUIAllocateViewElement(gui_state* State) {
	gui_view* View = 0;

	if (State->FreeViewSentinel->NextBro != State->FreeViewSentinel) {
		View = State->FreeViewSentinel->NextBro;

		View->NextBro->PrevBro = View->PrevBro;
		View->PrevBro->NextBro = View->NextBro;
	}
	else {
		View = PushStruct(&State->GUIMem, gui_view);
	}

	*View = {};

	return(View);
}

inline void GUIInsertViewElement(gui_state* State, gui_view* ToInsert) {
	ToInsert->NextBro = State->ViewSentinel->NextBro;
	ToInsert->PrevBro = State->ViewSentinel;

	ToInsert->NextBro->PrevBro = ToInsert;
	ToInsert->PrevBro->NextBro = ToInsert;
}

inline void GUIFreeViewElement(gui_state* State, gui_view* ToFree) {
	ToFree->NextBro->PrevBro = ToFree->PrevBro;
	ToFree->PrevBro->NextBro = ToFree->NextBro;

	ToFree->NextBro = State->FreeViewSentinel->NextBro;
	ToFree->PrevBro = State->FreeViewSentinel;

	ToFree->NextBro->PrevBro = ToFree;
	ToFree->PrevBro->NextBro = ToFree;
}

void GUIBeginView(gui_state* GUIState, char* ViewName, u32 ViewType) {
	/*
		NOTE(DIMA):
			Here I make unique id for view and calculate 
			it's hash. Then I try to find view in Global
			list with the same ID. It's perfomance better 
			than comparing two strings. But there is the 
			caveat. We need to make sure that the hash
			calculation function is crypto-strong enough
			so that the possibility to have 2 same id's 
			for different strings is very small.
	*/

	char IdBuf[256];
	stbsp_snprintf(
		IdBuf, sizeof(IdBuf),
		"%s_TreeID_%u",
		ViewName,
		GUITreeElementID(GUIState->CurrentNode));
	u32 IdBufHash = GUIStringHashFNV(IdBuf);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_View, IdBuf, 0);
	gui_element* CurrentElement = GUIGetCurrentElement(GUIState);
	gui_view* ParentView = GUIState->CurrentView;

	if (!CurrentElement->Cache.IsInitialized) {
		//IMPORTANT(DIMA): Think about how to choose position that we want
		CurrentElement->Cache.View.Position = V2(ParentView->CurrentX, ParentView->CurrentY);
		CurrentElement->Cache.View.Dimension = V2(100, 100);

		CurrentElement->Cache.IsInitialized = true;
	}
	v2* ViewPosition = &CurrentElement->Cache.View.Position;

	//NOTE(Dima): Find view in the existing list
	gui_view* View = 0;
	for (gui_view* At = GUIState->ViewSentinel->NextBro;
		At != GUIState->ViewSentinel;
		At = At->NextBro)
	{
		if (IdBufHash == At->ID) {
			View = At;
			break;
		}
	}

	//NOTE(Dima): View not found. Should allocate it
	if (View == 0) {
		View = GUIAllocateViewElement(GUIState);
		GUIInsertViewElement(GUIState, View);

		View->ID = IdBufHash;
		View->ViewType = ViewType;
		View->Parent = ParentView;
	}

	View->CurrentX = ViewPosition->x;
	View->CurrentY = ViewPosition->y;

	GUIState->CurrentView = View;

	if (ViewType == GUIView_Tree) {
		GUITreeBegin(GUIState, ViewName);
	}
	else if (ViewType == GUIView_Window) {

	}
}

void GUIEndView(gui_state* GUIState, u32 ViewType) {

	gui_view* View = GUIGetCurrentView(GUIState);

	Assert(View->ViewType == ViewType);

	//TODO(Dima): Remove it from here
	//GUIBeginRootBlock(State, "GUI");
	//
	//gui_interaction PlusMinusInteraction = GUIVariableInteraction(&State->PlusMinusSymbol, GUIVarType_B32);
	//GUIBoolButton(State, "PlusMinus", &PlusMinusInteraction);
	//
	//gui_interaction MemInteraction = GUIVariableInteraction(&State->GUIMem, GUIVarType_StackedMemory);
	//GUIStackedMemGraph(State, "GUI memory graph", &MemInteraction);
	//
	//gui_view* View = GUIGetCurrentView(State);
	//gui_interaction FontScaleInteraction = GUIVariableInteraction(&State->FontScale, GUIVarType_F32);
	//GUISlider(State, "Font scale", 0.5f, 1.5f, &FontScaleInteraction);
	//
	//gui_interaction WalkInteraction = GUIVariableInteraction(&State->WalkaroundEnabled, GUIVarType_B32);
	//GUIBoolButton(State, "Walkaround", &WalkInteraction);
	//
	//GUIEndRootBlock(State);

	if (ViewType == GUIView_Tree) {
		GUITreeEnd(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_View);

	GUIState->CurrentView = GUIState->CurrentView->Parent;
}

void GUIBeginRow(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	Assert(!View->RowBeginned);

	char NameBuf[16];
	stbsp_sprintf(NameBuf, "Row:%u", State->CurrentNode->RowCount);

	GUIBeginElement(State, GUIElement_Row, NameBuf, 0);

	View->RowBeginX = View->CurrentX;
	View->RowBeginned = true;
}

void GUIEndRow(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);
	
	Assert(View->RowBeginned);
	
	b32 NeedShow = GUIElementShouldBeUpdated(State->CurrentNode);

	if (NeedShow) {
		View->CurrentX = View->RowBeginX;
		View->CurrentY += View->RowBiggestHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
	}

	View->RowBeginned = false;
	View->RowBiggestHeight = 0;

	GUIEndElement(State, GUIElement_Row);

	State->CurrentNode->RowCount++;
}

inline void GUIPreAdvanceCursor(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	gui_element* Element = State->CurrentNode;

	View->CurrentPreAdvance = (Element->Depth - 1) * 2 * State->FontScale * State->FontInfo->AscenderHeight;
	View->CurrentX += View->CurrentPreAdvance;
}

inline void GUIDescribeElementDim(gui_state* State, v2 ElementDim) {
	gui_view* View = GUIGetCurrentView(State);

	View->LastElementWidth = ElementDim.x;
	View->LastElementHeight = ElementDim.y;

	if (ElementDim.y > View->RowBiggestHeight) {
		View->RowBiggestHeight = ElementDim.y;
	}
}

inline void GUIAdvanceCursor(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	if (View->RowBeginned) {
		View->CurrentX += View->LastElementWidth + State->FontInfo->AscenderHeight * State->FontScale;
	}
	else {
#if 0
		View->CurrentY += GetNextRowAdvance(State->FontInfo, 1.2f);
#else
		View->CurrentY += View->LastElementHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
#endif
	}
	
	View->CurrentX -= View->CurrentPreAdvance;
}

inline gui_element* GUIAllocateListElement(gui_state* State) {
	gui_element* Element = 0;

	if (State->FreeElementsSentinel->NextBro != State->FreeElementsSentinel) {
		Element = State->FreeElementsSentinel->NextBro;

		Element->NextBro->PrevBro = Element->PrevBro;
		Element->PrevBro->NextBro = Element->NextBro;
	}
	else {
		Element = PushStruct(&State->GUIMem, gui_element);
	}

	return(Element);
}

inline void GUIInsertListElement(gui_element* Sentinel, gui_element* ToInsert) {
	ToInsert->NextBro = Sentinel->NextBro;
	ToInsert->PrevBro = Sentinel;

	ToInsert->PrevBro->NextBro = ToInsert;
	ToInsert->NextBro->PrevBro = ToInsert;
}

inline void GUIFreeListElement(gui_state* State, gui_element* Element) {
	Element->NextBro->PrevBro = Element->PrevBro;
	Element->PrevBro->NextBro = Element->NextBro;

	Element->NextBro = State->FreeElementsSentinel->NextBro;
	Element->PrevBro = State->FreeElementsSentinel;

	Element->NextBro->PrevBro = Element;
	Element->PrevBro->NextBro = Element;
}

inline void GUIInitElementChildrenSentinel(gui_state* GUIState, gui_element* Element) {
	Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
	Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->Parent = Element;
}

static gui_element* GUIRequestElement(
	gui_state* GUIState, 
	u32 ElementType, 
	char* ElementName,
	gui_interaction* Interaction) 
{
	gui_element* Parent = GUIState->CurrentNode;

	gui_element* Element = 0;

	b32 ElementIsDynamic =
		(ElementType != GUIElement_None &&
		ElementType != GUIElement_StaticItem);

	u32 ElementHash = 0;
	if (ElementIsDynamic)
	{
		ElementHash = GUIStringHashFNV(ElementName);

		//NOTE(DIMA): Finding the element in the hierarchy
		for (gui_element* Node = Parent->ChildrenSentinel->NextBro;
			Node != Parent->ChildrenSentinel;
			Node = Node->NextBro)
		{
			//TODO(Dima): Test perfomance
#if 1
			if (StringsAreEqual(ElementName, Node->Name)) {
#else
			if (ElementHash == Node->ID) {
#endif
				Element = Node;
				break;
			}
		}
	}

	if (ElementType == GUIElement_StaticItem) {
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		Element->Expanded = 1;
		Element->Depth = Parent->Depth + 1;
		Element->RowCount = 0;
		Element->ID = 0;

		Element->ChildrenSentinel = 0;
	}

	//NOTE(Dima): Element not exist or not found. We should allocate it
	if (Element == 0) {
		//NOTE(DIMA): If the "Free store" of elements is not empty, get the memory from there
		//TODO(DIMA): Some elements memory might be initialzed again if we get it from here
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		//NOTE(Dima): Pre-Setting common values
		Element->TempParentTree = GUIState->CurrentTreeParent;

		if ((ElementType == GUIElement_TreeNode) ||
			(ElementType == GUIElement_CachedItem))
		{
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_Row) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;
			
			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_InteractibleItem) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			Element->ChildrenSentinel = 0;
		}

		if (ElementType == GUIElement_View) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		CopyStrings(Element->Name, ElementName);
		CopyStrings(Element->Text, ElementName);

		Element->ID = ElementHash;
		Element->RowCount = 0;
		Element->Cache = {};
	}

	//NOTE(Dima): Post-Setting common values
	Element->Type = ElementType;
	Element->Parent = Parent;
	Element->TempParent = 0;
	Element->RowCount = 0;

	//NOTE(Dima): Setting interaction ID for dynamic(cached) elements
	if (ElementIsDynamic && Interaction) {
		Interaction->ID = GUITreeElementID(Element);
	}

	return(Element);
}

b32 GUIBeginElement(
	gui_state* State, 
	u32 ElementType, 
	char* ElementName, 
	gui_interaction* ElementInteraction) 
{
	gui_element* Element = GUIRequestElement(State, ElementType, ElementName, ElementInteraction);

	State->CurrentNode = Element;

	if (Element->Type == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element;
	}

	b32 NeedShow = GUIElementShouldBeUpdated(State->CurrentNode);

	return(NeedShow);
}

void GUIEndElement(gui_state* State, u32 ElementType) {
	gui_element* Element = State->CurrentNode;

	Assert(ElementType == Element->Type);

#if 1
	gui_view* View = GUIGetCurrentView(State);

	if (GUIElementShouldBeUpdated(Element)) {
		//NOTE(Dima): Here I remember view Y for exit sliding effect
		gui_element* CurrentTreeParent = State->CurrentTreeParent;
		Assert((CurrentTreeParent->Type == GUIElement_TreeNode) ||
			(CurrentTreeParent == State->RootNode));
		CurrentTreeParent->Cache.TreeNode.StackY = View->CurrentY;
	}
#endif

	if (ElementType == GUIElement_StaticItem) {
		GUIFreeListElement(State, Element);
	}

	if (ElementType == GUIElement_Row) {
		//View->CurrentNode->Parent->RowCount = 0;
	}

	if (ElementType == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element->TempParentTree;
	}


	State->CurrentNode = Element->Parent;
}

enum print_text_type {
	PrintTextType_PrintText,
	PrintTextType_GetTextSize,
};

static rect2 PrintTextInternal(gui_state* State, u32 Type, char* Text, float Px, float Py, float Scale, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 TextRect = {};

	float CurrentX = Px;
	float CurrentY = Py;

	char* At = Text;

	font_info* FontInfo = State->FontInfo;
	render_stack* Stack = State->RenderStack;

	while (*At) {
		int GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		glyph_info* Glyph = &FontInfo->Glyphs[GlyphIndex];

		float BitmapScale = Glyph->Height * Scale;

		if (Type == PrintTextType_PrintText) {
			float BitmapMinX;
			float BitmapMinY;
			
			BitmapMinY = CurrentY + (Glyph->YOffset - 1.0f) * Scale;
			BitmapMinX = CurrentX + (Glyph->XOffset - 1.0f) * Scale;

			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX + 2, BitmapMinY + 2 }, BitmapScale, V4(0.0f, 0.0f, 0.0f, 1.0f));
			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX, BitmapMinY }, BitmapScale, Color);
		}

		float Kerning = 0.0f;
		if (*(At + 1)) {
			Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
		}

		CurrentX += (Glyph->Advance + Kerning) * Scale;

		++At;
	}

	TextRect.Min.x = Px;
	TextRect.Min.y = Py - FontInfo->AscenderHeight * Scale;
	TextRect.Max.x = CurrentX;
	TextRect.Max.y = Py - FontInfo->DescenderHeight * Scale;

	return(TextRect);
}

void GUILabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, 1.0f, GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
}

void GUIAnchor(gui_state* GUIState, char* Name, v2 Pos, v2 Dim, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		rect2 WorkRect = Rect2MinDim(Pos, Dim);
		v4 WorkColor = GUIState->ColorTable[GUIState->ColorTheme.OutlineColor];
		v2 MouseP = GUIState->Input->MouseP;

		gui_element* Anchor = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Anchor->Cache;

		if (!Cache->IsInitialized) {
			Cache->Anchor.OffsetInAnchor = {};

			Cache->IsInitialized = true;
		}
		v2* OffsetInAnchor = &Cache->Anchor.OffsetInAnchor;


		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);
		if (MouseInRect(GUIState->Input, WorkRect)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				IsHot = GUISetInteractionHot(GUIState, Interaction, true);
				*OffsetInAnchor = MouseP - Pos;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left)) {
			IsHot = GUISetInteractionHot(GUIState, Interaction, false);
			*OffsetInAnchor = {};
		}

		if (IsHot) {

			v2 WorkRectP = Interaction->ResizeContext.Position;
			/*Getting true position*/
			MouseP = MouseP - *OffsetInAnchor;

			float MinDimLen = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
			
			Assert((Interaction->Type == GUIInteraction_MoveInteraction) ||
				(Interaction->Type == GUIInteraction_ResizeInteraction));

			if (Interaction->Type == GUIInteraction_ResizeInteraction) {

				v2* WorkDim = Interaction->ResizeContext.DimensionPtr;
				switch (Interaction->ResizeContext.Type) {
					case GUIResizeInteraction_Default: {
						*WorkDim = MouseP - WorkRectP;

						if (MouseP.x - WorkRectP.x < MinDimLen) {
							WorkDim->x = MinDimLen;
						}

						if (MouseP.y - WorkRectP.y < MinDimLen) {
							WorkDim->y = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Horizontal: {
						if (MouseP.x - WorkRectP.x < MinDimLen) {
							WorkDim->x = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Vertical: {
						if (MouseP.y - WorkRectP.y < MinDimLen) {
							WorkDim->y = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Proportional: {
						float WidthToHeight = WorkDim->x / WorkDim->y;
						WorkDim->y = MouseP.y - WorkRectP.y;
						WorkDim->x = WorkDim->y * WidthToHeight;

						if (WorkDim->y < MinDimLen) {
							WorkDim->y = MinDimLen;
							WorkDim->x = WorkDim->y * WidthToHeight;
						}
					}break;
				}
			}
			else if (Interaction->Type == GUIInteraction_MoveInteraction) {
				v2* WorkP = Interaction->MoveContext.MovePosition;
				switch (Interaction->MoveContext.Type) {
					case GUIMoveInteraction_Move: {
						*WorkP = MouseP;
					}break;
				}
			}
		}


		PushRect(GUIState->RenderStack, WorkRect, WorkColor);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIWindow(gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0);

	if (NeedShow) {
		GUIPreAdvanceCursor(GUIState);

		gui_view* View = GUIGetCurrentView(GUIState);

		gui_element* Window = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Window->Cache;

		if (!Cache->IsInitialized) {
			if (CreationFlags & GUIWindow_DefaultSize) {
				Cache->View.Dimension.x = 100;
				Cache->View.Dimension.y = 100;
			}
			else {
				Cache->View.Dimension.x = Width;
				Cache->View.Dimension.y = Height;
			}

			Cache->View.Position = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

			Cache->IsInitialized = true;
		}

		v2* WindowDimension = &Cache->View.Dimension;
		v2* WindowPosition = &Cache->View.Position;

		int WindowOutlineWidth = 3;
		int InnerSubWindowWidth = 2;
		rect2 WindowRect = Rect2MinDim(*WindowPosition, *WindowDimension);

		PushRect(GUIState->RenderStack, WindowRect, GUIState->ColorTable[GUIState->ColorTheme.WindowBackgroundColor]);
		PushRectOutline(GUIState->RenderStack, WindowRect, WindowOutlineWidth, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		if (CreationFlags & GUIWindow_TopBar) {
			float TopBarHeight = GUIState->FontScale * GetNextRowAdvance(GUIState->FontInfo);

			if (WindowDimension->y > TopBarHeight) {
				PushRect(
					GUIState->RenderStack, 
					Rect2MinDim(
						*WindowPosition + V2(0, TopBarHeight), 
						V2(WindowDimension->x, InnerSubWindowWidth)), 
					GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);
			}
		}

		if (CreationFlags & GUIWindow_Resizable) {
			gui_interaction ResizeInteraction = GUIResizeInteraction(WindowRect.Min, WindowDimension, GUIResizeInteraction_Default);
			GUIAnchor(GUIState, "AnchorBR", WindowRect.Max, V2(5, 5), &ResizeInteraction);
		}

		gui_interaction MoveInteraction = GUIMoveInteraction(WindowPosition, GUIMoveInteraction_Move);
		GUIAnchor(GUIState, "AnchorMV", WindowRect.Min, V2(5, 5), &MoveInteraction);

		GUIDescribeElementDim(GUIState, *WindowDimension);
		GUIAdvanceCursor(GUIState);
	}
	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIImageView(gui_state* GUIState, char* Name, gui_interaction* Interaction) {
	GUITreeBegin(GUIState, Name);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0);

	if (NeedShow) {
		rgba_buffer* Buffer = Interaction->VariableLink.Value_RGBABuffer;

		gui_element* ImageView = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &ImageView->Cache;

		if (!Cache->IsInitialized) {
			/*
			Cache->ImageView.Dimension = 
				V2(Interaction->VariableLink.Value_RGBABuffer->Width,
				Interaction->VariableLink.Value_RGBABuffer->Height);
			*/

			Cache->ImageView.Dimension = V2(
				(float)Interaction->VariableLink.Value_RGBABuffer->Width / 
				(float)Interaction->VariableLink.Value_RGBABuffer->Height * 100, 100);
			Cache->IsInitialized = true;
		}
		v2* WorkDim = &Cache->ImageView.Dimension;

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 ImageRect;
		ImageRect.Min = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);
		ImageRect.Max = ImageRect.Min + *WorkDim;

		PushBitmap(GUIState->RenderStack, Buffer, ImageRect.Min, GetRectHeight(ImageRect));
		PushRectOutline(GUIState->RenderStack, ImageRect, 3, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		gui_interaction ResizeInteraction = GUIResizeInteraction(ImageRect.Min, WorkDim, GUIResizeInteraction_Proportional);
		GUIAnchor(GUIState, "Anchor0", ImageRect.Max, V2(5, 5), &ResizeInteraction);

		GUIDescribeElementDim(GUIState, GetRectDim(ImageRect));
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);

	GUITreeEnd(GUIState);
}

void GUIStackedMemGraph(gui_state* GUIState, char* Name, gui_interaction* Interaction) {
	GUITreeBegin(GUIState, Name);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, Interaction);

	Assert(Interaction->VariableLink.Type == GUIVarType_StackedMemory);

	if (NeedShow) {
		stacked_memory* WorkMem = Interaction->VariableLink.Value_StackedMemory;

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		gui_element* StackedMem = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &StackedMem->Cache;

		if (!Cache->IsInitialized) {

			Cache->StackedMem.Dimension =
				V2(GUIState->FontInfo->AscenderHeight * 40.0f, 
				GUIState->FontInfo->AscenderHeight * 3.0f);

			Cache->IsInitialized = true;
		}

		v2* WorkDim = &Cache->StackedMem.Dimension;

		rect2 GraphRect;
		GraphRect.Min.x = View->CurrentX;
		GraphRect.Min.y = View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		GraphRect.Max = GraphRect.Min + *WorkDim;

		v2 GraphRectDim = GetRectDim(GraphRect);

		u64 OccupiedCount = WorkMem->Used;
		u64 FreeCount = (u64)WorkMem->MaxSize - OccupiedCount;
		u64 TotalCount = WorkMem->MaxSize;
		
		float OccupiedPercentage = (float)OccupiedCount / (float)TotalCount;

		rect2 OccupiedRect = Rect2MinDim(GraphRect.Min, V2(GraphRectDim.x * OccupiedPercentage, GraphRectDim.y));
		rect2 FreeRect = Rect2MinDim(
			V2(OccupiedRect.Max.x, OccupiedRect.Min.y), 
			V2(GraphRectDim.x * (1.0f - OccupiedPercentage), GraphRectDim.y));

		float Inner = 2.0f;
		float Outer = 3.0f;

		PushRect(GUIState->RenderStack, OccupiedRect, GUIState->ColorTable[GUIState->ColorTheme.FirstColor]);
		PushRectOutline(GUIState->RenderStack, OccupiedRect, Inner, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);
		PushRect(GUIState->RenderStack, FreeRect, GUIState->ColorTable[GUIState->ColorTheme.SecondaryColor]);
		PushRectOutline(GUIState->RenderStack, FreeRect, Inner, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		PushRectOutline(GUIState->RenderStack, GraphRect, Outer, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		if (MouseInRect(GUIState->Input, GraphRect)) {
			char InfoStr[128];
			stbsp_sprintf(
				InfoStr, 
				"Occupied: %llu(%.2f%%); Total: %llu",
				OccupiedCount,
				(float)OccupiedCount / (float)TotalCount * 100.0f,
				TotalCount);

			GUILabel(GUIState, InfoStr, GUIState->Input->MouseP);
		}

		gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, WorkDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(5, 5), &ResizeInteraction);

		GUIDescribeElementDim(
			GUIState,
			V2(GraphRectDim.x + Outer,
				GraphRectDim.y + (2.0f * Outer)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
	GUITreeEnd(GUIState);
}

void GUIText(gui_state* GUIState, char* Text) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(
			GUIState, 
			PrintTextType_PrintText, 
			Text, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale, GUIState->ColorTable[GUIState->ColorTheme.TextColor]);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, GetRectDim(Rc));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

inline void GUIActionTextAction(gui_interaction* Interaction) {
	if (Interaction) {
		if (Interaction->Type == GUIInteraction_VariableLink) {
			if(Interaction->VariableLink.Type = GUIVarType_B32)
			{
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}
	}
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, GUIState->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextColor];
		if (MouseInRect(GUIState->Input, Rc)) {
			TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIActionTextAction(Interaction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, Rc , 2, GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor]);

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIActionTextAction(Interaction);
			}
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, GetRectDim(Rc));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			ButtonName,
			View->CurrentX,
			View->CurrentY,
			GUIState->FontScale,
			GUIState->ColorTable[GUIState->ColorTheme.TextColor]);

		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float OutlineWidth = 1;
		v4 ButRectHighlight = GUIState->ColorTable[GUIState->ColorTheme.OutlineColor];
		v4 TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextColor];

		if (MouseInRect(GUIState->Input, NameRc)) {
			TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, NameRc, 2, GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor]);

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PushRect(GUIState->RenderStack, NameRc, GUIState->ColorTable[GUIState->ColorTheme.FirstColor]);
		PushRectOutline(GUIState->RenderStack, NameRc, OutlineWidth, ButRectHighlight);

		PrintTextInternal(GUIState, PrintTextType_PrintText, ButtonName, View->CurrentX, View->CurrentY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, V2(NameRc.Max.x - View->CurrentX, NameRc.Max.y - NameRc.Min.y + OutlineWidth));
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0);

	if (NeedShow) {

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(
			GUIState, 
			PrintTextType_PrintText, 
			ButtonName, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale, 
			GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float PrintButY = View->CurrentY;

		rect2 FalseRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "false", PrintButX, PrintButY, GUIState->FontScale);
		v2 FalseDim = GetRectDim(FalseRc);
		rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), FalseDim);

		float OutlineWidth = 1;

		char TextToPrint[8];
		if (*Interaction->VariableLink.Value_B32) {
			rect2 TrueRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "true", PrintButX, PrintButY, GUIState->FontScale);
			v2 TrueDim = GetRectDim(TrueRc);
			PrintButX = FalseRc.Min.x + (FalseDim.x - TrueDim.x) * 0.5f;

			stbsp_sprintf(TextToPrint, "%s", "true");
		}
		else {
			stbsp_sprintf(TextToPrint, "%s", "false");
		}

		v4 ButRectHighlight = GUIState->ColorTable[GUIState->ColorTheme.OutlineColor];
		v4 TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextColor];
		if (MouseInRect(GUIState->Input, ButRc)) {
			TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, NameRc, 2, GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor]);

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PushRect(GUIState->RenderStack, ButRc, GUIState->ColorTable[GUIState->ColorTheme.FirstColor]);
		PushRectOutline(GUIState->RenderStack, ButRc, OutlineWidth, ButRectHighlight);

		PrintTextInternal(GUIState, PrintTextType_PrintText, TextToPrint, PrintButX, PrintButY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, V2(FalseRc.Max.x - View->CurrentX, FalseRc.Max.y - FalseRc.Min.y + OutlineWidth));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(State, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(State);
		gui_vertical_slider_cache* Cache = &State->CurrentNode->Cache.VerticalSlider;
		float FontScale = State->FontScale;
		float SmallTextScale = FontScale * 0.8f;
		float NextRowAdvanceFull = GetNextRowAdvance(State->FontInfo, FontScale);
		float NextRowAdvanceSmall = GetNextRowAdvance(State->FontInfo, SmallTextScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull, NextRowAdvanceFull * 5);

		GUIPreAdvanceCursor(State);

		float* Value = Interaction->VariableLink.Value_F32;
		Assert(Max > Min);
		
		//NOTE(DIMA): Calculating Max text rectangle
		//TODO(DIMA): Cache theese calculations
		char MaxValueTxt[16];
		stbsp_sprintf(MaxValueTxt, "%.2f", Max);
		v2 MaxValueRcMin = V2(View->CurrentX, View->CurrentY);
		rect2 MaxValueRcSize = PrintTextInternal(
			State,
			PrintTextType_GetTextSize,
			MaxValueTxt,
			0, 0,
			SmallTextScale);
		v2 MaxValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Calculating vertical rectangle
		v2 WorkRectMin = V2(View->CurrentX, MaxValueRcMin.y - State->FontInfo->DescenderHeight * SmallTextScale);
		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);

		//NOTE(DIMA): Drawing vertical rectangle
		i32 RectOutlineWidth = 1;

		PushRect(State->RenderStack, WorkRect, State->ColorTable[State->ColorTheme.FirstColor]);
		PushRectOutline(State->RenderStack, WorkRect, RectOutlineWidth, State->ColorTable[State->ColorTheme.OutlineColor]);

		//NOTE(DIMA): Calculating Min text rectangle
		//TODO(DIMA): Cache theese calculations
		char MinValueTxt[16];
		stbsp_sprintf(MinValueTxt, "%.2f", Min);
		v2 MinValueRcMin = V2(WorkRect.Min.x, WorkRect.Max.y + NextRowAdvanceSmall);
		rect2 MinValueRcSize = PrintTextInternal(
			State,
			PrintTextType_GetTextSize,
			MinValueTxt,
			0, 0,
			SmallTextScale);
		v2 MinValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Drawing Max value text
		rect2 MaxValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MaxValueTxt,
			MaxValueRcMin.x + 0.5f * (WorkRectDim.x - MaxValueRcDim.x), 
			MaxValueRcMin.y,
			SmallTextScale,
			State->ColorTable[State->ColorTheme.TextColor]);

		//NOTE(DIMA): Printing Min value text
		rect2 MinValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MinValueTxt,
			MinValueRcMin.x + 0.5f * (WorkRectDim.x - MinValueRcDim.x),
			MinValueRcMin.y,
			SmallTextScale,
			State->ColorTable[State->ColorTheme.TextColor]);

		//NOTE(DIMA): Printing name of the element that consist from first 3 chars of the name
		char SmallTextToPrint[8];
		stbsp_sprintf(SmallTextToPrint, "%.4s", Name);
		char *SmallBufAt = SmallTextToPrint;
		while (*SmallBufAt) {
			char CurChar = *SmallBufAt;

			if (CurChar >= 'a' && CurChar <= 'z') {
				*SmallBufAt = CurChar - 'a' + 'A';
			}

			SmallBufAt++;
		}
		rect2 SmallTextRect = PrintTextInternal(State, PrintTextType_GetTextSize, SmallTextToPrint, 0, 0, SmallTextScale);
		v2 SmallTextRcDim = GetRectDim(SmallTextRect);

		float SmallTextX = WorkRect.Min.x + WorkRectDim.x * 0.5f - SmallTextRcDim.x * 0.5f;
		float SmallTextY = MinValueRc.Min.y + MinValueRcDim.y + State->FontInfo->AscenderHeight * SmallTextScale;

		rect2 SmallTextRc = PrintTextInternal(
			State, 
			PrintTextType_PrintText, 
			SmallTextToPrint, 
			SmallTextX, 
			SmallTextY, 
			SmallTextScale, 
			State->ColorTable[State->ColorTheme.TextColor]);

		//NOTE(Dima): Processing the value
		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = 1.0f - (((float)(*Value) - Min) / (float)Range);

		float CursorWidth = WorkRectDim.x;
		float CursorHeight = CursorWidth * 0.66f;

		float CursorX = WorkRectMin.x - (CursorWidth - WorkRectDim.x) * 0.5f;
		float CursorY = WorkRectMin.y + (WorkRectDim.y - CursorHeight) * RelativePos01;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);

		float MaxWidth = Max(Max(Max(WorkRectDim.x, MaxValueRcDim.x), MinValueRcDim.x), SmallTextRcDim.x);


		//NOTE(DIMA): Processing interactions
		v4 CursorColor = State->ColorTable[State->ColorTheme.SecondaryColor];
		b32 IsHot = GUIInteractionIsHot(State, Interaction);
		b32 MouseInWorkRect = MouseInRect(State->Input, WorkRect);
		b32 MouseInCursRect = MouseInRect(State->Input, CursorRect);
		if (MouseInWorkRect || MouseInCursRect) {
			if (MouseButtonWentDown(State->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(State, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseInCursRect && !IsHot) {
			char ValStr[64];
			stbsp_sprintf(ValStr, "%.3f", *Value);

			GUILabel(State, ValStr, State->Input->MouseP);
		}

		if (MouseButtonWentUp(State->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(State, Interaction, false);
			IsHot = false;
		}

		if (GUIWalkaroundIsHere(State)) {

			if (ButtonWentDown(State->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(State);
			}

			if (GUIWalkaroundIsHot(State)) {
				PushRectOutline(State->RenderStack, WorkRect, 2, State->ColorTable[State->ColorTheme.WalkaroundHotColor]);

				CursorColor = State->ColorTable[State->ColorTheme.TextHighlightColor];

				float ChangeStep = 0.02f * Range;
				if (ButtonIsDown(State->Input, KeyType_Down)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(State->Input, KeyType_Up)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				char ValStr[16];
				stbsp_sprintf(ValStr, "%.3f", *Value);
				GUILabel(State, ValStr, CursorRect.Max);
			}
			else {
				//PushRectOutline(State->RenderStack, Rect2MinDim(MaxValueRcMin, V2(MaxWidth, SmallTextRc.Max.y - MaxValueRc.Min.y)));
				PushRectOutline(State->RenderStack, SmallTextRc, 2, State->ColorTable[State->ColorTheme.TextHighlightColor]);
				PushRectOutline(State->RenderStack, WorkRect, 2, State->ColorTable[State->ColorTheme.TextHighlightColor]);
			}
		}

		if (IsHot) {
			CursorColor = State->ColorTable[State->ColorTheme.TextHighlightColor];

			v2 InteractMouseP = State->Input->MouseP;
			if (InteractMouseP.y > (WorkRect.Min.y - 0.5f * CursorHeight)) {
				*Value = Max;
			}

			if (InteractMouseP.y < (WorkRect.Max.y + 0.5f * CursorHeight)) {
				*Value = Min;
			}

			float AT = InteractMouseP.y - (WorkRect.Min.y + 0.5f * CursorHeight);
			AT = Clamp(AT, 0.0f, WorkRectDim.y - CursorHeight);
			float NewVal01 = 1.0f - (AT / (WorkRectDim.y - CursorHeight));
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;

			char ValStr[16];
			stbsp_sprintf(ValStr, "%.3f", *Value);
			GUILabel(State, ValStr, CursorRect.Max);
		}

		//NOTE(DIMA): Drawing cursor
		PushRect(State->RenderStack, CursorRect, CursorColor);
		PushRectOutline(State->RenderStack, CursorRect, 2, State->ColorTable[State->ColorTheme.OutlineColor]);

		//NOTE(DIMA): Postprocessing
		GUIDescribeElementDim(
			State, 
			V2(MaxWidth,
			SmallTextRc.Max.y - MaxValueRc.Min.y));

		GUIAdvanceCursor(State);
	}

	GUIEndElement(State, GUIElement_InteractibleItem);
}

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction);
	
	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		float NextRowAdvanceFull = GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale);

		float* Value = Interaction->VariableLink.Value_F32;

		Assert(Max > Min);

		rect2 NameTextSize = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText, 
			Name, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale,
			GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
		v2 NameTextDim = GetRectDim(NameTextSize);

		char ValueBuf[32];
		stbsp_sprintf(ValueBuf, "%.3f", *Value);
		rect2 ValueTextSize = PrintTextInternal(GUIState, PrintTextType_GetTextSize, ValueBuf, 0, 0, GUIState->FontScale);

		//NOTE(Dima): Next element to the text is advanced by AscenderHeight
		v2 WorkRectMin = V2(
			View->CurrentX + NameTextDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale,
			View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull * 10, NextRowAdvanceFull);

		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);
		v4 WorkRectColor = GUIState->ColorTable[GUIState->ColorTheme.FirstColor];

		float RectOutlineWidth = 1.0f;

		PushRect(GUIState->RenderStack, WorkRect, WorkRectColor);
		PushRectOutline(GUIState->RenderStack, WorkRect, RectOutlineWidth, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = ((float)(*Value) - Min) / (float)Range;

		float CursorHeight = WorkRectDim.y;
		float CursorWidth = CursorHeight * 0.75f;

		float CursorX = WorkRectMin.x + (WorkRectDim.x - CursorWidth) * RelativePos01;
		float CursorY = WorkRectMin.y - (CursorHeight - WorkRectDim.y) * 0.5f;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);
		v4 CursorColor = GUIState->ColorTable[GUIState->ColorTheme.SecondaryColor];

		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);

		if (MouseInRect(GUIState->Input, CursorRect) || MouseInRect(GUIState->Input, WorkRect)) {

			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(GUIState, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(GUIState, Interaction, false);
			IsHot = false;
		}

		if(IsHot){
			CursorColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];

			v2 InteractMouseP = GUIState->Input->MouseP;
#if 0
			if (InteractMouseP.x > (WorkRect.Max.x - 0.5f * CursorWidth)) {
				*Value = Max;
				InteractMouseP.x = (WorkRect.Max.x - 0.5f * CursorWidth);
			}

			if (InteractMouseP.x < (WorkRect.Min.x + 0.5f * CursorWidth)) {
				*Value = Min;
				InteractMouseP.x = (WorkRect.Min.x + 0.5f * CursorWidth);
			}
#endif

			float AT = InteractMouseP.x - (WorkRect.Min.x + 0.5f * CursorWidth);
			AT = Clamp(AT, 0.0f, WorkRectDim.x - CursorWidth);
			float NewVal01 = AT / (WorkRectDim.x - CursorWidth);
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(GUIState);
			}

			if (GUIState->WalkaroundIsHot) {
				PushRectOutline(GUIState->RenderStack, WorkRect, 2, GUIState->ColorTable[GUIState->ColorTheme.WalkaroundHotColor]);

				CursorColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];

				float ChangeStep = Range * 0.02f;
				if (ButtonIsDown(GUIState->Input, KeyType_Right)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(GUIState->Input, KeyType_Left)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}
			}
			else {
				PushRectOutline(GUIState->RenderStack, NameTextSize, 2, GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor]);
			}
		}

		PushRect(GUIState->RenderStack, CursorRect, CursorColor);
		PushRectOutline(GUIState->RenderStack, CursorRect, 2, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		float ValueTextY = WorkRectMin.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float ValueTextX = WorkRectMin.x + WorkRectDim.x * 0.5f - (ValueTextSize.Max.x - ValueTextSize.Min.x) * 0.5f;
		PrintTextInternal(GUIState, PrintTextType_PrintText, ValueBuf, ValueTextX, ValueTextY, GUIState->FontScale);

#if 0
		char TextBuf[64];
		stbsp_snprintf(TextBuf, sizeof(TextBuf), "Min: %.3f; Max: %.3f;", Min, Max);

		float DrawTextX = View->CurrentX + WorkRectMin.x + WorkRectDim.x + 10;
		PrintTextInternal(GUIState, PrintTextType_PrintText, TextBuf, DrawTextX, View->CurrentY, View->FontScale);
#endif

		GUIDescribeElementDim(
			GUIState,
			V2(WorkRect.Max.x - View->CurrentX, 
			WorkRect.Max.y - WorkRect.Min.y + (2.0f * RectOutlineWidth)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

#if 0
void GUIResizableRect(gui_state* State) {
	render_stack* RenderStack = State->RenderStack;
	input_system* Input = State->Input;;

	rect2* MainRect = &State->TempRect.Rect;
	v2 RectDim = GetRectDim(*MainRect);

	PushRectOutline(RenderStack, *MainRect, 2);
	PushRect(RenderStack, *MainRect, V4(0.0f, 0.0f, 0.0f, 0.7));

	v2 AnchorDim = V2(7, 7);

	rect2 SizeAnchorRect;
	SizeAnchorRect.Min = MainRect->Max - V2(3.0f, 3.0f);
	SizeAnchorRect.Max = SizeAnchorRect.Min + AnchorDim;
	v4 SizeAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

	rect2 PosAnchorRect;
	PosAnchorRect.Min = MainRect->Min - V2(3.0f, 3.0f);
	PosAnchorRect.Max = PosAnchorRect.Min + AnchorDim;
	v4 PosAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

	if (MouseInRect(State->Input, SizeAnchorRect)) {
		SizeAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

		if (MouseButtonWentDown(Input, MouseButton_Left) && State->TempRect.SizeInteraction.IsHot) {
			State->TempRect.SizeInteraction.IsHot = true;
		}
	}

	if (MouseInRect(Input, PosAnchorRect)) {
		PosAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

		if (MouseButtonWentDown(Input, MouseButton_Left) && State->TempRect.PosInteraction.IsHot) {
			State->TempRect.PosInteraction.IsHot = true;
		}
	}

	if (MouseButtonWentUp(Input, MouseButton_Left)) {
		State->TempRect.SizeInteraction.IsHot = false;
		State->TempRect.PosInteraction.IsHot = false;
	}

	if (State->TempRect.PosInteraction.IsHot) {
		MainRect->Min = Input->MouseP;
		MainRect->Max = MainRect->Min + RectDim;
		PosAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
	}

	v2 ResizedRectDim = RectDim;
	if (State->TempRect.SizeInteraction.IsHot) {
		MainRect->Max = Input->MouseP;
		SizeAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
		ResizedRectDim = GetRectDim(*MainRect);
	}

	if (ResizedRectDim.x < 10) {
		MainRect->Max.x = MainRect->Min.x + 10;
	}

	if (ResizedRectDim.y < 10) {
		MainRect->Max.y = MainRect->Min.y + 10;
	}

	PushRect(RenderStack, SizeAnchorRect.Min, AnchorDim, SizeAnchorColor);
	PushRect(RenderStack, PosAnchorRect.Min, AnchorDim, PosAnchorColor);
}
#endif

void GUITreeBegin(gui_state* State, char* NodeName) {
	gui_view* View = GUIGetCurrentView(State);
	
	b32 NeedShow = GUIBeginElement(State, GUIElement_TreeNode, NodeName, 0);
	gui_element* Elem = GUIGetCurrentElement(State);
	gui_tree_node_cache* Cache = &Elem->Cache.TreeNode;

	gui_interaction ActionInteraction = GUIVariableInteraction(&State->CurrentNode->Expanded, GUIVarType_B32);
	char TextBuf[64];
	if (State->PlusMinusSymbol) {
		stbsp_sprintf(TextBuf, "%c %s", Elem->Expanded ? '+' : '-', Elem->Text);
	}
	else {
		stbsp_sprintf(TextBuf, "%s", Elem->Text);
	}
	
	if (NeedShow) {
		GUIPreAdvanceCursor(State);

		//gui_interaction Interaction_ = GUIVariableInteraction(&State->CurrentNode->Expanded, GUIVarType_B32);
		//gui_interaction* Interaction = &Interaction_;

		rect2 Rc = PrintTextInternal(State, PrintTextType_GetTextSize, TextBuf, View->CurrentX, View->CurrentY, State->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = State->ColorTable[State->ColorTheme.TextColor];
		if (MouseInRect(State->Input, Rc)) {
			TextHighlightColor = State->ColorTable[State->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(State->Input, MouseButton_Left)) {
				Elem->Expanded = !Elem->Expanded;
				Cache->ExitState = 0;
			}
		}

		if (GUIWalkaroundIsHere(State)) {
			PushRectOutline(State->RenderStack, Rc, 2, State->ColorTable[State->ColorTheme.TextHighlightColor]);
		}

		PrintTextInternal(State, PrintTextType_PrintText, TextBuf, View->CurrentX, View->CurrentY, State->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(State, GetRectDim(Rc));

		GUIAdvanceCursor(State);

	}
	Elem->Cache.TreeNode.StackBeginY = View->CurrentY;
}

void GUITreeEnd(gui_state* State) {	
	gui_element* Elem = GUIGetCurrentElement(State);
	gui_view* View = GUIGetCurrentView(State);
	
	gui_tree_node_cache* Cache = &Elem->Cache.TreeNode;

	if ((Elem->Expanded == false) && 
		(Cache->ExitState == GUITreeNodeExit_None)) 
	{
		Cache->ExitY = Cache->StackY - Cache->StackBeginY;
		//Cache->ExitY = 100;
		Cache->ExitState = GUITreeNodeExit_Exiting;
	}

	if (Cache->ExitState == GUITreeNodeExit_Exiting) {
		float ExitSpeed = 5.0f;
		//float ExitSpeed = Cache->ExitY / 8.0f + 4;
		Cache->ExitY -= ExitSpeed;

		if (Cache->ExitY < 0.0f) {
			Cache->ExitY = 0.0f;
			Cache->ExitState = GUITreeNodeExit_Finished;
		}

		GUIPreAdvanceCursor(State);
		GUIDescribeElementDim(State, V2(0, Cache->ExitY));
		GUIAdvanceCursor(State);
	}

	GUIEndElement(State, GUIElement_TreeNode);
}

#if 0
static gui_element* GUIWalkaroundBFS(gui_element* At, char* NodeName) {
	gui_element* Result = 0;

	//NOTE(Dima): First - walk through all elements on the level
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		if (StringsAreEqual(Element->Name, NodeName)) {
			Result = Element;
			return(Result);
		}
	}

	//NOTE(Dima): If nothing found - recursivery go through every children
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		Result = GUIWalkaroundBFS(Element, NodeName);
		if (Result) {
			return(Result);
		}
	}

	//NOTE(Dima): Return 0 if nothing found
	return(Result);
}

void GUIBeginTreeFind(gui_state* State, char* NodeName) {
	gui_view* View = GUIGetCurrentView(State);

	gui_element* NeededElement = GUIWalkaroundBFS(State->RootNode, NodeName);
	Assert(NeededElement);

	gui_element* OldParent = View->CurrentNode;
	View->CurrentNode = NeededElement->Parent;
	View->CurrentNode->TempParent = OldParent;

	GUITreeBegin(State, NodeName);
}

void GUIEndTreeFind(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	GUITreeEnd(State);

	gui_element* Temp = View->CurrentNode;
	View->CurrentNode = View->CurrentNode->TempParent;
	Temp->TempParent = 0;
}
#endif