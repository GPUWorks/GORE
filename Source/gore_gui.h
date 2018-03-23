#ifndef GORE_GUI_H
#define GORE_GUI_H

#include "gore_render_stack.h"

#include "gore_asset.h"
#include "gore_input.h"

/*
	NOTE(dima): 
		It's not guaranteed that the gui code is thread safe
		so use it at your own risk in multithreaded code.

		My advice to you is that if you are going to use this
		code, make sure that you call all functions in one 
		thread.
*/

#define GUI_VALUE_VIEW_MULTIPLIER 8
#define GUI_VALUE_COLOR_VIEW_MULTIPLIER 5

#define GUI_EXT_COLORS_ENABLED 1

struct gui_state;

enum gui_variable_type {
	GUIVarType_F32,
	GUIVarType_U32,
	GUIVarType_I32,
	GUIVarType_U64,
	GUIVarType_I64,
	GUIVarType_B32,
	GUIVarType_STR,
	GUIVarType_StackedMemory,
	GUIVarType_RGBABuffer,
};

struct gui_variable_link {
	u32 Type;
	union {
		float* Value_F32;
		u32* Value_U32;
		i32* Value_I32;
		u64* Value_U64;
		i64* Value_I64;
		b32* Value_B32;
		char* Value_STR;
		stacked_memory* Value_StackedMemory;
		rgba_buffer* Value_RGBABuffer;
	};
};

enum gui_move_interaction_type {
	GUIMoveInteraction_None,

	GUIMoveInteraction_Move,
};

enum gui_resize_interaction_type {
	GUIResizeInteraction_None,

	GUIResizeInteraction_Default,
	GUIResizeInteraction_Proportional,
	GUIResizeInteraction_Horizontal,
	GUIResizeInteraction_Vertical,
};

struct gui_resize_interaction_context {
	v2* DimensionPtr;
	v2 Position;
	v2 MinDim;
	v2 OffsetInAnchor;
	u32 Type;
};

struct gui_move_interaction_context {
	v2* MovePosition;
	u32 Type;
	v2 OffsetInAnchor;
};

struct gui_tree_interaction_context {
	struct gui_element* Elem;
	//rect2 ElemRc;
	v4* TextHighlightColor;
};

struct gui_bool_interaction_context {
	b32* InteractBool;
};

struct gui_menu_bar_interaction_context {
	gui_element* MenuElement;
};

struct gui_radio_button_interaction_context {
	gui_element* RadioGroup;
	u32 PressedIndex;
};

struct gui_state_changer_group_interaction_context {
	gui_element* StateChangerGroup;
	u32 IncrementDirection;
};

enum gui_return_mouse_action_type {
	GUIReturnMouseAction_WentDown,
	GUIReturnMouseAction_WentUp,
	GUIReturnMouseAction_IsDown,
};

struct gui_return_mouse_action_interaction_context {
	b32* ActionHappened;

	u32 ActionType;
	u32 MouseButtonIndex;
	input_system* Input;
};

enum gui_interaction_type {
	GUIInteraction_None,

	GUIInteraction_VariableLink,
	GUIInteraction_Resize,
	GUIInteraction_Move,
	GUIInteraction_Tree,
	GUIInteraction_Bool,
	GUIInteraction_MenuBar,
	GUIInteraction_RadioButton,
	GUIInteraction_StateChangerGroup,
	GUIInteraction_ReturnMouseAction,
};

struct gui_interaction {
	u32 ID;

	u32 Type;
	union {
		gui_variable_link VariableLink;
		gui_resize_interaction_context ResizeContext;
		gui_move_interaction_context MoveContext;
		gui_tree_interaction_context TreeInteraction;
		gui_bool_interaction_context BoolInteraction;
		gui_menu_bar_interaction_context MenuMarInteraction;
		gui_radio_button_interaction_context RadioButtonInteraction;
		gui_state_changer_group_interaction_context StateChangerGroupInteraction;
		gui_return_mouse_action_interaction_context ReturnMouseActionInteraction;
	};
};

#if 0
enum gui_interaction_rule_type {
	GUIInteractionRule_MouseClick,
	GUIInteractionRule_MouseOver,
	GUIInteractionRule_ButtonDown,
	GUIInteractionRule_ButtonUp,
};

struct gui_interaction_rule_mouse_click_data {
	u32 MouseButtonID;
};

struct gui_interaction_rule_button_data {
	u32 KeyID;
};

struct gui_interaction_rule_mouse_over_data {
	v2 MouseP;
};

struct gui_interaction_rule {
	u32 InteractionRuleType;

	union {
		gui_interaction_rule_mouse_click_data MouseClickData;
		gui_interaction_rule_mouse_over_data MouseOverData;
		gui_interaction_rule_button_data ButtonData;
	};
};
#endif


inline gui_variable_link GUIVariableLink(void* Variable, u32 Type) {
	gui_variable_link Link;

	switch (Type) {
		case GUIVarType_B32: {Link.Value_B32 = (b32*)Variable; }break;
		case GUIVarType_F32: {Link.Value_F32 = (float*)Variable; }break;
		case GUIVarType_I32: {Link.Value_I32 = (i32*)Variable; }break;
		case GUIVarType_U32: {Link.Value_U32 = (u32*)Variable; }break;
		case GUIVarType_I64: {Link.Value_I64 = (i64*)Variable; }break;
		case GUIVarType_U64: {Link.Value_U64 = (u64*)Variable; }break;
		case GUIVarType_STR: {Link.Value_STR = (char*)Variable; }break;
		case GUIVarType_StackedMemory: {
			Link.Value_StackedMemory = (stacked_memory*)Variable;
		}break;
		case GUIVarType_RGBABuffer: {
			Link.Value_RGBABuffer = (rgba_buffer*)Variable;
		}break;
	}

	Link.Type = Type;

	return(Link);
}

