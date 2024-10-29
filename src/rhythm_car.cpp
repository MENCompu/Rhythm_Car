/*
 * TODO list:
 *  - Input System.
 *    - Handle extended input begin and end properly.
 *    - Revisit queue. It's better to separate (extended / begin, end / held) actions from each other or mix up.
 *    - synchronization:
 *      - handle each input async.
 *      - taking care of input in frame boundary and timestamps.
 *    - support much more devices.
 *  - Hot reloading and extern development tools
 *    - memory layout agnostic hot reloading.
 *    - Better global variables handeling when hot reloading.
 *    - go and rebuild yourself at runtime.
 *  - Save system.
 *    - robust and binary format.
 *    - version agnostic parser.
 *    - Handle crashes.
 *  - Debugging tools
 *    - See memory usage/allocations categorize by types.
 */

//#define BUILD_32
#define BUILD_64

#define ASPECT_RATIO (16.0f / 9.0f)
#define DEADZONE 8192

#define ROAD_WIDTH 80.0f

#define ROAD_WIDTH_PERFECT (ROAD_WIDTH * 0.3f)
#define ROAD_WIDTH_GOOD (ROAD_WIDTH * 0.7f)
#define ROAD_WIDTH_BAD (ROAD_WIDTH * 1.0f)

#if 1
#if 0
#pragma warning(push, 1)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#include <windows.h>

#include <asio.cpp>
#include <asiodrivers.cpp>
#include <asiolist.cpp>

#pragma warning(pop)
#endif

#include <third_party.h>
#include <time.h>

#pragma warning(push, 1)
#define assert(...)
#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"
#pragma warning(pop)

#if 1
#include "Utils.h"
#include "types.h"
#include "Memory_Utils.h"
#include "String.h"
#include "logger.h"
#include "Math.h"
#include "Vectors.h"
#include "file_system.h"
#include "windows_layer.h"
#include "time.h"
#include "file_format_utils.h"
#include "virtual_mem_allocator.h"
#else
#include <menc_lib.h>
#endif

#include "intrinsics.c"

#include "profiler.h"
#include "profiler.c"

// TODO(JENH): This should be better handled.
Public Global volatile b8 converterFinish;

#include "base_bin.h"
#include "Memory_Managment.h"

#include "logger.c"

#include "free_list.h"
#include "free_list.c"

#include "gp_allocator.h"
#include "gp_allocator.c"

//#include "base_thread.h"

#define MAIN_DLL_EXPORT
#include "main_dll.h"

#define JENH_MATRICES_IMPL
#include "matrices.h"

#include "Input.h"

#include "vulkan.h"
#include "GAPI_Interface_Types.h"
#include "asset_system.h"
#include "render.h"

#include "collision_system.h"
#include "collision_system.c"

#include "GAPI_Interface.h"
#include "parse.h"
#include "ui_window.h"
#include "ui.h"
#include "time.c"
#include "asset_converter.h"

#include "entity.h"
#include "input.h"
#include "Config.h"

#include "rhythm_car.h"
Private Global Game_State gGameState;

Public Fn_Prot_UI_On_Resize(Scene_Window_On_Resize);
Public Fn_Prot_UI_Cursor_Click(UI_Text_Cursor_Click) {
    gGameState.textToWrite = (UI_Text*)UI_Control_Get_Data(inControl);
}

#include "ui_window.cpp"

#include "ui.cpp"
#include "texture_loader.cpp"
#include "mesh_converter.cpp"
#include "audio_loader.cpp"
#include "shader_loader.cpp"
#include "font_converter.cpp"
#include "asset_system.cpp"
#include "asset_converter.cpp"

#include "application.c"
Public void GetSoundSamples(Sound_Buffer* soundBuffer);

#include "input.c"
#include "hid.h"
#include "windows_layer.cpp"
#include "file_system.c"

//#include "vulkan.cpp"
#include "render.cpp"
#include "entity.c"

#include "Config.cpp"

//#include "audio.cpp"
#endif

Private Global s16 sineSamples[4096];

Public void Create_Sine_Wave() {
    //u32 wavePeriod = (u32)((f32)ArrayCount(sineSamples) / pitchHz);
    f32 pitchHz = 110;
    u32 toneVolume = 300;

    f32 t = 0.0f;

    Foreach (s16, sample, sineSamples, ArrayCount(sineSamples)) {
        *sample = (s16)((f32)toneVolume * Sin32(t));

        t += (pitchHz * (2.0f * JENH_PI)) / ArrayCount(sineSamples);
        if (t > 2.0f * JENH_PI) {
            t -= 2.0f * JENH_PI;
        }
    }
}

#if 0
Public void Draw_Audio_Wave(s16* inAudioWave, u32 inSampleCount, s16 inMinAmplitude, s16 inMaxAmplitude, Mesh* outMesh) {
    f32x2 center   = gGameState.audioPanel->control->absTrans.position.xy;
    f32x2 halfDims = F32x2_Elem_Prod(gGameState.audioPanel->control->halfDims, gGameState.audioPanel->control->absTrans.scale.xy);

    s16 prevSample = inAudioWave[0];

    f32 posX = center.x - halfDims.x;
    f32 posY = center.y + ( ( Is_Pos(prevSample) ) ? (f32)prevSample / (f32)inMaxAmplitude * halfDims.y :
                             -((f32)prevSample / (f32)inMinAmplitude * halfDims.y ));

    Vertex_Data* prevVertex = &gGameState.waveVertices[0];
    prevVertex->pos = F32x3(posX, posY, -1.0f);
    prevVertex->normal = { 0 };
    prevVertex->tex = { 0 };

    for (u32 i = 1; i < inSampleCount; ++i) {
        s16 sample = inAudioWave[i];

        f32 t = (f32)i / (f32)inSampleCount;
        posX = (center.x - halfDims.x) * (1.0f  - t) + t * (center.x + halfDims.x);
        posY = center.y + ( ( Is_Pos(sample) ) ? (f32)sample / (f32)inMaxAmplitude * halfDims.y :
                                 -((f32)sample / (f32)inMinAmplitude * halfDims.y ));

        Vertex_Data* vertex = &gGameState.waveVertices[i];
        vertex->pos = F32x3(posX, posY, -1.0f);
        vertex->normal = { 0 };
        vertex->tex = { 0 };

        gGameState.waveIndices[(2 * (i - 1)) + 0] = i - 1;
        gGameState.waveIndices[(2 * (i - 1)) + 1] = i;

        prevVertex = vertex;
        prevSample = sample;
    }

    if ( outMesh->vertexOffset != 0 ) {
        GAPI_Mesh_Destroy(outMesh);
    }

    GAPI_Mesh_Create(gGameState.waveVertices, gGameState.waveIndices, outMesh);
}
#endif

Private void Coll_Bezier_Curve_Point() {
    Foreach (Array_f32x3, bezCurve, gGameState.road.curvePoints.E, gGameState.road.curvePoints.count) {
        Foreach (f32x3, point, bezCurve->E, bezCurve->count) {
            f32 scale = 15.0f;

            Transform cubeTrans;
            cubeTrans.position = *point;
            cubeTrans.scale = F32x3(scale, scale, scale);
            cubeTrans.rotation = F32x3(0.0f, 0.0f, 0.0f);

            Model* model = Asset_System_Get_Model(Converter_Get_Asset_Handle(LitToStr("cube.obj")));
            Mesh* mesh = &model->meshes[0];

            f32x3 center = (CreateSRTModel(&cubeTrans) * F32x4(mesh->center, 1.0f)).xyz;
            f32x3 halfDims = (CreateSRTModel(&cubeTrans) * F32x4(mesh->halfDim, 0.0f)).xyz;

            Coll_Rect_3D rect;
            Coll_Rect_3D_Create(center, halfDims, &rect);

            if ( Coll_3D_Is_Intersecting_Rect_Point(&rect, gGameState.pointerPos) ) {
                gGameState.underCursorPoint = point;
            }
        }
    }
}

