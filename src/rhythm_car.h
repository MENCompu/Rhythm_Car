#ifndef RHYTHMCAR_H
#define RHYTHMCAR_H

typedef struct {
    u32 samplesPerSecond;
    u32 sampleCount;
    s16* samples;
} Sound_Buffer;

typedef struct {
    f32x3 pos;
    f32x3 rotation;
    f32x3 forward;
    f32x3 right;
    f32x3 up;

    f32x4x4 viewMat;

    f32 speed;
} Camera;

typedef struct {
    UI_Text* score;
    UI_Text* time;
    UI_Text* ms;
} Debug_Text;

#if 0
typedef struct {
    Mesh mesh;
    f32x4 color;

    f32x4x4 model;
    f32 width;

    f32x3 endCurve;

    u32 lastUnitOffset;
    Asset_Handle unit;
} Road;
#endif

typedef struct Road_Point {
    f32x3 pos;
} Road_Point;

typedef struct {
    u32 count;
    Road_Point* E;
} Array_Road_Point;

typedef struct {
    u32 count;
    Array_f32x3 E[6];
} Array_Array_f32x3;

typedef struct {
    f32 distance;
    f32 endTime;
} Curve_Data;

typedef struct {
    u32 count;
    Curve_Data* E;
} Array_Curve_Data;

#if 1
typedef struct {
    u32 indexPoint;
    //Array_Road_Point points;
    Array_Array_f32x3 curvePoints;
    Array_Curve_Data curvesData;
} Road;
#endif

#if 0
typedef union {
    struct {
        f32x3 p0;
        f32x3 p1;
        f32x3 p2;
        f32x3 p3;
    };
    f32x3 E[4];
} Bezier_Curve_3D;
#endif

#if 0
typedef struct {
    Transform trans;
    f32x4 color;
    Asset_Handle model;
} Debug_Cube;

typedef struct {
    u32 count;
    Debug_Cube* E;
} Array_Debug_Cube;
#endif

typedef struct {
    f32 cubeScale;
    f32 axesDist;

    Basis_3D basis;
} Debug_Basis;

Array(Array_Debug_Basis, Debug_Basis);

typedef struct {
    Mesh perfect;
    Mesh good;
    Mesh bad;
} Road_Meshes;

typedef struct {
    b32 isInitialized;

    String binDir;
    s32x2 windowDims;

    Camera camera;

    f32x4x4 teapotModel;
    f32 worldToPixels;

    Frustum frustumProj;

    b8 debugMode;
    f32x3 pointerPos;

    Array_Debug_Basis debugBasis;

    Road road;

    Sound music;

    f32 tSin;

    Entity entities[32];

    f32 songTime;
    f32 time;

    f32 score;
    u64 totalScore;

    b8 isUIActive;
    Debug_Text debugText;

    UI_Window* currentWindow;
    UI_Text* textToWrite;

    u32 frameCounter;

    UI_Button* button;

    UI_Text* newControl;

    Mesh terrainMesh;
    Material terrainMaterial;

    Raw_Mesh roadUnitMesh;

    u32 prevVertexBase;

    f32x3* underCursorPoint;

    Road_Meshes roadMeshes;
    Vertex_Data roadVertices[4096];
    u32 roadIndices[4096];

    s64 initTimeRoad;
    b8 isDriving;

    f32 playerRot;
    f32 playerSpeed;

    f32x3 positionPlayer;
    f32x3 forwardPlayer;
    f32 t;

    u32 currentASIOIndex;
    u64 currentASIOTime;

    UI_Panel* audioPanel;
    u32 sampleIndex;
    Vertex_Data waveVertices[4096];
    u32 waveIndices[8192];
    Mesh waveMesh;

    b8 sponza;
} Game_State;

#endif //RHYTHMCAR_H