inline gui_interaction GUIVariableInteraction(void* Variable, u32 Type) {
	gui_interaction Result = {};

	Result.VariableLink = GUIVariableLink(Variable, Type);
	Result.Type = GUIInteraction_VariableLink;
	Result.ID = 0;

	return(Result);
}

/* Type is the value of enum: gui_resize_interaction_type */
inline gui_interaction GUIResizeInteraction(v2 Position, v2* DimensionPtr, u32 Type) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Resize;

	Result.ResizeContext.DimensionPtr = DimensionPtr;
	Result.ResizeContext.Position = Position;
	Result.ResizeContext.Type = Type;

	return(Result);
}

/* Type is the value of enum: gui_move_interaction_type */
inline gui_interaction GUIMoveInteraction(v2* MovePosition, u32 Type) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Move;

	Result.MoveContext.MovePosition = MovePosition;
	Result.MoveContext.Type = Type;

	return(Result);
}

inline gui_interaction GUITreeInteraction(struct gui_element* Elem, rect2 ElemRc) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Tree;

	Result.TreeInteraction.Elem = Elem;

	return(Result);
}

inline gui_interaction GUIBoolInteraction(b32* InteractBool) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Bool;

	Result.BoolInteraction.InteractBool = InteractBool;

	return(Result);
}

inline gui_interaction GUIMenuBarInteraction(gui_element* MenuElement) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_MenuBar;
	Result.MenuMarInteraction.MenuElement = MenuElement;

	return(Result);
}

inline gui_interaction GUIRadioButtonInteraction(gui_element* RadioGroup, u32 PressedIndex) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_RadioButton;
	Result.RadioButtonInteraction.RadioGroup = RadioGroup;
	Result.RadioButtonInteraction.PressedIndex = PressedIndex;

	return(Result);
}


/*
	IncrementDirection: 0 -> Forward; 1 -> Backward;
*/
inline gui_interaction GUIStateChangerGroupInteraction(gui_element* StateChangerGroup, u32 IncrementDirection) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_StateChangerGroup;
	Result.StateChangerGroupInteraction.StateChangerGroup = StateChangerGroup;
	Result.StateChangerGroupInteraction.IncrementDirection = IncrementDirection;

	return(Result);
}

inline gui_interaction GUIReturnMouseActionInteraction(input_system* Input, b32* ActionHappened, u32 ActionType, u32 MouseButtonIndex) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_ReturnMouseAction;
	Result.ReturnMouseActionInteraction.Input = Input;
	Result.ReturnMouseActionInteraction.MouseButtonIndex = MouseButtonIndex;
	Result.ReturnMouseActionInteraction.ActionType = ActionType;
	Result.ReturnMouseActionInteraction.ActionHappened = ActionHappened;

	return(Result);
}

inline gui_interaction GUINullInteraction() {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_None;

	return(Result);
}

enum gui_layout_type {
	GUILayout_Simple,
	GUILayout_Tree,
};

struct gui_layout {
	u32 ID;

	u32 ViewType;

	float CurrentX;
	float CurrentY;

	v2 LastElementP;
	v2 LastElementDim;

	float RowBeginX;
	float RowBiggestHeight;

	float CurrentPreAdvance;

	gui_layout* Parent;
	gui_layout* NextBro;
	gui_layout* PrevBro;

	int BeginnedRowsCount;
	b32 NeedHorizontalAdvance;
};

/*
	GUIElement_None - for the root element. Do not use it.

	GUIElement_TreeNode - this is for the tree view.
	This value means that this element will be cached
	and GUIActionText will be used for transitions 
	between root-child elements;

	GUIElement_InteractibleItem means that the element
	will have it's own unique ID by which it will be
	identified when GUIInteractionIsHot() called;
	This also means that this element will be cached;

	GUIElement_CachedItem means that the the element 
	will be cached. It means that it and it's children 
	elements won't be freed at the end of the frame.
	This is useful for elements that will have
	some other interactible elements in it;
	Difference between GUIElement_InteractibleItem
	is that the GUIElement_InteractibleItem should
	not have children.

	GUIElement_StaticItem means that the element
	will be freed at the end of every frame and 
	allocated at the beginning of every frame;
	This means also that the ID of the element 
	will not be calculated. The element should(or can) 
	have no name;

	P.S. Cahing means that the element won't be 
	created from scratch. It means that the element
	will not be freed at the end of the frame
*/
enum gui_element_type {
	GUIElement_None,

	GUIElement_TreeNode,
	GUIElement_InteractibleItem,
	GUIElement_CachedItem,
	GUIElement_StaticItem,
	GUIElement_Row,
	GUIElement_Layout,
	GUIElement_RadioGroup,
	GUIElement_StateChangerGroup,

	GUIElement_MenuBar,
	GUIElement_MenuItem,
};

enum gui_tree_node_exit_state {
	GUITreeNodeExit_None, 
	GUITreeNodeExit_Exiting,
	GUITreeNodeExit_Finished,
};

struct gui_tree_node_cache {
	u32 ExitState;

	float StackBeginY;
	float StackY;
	float ExitY;
};

struct gui_vertical_slider_cache {
};

struct gui_slider_cache {

};

struct gui_button_cache {
	v2 ButtonRectDim;
};

struct gui_bool_button_cache {
	v2 ButtonRectDim;
	v2 ButtonTrueDim;
};

struct gui_image_view_cache {
	v2 Dimension;
};

struct gui_stackedmem_cache {
	v2 Dimension;
};

struct gui_anchor_cache {
	v2 OffsetInAnchor;
};

struct gui_layout_cache {
	v2 Position;
	v2 Dimension;
};

struct gui_dimensional_cache {
	v2 Position;
	v2 Dimension;
};

struct gui_menu_node_cache{
	float MaxWidth;
	float MaxHeight;

	float SumHeight;
	float SumWidth;

	u32 ChildrenCount;
	u32 Type;
};