Private void Update_Cursor_Position() {
    POINT cursorPoint;
    Win32_Check( GetCursorPos(&cursorPoint), > 0 );
    f32x2 cursorPos = F32x2((f32)cursorPoint.x, (f32)cursorPoint.y);

    //Point_To_Camera_Space(cursorPos);

    s32x2 winDimsI = OS_Window_Get_Dims();
    f32x2 winDims = F32x2((f32)winDimsI.x, (f32)winDimsI.y);

    f32x2 NDCPoint = (2.0f * ElemDiv(cursorPos, winDims)) - F32x2(1.0f, 1.0f);
    // NOTE(JENH): y cursor position goes from top to bottom.
    NDCPoint.y *= -1.0f;

    // TODO(JENH): This is asumming that right == -left and top == -bottom;
    f32x2 halfNearPlaneDims = F32x2(gGameState.frustumProj.right, gGameState.frustumProj.top);
    f32x3 projPoint = Normalize(F32x3(ElemProd(halfNearPlaneDims, NDCPoint), gGameState.frustumProj.nearPlane));

    Camera *camera = &gGameState.camera;

    f32x4x4 camTrans = GetCameraTransform(&gGameState.camera);
    f32x4 vectorFromCam = camTrans * F32x4(projPoint, 0.0f);

    f32 x0 = gGameState.camera.pos.x;
    f32 x1 = vectorFromCam.x;
    f32 y0 = gGameState.camera.pos.y;
    f32 y1 = vectorFromCam.y;
    f32 z0 = gGameState.camera.pos.z;
    f32 z1 = vectorFromCam.z;

    f32x3 groundPoint = F32x3((x1 / y1) * (-y0) + x0, 0.0f, (z1 / y1) * (-y0) + z0);

    gGameState.pointerPos = groundPoint;
}

Private void Draw_Debug_Cube(f32x3 pos, f32 scale, f32x4 inColor, f32x3 rotation = {0.0f, 0.0f, 0.0f}) {
    Transform trans;
    trans.position = pos;
    trans.scale = F32x3(scale, scale, scale);
    trans.rotation = rotation;

    Model* model = Asset_System_Get_Model(Converter_Get_Asset_Handle(LitToStr("cube.obj")));

    f32x4x4* PVM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PVM = gRendererState.projection * gRendererState.view * CreateSRTModel(&trans);

    Renderer_Draw_Data* drawData = Array_Push(gRendererState.opaqueObjects, &gRendererState.opaqueObjectCount);

    Material* material = Renderer_Material_From_Color(inColor, PI_Material);

    drawData->PVM  = PVM;
    drawData->mesh = &model->meshes[0];
    drawData->material = material;
    drawData->pipelineID = PI_Material;
}

Intern void PushDebugBasis(f32x3 pos, f32x3 xAxis, f32x3 yAxis, f32x3 zAxis, f32 cubeScale, f32 axesDist) {
    Debug_Basis *debugBasis = &gGameState.debugBasis.A[gGameState.debugBasis.size++];

    debugBasis->basis.o = pos;
    debugBasis->basis.axes.x = xAxis;
    debugBasis->basis.axes.y = yAxis;
    debugBasis->basis.axes.z = zAxis;

    debugBasis->cubeScale = cubeScale;
    debugBasis->axesDist = axesDist;
}

Intern void UpdateCameraAxes(Camera *camera) {
    camera->forward = F32x3(Cos32(camera->rotation.yaw) * Cos32(camera->rotation.pitch),
                            Sin32(camera->rotation.pitch),
                            Sin32(camera->rotation.yaw) * Cos32(camera->rotation.pitch));

    f32x3 upWorld = F32x3(0.0f, 1.0f, 0.0f);
    camera->right = Normalize(Cross_f32x3(camera->forward, upWorld));
    camera->up = Cross_f32x3(camera->right, camera->forward);
}

Private void Generate_Road_Geometry(f32 inScale, f32 inHeight, Mesh* outMesh) {
    // rebuild road mesh.
    if ( outMesh->vertexCount != 0 ) {
        GAPI_Mesh_Destroy(outMesh);
    }

    outMesh->vertexCount = 0;
    outMesh->indexCount  = 0;
    gGameState.prevVertexBase = 0;

    Foreach (Array_f32x3, bezCurve, gGameState.road.curvePoints.E, gGameState.road.curvePoints.count) {
        Assert( ((bezCurve->count - 1) % 3) == 0 );
        b8 hola = JENH_TRUE;

        Foreach (u32, unitIndex, (u32*)gGameState.roadUnitMesh.indices, gGameState.roadUnitMesh.indexCount) {
            u32* newIndex = Array_Push(gGameState.roadIndices, &outMesh->indexCount);
            *newIndex = outMesh->vertexCount + *unitIndex;
        }

        for (u32 i = 0; (i + 1) < bezCurve->count; i += 3) {
            f32x3* curve = &bezCurve->E[i];

            u32 j = 1;

            if ( hola ) {
                hola = JENH_FALSE;
                j = 0;
            }

            u32 steps = 10;
            for (; j < steps; ++j) {
                f32 t = (f32)j * (1.0f / (f32)(steps - 1));
                f32x3 newPos = BezierCurve3(t, curve[0], curve[1], curve[2], curve[3]);

                f32x3 zAxis = Normalize(-BezierCurve3Tan(t, curve[0], curve[1], curve[2], curve[3]));
                f32x3 xAxis = Normalize(Cross(F32x3(0.0f, 1.0f, 0.0f), zAxis));
                f32x3 yAxis = Cross(zAxis, xAxis);

                f32 normalScale = 10.0f;

                f32x4x4 unitTrans = {
#if 0
                    F32x4(xAxis, 0.0f),
                    F32x4(yAxis, 0.0f),
                    F32x4(zAxis, 0.0f),
#else
                    F32x4(F32x3_Elem_Prod(xAxis, F32x3(inScale, inScale, inScale)), 0.0f),
                    F32x4(F32x3_Elem_Prod(yAxis, F32x3(normalScale, normalScale, normalScale)), 0.0f),
                    F32x4(F32x3_Elem_Prod(zAxis, F32x3(normalScale, normalScale, normalScale)), 0.0f),
#endif
                    F32x4(newPos, 1.0f),
                };

                u32 vertexBase = outMesh->vertexCount;

                Foreach (Vertex_Data, unitVertex, (Vertex_Data*)gGameState.roadUnitMesh.vertices, gGameState.roadUnitMesh.vertexCount) {
                    Vertex_Data* newVertex = Array_Push(gGameState.roadVertices, &outMesh->vertexCount);

                    newVertex->pos = (unitTrans * F32x4(unitVertex->pos, 1.0f)).xyz;
                    newVertex->pos.y *= inHeight;
                    newVertex->normal = unitVertex->normal;
                    newVertex->tex = unitVertex->tex;
                }

                u32 indicesToMerge[] =  { 1, 3, 0, 2, 1 };

                for (u32 k = 0; k < ArrayCount(indicesToMerge) - 1; ++k) {
                    u32 baseIndex = outMesh->indexCount;
                    outMesh->indexCount += 6;

                    gGameState.roadIndices[baseIndex + 0] = vertexBase + indicesToMerge[k + 0];
                    gGameState.roadIndices[baseIndex + 1] = gGameState.prevVertexBase + indicesToMerge[k + 0];
                    gGameState.roadIndices[baseIndex + 2] = vertexBase + indicesToMerge[k + 1];

                    gGameState.roadIndices[baseIndex + 3] = gGameState.prevVertexBase + indicesToMerge[k + 1];
                    gGameState.roadIndices[baseIndex + 4] = vertexBase + indicesToMerge[k + 1];
                    gGameState.roadIndices[baseIndex + 5] = gGameState.prevVertexBase + indicesToMerge[k + 0];
                }

                gGameState.prevVertexBase = vertexBase;
            }
        }

        Foreach (u32, unitIndex, (u32*)gGameState.roadUnitMesh.indices, gGameState.roadUnitMesh.indexCount) {
            u32* newIndex = Array_Push(gGameState.roadIndices, &outMesh->indexCount);
            *newIndex = gGameState.prevVertexBase + *unitIndex;
        }

        gGameState.prevVertexBase += 4;
    }

    GAPI_Mesh_Create(gGameState.roadVertices, gGameState.roadIndices, outMesh);

}

