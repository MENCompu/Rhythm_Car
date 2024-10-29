#ifndef RC_UI_WINDOW_H
#define RC_UI_WINDOW_H

#if 0
#define AXIS_HOR  0
#define AXIS_VERT 1

typedef enum {
    Direction_Left  = 0b00,
    Direction_Down  = 0b01,
    Direction_Right = 0b10,
    Direction_Up    = 0b11,
} Direction;

struct UI_Window;

typedef struct UI_Window_Node {
    File_Ptr(UI_Window*, window);
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
    UI_Control control;

    f32x4 color;

    Mesh mesh;
    Material material;

    File_Ptr(struct UI_Control*, number);
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

//Public void MoveScreen(UI_Window* inUIWindow, s32x2 cursorOffsetTopLeft, s32x2 cursorOffsetBottomRight);
//Public inline void MoveScreenIfNecessary(UI_Window* inUIWindow);

#if 0
typedef struct {
    v2_u32 position;
    v2_f32 relPositionForResizing;

    u32 savedPosXForVertMov;

    char *positionInText;

    v3_f32 color;
} Cursor;
#endif
#endif

#endif // RC_UI_WINDOW_H