struct gui_radio_group_cache {
	u32 ActiveIndex;
};

struct gui_radio_button_cache {
	b32 IsActive;
};

struct gui_state_changer_group_cache {
	gui_element* ActiveElement;
};

struct gui_state_changer_cache {
	u32 StateID;
};

struct gui_element_cache {
	union {
		gui_tree_node_cache TreeNode;
		gui_vertical_slider_cache VerticalSlider;
		gui_slider_cache Slider;
		gui_button_cache Button;
		gui_bool_button_cache BoolButton;
		gui_image_view_cache ImageView;
		gui_stackedmem_cache StackedMem;
		gui_anchor_cache Anchor;
		gui_layout_cache Layout;
		gui_menu_node_cache MenuNode;
		gui_dimensional_cache Dimensional;
		gui_radio_group_cache RadioCache;
		gui_radio_button_cache RadioButton;
		gui_state_changer_group_cache StateChangerGroupCache;
		gui_state_changer_cache StateChangerCache;
	};

	b32 IsInitialized;
};

struct gui_element {
	u32 ID;

	s32 Depth;

	gui_element* Parent;
	//gui_element* TempParent;

	//NOTE(Dima): Used for remembering last tree parent for tree nodes
	gui_element* TempParentTree;

	gui_element* NextBro;
	gui_element* PrevBro;

	gui_element* ChildrenSentinel;

	b32 Expanded;
	char Name[32];
	char Text[32];

	u16 RowCount;

	u32 Type;

	gui_element_cache Cache;
};

enum gui_color_table_type {
	GUIColor_Black,
	GUIColor_White,

	GUIColor_Red,
	GUIColor_Green,
	GUIColor_Blue,

	GUIColor_Yellow,
	GUIColor_Magenta,
	GUIColor_Cyan,

	GUIColor_PrettyBlue,
	GUIColor_PrettyGreen,

	GUIColor_Purple,

	GUIColor_Orange,
	GUIColor_OrangeRed,
	GUIColor_BloodOrange,
	GUIColor_Brown,
	GUIColor_Amber,
	GUIColor_Cerise,
	GUIColor_CerisePink,
	GUIColor_ChinaPink,
	GUIColor_ChinaRose,

	GUIColor_DarkRed,
	GUIColor_RoyalBlue,
	GUIColor_PrettyPink,
	GUIColor_BluishGray,

	GUIColor_FPSBlue,
	GUIColor_FPSOrange,

	GUIColor_Burlywood,
	GUIColor_DarkGoldenrod,
	GUIColor_OliveDrab,