Private f32 Bezier_Curve_From_Time(f32 inT, f32x3** outCurve) {
    Array_f32x3* bezCurve = &gGameState.road.curvePoints.E[0];

    Curve_Data* curveData = 0;
    f32x3* curve = 0;
    u32 index = 0;

    for (u32 i = 0; i < gGameState.road.curvesData.count; ++i) {
        Curve_Data* data = &gGameState.road.curvesData.E[i];

        if ( inT <= data->endTime ) {
            index = i;
            curveData = data;
            *outCurve = &bezCurve->E[i * 3];
            break;
        }
    }

    Assert( curveData );

    f32x3 prevPoint = BezierCurve3(0.0f, (*outCurve)[0], (*outCurve)[1], (*outCurve)[2], (*outCurve)[3]);

    f32 distanceAtT;
    if ( index == 0 ) {
        f32 timeRange = curveData->endTime;
        f32 t = inT;

        distanceAtT = (t / timeRange) * curveData->distance;
    } else {
        f32 timeRange = curveData->endTime - gGameState.road.curvesData.E[index - 1].endTime;
        f32 t = inT - gGameState.road.curvesData.E[index - 1].endTime;

        distanceAtT = (t / timeRange) * curveData->distance;
    }

    f32 distance = 0.0f;

    u32 steps = 10000;
    for (u32 i = 1; i < steps; ++i) {
        f32 t = (f32)i * (1.0f / (f32)(steps - 1));
        f32x3 newPos = BezierCurve3(t, (*outCurve)[0], (*outCurve)[1], (*outCurve)[2], (*outCurve)[3]);

        distance += Len(newPos - prevPoint);
        prevPoint = newPos;

        if ( distanceAtT <= distance ) {
            return t;
        }
    }

    LogError("Failed to map correctly the current song time to the road curve");
    return 0.0f;

#if 0
    f32x3 v1 = -3.0f * curve[0] + 9.0f  * curve[1] - 9.0f * curve[2] + 3.0f * curve[3];
    f32x3 v2 =  6.0f * curve[0] - 12.0f * curve[1] + 6.0f * curve[2];
    f32x3 v3 = -3.0f * curve[0] + 3.0f  * curve[1];

    f32 tInc = (gGameState.playerSpeed * deltaTime) / Len(Sq32(gGameState.t) * v1 + gGameState.t * v2 + v3);

    if ( (gGameState.t + tInc) > 1.0f ) {
        gGameState.initTimeRoad = Time_OS_Counter();
        gGameState.initIndexCurve += 3;
        if ( (gGameState.initIndexCurve + 1) == gGameState.road.curvePoints.E[0].count ) {
            gGameState.isDriving = JENH_FALSE;
            goto done_driving;
        }

        f32 movRemainder = (gGameState.playerSpeed * deltaTime) -
                           Len(curve[3] - BezierCurve3(gGameState.t, curve[0], curve[1], curve[2], curve[3]));

        curve = &gGameState.road.curvePoints.E[0].E[gGameState.initIndexCurve];

        f32x3 v1 = -3.0f * curve[0] + 9.0f  * curve[1] - 9.0f * curve[2] + 3.0f * curve[3];
        f32x3 v2 =  6.0f * curve[0] - 12.0f * curve[1] + 6.0f * curve[2];
        f32x3 v3 = -3.0f * curve[0] + 3.0f  * curve[1];

        // TODO(JENH): Should recalculate the t but instead of using the plain speed use the remainder distance
        //             substracted form the prev curve movement.
        //             This Works weird actually.
        gGameState.t = movRemainder / Len(Sq32(gGameState.t) * v1 + gGameState.t * v2 + v3);
    } else {
        gGameState.t += tInc;
    }
#endif
}

Private void Update_Total_Road_Distance() {
    Array_f32x3* bezCurve = &gGameState.road.curvePoints.E[0];

    f32x3 prevPoint = BezierCurve3(0.0f, bezCurve->E[0], bezCurve->E[1], bezCurve->E[2], bezCurve->E[3]);

    f32 totalDistance = 0.0f;

    u32 bezierCurveCount = (bezCurve->count - 1) / 3;
    gGameState.road.curvesData.count = bezierCurveCount;

    // Cleanup curve data
    Foreach (Curve_Data, curveData, gGameState.road.curvesData.E, gGameState.road.curvesData.count) {
        *curveData = { 0 };
    }

    for (u32 i = 0; i < bezierCurveCount; ++i) {
        f32x3* curve = &bezCurve->E[i * 3];

        u32 steps = 100;
        for (u32 j = 1; j < steps; ++j) {
            f32 t = (f32)j * (1.0f / (f32)(steps - 1));
            f32x3 newPos = BezierCurve3(t, curve[0], curve[1], curve[2], curve[3]);

            // TODO(JENH): This is a hack. Here I am saving the lengths at the place of times to later compute the actual times.
            gGameState.road.curvesData.E[i].distance += Len(newPos - prevPoint);

            prevPoint = newPos;
        }

        totalDistance += gGameState.road.curvesData.E[i].distance;
    }

    f32 time = 0.0f;
    Foreach (Curve_Data, curveData, gGameState.road.curvesData.E, gGameState.road.curvesData.count) {
        time += (curveData->distance / totalDistance) * gGameState.songTime;
        curveData->endTime = time;
    }

    gGameState.totalScore = (u64)totalDistance * 100;
    gGameState.playerSpeed = totalDistance / gGameState.songTime;
}

Private void Set_UI_Inputs() {
    if ( gGameState.isUIActive ) {
        Input_Action_Change_Entry entries[] = {
            { IACT_Move_To_Top, AI_UI_Move },
            { IACT_Move_To_Top, AI_UI_Click },
            { IACT_Move_To_Top, AI_UI_Cursor },
            { IACT_Move_To_Top, AI_UI_Create },
        };
        Input_Change_Actions(entries, ArrayCount(entries));
    } else {
        Input_Action_Change_Entry entries[] = {
            { IACT_Move_To_Btm, AI_UI_Move },
            { IACT_Move_To_Btm, AI_UI_Click },
            { IACT_Move_To_Btm, AI_UI_Cursor },
            { IACT_Move_To_Btm, AI_UI_Create },
        };
        Input_Change_Actions(entries, ArrayCount(entries));
    }
}

