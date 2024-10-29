#ifndef RC_INPUT_H
#define RC_INPUT_H

// TODO(JENH): Implement priority actions.

#define ACTIONS_LIST                                      \
    /* action name  ,   begin   ,    end    ,   held   */ \
    X(Move_Left     , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Up       , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Right    , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Down     , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Forward  , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Backward , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Sprint        , JENH_TRUE , JENH_TRUE , JENH_FALSE) \
    X(FullScreen    , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Close         , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Debug_Mode    , JENH_TRUE , JENH_TRUE , JENH_FALSE) \
    X(Insert_Road   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Render_AABB   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Split_Left    , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Split_Up      , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Split_Right   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Split_Down    , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Window_Left   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Window_Up     , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Window_Right  , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Window_Down   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Size_Left     , JENH_FALSE, JENH_TRUE , JENH_TRUE ) \
    X(Size_Down     , JENH_FALSE, JENH_TRUE , JENH_TRUE ) \
    X(Size_Right    , JENH_FALSE, JENH_TRUE , JENH_TRUE ) \
    X(Size_Up       , JENH_FALSE, JENH_TRUE , JENH_TRUE ) \
    X(Load_Config   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Save_Config   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Text_Mode     , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Create_Text   , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(UI_Click      , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(UI_Create     , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(UI_Mode       , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Drive         , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
    X(Player_Left   , JENH_FALSE, JENH_FALSE, JENH_TRUE ) \
    X(Player_Right  , JENH_FALSE, JENH_FALSE, JENH_TRUE ) \
    X(Sponza        , JENH_TRUE , JENH_FALSE, JENH_FALSE) \
                                           \
    /* Extended Actions */                 \
    X(Move_Cam      , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(UI_Cursor     , JENH_FALSE, JENH_FALSE, JENH_TRUE ) \
    X(UI_Move       , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Move_Curve    , JENH_TRUE , JENH_FALSE, JENH_TRUE ) \
    X(Debug_Pointer , JENH_FALSE, JENH_FALSE, JENH_TRUE ) \
    X(Player_Move   , JENH_FALSE, JENH_FALSE, JENH_TRUE )

// NOTE(JENH): See keyboards scan codes
//             https://www.plantation-productions.com/Webster/www.artofasm.com/DOS/pdf/apndxc.pdf
//             Names are from QWERTY keyboard layout.

#define KBD_EXT_INDEX_MASK 0b10000000

typedef enum {
    II_Nul                = 0x00,
    II_Kbd_Esc            = 0x01,
    II_Kbd_1              = 0x02,
    II_Kbd_2              = 0x03,
    II_Kbd_3              = 0x04,
    II_Kbd_4              = 0x05,
    II_Kbd_5              = 0x06,
    II_Kbd_6              = 0x07,
    II_Kbd_7              = 0x08,
    II_Kbd_8              = 0x09,
    II_Kbd_9              = 0x0a,
    II_Kbd_0              = 0x0b,
    II_Kbd_Hyphen         = 0x0c,
    II_Kbd_Equal          = 0x0d,
    II_Kbd_Bksp           = 0x0e,
    II_Kbd_Tab            = 0x0f,
    II_Kbd_Q              = 0x10,
    II_Kbd_W              = 0x11,
    II_Kbd_E              = 0x12,
    II_Kbd_R              = 0x13,
    II_Kbd_T              = 0x14,
    II_Kbd_Y              = 0x15,
    II_Kbd_U              = 0x16,
    II_Kbd_I              = 0x17,
    II_Kbd_O              = 0x18,
    II_Kbd_P              = 0x19,
    II_Kbd_Sqr_Bcks_Open  = 0x1a,
    II_Kbd_Sqr_Bcks_Close = 0x1b,
    II_Kbd_Enter          = 0x1c,
    II_Kbd_Ctrl_L         = 0x1d,
    II_Kbd_A              = 0x1e,
    II_Kbd_S              = 0x1f,
    II_Kbd_D              = 0x20,
    II_Kbd_F              = 0x21,
    II_Kbd_G              = 0x22,
    II_Kbd_H              = 0x23,
    II_Kbd_J              = 0x24,
    II_Kbd_K              = 0x25,
    II_Kbd_L              = 0x26,
    II_Kbd_Semicolon      = 0x27,
    II_Kbd_Quotation      = 0x28,
    II_Kbd_Apostrophe     = 0x29,
    II_Kbd_Shift_L        = 0x2a,
    II_Kbd_Backslash      = 0x2b,
    II_Kbd_Z              = 0x2c,
    II_Kbd_X              = 0x2d,
    II_Kbd_C              = 0x2e,
    II_Kbd_V              = 0x2f,
    II_Kbd_B              = 0x30,
    II_Kbd_N              = 0x31,
    II_Kbd_M              = 0x32,
    II_Kbd_Comma          = 0x33,
    II_Kbd_Dot            = 0x34,
    II_Kbd_Slash          = 0x35,
    II_Kbd_Shift_R        = 0x36,
    II_Kbd_Asterisk       = 0x37,
    II_Kbd_Alt_L          = 0x38,
    II_Kbd_Space          = 0x39,
    II_Kbd_Caps           = 0x3a,
    II_Kbd_F1             = 0x3b,
    II_Kbd_F2             = 0x3c,
    II_Kbd_F3             = 0x3d,
    II_Kbd_F4             = 0x3e,
    II_Kbd_F5             = 0x3f,
    II_Kbd_F6             = 0x40,
    II_Kbd_F7             = 0x41,
    II_Kbd_F8             = 0x42,
    II_Kbd_F9             = 0x43,
    II_Kbd_F10            = 0x44,
    II_Kbd_NumLock        = 0x45,
    II_Kbd_rollLock       = 0x46,
    II_Kbd_Numpad_7       = 0x47,
    II_Kbd_Numpad_8       = 0x48,
    II_Kbd_Numpad_9       = 0x49,
    II_Kbd_Numpad_Minus   = 0x4a,
    II_Kbd_Numpad_4       = 0x4b,
    II_Kbd_Numpad_5       = 0x4c,
    II_Kbd_Numpad_6       = 0x4d,
    II_Kbd_Numpad_Plus    = 0x4e,
    II_Kbd_Numpad_1       = 0x4f,
    II_Kbd_Numpad_2       = 0x50,
    II_Kbd_Numpad_3       = 0x51,
    II_Kbd_Numpad_0       = 0x52,
    II_Kbd_Del            = 0x53,
    II_Kbd_F11            = 0x57,
    II_Kbd_F12            = 0x58,
    //II_Kbd_F12            = 0x59,
    II_Kbd_Slash_2        = KBD_EXT_INDEX_MASK|0x35,
    II_Kbd_Enter_2        = KBD_EXT_INDEX_MASK|0x1c,
    II_Kbd_Ins_2          = KBD_EXT_INDEX_MASK|0x52,
    II_Kbd_Del_2          = KBD_EXT_INDEX_MASK|0x53,
    II_Kbd_Home_2         = KBD_EXT_INDEX_MASK|0x47,
    II_Kbd_End_2          = KBD_EXT_INDEX_MASK|0x4f,
    II_Kbd_Pageup_2       = KBD_EXT_INDEX_MASK|0x49,
    II_Kbd_Pagedown_2     = KBD_EXT_INDEX_MASK|0x51,
    II_Kbd_Left           = KBD_EXT_INDEX_MASK|0x4b,
    II_Kbd_Right          = KBD_EXT_INDEX_MASK|0x4d,
    II_Kbd_Up             = KBD_EXT_INDEX_MASK|0x48,
    II_Kbd_Down           = KBD_EXT_INDEX_MASK|0x50,
    II_Kbd_Alt_R          = KBD_EXT_INDEX_MASK|0x38,
    II_Kbd_Ctrl_R         = KBD_EXT_INDEX_MASK|0x1d,
    II_Kbd_Pause          = KBD_EXT_INDEX_MASK|0x7f,

    INPUT_INDEX_KBD_MAX = 0b111111111,

    // Mouse
    II_Ms_Button1,
    II_Ms_Button2,
    II_Ms_Button3,
    II_Ms_Button4,
    II_Ms_Button5,

    // Xbox 360 controller
    II_Xbox_360_A,
    II_Xbox_360_B,
    II_Xbox_360_X,
    II_Xbox_360_Y,

    II_Xbox_360_LB,
    II_Xbox_360_RB,

    II_Xbox_360_Back,
    II_Xbox_360_Start,

    II_Xbox_360_LSB,
    II_Xbox_360_RSB,

    // Extended ipputs
    II_Ms_Wheel_V,
    II_Ms_Wheel_H,
    II_Ms_Movement,

    II_Xbox_360_LS,
    II_Xbox_360_RS,

    II_Xbox_360_LT,
    II_Xbox_360_RT,

    INPUT_INDEX_COUNT
} Input_Index;

#define EXTENDED_INPUT_INDEX_START II_Ms_Wheel_V
#define EXTENDED_INPUT_INDEX_COUNT (INPUT_INDEX_COUNT - EXTENDED_INPUT_INDEX_START)

Array_Ptr(Array_Input_Index, Input_Index);
Bit_Flags(Bit_Flags_Inputs_State, INPUT_INDEX_COUNT);

typedef union {
    struct {
        s32 move;
    };
    struct {
        s32 moveX;
        s32 moveY;
    };
} Input_Data, Action_Data;

#define ACTION_HELD_MASK  (0b00000000 << 8)
#define ACTION_BEGIN_MASK (0b01000000 << 8)
#define ACTION_END_MASK   (0b10000000 << 8)

#define ActionHeld(b, name) ActionHeld_##b(name)
#define ActionHeld_1(name) A_H_##name = (AI_##name)|ACTION_HELD_MASK,
#define ActionHeld_0(name)

#define ActionBegin(b, name) ActionBegin_##b(name)
#define ActionBegin_1(name) A_B_##name = (AI_##name)|ACTION_BEGIN_MASK,
#define ActionBegin_0(name)

#define ActionEnd(b, name) ActionEnd_##b(name)
#define ActionEnd_1(name) A_E_##name = (AI_##name)|ACTION_END_MASK,
#define ActionEnd_0(name)

#define EXTENDED_ACTION_INDEX_START AI_Move_Cam

typedef enum {
    AI_Nul = 0x00,

    #define X(enumName, begin, end, held) AI_##enumName,
    ACTIONS_LIST
    #undef X

    ACTION_COUNT,
} Action_Index_;

typedef u8 Action_Index;

Bit_Flags(Bit_Flags_Actions_State, ACTION_COUNT);

typedef enum {
    A_Nul = 0x00,

    #define X(enumName, begin, end, held) \
        ActionHeld(held, enumName) \
        ActionBegin(begin, enumName) \
        ActionEnd(end, enumName)
    ACTIONS_LIST
    #undef X
} Action_ID;

typedef struct {
    Action_ID ID;
    f32 timeStamp;
    Action_Data data;
} Action;

typedef struct {
    Action_ID actionIDAttached;
    Input_Data data;
} Extended_Input;

typedef enum {
    BT_In_Order,
    BT_In_Any_Order,
} Binding_Type;

typedef struct {
    Array_Input_Index inputs;
} Binding;

#define MAX_BINDINGS_PER_ACTION 64
Bit_Flags(Bit_Flags_Bindings_State, MAX_BINDINGS_PER_ACTION);

//Array_Ptr(Array_Binding, Binding);
typedef struct {
    u32 size;
    File_Ptr(Binding*, A);
} Array_Binding;

typedef struct {
    u8 actionType;
    Array_Binding bindings;
} Map_Action_Bindings;

typedef struct {
    Action_Index action;
    u8 binding;
} Binding_Indices;

//Array_Ptr(, Binding_Indices);
typedef struct {
    u32 size;
    File_Ptr(Binding_Indices*, A);
} Array_Binding_Indices;

typedef struct {
    u32 maxCount;
    u32 size;
    File_Ptr(Binding_Indices*, A);
} Input_Binding_Indices_Collection;

typedef struct {
    struct {
        Bit_Flags_Inputs_State inputsState;
        Bit_Flags_Bindings_State bindingsState[ACTION_COUNT];
    } /* Input_State */;

    Extended_Input extendedInputs[EXTENDED_INPUT_INDEX_COUNT];
    Input_Binding_Indices_Collection inputToBindings[INPUT_INDEX_COUNT];
    Map_Action_Bindings bindingsPerAction[ACTION_COUNT];

    u32 bindingIndicesCount;
} Input_Mapping;

typedef struct {
    u16 size;
    Action A[ACTION_COUNT];
} Held_Action_Buffer;

#define POOL_INPUT_FRAME_COUNT 256
typedef struct {
    u32 size;
    Action A[POOL_INPUT_FRAME_COUNT];
} Action_Pool;

typedef struct {
    Held_Action_Buffer heldBuffer;
    Action_Pool *pool;
} Msgs_Actions;

typedef struct {
    Bit_Flags_Actions_State state;
    Action_Pool *pool;
} Game_Actions;

Public void Input_Init();
Public Input_Mapping* Input_Get_Mappings();
Public inline u32 ExtendedInput(Input_Index inputIndex);
Public void Input_Update_Actions(Input_Index inInputIndex, b8 inIsPress, f32 inTimeStamp);
Public b8 Input_Text_Mode_Enabled();

// NOTE(JENH): rebinding and undo/redo structs.

typedef struct Node_Binding {
    Binding binding;
    struct Node_Binding *next;
} Node_Binding;

typedef struct Node_Binding_Indices {
    Binding_Indices indices;
    struct Node_Binding_Indices *next;
} Node_Binding_Indices;

typedef enum {
    RT_Append,
    RT_Del,
} Record_Type;

typedef struct Node_Records_Bindings {
    Record_Type type;
    Binding *record;
    struct Node_Records_Bindings *next;
} Node_Records_Bindings;

typedef struct {
    Node_Records_Bindings *tail;
    Node_Records_Bindings *list;
} List_Records_Bindings;

typedef struct {
    //Node_Binding *tail;
    u32 size;
    Node_Binding *list;
} List_Node_Binding;

typedef struct {
    u32 size;
    Node_Binding_Indices *list;
} List_Node_Binding_Indices;

typedef struct {
    List_Node_Binding bindings[ACTION_COUNT];
    List_Node_Binding_Indices inputs[INPUT_INDEX_COUNT];
} Input_Records_State;

typedef struct {
    Input_Records_State initState;
    List_Records_Bindings records[ACTION_COUNT];
} Input_Records;

#define CKF_LRShifts   CKF_LeftShift   | CKF_RightShift
#define CKF_LRControls CKF_LeftControl | CKF_RightControl
#define CKF_LRAlts     CKF_LeftAlt     | CKF_RightAlt

#endif //TE_INPUT_H