	GUIColor_Black_x20,

#if GUI_EXT_COLORS_ENABLED
	GUIColorExt_AliceBlue,
	GUIColorExt_AntiqueWhite,
	GUIColorExt_AntiqueWhite1,
	GUIColorExt_AntiqueWhite2,
	GUIColorExt_AntiqueWhite3,
	GUIColorExt_AntiqueWhite4,
	GUIColorExt_aquamarine1,
	GUIColorExt_aquamarine2,
	GUIColorExt_aquamarine4,
	GUIColorExt_azure1,
	GUIColorExt_azure2,
	GUIColorExt_azure3,
	GUIColorExt_azure4,
	GUIColorExt_beige,
	GUIColorExt_bisque1,
	GUIColorExt_bisque2,
	GUIColorExt_bisque3,
	GUIColorExt_bisque4,
	GUIColorExt_black,
	GUIColorExt_BlanchedAlmond,
	GUIColorExt_blue1,
	GUIColorExt_blue2,
	GUIColorExt_blue4,
	GUIColorExt_BlueViolet,
	GUIColorExt_brown,
	GUIColorExt_brown1,
	GUIColorExt_brown2,
	GUIColorExt_brown3,
	GUIColorExt_brown4,
	GUIColorExt_burlywood,
	GUIColorExt_burlywood1,
	GUIColorExt_burlywood2,
	GUIColorExt_burlywood3,
	GUIColorExt_burlywood4,
	GUIColorExt_CadetBlue,
	GUIColorExt_CadetBlue1,
	GUIColorExt_CadetBlue2,
	GUIColorExt_CadetBlue3,
	GUIColorExt_CadetBlue4,
	GUIColorExt_chartreuse1,
	GUIColorExt_chartreuse2,
	GUIColorExt_chartreuse3,
	GUIColorExt_chartreuse4,
	GUIColorExt_chocolate,
	GUIColorExt_chocolate1,
	GUIColorExt_chocolate2,
	GUIColorExt_chocolate3,
	GUIColorExt_coral,
	GUIColorExt_coral1,
	GUIColorExt_coral2,
	GUIColorExt_coral3,
	GUIColorExt_coral4,
	GUIColorExt_CornflowerBlue,
	GUIColorExt_cornsilk1,
	GUIColorExt_cornsilk2,
	GUIColorExt_cornsilk3,
	GUIColorExt_cornsilk4,
	GUIColorExt_cyan1,
	GUIColorExt_cyan2,
	GUIColorExt_cyan3,
	GUIColorExt_cyan4,
	GUIColorExt_DarkGoldenrod,
	GUIColorExt_DarkGoldenrod1,
	GUIColorExt_DarkGoldenrod2,
	GUIColorExt_DarkGoldenrod3,
	GUIColorExt_DarkGoldenrod4,
	GUIColorExt_DarkGreen,
	GUIColorExt_DarkKhaki,
	GUIColorExt_DarkOliveGreen,
	GUIColorExt_DarkOliveGreen1,
	GUIColorExt_DarkOliveGreen2,
	GUIColorExt_DarkOliveGreen3,
	GUIColorExt_DarkOliveGreen4,
	GUIColorExt_DarkOrange,
	GUIColorExt_DarkOrange1,
	GUIColorExt_DarkOrange2,
	GUIColorExt_DarkOrange3,
	GUIColorExt_DarkOrange4,
	GUIColorExt_DarkOrchid,
	GUIColorExt_DarkOrchid1,
	GUIColorExt_DarkOrchid2,
	GUIColorExt_DarkOrchid3,
	GUIColorExt_DarkOrchid4,
	GUIColorExt_DarkSalmon,
	GUIColorExt_DarkSeaGreen,
	GUIColorExt_DarkSeaGreen1,
	GUIColorExt_DarkSeaGreen2,
	GUIColorExt_DarkSeaGreen3,
	GUIColorExt_DarkSeaGreen4,
	GUIColorExt_DarkSlateBlue,
	GUIColorExt_DarkSlateGray,
	GUIColorExt_DarkSlateGray1,
	GUIColorExt_DarkSlateGray2,
	GUIColorExt_DarkSlateGray3,
	GUIColorExt_DarkSlateGray4,
	GUIColorExt_DarkTurquoise,
	GUIColorExt_DarkViolet,
	GUIColorExt_DeepPink1,
	GUIColorExt_DeepPink2,
	GUIColorExt_DeepPink3,
	GUIColorExt_DeepPink4,
	GUIColorExt_DeepSkyBlue1,
	GUIColorExt_DeepSkyBlue2,
	GUIColorExt_DeepSkyBlue3,
	GUIColorExt_DeepSkyBlue4,
	GUIColorExt_DimGray,
	GUIColorExt_DodgerBlue1,
	GUIColorExt_DodgerBlue2,
	GUIColorExt_DodgerBlue3,
	GUIColorExt_DodgerBlue4,
	GUIColorExt_firebrick,
	GUIColorExt_firebrick1,
	GUIColorExt_firebrick2,
	GUIColorExt_firebrick3,
	GUIColorExt_firebrick4,
	GUIColorExt_FloralWhite,
	GUIColorExt_ForestGreen,
	GUIColorExt_gainsboro,
	GUIColorExt_GhostWhite,
	GUIColorExt_gold1,
	GUIColorExt_gold2,
	GUIColorExt_gold3,
	GUIColorExt_gold4,
	GUIColorExt_goldenrod,
	GUIColorExt_goldenrod1,
	GUIColorExt_goldenrod2,
	GUIColorExt_goldenrod3,
	GUIColorExt_goldenrod4,
	GUIColorExt_gray,
	GUIColorExt_gray1,
	GUIColorExt_gray2,
	GUIColorExt_gray3,
	GUIColorExt_gray4,
	GUIColorExt_gray5,
	GUIColorExt_gray6,
	GUIColorExt_gray7,
	GUIColorExt_gray8,
	GUIColorExt_gray9,
	GUIColorExt_gray10,
	GUIColorExt_gray11,
	GUIColorExt_gray12,
	GUIColorExt_gray13,
	GUIColorExt_gray14,
	GUIColorExt_gray15,
	GUIColorExt_gray16,
	GUIColorExt_gray17,
	GUIColorExt_gray18,
	GUIColorExt_gray19,
	GUIColorExt_gray20,
	GUIColorExt_gray21,
	GUIColorExt_gray22,
	GUIColorExt_gray23,
	GUIColorExt_gray24,
	GUIColorExt_gray25,
	GUIColorExt_gray26,
	GUIColorExt_gray27,
	GUIColorExt_gray28,
	GUIColorExt_gray29,
	GUIColorExt_gray30,
	GUIColorExt_gray31,
	GUIColorExt_gray32,
	GUIColorExt_gray33,
	GUIColorExt_gray34,
	GUIColorExt_gray35,
	GUIColorExt_gray36,
	GUIColorExt_gray37,
	GUIColorExt_gray38,
	GUIColorExt_gray39,
	GUIColorExt_gray40,
	GUIColorExt_gray41,
	GUIColorExt_gray42,
	GUIColorExt_gray43,
	GUIColorExt_gray44,
	GUIColorExt_gray45,
	GUIColorExt_gray46,
	GUIColorExt_gray47,
	GUIColorExt_gray48,
	GUIColorExt_gray49,
	GUIColorExt_gray50,
	GUIColorExt_gray51,
	GUIColorExt_gray52,
	GUIColorExt_gray53,
	GUIColorExt_gray54,
	GUIColorExt_gray55,
	GUIColorExt_gray56,
	GUIColorExt_gray57,
	GUIColorExt_gray58,
	GUIColorExt_gray59,
	GUIColorExt_gray60,
	GUIColorExt_gray61,
	GUIColorExt_gray62,
	GUIColorExt_gray63,
	GUIColorExt_gray64,
	GUIColorExt_gray65,
	GUIColorExt_gray66,
	GUIColorExt_gray67,
	GUIColorExt_gray68,
	GUIColorExt_gray69,
	GUIColorExt_gray70,
	GUIColorExt_gray71,
	GUIColorExt_gray72,
	GUIColorExt_gray73,
	GUIColorExt_gray74,
	GUIColorExt_gray75,
	GUIColorExt_gray76,
	GUIColorExt_gray77,
	GUIColorExt_gray78,
	GUIColorExt_gray79,
	GUIColorExt_gray80,
	GUIColorExt_gray81,
	GUIColorExt_gray82,
	GUIColorExt_gray83,
	GUIColorExt_gray84,
	GUIColorExt_gray85,
	GUIColorExt_gray86,
	GUIColorExt_gray87,
	GUIColorExt_gray88,
	GUIColorExt_gray89,
	GUIColorExt_gray90,
	GUIColorExt_gray91,
	GUIColorExt_gray92,
	GUIColorExt_gray93,
	GUIColorExt_gray94,
	GUIColorExt_gray95,
	GUIColorExt_gray97,
	GUIColorExt_gray98,
	GUIColorExt_gray99,
	GUIColorExt_green1,
	GUIColorExt_green2,
	GUIColorExt_green3,
	GUIColorExt_green4,
	GUIColorExt_GreenYellow,
	GUIColorExt_honeydew1,
	GUIColorExt_honeydew2,
	GUIColorExt_honeydew3,
	GUIColorExt_honeydew4,
	GUIColorExt_HotPink,
	GUIColorExt_HotPink1,
	GUIColorExt_HotPink2,
	GUIColorExt_HotPink3,
	GUIColorExt_HotPink4,
	GUIColorExt_IndianRed,
	GUIColorExt_IndianRed1,
	GUIColorExt_IndianRed2,
	GUIColorExt_IndianRed3,
	GUIColorExt_IndianRed4,
	GUIColorExt_ivory1,
	GUIColorExt_ivory2,
	GUIColorExt_ivory3,
	GUIColorExt_ivory4,
	GUIColorExt_khaki,
	GUIColorExt_khaki1,
	GUIColorExt_khaki2,
	GUIColorExt_khaki3,
	GUIColorExt_khaki4,
	GUIColorExt_lavender,
	GUIColorExt_LavenderBlush1,
	GUIColorExt_LavenderBlush2,
	GUIColorExt_LavenderBlush3,
	GUIColorExt_LavenderBlush4,
	GUIColorExt_LawnGreen,
	GUIColorExt_LemonChiffon1,
	GUIColorExt_LemonChiffon2,
	GUIColorExt_LemonChiffon3,
	GUIColorExt_LemonChiffon4,
	GUIColorExt_light,
	GUIColorExt_LightBlue,
	GUIColorExt_LightBlue1,
	GUIColorExt_LightBlue2,
	GUIColorExt_LightBlue3,
	GUIColorExt_LightBlue4,
	GUIColorExt_LightCoral,
	GUIColorExt_LightCyan1,
	GUIColorExt_LightCyan2,
	GUIColorExt_LightCyan3,
	GUIColorExt_LightCyan4,
	GUIColorExt_LightGoldenrod1,
	GUIColorExt_LightGoldenrod2,
	GUIColorExt_LightGoldenrod3,
	GUIColorExt_LightGoldenrod4,
	GUIColorExt_LightGoldenrodYellow,
	GUIColorExt_LightGray,
	GUIColorExt_LightPink,
	GUIColorExt_LightPink1,
	GUIColorExt_LightPink2,
	GUIColorExt_LightPink3,
	GUIColorExt_LightPink4,
	GUIColorExt_LightSalmon1,
	GUIColorExt_LightSalmon2,
	GUIColorExt_LightSalmon3,
	GUIColorExt_LightSalmon4,
	GUIColorExt_LightSeaGreen,
	GUIColorExt_LightSkyBlue,
	GUIColorExt_LightSkyBlue1,
	GUIColorExt_LightSkyBlue2,
	GUIColorExt_LightSkyBlue3,
	GUIColorExt_LightSkyBlue4,
	GUIColorExt_LightSlateBlue,
	GUIColorExt_LightSlateGray,
	GUIColorExt_LightSteelBlue,
	GUIColorExt_LightSteelBlue1,
	GUIColorExt_LightSteelBlue2,
	GUIColorExt_LightSteelBlue3,
	GUIColorExt_LightSteelBlue4,
	GUIColorExt_LightYellow1,
	GUIColorExt_LightYellow2,
	GUIColorExt_LightYellow3,
	GUIColorExt_LightYellow4,
	GUIColorExt_LimeGreen,
	GUIColorExt_linen,
	GUIColorExt_magenta,
	GUIColorExt_magenta2,
	GUIColorExt_magenta3,
	GUIColorExt_magenta4,
	GUIColorExt_maroon,
	GUIColorExt_maroon1,
	GUIColorExt_maroon2,
	GUIColorExt_maroon3,
	GUIColorExt_maroon4,
	GUIColorExt_medium,
	GUIColorExt_MediumAquamarine,
	GUIColorExt_MediumBlue,
	GUIColorExt_MediumOrchid,
	GUIColorExt_MediumOrchid1,
	GUIColorExt_MediumOrchid2,
	GUIColorExt_MediumOrchid3,
	GUIColorExt_MediumOrchid4,
	GUIColorExt_MediumPurple,
	GUIColorExt_MediumPurple1,
	GUIColorExt_MediumPurple2,
	GUIColorExt_MediumPurple3,
	GUIColorExt_MediumPurple4,
	GUIColorExt_MediumSeaGreen,
	GUIColorExt_MediumSlateBlue,
	GUIColorExt_MediumSpringGreen,
	GUIColorExt_MediumTurquoise,
	GUIColorExt_MediumVioletRed,
	GUIColorExt_MidnightBlue,
	GUIColorExt_MintCream,
	GUIColorExt_MistyRose1,
	GUIColorExt_MistyRose2,
	GUIColorExt_MistyRose3,
	GUIColorExt_MistyRose4,
	GUIColorExt_moccasin,
	GUIColorExt_NavajoWhite1,
	GUIColorExt_NavajoWhite2,
	GUIColorExt_NavajoWhite3,
	GUIColorExt_NavajoWhite4,
	GUIColorExt_NavyBlue,
	GUIColorExt_OldLace,
	GUIColorExt_OliveDrab,
	GUIColorExt_OliveDrab1,
	GUIColorExt_OliveDrab2,
	GUIColorExt_OliveDrab4,
	GUIColorExt_orange1,
	GUIColorExt_orange2,
	GUIColorExt_orange3,
	GUIColorExt_orange4,
	GUIColorExt_OrangeRed1,
	GUIColorExt_OrangeRed2,
	GUIColorExt_OrangeRed3,
	GUIColorExt_OrangeRed4,
	GUIColorExt_orchid,
	GUIColorExt_orchid1,
	GUIColorExt_orchid2,
	GUIColorExt_orchid3,
	GUIColorExt_orchid4,
	GUIColorExt_pale,
	GUIColorExt_PaleGoldenrod,
	GUIColorExt_PaleGreen,
	GUIColorExt_PaleGreen1,
	GUIColorExt_PaleGreen2,
	GUIColorExt_PaleGreen3,
	GUIColorExt_PaleGreen4,
	GUIColorExt_PaleTurquoise,
	GUIColorExt_PaleTurquoise1,
	GUIColorExt_PaleTurquoise2,
	GUIColorExt_PaleTurquoise3,
	GUIColorExt_PaleTurquoise4,
	GUIColorExt_PaleVioletRed,
	GUIColorExt_PaleVioletRed1,
	GUIColorExt_PaleVioletRed2,
	GUIColorExt_PaleVioletRed3,
	GUIColorExt_PaleVioletRed4,
	GUIColorExt_PapayaWhip,
	GUIColorExt_PeachPuff1,
	GUIColorExt_PeachPuff2,
	GUIColorExt_PeachPuff3,
	GUIColorExt_PeachPuff4,
	GUIColorExt_pink,
	GUIColorExt_pink1,
	GUIColorExt_pink2,
	GUIColorExt_pink3,
	GUIColorExt_pink4,
	GUIColorExt_plum,
	GUIColorExt_plum1,
	GUIColorExt_plum2,
	GUIColorExt_plum3,
	GUIColorExt_plum4,
	GUIColorExt_PowderBlue,
	GUIColorExt_purple,
	GUIColorExt_rebeccapurple,
	GUIColorExt_purple1,
	GUIColorExt_purple2,
	GUIColorExt_purple3,
	GUIColorExt_purple4,
	GUIColorExt_red1,
	GUIColorExt_red2,
	GUIColorExt_red3,
	GUIColorExt_red4,
	GUIColorExt_RosyBrown,
	GUIColorExt_RosyBrown1,
	GUIColorExt_RosyBrown2,
	GUIColorExt_RosyBrown3,
	GUIColorExt_RosyBrown4,
	GUIColorExt_RoyalBlue,
	GUIColorExt_RoyalBlue1,
	GUIColorExt_RoyalBlue2,
	GUIColorExt_RoyalBlue3,
	GUIColorExt_RoyalBlue4,
	GUIColorExt_SaddleBrown,
	GUIColorExt_salmon,
	GUIColorExt_salmon1,
	GUIColorExt_salmon2,
	GUIColorExt_salmon3,
	GUIColorExt_salmon4,
	GUIColorExt_SandyBrown,
	GUIColorExt_SeaGreen1,
	GUIColorExt_SeaGreen2,
	GUIColorExt_SeaGreen3,
	GUIColorExt_SeaGreen4,
	GUIColorExt_seashell1,
	GUIColorExt_seashell2,
	GUIColorExt_seashell3,
	GUIColorExt_seashell4,
	GUIColorExt_sienna,
	GUIColorExt_sienna1,
	GUIColorExt_sienna2,
	GUIColorExt_sienna3,
	GUIColorExt_sienna4,
	GUIColorExt_SkyBlue,
	GUIColorExt_SkyBlue1,
	GUIColorExt_SkyBlue2,
	GUIColorExt_SkyBlue3,
	GUIColorExt_SkyBlue4,
	GUIColorExt_SlateBlue,
	GUIColorExt_SlateBlue1,
	GUIColorExt_SlateBlue2,
	GUIColorExt_SlateBlue3,
	GUIColorExt_SlateBlue4,
	GUIColorExt_SlateGray,
	GUIColorExt_SlateGray1,
	GUIColorExt_SlateGray2,
	GUIColorExt_SlateGray3,
	GUIColorExt_SlateGray4,
	GUIColorExt_snow1,
	GUIColorExt_snow2,
	GUIColorExt_snow3,
	GUIColorExt_snow4,
	GUIColorExt_SpringGreen1,
	GUIColorExt_SpringGreen2,
	GUIColorExt_SpringGreen3,
	GUIColorExt_SpringGreen4,
	GUIColorExt_SteelBlue,
	GUIColorExt_SteelBlue1,
	GUIColorExt_SteelBlue2,
	GUIColorExt_SteelBlue3,
	GUIColorExt_SteelBlue4,
	GUIColorExt_tan,
	GUIColorExt_tan1,
	GUIColorExt_tan2,
	GUIColorExt_tan3,
	GUIColorExt_tan4,
	GUIColorExt_thistle,
	GUIColorExt_thistle1,
	GUIColorExt_thistle2,
	GUIColorExt_thistle3,
	GUIColorExt_thistle4,
	GUIColorExt_tomato1,
	GUIColorExt_tomato2,
	GUIColorExt_tomato3,
	GUIColorExt_tomato4,
	GUIColorExt_turquoise,
	GUIColorExt_turquoise1,
	GUIColorExt_turquoise2,
	GUIColorExt_turquoise3,
	GUIColorExt_turquoise4,
	GUIColorExt_violet,
	GUIColorExt_VioletRed,
	GUIColorExt_VioletRed1,
	GUIColorExt_VioletRed2,
	GUIColorExt_VioletRed3,
	GUIColorExt_VioletRed4,
	GUIColorExt_wheat,
	GUIColorExt_wheat1,
	GUIColorExt_wheat2,
	GUIColorExt_wheat3,
	GUIColorExt_wheat4,
	GUIColorExt_white,
	GUIColorExt_WhiteSmoke,
	GUIColorExt_yellow1,
	GUIColorExt_yellow2,
	GUIColorExt_yellow3,
	GUIColorExt_yellow4,
	GUIColorExt_YellowGreen,
#endif

