#ifndef RC_UI_H
#define RC_UI_H

#define MAX_CONTROL_COUNT 4096

#define MAX_TEXT_COUNT  4096
#define MAX_PANEL_COUNT 4096
#define MAX_BUTTON_COUNT 4096

#define Fn_Prot_UI_Cursor_Over(inFuncName) void inFuncName(struct UI_Control* inControl)
#define Fn_Prot_UI_Cursor_Click(inFuncName) void inFuncName(struct UI_Control* inControl)
#define Fn_Prot_UI_On_Resize(funcName) void funcName(struct UI_Control* inControl, void* inArgs)

typedef Fn_Prot_UI_Cursor_Over(Fn_UI_Cursor_Over);
typedef Fn_Prot_UI_Cursor_Click(Fn_UI_Cursor_Click);
typedef Fn_Prot_UI_On_Resize(Fn_UI_On_Resize);

Public Fn_Prot_UI_Cursor_Over(Stub_Cursor_Over) {}
Public Fn_Prot_UI_Cursor_Click(Stub_Cursor_Click) {}
Public Fn_Prot_UI_On_Resize(Stub_On_Resize) {}

typedef enum {
    UT_Text,
    UT_Panel,
    UT_Window,
    UT_Button,

    UI_TYPE_COUNT,
} UI_Type;

typedef enum {
    URWO_Nul,
    URWO_Align_Top_Left,

    URWO_Align_Top_Down,
    URWO_Align_Down,
    URWO_Align_Right,
    URWO_Align_Up,
    URWO_Size,
} UI_Relative_Op;

typedef struct UI_Control {
    UI_Type type;
    File_Ptr(void*, data);

    File_Ptr(struct UI_Control*, prev);
    File_Ptr(struct UI_Control*, next);

    b8 isActive;
    b8 updateEventAlreadyPush;
    b8 shouldRender;

    Transform prevRelTrans;
    Transform absTrans;

    Transform rendererTrans;

    UI_Relative_Op relOp;

    f32x2 pivot;

    f32x2 center;
    f32x2 halfDims;

    Fn_UI_Cursor_Click* Cursor_Click;
    Fn_UI_Cursor_Over*  Cursor_Over;
    Fn_UI_On_Resize* On_Resize;
} UI_Control;

typedef enum {
    UUET_Text,
    UUET_Transform,
} UI_Update_Event_Type;

typedef struct UI_Update_Event {
    UI_Update_Event_Type type;
    UI_Control* control;
} UI_Update_Event;

Public UI_Control* UI_Control_Create(UI_Type inType, void* inData, Transform* inAbsTrans, UI_Control* inPrevControl, Transform* inRelTrans,
                                     Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click);

Public void UI_Control_Set_Relative(UI_Control* inControl);
Public inline void UI_Control_Update(UI_Control* inControl, UI_Update_Event_Type inType);

typedef struct {
    UI_Control* control;
    String data;
} UI_String;

Public String* UI_String_Get(UI_String* inUIString);
Public String* UI_String_Set(UI_String* inUIString, String inStr);
Public String* UI_String_Get_To_Change(UI_String* inUIString);

Public void UI_On_Resize(UI_Control* inControl);

Public void* UI_Control_Get_Data(UI_Control* inControl);

Public inline void UI_Control_Draw_OutLine(UI_Control* inControl, f32 inLayer, f32x4 inColor);
Private void UI_Control_Draw_Line(UI_Control* inControl, f32 inPos, Axis inAxis, f32 inLayer, f32x4 inColor);

// UI Text

typedef struct {
    u32 size;
    File_Ptr(char*, str);
} File_String_2;

typedef struct {
    UI_Control* control;

    File_String_2 text;
    Asset_Handle font;
    Mesh mesh;
    Material material;
} UI_Text;

Public UI_Text* UI_Text_Create(String inText, b8 saveText, Asset_Handle inFont, Transform* inAbsTrans, UI_Control* inPrevControl,
                               Transform* inRelTrans, UI_Relative_Op inRelOp, Fn_UI_Cursor_Over* inUI_Cursor_Over,
                               Fn_UI_Cursor_Click* inUI_Cursor_Click);

Public void UI_Text_Destroy(UI_Control* inControl);

// UI Button

typedef struct {
    UI_Control* control;

    f32x4 color;
    Asset_Handle tex;
    Mesh mesh;
    Material material;
} UI_Button;

Public UI_Button* UI_Button_Create(f32x4 inColor, Asset_Handle inTex, Transform* inAbsTrans, UI_Control* inPrevControl,
                                   Transform* inRelTrans, Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click);

// UI Panel

typedef struct {
    UI_Control* control;

    f32x4 color;

    Asset_Handle tex;
    Mesh mesh;
    Material material;
} UI_Panel;

// UI Windows

#define AXIS_HOR  0
#define AXIS_VERT 1

typedef enum {
    Direction_Left  = 0b00,
    Direction_Down  = 0b01,
    Direction_Right = 0b10,
    Direction_Up    = 0b11,
} Direction;

typedef struct UI_Window_Node {
    File_Ptr(struct UI_Window*, window);
    File_Ptr(UI_Window_Node*, next);
} UI_Window_Node;

typedef enum {
    LE_First = 0x0,
    LE_Tail  = 0x1,
} List_Element;

typedef union {
    struct {
        union {
            File_Ptr(UI_Window_Node*, list);
            File_Ptr(UI_Window_Node*, first);
        };
        File_Ptr(UI_Window_Node*, tail);
    };
    UI_Window_Node* LE[2];
} UI_Window_List;

typedef struct UI_Window {
    UI_Control* control;

    b8 hasScene;

    f32x4 color;

    Mesh mesh;
    Material material;

    File_Ptr(UI_Text*, number);
    String numberString;
    char numberBuffer[8];

    union {
        struct {
            UI_Window_List adjacentLeft;
            UI_Window_List adjacentDown;
            UI_Window_List adjacentRight;
            UI_Window_List adjacentUp;
        };
        UI_Window_List adjacents[4];
    };
} UI_Window;

#define UI_WINDOW_MAX_COUNT 256

typedef struct {
    UI_Window_Node* freeList;
    u32 nodeCount;
    UI_Window_Node pool[UI_WINDOW_MAX_COUNT * 4];
} UI_Window_Node_Mem;

Private UI_Window* UI_Window_Create(f32x4 inColor, Fn_UI_On_Resize* inOnResizeFunc,
                                    Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click);

typedef union {
    UI_Window window;
    UI_Text   text;
    UI_Button button;
    UI_Panel  panel;
} UI_Control_Data;

#endif // RC_UI_H
