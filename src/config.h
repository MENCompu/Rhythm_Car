#ifndef JENH_CONFIG_H

#define JENH_CONFIG_H

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN    1

#define CURRENT_VERSION 0

#define MAGIC_CFG Magic_4("jcfg")

// FieldOffset(UI_Window, control.prev), CI_UI_Controls, FieldOffset(UI_Window, control.next), CI_UI_Controls, \

// TODO(JENH): This is a hack.
Private Global u32 gStringTableSize = KiB(64); // UI_STRING_TABLE_SIZE
Private Global u32 gInputToBindingsSize = INPUT_INDEX_COUNT;

#define CONFIG_DEF_LIST \
    /* ID, type, elemSize, location, countLocation, (filePtrOffset, configID) */ \
    X(CI_UI_Windows     , CPET_Array, sizeof(UI_Window), &gUIWindowState.windows, &gUIWindowState.windowCount, \
      FieldOffset(UI_Window, control), CI_UI_Controls, FieldOffset(UI_Window, number), CI_UI_Controls, \
      FieldOffset(UI_Window, adjacentLeft.list), CI_UI_Window_Nodes, FieldOffset(UI_Window, adjacentLeft.tail), CI_UI_Window_Nodes, \
      FieldOffset(UI_Window, adjacentDown.list), CI_UI_Window_Nodes, FieldOffset(UI_Window, adjacentDown.tail), CI_UI_Window_Nodes, \
      FieldOffset(UI_Window, adjacentRight.list), CI_UI_Window_Nodes, FieldOffset(UI_Window, adjacentRight.tail), CI_UI_Window_Nodes, \
      FieldOffset(UI_Window, adjacentUp.list), CI_UI_Window_Nodes, FieldOffset(UI_Window, adjacentUp.tail), CI_UI_Window_Nodes) \
    X(CI_UI_Window_Nodes, CPET_Array, sizeof(UI_Window_Node), &gUIWindowState.nodeMem.pool, &gUIWindowState.nodeMem.nodeCount, \
      FieldOffset(UI_Window_Node, window), CI_UI_Windows, FieldOffset(UI_Window_Node, next), CI_UI_Window_Nodes) \
    X(CI_UI_Texts       , CPET_Array, sizeof(UI_Text), &gUIState.texts, &gUIState.textCount, FieldOffset(UI_Text, text.str), CI_UI_String) \
    X(CI_UI_Buttons     , CPET_Array, sizeof(UI_Button), &gUIState.buttons, &gUIState.buttonCount) \
    X(CI_UI_String      , CPET_Array, sizeof(char), &gUIState.stringTable, &gStringTableSize) \
    X(CI_UI_Controls    , CPET_Array, sizeof(UI_Control), &gUIState.controls, &gUIState.controlCount, \
      FieldOffset(UI_Control, prev), CI_UI_Controls, FieldOffset(UI_Control, next), CI_UI_Controls, \
      FieldOffset(UI_Control, data), CI_UI_Controls)

#if 0
    X(CI_Binding_Indices, CPET_Array, sizeof(Input_Binding_Indices_Collection), &gInputSystem.inputToBindings, &gInputToBindingsSize,
      FieldOffset(Input_Binding_Indices_Collection, A), CI_Binding_Indices)
    X(CI_Binding_Indices_Arrays, CPET_Array, sizeof(Binding_Indices), &gInputSystem.inputToBindings, &gInputToBindingsSize,
      FieldOffset(Input_Binding_Indices_Collection, A), CI_Binding_Indices)
#endif

typedef struct {
    u32 windowCount;

    u64 windows;

    u64 freeList;

    u64 windowNodePool;
} Config_Section_Headers;

typedef enum {
    #define X(configID, inType, inElemSize, location, countLocation, ...) configID,
    CONFIG_DEF_LIST
    #undef X

    CONFIG_ID_MAX,
} Config_ID;

typedef enum {
    CPET_Var,
    CPET_Array,
} Config_Def_Type;

// IMPORTANT: Don't change order of fields.
typedef struct {
    u16 offset;
    Config_ID ID;
} File_Ptr_Offset;

typedef struct {
    u16 count;
    File_Ptr(File_Ptr_Offset*, E);
} Array_File_Ptr_Offset;

typedef struct {
    Config_ID ID;
    Config_Def_Type type;

    u16 elemSize;

    union {
        struct {
            u32 count;
            u64 arrayOffset;
        };
    };

    Array_File_Ptr_Offset filePtrOffsets;
} Config_Def;

typedef struct {
    union {
        void* dataLoc;
        void* arrayLoc;
    };
    u32* countVarLoc;
} Config_Serialize_Data;

typedef struct {
    u32 magic;
    u8  version;
    u8  endiannes;
    u8  padding[8];

    u32 filePtrOffsetCount;
    u64 filePtrOffsetsOffset;

    u32 defCount;
    Config_Def defs[];

#if 0
    u64 freeList;

    u32 windowCount;
    u64 windows;

    u32 nodeCount;
    u64 windowNodePool;

    u32 controlCount;
    u64 controls;

    u32 textCount;
    u64 texts;

    u32 buttonCount;
    u64 buttons;

    u32 uiStringTableSize;
    u64 uiStringTable;

    Free_List uiFreeList;
#endif
} Config_Header;

#if 0
    CI_UI_Windows      = 0x01,
    CI_UI_Window_Nodes = 0x02,
    CI_UI_Controls     = 0x03,
    CI_UI_Texts        = 0x04,
    CI_UI_Buttons      = 0x05,
    CI_UI_String       = 0x06,
#endif

#endif // JENH_CONFIG_H