	GUIColor_Count,
};

struct gui_color_theme {
	u32 TextColor;
	u32 TextHighlightColor;
	u32 TooltipTextColor;

	u32 OutlineColor;

	u32 GraphColor1;
	u32 GraphColor2;
	u32 GraphColor3;
	u32 GraphColor4;
	u32 GraphColor5;
	u32 GraphColor6;
	u32 GraphColor7;
	u32 GraphColor8;
	u32 GraphBackColor;

	u32 LogColor;
	u32 ErrLogColor;
	u32 OkLogColor;
	u32 WarningLogColor;

	u32 ButtonTextColor;
	u32 ButtonBackColor;
	u32 ButtonTextHighColor;
	u32 ButtonTextHighColor2;
	u32 ButtonOutlineColor;

	u32 AnchorColor;

	u32 WalkaroundHotColor;
};

inline gui_color_theme GUIDefaultColorTheme() {
	gui_color_theme Result;

	Result.TextColor = GUIColorExt_burlywood;
	Result.TextHighlightColor = GUIColor_Yellow;
	Result.TooltipTextColor = GUIColor_White;

	Result.OutlineColor = GUIColor_Black;

	Result.WalkaroundHotColor = GUIColor_PrettyGreen;

	Result.GraphColor1 = GUIColorExt_green3;
	Result.GraphColor2 = GUIColorExt_purple1;
	Result.GraphColor3 = GUIColorExt_red3;
	Result.GraphColor4 = GUIColorExt_blue1;
	Result.GraphColor5 = GUIColorExt_orange2;
	Result.GraphColor6 = GUIColorExt_magenta;
	Result.GraphColor7 = GUIColorExt_DarkGoldenrod3;
	Result.GraphColor8 = GUIColorExt_chartreuse3;

	Result.GraphBackColor = GUIColorExt_gray10;

	//Result.GraphColor1 = GUIColorExt_green3;
	//Result.GraphBackColor = GUIColorExt_red4;

	Result.LogColor = Result.TextColor;
	Result.OkLogColor = GUIColorExt_OliveDrab;
	Result.WarningLogColor = GUIColor_DarkGoldenrod;
	Result.ErrLogColor = GUIColorExt_red3;


	Result.ButtonTextColor = GUIColorExt_burlywood;
	Result.ButtonBackColor = GUIColorExt_gray10;
	Result.ButtonTextHighColor = GUIColorExt_DarkGoldenrod1;
	Result.ButtonOutlineColor = GUIColor_Black;
	Result.ButtonTextHighColor2 = Result.TextHighlightColor;

	Result.AnchorColor = GUIColorExt_OrangeRed1;

	return(Result);
}