Intern void Input_Process(f32 deltaTime) {
    Game_Actions *actions = &gInputSystem.gameActions;

    foreach (Action, action, *actions->pool) {
        switch (action->ID) {
            case A_B_Move_Forward: {
                //LogDebug("forward");
                goto held_move_forward;
            } break;

            case A_H_Move_Forward: {
                held_move_forward:
                MoveCamera(&gGameState.camera, Forward, deltaTime);
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_B_Move_Backward: {
                //LogDebug("backward");
                goto held_move_backward;
            } break;

            case A_H_Move_Backward: {
                held_move_backward:
                MoveCamera(&gGameState.camera, Backward, deltaTime);
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_B_Move_Up: {
                //LogDebug("up");
                goto held_move_up;
            } break;

            case A_H_Move_Up: {
                held_move_up:
                MoveCamera(&gGameState.camera, Up, deltaTime);
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_B_Move_Down: {
                //LogDebug("down");
                goto held_move_down;
            } break;

            case A_H_Move_Down: {
                held_move_down:

                if ( !gGameState.debugMode ) {
                    MoveCamera(&gGameState.camera, Down, deltaTime);
                }
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_B_Move_Left: {
                //LogDebug("left");
                goto held_move_left;
            } break;

            case A_H_Move_Left: {
                held_move_left:
                MoveCamera(&gGameState.camera, Left, deltaTime);
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_B_Move_Right: {
                //LogDebug("right");
                goto held_move_right;
            } break;

            case A_H_Move_Right: {
                held_move_right:
                MoveCamera(&gGameState.camera, Right, deltaTime);
                //LogDebug("cam pos: (%.2f, %.2f, %.2f)", gGameState.camera.pos.x, gGameState.camera.pos.y, gGameState.camera.pos.z);
            } break;

            case A_H_Player_Move: {
                //f32 rot = ((f32)(action->data.moveX - (Sign(action->data.moveX) * DEADZONE)) / (f32)(MAX_S16 - DEADZONE));
                //LogInfo("rot: %f", rot);
                if ( gGameState.isDriving ) {
                    gGameState.playerRot -= ((f32)(action->data.moveX - (Sign(action->data.moveX) * DEADZONE)) / (f32)(MAX_S16 - DEADZONE))
                                            * Radians(2.0f);
                }
            } break;

#if 1
            case A_H_Player_Left: {
                if ( gGameState.isDriving ) {
                    gGameState.playerRot += Radians(2.0f);
                }
            } break;

            case A_H_Player_Right: {
                if ( gGameState.isDriving ) {
                    gGameState.playerRot -= Radians(2.0f);
                }
            } break;
#endif

            case A_B_Move_Cam: {
                //LogDebug("hola");
            } break;

            case A_H_Move_Cam: {
                f32 pitch = -((f32)action->data.moveY * 0.001f);
                f32 yaw   =  ((f32)action->data.moveX * 0.001f);

                gGameState.camera.rotation += F32x3(pitch, yaw, 0.0f);

                f32 epsilon = 0.01f;
                f32 d90 = 0.5f * JENH_PI;
                f32 d360 = 2.0f * JENH_PI;
                gGameState.camera.rotation.pitch = Clip(-d90 + epsilon, gGameState.camera.rotation.pitch, d90 - epsilon);
                if (gGameState.camera.rotation.yaw < -d360) {
                    gGameState.camera.rotation.yaw += -d360;
                } else if (gGameState.camera.rotation.yaw > d360) {
                    gGameState.camera.rotation.yaw -= d360;
                }

                UpdateCameraAxes(&gGameState.camera);

                //LogDebug("x axis: (%.2f, %.2f, %.2f)", camera->right.x, camera->right.y, camera->right.z);
                //LogDebug("y axis: (%.2f, %.2f, %.2f)", camera->up.x, camera->up.y, camera->up.z);
                //LogDebug("z axis: (%.2f, %.2f, %.2f)", camera->forward.x, camera->forward.y, camera->forward.z);
            } break;

            case A_B_Debug_Mode: {
                gGameState.debugMode = !gGameState.debugMode;

                if ( gGameState.debugMode ) {
                    SetMouseAbsolute();
                    if ( gGameState.isUIActive ) {
                        Input_Action_Change_Entry entries[] = {
                            { IACT_Move_To_Top, AI_UI_Cursor },
                        };
                        Input_Change_Actions(entries, ArrayCount(entries));
                    } else {
                        Input_Action_Change_Entry entries[] = {
                            { IACT_Move_To_Top, AI_Debug_Pointer },
                        };
                        Input_Change_Actions(entries, ArrayCount(entries));
                    }
                } else {
                    SetMouseRelative();
                    Input_Action_Change_Entry entries[] = {
                        { IACT_Move_To_Top, AI_Move_Cam },
                    };
                    Input_Change_Actions(entries, ArrayCount(entries));
                }
            } break;

            case A_E_Debug_Mode: {
#if 0
                SetMouseRelative();
                gGameState.debugMode = JENH_FALSE;
#endif
            } break;

            case A_B_Sprint: {
                gGameState.camera.speed = 350.0f;
            } break;

            case A_E_Sprint: {
                gGameState.camera.speed = 200.0f;
            } break;

            case A_B_UI_Click: {
                UI_Cursor_Click();

#if 0
                if ( gUIState.selected ) {
                    Input_Action_Change_Entry entries[] = {
                        { IACT_Move_To_Top, AI_UI_Move },
                    };
                    Input_Change_Actions(entries, ArrayCount(entries));
                }
#endif
            } break;

            case A_B_UI_Move: {
                UI_Select_From_Cursor();

                if ( !gUIState.selected ) {
                    //Input_Disable_Held_Action();
                }
            } break;

            case A_H_UI_Move: {
                if ( gUIState.isSomeEdgeLineSelected ) {
                    s32x2 winDims = OS_Window_Get_Dims();
                    f32x2 mousePoint = F32x2((((f32)action->data.moveX / (f32)winDims.width * 2.0f) - 1.0f) * ASPECT_RATIO,
                                             (((f32)winDims.height - (f32)action->data.moveY) / (f32)winDims.height) * 2.0f - 1.0f);

                    Transform* trans = &gUIState.selected->absTrans;

                    Direction dir = gUIState.selectedLineDirection;

                    Axis axis = Axis_Form_Direction(dir);
                    f32 sign = ( dir & 0b10 ) ? +1.0f : -1.0f;

                    f32 newScaleDif = ((F32_Abs(mousePoint.E[axis] - trans->position.E[axis]) / gUIState.selected->halfDims.E[axis])
                                      - trans->scale.E[axis]) * 0.5f;

                    trans->scale.E[axis] += newScaleDif;
                    trans->position.E[axis] += sign * newScaleDif;

                    UI_Control_Set_Relative(gUIState.selected);
                } else if ( gUIState.selected ) {
                    s32x2 winDims = OS_Window_Get_Dims();
                    f32x2 mousePoint = F32x2((((f32)action->data.moveX / (f32)winDims.width * 2.0f) - 1.0f) * ASPECT_RATIO,
                                             (((f32)winDims.height - (f32)action->data.moveY) / (f32)winDims.height) * 2.0f - 1.0f);

                    gUIState.selected->absTrans.position.xy = mousePoint;
                    UI_Control_Set_Relative(gUIState.selected);
                }
            } break;

            case A_H_UI_Cursor: {
                s32x2 winDims = OS_Window_Get_Dims();
                f32x2 mousePoint = F32x2((((f32)action->data.moveX / (f32)winDims.width * 2.0f) - 1.0f) * ASPECT_RATIO,
                                         (((f32)winDims.height - (f32)action->data.moveY) / (f32)winDims.height) * 2.0f - 1.0f);

                UI_Cursor_Update(mousePoint);
            } break;

            case A_B_UI_Create: {
                Transform trans;
                trans.position = F32x3(0.02f, -0.02f, -10.0f);
                trans.scale = F32x3(0.05f, 0.05f, 0.05f);
                trans.rotation = F32x3(0.0f, 0.0f, 0.0f);

                gGameState.newControl = UI_Text_Create(LitToStr("new text"), JENH_TRUE, Converter_Get_Asset_Handle(LitToStr("font.fnt")), &trans,
                                                       0, 0, URWO_Nul, 0, 0);
            } break;

            case A_B_UI_Mode: {
                gGameState.isUIActive = !gGameState.isUIActive;
                Set_UI_Inputs();
            } break;

            case A_B_Move_Curve: {
                Coll_Bezier_Curve_Point();
            } break;

            case A_H_Move_Curve: {
                Update_Cursor_Position();

                if ( gGameState.underCursorPoint ) {
                    *gGameState.underCursorPoint = gGameState.pointerPos;

                    Generate_Road_Geometry(ROAD_WIDTH_PERFECT, 0.7f, &gGameState.roadMeshes.perfect);
                    Generate_Road_Geometry(ROAD_WIDTH_GOOD, 0.6f, &gGameState.roadMeshes.good);
                    Generate_Road_Geometry(ROAD_WIDTH_BAD, 0.5f, &gGameState.roadMeshes.bad);

                    Update_Total_Road_Distance();
                }
            } break;

            case A_B_Split_Left: {
                gGameState.currentWindow = UI_Window_Split(gGameState.currentWindow, Direction_Left, F32x4(0.5f, 0.5f, 0.5f, 1.0f), Stub_On_Resize);
            } break;

            case A_B_Split_Down: {
                gGameState.currentWindow = UI_Window_Split(gGameState.currentWindow, Direction_Down, F32x4(0.5f, 0.5f, 0.5f, 1.0f), Stub_On_Resize);
            } break;

            case A_B_Split_Right: {
                gGameState.currentWindow = UI_Window_Split(gGameState.currentWindow, Direction_Right, F32x4(0.5f, 0.5f, 0.5f, 1.0f), Stub_On_Resize);
            } break;

            case A_B_Split_Up: {
                gGameState.currentWindow = UI_Window_Split(gGameState.currentWindow, Direction_Up, F32x4(0.5f, 0.5f, 0.5f, 1.0f), Stub_On_Resize);
            } break;

            case A_B_Window_Left: {
                if ( gGameState.currentWindow->adjacentLeft.first ) {
                    gGameState.currentWindow = gGameState.currentWindow->adjacentLeft.first->window;
                }
            } break;

            case A_B_Window_Down: {
                if ( gGameState.currentWindow->adjacentDown.first ) {
                    gGameState.currentWindow = gGameState.currentWindow->adjacentDown.first->window;
                }
            } break;

            case A_B_Window_Up: {
                if ( gGameState.currentWindow->adjacentUp.first ) {
                    gGameState.currentWindow = gGameState.currentWindow->adjacentUp.first->window;
                }
            } break;

            case A_B_Window_Right: {
                if ( gGameState.currentWindow->adjacentRight.first ) {
                    gGameState.currentWindow = gGameState.currentWindow->adjacentRight.first->window;
                }
            } break;

            case A_H_Size_Left: {
                if ( gGameState.frameCounter == 0 || gGameState.frameCounter >= 30 ) {
                    UI_Window_Resize(gGameState.currentWindow, 0.01f, Direction_Left);
                }

                ++gGameState.frameCounter;
            } break;

            case A_H_Size_Down: {
                if ( gGameState.frameCounter == 0 || gGameState.frameCounter >= 30 ) {
                    UI_Window_Resize(gGameState.currentWindow, 0.01f, Direction_Down);
                }

                ++gGameState.frameCounter;
            } break;

            case A_H_Size_Right: {
                if ( gGameState.frameCounter == 0 || gGameState.frameCounter >= 30 ) {
                    UI_Window_Resize(gGameState.currentWindow, 0.01f, Direction_Right);
                }

                ++gGameState.frameCounter;
            } break;

            case A_H_Size_Up: {
                if ( gGameState.frameCounter == 0 || gGameState.frameCounter >= 30 ) {
                    UI_Window_Resize(gGameState.currentWindow, 0.01f, Direction_Up);
                }

                ++gGameState.frameCounter;
            } break;

            case A_E_Size_Left:
            case A_E_Size_Down:
            case A_E_Size_Right:
            case A_E_Size_Up: {
                gGameState.frameCounter = 0;
            } break;

            case A_B_Save_Config: {
                Config_Save();
            } break;

            case A_B_Load_Config: {
                UI_Cleanup();
                Config_Load();

                gGameState.currentWindow = UI_Config_Callback();
            } break;

            case A_B_Text_Mode: {
                gInputSystem.textMode = !gInputSystem.textMode;
            } break;

            case A_B_Create_Text: {
                Transform trans;
                trans.position = F32x3(0.02f, -0.02f, -10.0f);
                trans.scale = F32x3(0.05f, 0.05f, 0.05f);
                trans.rotation = F32x3(0.0f, 0.0f, 0.0f);

                Transform relTrans = { 0 };
                relTrans.position.xy = F32x2(-1.0f, -1.0f);

#if 0
                gGameState.textToWrite = UI_Text_Create(LitToStr("pene"), JENH_TRUE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                                        &trans, UI_Window_Get_Last_Child_Control(gGameState.currentWindow), &relTrans,
                                                        URWO_Align_Top_Left, 0, UI_Text_Cursor_Click);
#endif
            } break;

            case A_B_Drive: {
                gGameState.score = 0;
                gGameState.time = 0.0f;

                gGameState.initTimeRoad = Time_OS_Counter();
                gGameState.isDriving = JENH_TRUE;
                gGameState.positionPlayer = F32x3(0.0f, 0.0f, 0.0f);
                gGameState.t = 0.0f;

                f32x3* curve = &gGameState.road.curvePoints.E[0].E[0];

                f32x3 tan = BezierCurve3Tan(gGameState.t, curve[0], curve[1], curve[2], curve[3]);

                gGameState.playerRot = -Atan2(-tan.x, tan.z);
            } break;

            case A_B_Render_AABB: {
                gRendererState.shouldRenderAABBs = !gRendererState.shouldRenderAABBs;
            } break;

            case A_B_Insert_Road: {
                f32x3 point = BezierCurve3(1.0f, F32x3(1.0f, 1.0f, 1.0f), F32x3(1.0f, 1.0f, 1.0f), F32x3(1.0f, 1.0f, 1.0f), F32x3(1.0f, 1.0f, 1.0f));

#if 0
                if (!gGameState.debugMode) { break; }

                f32x2 pointerPos = gGameState.pointerPos;

                Road *road = &gGameState.road;

                f32x3 point = BezierCurve3(1.0f, road->endCurve, road->endCurve, pointerPos, pointerPos);

                //BezierCurve3Tan(1.0f, road->endCurve, road->endCurve, pointerPos, pointerPos);
                f32x3 zAxis = Normalize(road->endCurve - point);
                f32x3 xAxis = Normalize(Cross(F32x3(0.0f, 1.0f, 0.0f), zAxis));
                f32x3 yAxis = Cross(zAxis, xAxis);

                PushDebugBasis(pointerPos, xAxis, yAxis, zAxis, 2.0f, 2.0f);

                road->endCurve = pointerPos;

                Mesh* unit = LoadAssetMesh(&road->unit);

                f32x4x4 unitTrans = {
                    F32x4(xAxis, 0.0f),
                    F32x4(yAxis, 0.0f),
                    F32x4(zAxis, 0.0f),
                    F32x4(pointerPos, 1.0f),
                };

                u16 lastUnitOffset = (u16)road->lastUnitOffset;
                u16 newUnitOffset = (u16)AddMeshVertices(&road->mesh, unit);

                for (u32 i = newUnitOffset; i < road->mesh.vertices.size; ++i) {
                    road->mesh.vertices.A[i] = (unitTrans * F32x4(road->mesh.vertices.A[i], 1.0f)).xyz;
                }

                for (u32 i = 0; i < unit->vertices.size; ++i) {
                    u16 newIndex1 = (u16)(newUnitOffset + i);
                    u16 newIndex2 = (u16)(newUnitOffset + i + 1);
                    u16 lastIndex1 = (u16)(lastUnitOffset + i);
                    u16 lastIndex2 = (u16)(lastUnitOffset + i + 1);

                    Tri_Indices *t1 = &road->mesh.indices.A[road->mesh.indices.size++];
                    t1->i1 = newIndex1;
                    t1->i2 = newIndex2;
                    t1->i3 = lastIndex1;

                    Tri_Indices *t2 = &road->mesh.indices.A[road->mesh.indices.size++];
                    t2->i1 = lastIndex1;
                    t2->i2 = lastIndex2;
                    t2->i3 = newIndex2;
                }

                road->lastUnitOffset = newUnitOffset;

                // NOTE(JENH): 01/14.
                // NOTE(JENH): Ignoring the y or height for now.
                f32x2 endVertexLeft = F32x2(road->endVertexLeft.x, road->endVertexLeft.z);
                f32x2 endVertexRight = F32x2(road->endVertexRight.x, road->endVertexRight.z);

                f32x2 newRoadDirection = Normalize((newRoadCenter - middleEndRoad));
                f32x2 newRoadLeft  = newRoadCenter + Perp(newRoadDirection)  * (0.5f * road->width);
                f32x2 newRoadRight = newRoadCenter + -Perp(newRoadDirection) * (0.5f * road->width);

                f32x3 newRoadLeft3D  = F32x3(newRoadLeft.x, 0.0f, newRoadLeft.y);
                f32x3 newRoadRight3D = F32x3(newRoadRight.x, 0.0f, newRoadRight.y);

                // NOTE(JENH): The y is actually the z coordinate.
                PushDebugCube(newRoadLeft3D , 2.0f, COLOR_YELLOW);
                PushDebugCube(newRoadRight3D, 2.0f, COLOR_YELLOW);

                // NOTE(JENH): extending road mesh.
                u32 leftIndex = gGameState.road.mesh.vertices.size++;
                u32 rightIndex = gGameState.road.mesh.vertices.size++;

                gGameState.road.mesh.vertices.A[leftIndex] = newRoadLeft3D;
                gGameState.road.mesh.vertices.A[rightIndex] = newRoadRight3D;

                u32 endIndexLeft = road->endIndexLeft;
                u32 endIndexRight = road->endIndexRight;

                // NOTE(JENH): Finding what combination of vertex will be used to form the triangles.
                f32x2 vectorRoadRight = endVertexRight - endVertexLeft;
                f32 relativeDirection = Dot(vectorRoadRight, newRoadDirection);

                if (relativeDirection < 0.0f) { // new road is at the left
                    road->mesh.indices.A[road->mesh.indices.size++] = TriIndices(endIndexLeft, endIndexRight, rightIndex);
                    road->mesh.indices.A[road->mesh.indices.size++] = TriIndices(leftIndex, rightIndex, endIndexLeft);
                } else { // new road is at the right
                    road->mesh.indices.A[road->mesh.indices.size++] = TriIndices(endIndexLeft, endIndexRight, leftIndex);
                    road->mesh.indices.A[road->mesh.indices.size++] = TriIndices(leftIndex, rightIndex, endIndexRight);
                }

                road->endVertexLeft  = newRoadLeft3D;
                road->endVertexRight = newRoadRight3D;
                road->endIndexLeft  = leftIndex;
                road->endIndexRight = rightIndex;
#endif
            } break;

#if 1
            case A_H_Debug_Pointer: {
                Update_Cursor_Position();
                Coll_Bezier_Curve_Point();
            } break;
#endif

            case A_B_FullScreen: {
                Window_Toggle_Fullsreen();
            } break;

            case A_B_Close: {
                Program_Close();
            } break;

            case A_B_Sponza: {
                gGameState.sponza = !gGameState.sponza;
            } break;

            case A_Nul: break;
            //NO_DEFAULT
        }

        // Update actions state.
        if (Flags_Has_All(action->ID, ACTION_BEGIN_MASK)) {
            Bit_Flags_Set(&actions->state, (Action_Index)action->ID);
            //LogDebug("begin time stamp: %.2f", Time_Sec_To_Ms(action->timeStamp));
        } else if (Flags_Has_All(action->ID, ACTION_END_MASK)) {
            Bit_Flags_Unset(&actions->state, (Action_Index)action->ID);
            //LogDebug("end   time stamp: %.2f", Time_Sec_To_Ms(action->timeStamp));
        }
    }

    gTimeState.timeSinceStartup += deltaTime;

    f32 theta = gTimeState.timeSinceStartup * JENH_PI;
    gGameState.teapotModel = Mat4Identity();
    RotateY(&gGameState.teapotModel, theta);

    UpdateViewMatrix(&gGameState.camera);
}

Public void GetSoundSamples(Sound_Buffer* soundBuffer) {
#if 0
    s16* src = (s16*)gGameState.music.samples + gGameState.sampleIndex;
    s16* dst = soundBuffer->samples;

    for (u32 i = 0; i < soundBuffer->sampleCount; ++i) {
        *dst++ = *src++;
    }

    gGameState.sampleIndex += soundBuffer->sampleCount;

    f32 toneHz = 0.2f;
    u32 wavePeriod = (u32)((f32)soundBuffer->samplesPerSecond / toneHz);
    u32 toneVolume = 300;

    s16 *sampleOut = soundBuffer->samples;
    for (u32 sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; sampleIndex++) {
        f32 sineValue = Sin32(gGameState.tSin);
#if 1
        s16 sampleValue = (s16)((f32)toneVolume * sineValue);
#else
        int16 sampleValue = 0;
#endif
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        gGameState.tSin += 2.0f * JENH_PI / toneHz;
        if (gGameState.tSin > 2.0f * JENH_PI) {
            gGameState.tSin -= 2.0f * JENH_PI;
        }
    }
#endif
}

Intern void WaitTilNextFrame() {
    f32 timeElapsedSec = Time_OS_Seconds_Elapsed(gTimeState.counterLastFrameOS, Time_OS_Counter());

    f32 safeMs = 1.0f;

    if ((timeElapsedSec + safeMs) < gTimeState.targetTimePerFrameSec) {
        u32 sleepMs = (u32)(Time_Sec_To_Ms(gTimeState.targetTimePerFrameSec - timeElapsedSec) - safeMs);
        Assert(sleepMs < 1000);
        Thread_Sleep(sleepMs);
    }

    f32 test = Time_OS_Seconds_Elapsed(Time_OS_Counter(), gTimeState.counterLastFrameOS);
    Assert(test < gTimeState.targetTimePerFrameSec);

    while (timeElapsedSec < gTimeState.targetTimePerFrameSec) {
        timeElapsedSec = Time_OS_Seconds_Elapsed(gTimeState.counterLastFrameOS, Time_OS_Counter());
    }
}

Public Fn_Prot_UI_On_Resize(Scene_Window_On_Resize) {
    Transform* trans = &inControl->rendererTrans;

    f32x2 position = F32x2(((trans->position.x - trans->scale.width) / ASPECT_RATIO + 1.0f) / 2.0f,
                           ((2.0f - (trans->position.y + 1.0f)) - trans->scale.height) / 2.0f);

    f32x2 dimensions = F32x2(trans->scale.width / ASPECT_RATIO, trans->scale.height);

    VK_Viewport_Set(PI_Material, position, dimensions);
}

Public Fn_Prot_UI_Cursor_Over(Button_Cursor_Over) {
    LogInfo("hola perras");
}

Public Fn_Prot_UI_Cursor_Click(Button_Cursor_Click) {
    LogInfo("click");
}

Public void Game_Init(String inBinDir) {
    gGameState.sponza = JENH_FALSE;

    Create_Sine_Wave();

    gGameState.waveMesh.vertexSize = sizeof(Vertex_Data);
    gGameState.waveMesh.vertexCount = ArrayCount(sineSamples) / 4;

    gGameState.waveMesh.indexSize = sizeof(u32);
    gGameState.waveMesh.indexCount = ArrayCount(sineSamples) / 4 * 2;

    gGameState.waveMesh.material = Renderer_Material_From_Color(F32x4(1.0f, 1.0f, 1.0f, 1.0f), PI_Wireframe);

    gGameState.isUIActive = JENH_TRUE;
    Set_UI_Inputs();

    Transform transPanel;
    transPanel.position = F32x3(1.4f, -0.8f, -3.0f);
    transPanel.scale = F32x3(0.32f, 0.09f, 1.0f);
    transPanel.rotation = F32x3(0.0f, 0.0f, 0.0f);

#if 0
    gGameState.audioPanel = UI_Panel_Create(F32x4(0.5f, 0.5f, 0.5f, 1.0f), Converter_Get_Asset_Handle(LitToStr("white.tga")),
                                            &transPanel, 0, 0, 0, 0);
#endif

#if 1
    Transform trans;
    trans.position = F32x3(-1.5f, 0.9f, -1.0f);
    trans.scale = F32x3(0.05f, 0.05f, 1.00f);
    trans.rotation = F32x3(0.0f, 0.0f, 0.0f);

    Transform relTrans = { 0 };
    relTrans.position.xy = F32x2(-1.0f, 1.0f);

    UI_Relative_Op relOp = (UI_Relative_Op)0;

#if 1
    gGameState.debugText.score = UI_Text_Create(LitToStr("hello"), JENH_FALSE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                                &trans, 0, 0, relOp, 0, UI_Text_Cursor_Click);

    Transform trans2;
    trans2.position = F32x3(-1.5f, 0.96f, -1.0f);
    trans2.scale = F32x3(0.05f, 0.05f, 1.00f);
    trans2.rotation = F32x3(0.0f, 0.0f, 0.0f);

    gGameState.debugText.ms = UI_Text_Create(LitToStr("hello"), JENH_FALSE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                             &trans2, 0, 0, (UI_Relative_Op)0, 0, UI_Text_Cursor_Click);
    UI_Control_Set_Relative(gGameState.debugText.ms->control);

    Transform trans3;
    trans3.position = F32x3(-1.5f, 0.86f, -1.0f);
    trans3.scale = F32x3(0.05f, 0.05f, 1.00f);
    trans3.rotation = F32x3(0.0f, 0.0f, 0.0f);

    gGameState.debugText.time = UI_Text_Create(LitToStr("hello"), JENH_FALSE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                               &trans3, 0, 0, (UI_Relative_Op)0, 0, UI_Text_Cursor_Click);
    UI_Control_Set_Relative(gGameState.debugText.time->control);
#else
    gGameState.debugText.ms = UI_Text_Create(LitToStr("hola papus"), JENH_FALSE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                               &trans, UI_Window_Get_Last_Child_Control(gGameState.currentWindow), &relTrans,
                                               relOp, 0, UI_Text_Cursor_Click);
#endif
#endif

#if 0
    Transform buttonTrans;
    buttonTrans.scale = F32x3(0.1f, 0.1f, 1.0f);
    buttonTrans.position = F32x3(0.0f, 0.0f, -15.0f);
    buttonTrans.rotation = F32x3(0.0f, 0.0f, 0.0f);

    gGameState.button = UI_Button_Create(F32x4(1.0f, 0.0f, 0.0f, 1.0f), Converter_Get_Asset_Handle(LitToStr("white.tga")),
                                         &buttonTrans, 0, 0, Button_Cursor_Over, Button_Cursor_Click);
#endif

    gGameState.binDir = inBinDir;

    gGameState.camera.pos = F32x3(0.0f, 0.0f, 100.0f);
    gGameState.camera.rotation = F32x3(0.0f, -0.5f * JENH_PI, 0.0f);
    gGameState.camera.speed = 200.0f;
    UpdateCameraAxes(&gGameState.camera);

    gGameState.teapotModel = Mat4Identity();
    Translate(&gGameState.teapotModel, F32x3(0.0f, 0.0f, 0.0f));
    gGameState.teapotModel.c1.xyz = F32x3(2.0f, 0.0f, 0.0f);
    gGameState.teapotModel.c2.xyz = F32x3(0.0f, 2.0f, 0.0f);
    gGameState.teapotModel.c3.xyz = F32x3(0.0f, 0.0f, 2.0f);

    gGameState.debugMode = JENH_FALSE;

    gGameState.debugBasis.A = ArenaPushArray(&dyMem.perma.arena, Debug_Basis, 256);
    gGameState.debugBasis.size = 0;

    Asset_Mesh_Get_Vertices(Converter_Get_Asset_Handle(LitToStr("plane.obj")), &gGameState.roadUnitMesh);

    gGameState.songTime = 20.0f;

    gGameState.road.curvePoints.count = 2;

    Array_f32x3* curve0 = &gGameState.road.curvePoints.E[0];
    gGameState.road.curvesData.E = ArenaPushArray(&dyMem.perma.arena, Curve_Data, 2);
    curve0->E = ArenaPushArray(&dyMem.perma.arena, f32x3, 7);

    for (u32 i = 0; i < 7; ++i) {
        curve0->E[curve0->count++] = F32x3(0.0f, 0.0f, -40.0f * (f32)i);
    }

    Array_f32x3* curve1 = &gGameState.road.curvePoints.E[1];
    curve1->E = ArenaPushArray(&dyMem.perma.arena, f32x3, 4);

    curve1->E[curve1->count++] = F32x3(100.0f, 0.0f, 100.0f);
    curve1->E[curve1->count++] = F32x3(100.0f, 0.0f, 100.0f);
    curve1->E[curve1->count++] = F32x3(100.0f, 0.0f, 100.0f);
    curve1->E[curve1->count++] = F32x3(100.0f, 0.0f, 100.0f);

    gGameState.roadMeshes.perfect.vertexSize = sizeof(Vertex_Data);
    gGameState.roadMeshes.perfect.indexSize  = sizeof(u32);
    gGameState.roadMeshes.perfect.material = Renderer_Material_From_Color(F32x4(0.45f, 1.0f, 0.45f, 1.0f), PI_Material);

    gGameState.roadMeshes.good.vertexSize = sizeof(Vertex_Data);
    gGameState.roadMeshes.good.indexSize  = sizeof(u32);
    gGameState.roadMeshes.good.material = Renderer_Material_From_Color(F32x4(1.0f, 1.0f, 0.45f, 1.0f), PI_Material);

    gGameState.roadMeshes.bad.vertexSize = sizeof(Vertex_Data);
    gGameState.roadMeshes.bad.indexSize  = sizeof(u32);
    gGameState.roadMeshes.bad.material = Renderer_Material_From_Color(F32x4(1.0f, 0.45f, 0.45f, 1.0f), PI_Material);

    Generate_Road_Geometry(ROAD_WIDTH_PERFECT, 0.7f, &gGameState.roadMeshes.perfect);
    Generate_Road_Geometry(ROAD_WIDTH_GOOD, 0.6f, &gGameState.roadMeshes.good);
    Generate_Road_Geometry(ROAD_WIDTH_BAD, 0.5f, &gGameState.roadMeshes.bad);

    Update_Total_Road_Distance();

    PushDebugBasis(F32x3(0.0f, 0.0f, 0.0f), F32x3(1.0f, 0.0f, 0.0f), F32x3(0.0f, 1.0f, 0.0f),
                   F32x3(0.0f, 0.0f, 1.0f), 2.0f, 2.0f);

    Asset_Terrain_Create(100, 100, F32x2(10.0f, 10.0f), &gGameState.terrainMesh);
    gGameState.terrainMesh.material = Renderer_Material_From_Color(F32x4(0.3f, 0.3f, 0.3f, 1.0f), PI_Wireframe);

    gGameState.playerRot = Radians(180.0f);

    gGameState.isInitialized = JENH_TRUE;
}

Public f32 UpdateTime() {
    f32 deltaTime;

    s64 newCounterOS  = Time_OS_Counter();
    u64 newCounterCPU = Time_CPU_Counter();

    deltaTime = Time_OS_Seconds_Elapsed(gTimeState.counterLastFrameOS, newCounterOS);
    //LogDebug("ms: %.2f", deltaTimeMs);

    //Renderer_Draw_Text(ms, font, &textTransform, &gGameState.debugText.fontBitmap);

    gTimeState.counterLastFrameOS = newCounterOS;

    return deltaTime;
}

Extern void Init1(HWND* outWindow, HINSTANCE* outInstance, s32x2* inWinDims) {
    OS_Init();

    while ( gStateOS.window == 0 );

    *outWindow = gStateOS.window;
    *outInstance = gStateOS.instance;

    *inWinDims = OS_Window_Get_Dims();
}

Extern void Init2(String inBinDir) {
    //Audio_Init();
    Renderer_Init();

#if 0
    String cacheDirPath = LitToStr("..\\assets\\assets\\"CACHED_ASSETS_DIR_NAME"\\");

    if ( Dir_Exists(cacheDirPath.str) ) {
        Dir_Recursive_Delete(cacheDirPath);
        Dir_Create(cacheDirPath.str);
    }

    if ( File_Exists("..\\assets\\assets\\asset_table.rcat") ) { File_Delete("..\\assets\\assets\\asset_table.rcat"); }
    if ( File_Exists("..\\assets\\assets\\saved_convert_table.bin") ) { File_Delete("..\\assets\\assets\\saved_convert_table.bin"); }
#endif

    srand((u32)time(0));

    // packed file
        String dirPath = LitToStr("..\\assets\\assets\\");

        Local_Str(assetTableFilePath, 256);
        CatStr(&assetTableFilePath, dirPath, LitToStr("asset_table.rcat"));
        assetTableFilePath.str[assetTableFilePath.size] = '\0';

        if ( !File_Exists(assetTableFilePath.str) ) {
            RCAT_Header header;
            header.magic = Magic_4("RCAT");
            header.version = RCAT_VERSION;
            header.assetCount = 0;

            File_Handle assetTableFile = File_Create(assetTableFilePath.str);
            File_Write_At(assetTableFile, 0, sizeof(RCAT_Header), &header);
            File_Close(assetTableFile);
        }
    //

    Asset_System_Init(assetTableFilePath);

    Thread_Create(Converter_Monitor_Main, dirPath.str);

    while ( !converterFinish );

    UI_Init();
    Input_Init();

    gGameState.currentWindow = UI_Window_Init(F32x2(1.0f * ASPECT_RATIO, 1.0f), F32x4(0.0f, 0.0f, 0.0f, 0.0f), Scene_Window_On_Resize);

    Game_Init(inBinDir);
    Time_Init();

    Config_Init();
    Profile_Init();

    Create_AABB_Cube(&gRendererState.cubeAABB);

    gStateApp.isRunning = JENH_TRUE;

    //ConfigStore();
}

Extern void MainLoop() {
    f32 deltaTime = UpdateTime();

#if 1
    f32 deltaTimeMs = Time_Sec_To_Ms(deltaTime);
    Local_Str(ms, 256);
    ms.size = (u32)sprintf(ms.str, "ms: %.2f", deltaTimeMs);
    UI_Text_Change_Text(ms, gGameState.debugText.ms);

    Local_Str(score, 256);
    score.size = (u32)sprintf(score.str, "score: %llu/%llu", (u64)gGameState.score, gGameState.totalScore);
    UI_Text_Change_Text(score, gGameState.debugText.score);

    Local_Str(time, 256);
    time.size = (u32)sprintf(time.str, "time: %.2f/%.2f", gGameState.time, gGameState.songTime);
    UI_Text_Change_Text(time, gGameState.debugText.time);
#endif

    Input_Process(deltaTime);

    Draw_Debug_Cube(gGameState.pointerPos, 10.0f, F32x4(0.0f, 1.0f, 0.0f, 1.0f));

    Transform transTerrain = { 0 };
    transTerrain.scale = F32x3(500.0f, 1.0f, 500.0f);

    Renderer_Draw_Raw(&gGameState.terrainMesh, &transTerrain);

    f32x3 rot = F32x3(0.0f, gGameState.playerRot, 0.0f);
    gGameState.forwardPlayer = Normalize(F32x3(Sin32(rot.yaw), 0.0f, Cos32(rot.yaw)));

    if ( gGameState.isDriving ) {
#if 1
        f32x3* curve;
        f32 t = Bezier_Curve_From_Time(gGameState.time, &curve);

        // TODO(JENH): This should be fixed.
        // Assert( gGameState.t <= t );

        gGameState.time += deltaTime;
        if ( gGameState.time >= gGameState.songTime ) {
            gGameState.isDriving = JENH_FALSE;
            goto done_driving;
        }

        gGameState.t = t;

        f32x3 tan = BezierCurve3Tan(t, curve[0], curve[1], curve[2], curve[3]);
        f32x3 normal = Normalize(Cross(tan, F32x3(0.0f, 1.0f, 0.0f)));

        f32x3 newPositionPoint = BezierCurve3(t, curve[0], curve[1], curve[2], curve[3]);

        f32x3 intersection = Intersection_Two_Lines(newPositionPoint, normal, gGameState.positionPlayer, gGameState.forwardPlayer);

        if ( Len(intersection - newPositionPoint) > ROAD_WIDTH ) {
            intersection = newPositionPoint + (normal * ROAD_WIDTH) * Sign(Dot(intersection - newPositionPoint, normal));
        }

        gGameState.positionPlayer = intersection;

        gGameState.positionPlayer.y += 20.0f;

        f32 scoreMul = gGameState.playerSpeed * deltaTime; // * ((f32)gGameState.totalScore / gGameState.playerSpeed);

        // Compute score.
        f32 distanceFromIntersection = Len(intersection - newPositionPoint);
        if ( distanceFromIntersection <= ROAD_WIDTH_PERFECT ) {
            gGameState.score += (f32)(100.0f * scoreMul);
        } else if ( distanceFromIntersection <= ROAD_WIDTH_GOOD ) {
            gGameState.score += (f32)(50.0f * scoreMul);
        } else {
            //Assert( distanceFromIntersection <= ROAD_WIDTH_BAD );
            gGameState.score += (f32)(10.0f * scoreMul);
        }

        gGameState.camera.pos = gGameState.positionPlayer - (80.0f * gGameState.forwardPlayer) + F32x3(0.0f, 50.0f, 0.0f);

#if 1
        gGameState.camera.forward = Normalize(gGameState.positionPlayer - gGameState.camera.pos);
        gGameState.camera.right = Normalize(Cross(gGameState.camera.forward, F32x3(0.0f, 1.0f, 0.0f)));
        gGameState.camera.up = Normalize(Cross(gGameState.camera.right, gGameState.camera.forward));

        UpdateViewMatrix(&gGameState.camera);
#endif

#endif
    }
done_driving:

    Transform transCar;
    transCar.position = gGameState.positionPlayer;
    transCar.scale = F32x3(4.0f, 4.0f, 5.0f);
    transCar.rotation = rot;

    if ( gGameState.sponza ) {
        Entity entity = {
            .transform = { .scale = F32x3(1.0f), },
            .model = Converter_Get_Asset_Handle(LitToStr("sponza.obj")),
        };

        Draw_Entity(entity);
    } else {
        Foreach (Array_f32x3, bezCurve, gGameState.road.curvePoints.E, gGameState.road.curvePoints.count) {
            Foreach (f32x3, point, bezCurve->E, bezCurve->count) {
                f32 scale = 15.0f;
                if ( gGameState.underCursorPoint != point ) {
                    Draw_Debug_Cube(*point, scale, F32x4(1.0f, 0.0f, 0.0f, 1.0f));
                } else {
                    Draw_Debug_Cube(*point, scale, F32x4(0.0f, 0.5f, 0.5f, 1.0f));
                }
            }
        }

        Renderer_Draw_Model(Converter_Get_Asset_Handle(LitToStr("falcon.obj")), &transCar);
        //Draw_Debug_Cube(gGameState.positionPlayer, 10.0f, F32x4(0.5f, 0.5f, 0.5f, 1.0f), rot);
        //Draw_Debug_Cube(gGameState.positionPlayer + 15.0f * gGameState.forwardPlayer, 5.0f, F32x4(0.0f, 0.0f, 0.0f, 1.0f), rot);

        Transform garbage = { 0 };
        garbage.position.z = -1.0f;
        garbage.scale = F32x3(1.0f, 1.0f, 1.0f);
        //garbage.scale = F32x3(10.0f, 10.0f, 10.0f);
        Renderer_Draw_Raw(&gGameState.roadMeshes.perfect, &garbage);
        Renderer_Draw_Raw(&gGameState.roadMeshes.good, &garbage);
        Renderer_Draw_Raw(&gGameState.roadMeshes.bad, &garbage);

        f32 yaw = gTimeState.timeSinceStartup * JENH_PI * 2 * 0.2f;
    }

    VK_Begin_Frame(OS_Window_Get_Dims());

    s32x2 winDims = OS_Window_Get_Dims();

    f32 FOV = JENH_PI / 2.0f;
    CreateFrustum(&gGameState.frustumProj, FOV, (f32)winDims.width / (f32)winDims.height, 0.1f, 5000.0f);

    f32x2 halfWinDim = { (f32)winDims.width / 2.0f, (f32)winDims.height / 2.0f };

    gRendererState.view = gGameState.camera.viewMat;
    gRendererState.projection = Perspective(&gGameState.frustumProj);
    gRendererState.uiProjection = Orthographic(1.0f * ASPECT_RATIO, 1.0f * ASPECT_RATIO, 1.0f, 1.0f, -100.0f, 100.0f);

    Renderer_Render_Frame();

    VK_End_Frame(OS_Window_Get_Dims());

    //Profile_Render();

    UpdateAudio();

    ClearTempMem();

    //WaitTilNextFrame();

    UpdateActionPools();
}

Extern b8 Should_Shutdown() {
    return !gStateApp.isRunning;
}

Extern void Shutdown() {
#if 0
    ASIO_Check( theAsioDriver->stop() == ASE_OK );
    ASIO_Check( theAsioDriver->disposeBuffers() == ASE_OK );
    asioDrivers->removeCurrentDriver();
#endif

    Asset_System_Cleanup();
    OS_Cleanup();
    Time_Cleanup();
}

#if 0
    Sound* music = &gGameState.music;

    Draw_Audio_Wave(&music->samples[gGameState.sampleIndex], gGameState.waveMesh.vertexCount, music->minAmplitude,
                    music->maxAmplitude, &gGameState.waveMesh);

    if ( gGameState.sampleIndex < music->sampleCount ) { gGameState.sampleIndex += (u32)(deltaTime * (f32)(music->sampleRate)); }

    if ( gGameState.isUIActive ) { // Render audio.
        f32x4x4* PVM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
        *PVM = gRendererState.uiProjection;

        Texture* tex = Asset_System_Get_Texture(gGameState.waveMesh.material->diffuseTex);

        Renderer_Draw_Data* drawData = ( tex->hasTransparency ) ? &gRendererState.transparentObjects[gRendererState.transparentObjectCount++]
                                                                : &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

        drawData->PVM  = PVM;
        drawData->mesh = &gGameState.waveMesh;
        drawData->material = gGameState.waveMesh.material;
        drawData->pipelineID = gGameState.waveMesh.material->pipelineID;
    }

    Renderer_Draw_Raw(&gGameState.waveMesh, &garbage);

#endif