struct gui_color_slot {
	v4 Color;
	u32 ColorU32;
	char* Name;
};


struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	float FontScale;
	float LastFontScale;

	b32 TextElemsCacheShouldBeReinitialized;

	input_system* Input;

	i32 ScreenWidth;
	i32 ScreenHeight;

	gui_element* CurrentNode;
	gui_element* CurrentTreeParent;
	gui_element* RootNode;
	gui_element* FreeElementsSentinel;
	gui_element* WalkaroundElement;
	b32 WalkaroundEnabled;
	b32 WalkaroundIsHot;

	gui_layout* CurrentLayout;
	gui_layout* LayoutSentinel;
	gui_layout* FreeLayoutSentinel;
	gui_layout* DefaultLayout;

	stacked_memory* GUIMem;

	b32 PlusMinusSymbol;

	//gui_interaction* HotInteraction;
	u32 HotInteractionID;

#define GUI_TOOLTIPS_MAX_COUNT 64
	char Tooltips[GUI_TOOLTIPS_MAX_COUNT][256];
	int TooltipCount;

	gui_color_theme ColorTheme;
	gui_color_slot ColorTable[GUIColor_Count];
};

inline v4 GUIGetColor(gui_state* GUIState, u32 ColorIndex) {
	v4 Result = GUIState->ColorTable[ColorIndex].Color;

	return(Result);
}


/*
inline v4 GUIGetThemeColor(gui_state* State, u32 Color) {
	v4 Result = State->ColorTable[State->ColorTheme[]]
}
*/

inline gui_layout* GUIGetCurrentLayout(gui_state* GUIState) {
	gui_layout* Result = 0;

	Result = GUIState->CurrentLayout;

	return(Result);
}

inline gui_element* GUIGetCurrentElement(gui_state* State) {
	gui_element* Result = State->CurrentNode;

	return(Result);
}

inline b32 GUIElementShouldBeUpdated(gui_element* Node) {
	FUNCTION_TIMING();

	b32 Result = 1;

	gui_element* At = Node->Parent;
	while (At->Parent != 0) {
		Result = At->Expanded & Result;

		if (Result == 0) {
			break;
		}

		At = At->Parent;
	}

	return(Result);
}

inline b32 GUIInteractionIsHot(gui_state* State, gui_interaction* Interaction) {
	b32 Result = 0;

	if (Interaction->ID == State->HotInteractionID) {
		Result = 1;
	}

	return(Result);
}

inline u32 GUIStringHashFNV(char* Name) {
	u32 Result = 2166136261;

	char* At = Name;
	while (*At) {

		Result *= 16777619;
		Result ^= *At;

		At++;
	}

	return(Result);
}

inline u32 GUITreeElementID(gui_element* Element) {
	u32 Result = 1;

	gui_element* At = Element;

	//TODO(Dima): Better hash function
	while (At->Parent != 0) {
		Result *= At->ID;

		At = At->Parent;
	}

	return(Result);
}

inline void GUISwapWalkaroundHot(gui_state* State) {
	State->WalkaroundIsHot = !State->WalkaroundIsHot;
}

inline b32 GUIWalkaroundIsHot(gui_state* State) {
	b32 Result = State->WalkaroundIsHot;

	return(Result);
}

inline b32 GUISetInteractionHot(gui_state* State, gui_interaction* Interaction, b32 IsHot) {
	b32 Result = IsHot;

	if (IsHot) {
		State->HotInteractionID = Interaction->ID;
	}
	else {
		State->HotInteractionID = 0;
	}

	return(Result);
}

#if 0
enum gui_align_type {
	GUIAlign_Begin,
	GUIAlign_Center,
	GUIAlign_End,
};

inline float GUIGetAlignValueForType(u32 AlignType) {
	float Result = 0.0f;

	if (AlignType == GUIAlign_Begin) {
		Result = 0.0f;
	}
	else if (AlignType == GUIAlign_Center) {
		Result = 0.5f;
	}
	else if (AlignType == GUIAlign_End) {
		Result = 1.0f;
	}
	else {
		Assert(!"Invalid align type")
	}

	return(Result);
}
#endif

enum gui_window_creation_flags {
	GUIWindow_Resizable = 1,
	GUIWindow_Movable = 2,
	GUIWindow_Collapsible = 4,
	GUIWindow_DefaultSize = 8,

	GUIWindow_Fullscreen = 16,

	GUIWindow_TopBar = 32,
	GUIWindow_TopBar_Movable = 64,
	//GUIWindow_TopBar_Close,
	//GUIWindow_TopBar_PrintName,
};

enum gui_menu_item_type {
	GUIMenuItem_MenuBarItem,
	GUIMenuItem_MenuItem,
};

extern void GUIInitState(gui_state* GUIState, stacked_memory* GUIMemory, font_info* FontInfo, input_system* Input, i32 Width, i32 Height);
extern void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack);
extern void GUIPrepareFrame(gui_state* GUIState);
extern void GUIEndFrame(gui_state* GUIState);

extern void GUIText(gui_state* GUIState, char* Text);
extern b32 GUIButton(gui_state* GUIState, char* ButtonName);
extern b32 GUIButtonAt(gui_state* GUIState, char* ButtonName, v2 At, rect2* ButtonRect = 0, v4* TextColor = 0);
extern void GUIBoolButton(gui_state* GUIState, char* ButtonName, b32* Value);
extern void GUIBoolButton2(gui_state* GUIState, char* ButtonName, b32* Value);
extern void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUITooltip(gui_state* GUIState, char* TooltipText);
extern void GUILabel(gui_state* GUIState, char* LabelText, v2 At);
extern void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction);
extern void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, gui_interaction* Interaction);
extern void GUIStackedMemGraph(gui_state* GUIState, char* Name, stacked_memory* MemoryStack);
extern void GUIImageView(gui_state* GUIState, char* Name, rgba_buffer* Buffer);
extern void GUIColorView(gui_state* GUIState, v4 Color, char* Name);
extern void GUIVector2View(gui_state* GUIState, v2 Value, char* Name);
extern void GUIVector3View(gui_state* GUIState, v3 Value, char* Name);
extern void GUIVector4View(gui_state* GUIState, v4 Value, char* Name);
extern void GUIInt32View(gui_state* GUIState, i32 Int, char* Name);

extern void GUIAnchor(gui_state* GUIState, char* Name, v2 Pos, v2 Dim, gui_interaction* Interaction, b32 Centered = 1);

extern void GUIWindow(gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height);

extern void GUIBeginMenuBar(gui_state* GUIState, char* MenuName);
extern void GUIEndMenuBar(gui_state* GUIState);
extern void GUIBeginMenuBarItem(gui_state* GUIState, char* Name);
extern void GUIEndMenuBarItem(gui_state* GUIState);
extern void GUIMenuBarItem(gui_state* GUIState, char* ItemName);

extern void GUIBeginLayout(gui_state* GUIState, char* LayoutName, u32 LayoutType);
extern void GUIChangeTreeNodeText(gui_state* GUIState, char* Text);
extern void GUIEndLayout(gui_state* GUIState, u32 LayoutType);
extern void GUIBeginRow(gui_state* State);
extern void GUIEndRow(gui_state* State);

extern gui_element* GUIBeginElement(
	gui_state* State,
	u32 ElementType,
	char* ElementName,
	gui_interaction* ElementInteraction,
	b32 InitExpandedState = 0,
	b32 IncrementDepth = 1);
extern void GUIEndElement(gui_state* State, u32 ElementType);

extern void GUIPreAdvanceCursor(gui_state* State);
extern void GUIDescribeElement(gui_state* State, v2 ElementDim, v2 ElementP);
extern void GUIAdvanceCursor(gui_state* State, float AdditionalYSpacing = 0.0f);

extern void GUITreeBegin(gui_state* State, char* NodeText, char* NameText = 0);
extern void GUITreeEnd(gui_state* State);


extern void GUIBeginRadioGroup(gui_state* GUIState, char* Name, u32 DefaultSetIndex);
extern void GUIRadioButton(gui_state* GUIState, char* Name, u32 UniqueIndex);
extern void GUIEndRadioGroup(gui_state* GUIState, u32* ActiveElement);

void GUIBeginStateChangerGroup(gui_state* GUIState, char* Name, u32 DefaultSetIndex);
void GUIStateChanger(gui_state* GUIState, char* Name, u32 StateID);
void GUIEndStateChangerGroupAt(gui_state* GUIState, v2 Pos, u32* ActiveElement);

extern rect2 GUITextBase(
	gui_state* GUIState,
	char* Text,
	v2 Pos,
	v4 TextColor = V4(1.0f, 1.0f, 1.0f, 1.0f),
	float FontScale = 1.0f,
	gui_interaction* Interaction = 0,
	v4 TextHighlightColor = V4(1.0f, 1.0f, 1.0f, 1.0f),
	v4 BackgroundColor = V4(0.0f, 0.5f, 1.0f, 1.0f),
	u32 OutlineWidth = 0,
	v4 OutlineColor = V4(0.0f, 0.0f, 0.0f, 1.0f));

#if 0
extern void GUIBeginTreeFind(gui_state* State, char* NodeName);
extern void GUIEndTreeFind(gui_state* State);
#endif

#endif