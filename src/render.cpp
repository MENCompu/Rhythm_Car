#include "render.h"

// Camera_Movement values:
//      Forward,
//      Backward,
//      Up,
//      Down,
//      Left,
//      Right,
//
// Intern void MoveCamera(Camera *camera, Camera_Movement mov, f32 deltaTime);

#define MoveCamera(camera, mov, deltaTime) MoveCamera##mov(camera, deltaTime)
#define MAX_LINE_COUNT 4096

typedef struct {
    u32 count;
    Line E[MAX_LINE_COUNT];
} Array_Line;

typedef struct {
    u32 count;
    Material E[4096];
} Colors;

typedef struct {
    u32 opaqueObjectCount;
    Renderer_Draw_Data opaqueObjects[4096];

    u32 transparentObjectCount;
    Renderer_Draw_Data transparentObjects[4096];

    u32 modelMatrixCount;
    f32x4x4 modelMatrices[4096];

    Mesh cubeAABB;
    Material materialAABB;

    b8 shouldRenderAABBs;

    Array_Line lines;

    Mesh tempMeshLines[MAX_LINE_COUNT];

    Colors colors;

    f32x4x4 view;
    f32x4x4 projection;
    f32x4x4 uiProjection;
} Renderer_State;

Private Global Renderer_State gRendererState;

Intern inline void MoveCameraForward(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(camera->forward.x, 0.0f, camera->forward.z));
    camera->pos += moveNorm * (deltaTime * camera->speed);
}

Intern inline void MoveCameraBackward(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(camera->forward.x, 0.0f, camera->forward.z));
    camera->pos -= moveNorm * (deltaTime * camera->speed);
}

Intern inline void MoveCameraUp(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(0.0f, 1.0f, 0.0f));
    camera->pos += moveNorm * (deltaTime * camera->speed);
}

Intern inline void MoveCameraDown(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(0.0f, 1.0f, 0.0f));
    camera->pos -= moveNorm * (deltaTime * camera->speed);
}

Intern inline void MoveCameraRight(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(camera->right.x, 0.0f, camera->right.z));
    camera->pos += moveNorm * (deltaTime * camera->speed);
}

Intern inline void MoveCameraLeft(Camera *camera, f32 deltaTime) {
    f32x3 moveNorm = Normalize(F32x3(camera->right.x, 0.0f, camera->right.z));
    camera->pos -= moveNorm * (deltaTime * camera->speed);
}

Intern void UpdateViewMatrix(Camera *camera) {
    f32x3 xAxis = camera->right;
    f32x3 yAxis = camera->up;
    f32x3 zAxis = camera->forward;

    f32x3 pos = camera->pos;

    f32x4x4 viewMat = {
        F32x4(xAxis.x, yAxis.x, -zAxis.x, 0.0f),
        F32x4(xAxis.y, yAxis.y, -zAxis.y, 0.0f),
        F32x4(xAxis.z, yAxis.z, -zAxis.z, 0.0f),
        F32x4(-Dot(pos, xAxis), -Dot(pos, yAxis), -Dot(pos, -zAxis), 1.0f)
    };

    camera->viewMat = viewMat;

    return;
}

Intern f32x4x4 GetCameraTransform(Camera *camera) {
    f32x4x4 trans = {
        F32x4(camera->right, 0.0f),
        F32x4(camera->up, 0.0f),
        F32x4(camera->forward, 0.0f),
        F32x4(camera->pos, 1.0f),
    };

    return trans;
}

Intern f32x4x4 CreateSRTModel(Transform* inTransform) {
    f32 pitch = inTransform->rotation.pitch;
    f32 yaw   = inTransform->rotation.yaw;
    f32 roll  = inTransform->rotation.roll;

    f32x3 scale = inTransform->scale;
    f32x3 translation = inTransform->position;

    f32x4x4 scaleMat = {
        F32x4(scale.x, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, scale.y, 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, scale.z, 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    f32x4x4 rotx = {
        F32x4(1.0f, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f,  Cos32(pitch), Sin32(pitch), 0.0f),
        F32x4(0.0f, -Sin32(pitch), Cos32(pitch), 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    f32x4x4 roty = {
        F32x4(Cos32(yaw), 0.0f, -Sin32(yaw), 0.0f),
        F32x4(0.0f, 1.0f, 0.0f, 0.0f),
        F32x4(Sin32(yaw), 0.0f, Cos32(yaw), 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    f32x4x4 rotz = {
        F32x4( Cos32(roll), Sin32(roll), 0.0f , 0.0f),
        F32x4(-Sin32(roll), Cos32(roll), 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, 1.0f, 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    f32x4x4 trans = {
        F32x4(1.0f, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, 1.0f, 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, 1.0f, 0.0f),
        ToF32x4(translation, 1.0f),
    };
    f32x4x4 ret = trans * rotz * roty * rotx * scaleMat * Mat4Identity();
    return ret;
}

Public void Create_AABB_Cube(Mesh* outMesh) {
    static Vertex_Data vertices[] = {
        { F32x3(-1.0f, -1.0f,  1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3(-1.0f,  1.0f,  1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3( 1.0f,  1.0f,  1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3( 1.0f, -1.0f,  1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },

        { F32x3(-1.0f, -1.0f, -1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3(-1.0f,  1.0f, -1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3( 1.0f,  1.0f, -1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
        { F32x3( 1.0f, -1.0f, -1.0f), F32x3(1.0f), F32x2(1.0f, 1.0f) },
    };

    static u32 indices[] = {
        0, 1,  1, 2,  2, 3,  3, 0,

        4, 5,  5, 6,  6, 7,  7, 4,

        0, 4,  1, 5,  2, 6,  3, 7,
    };

    outMesh->vertexSize = sizeof(Vertex_Data);
    outMesh->vertexCount = ArrayCount(vertices);
    outMesh->vertexOffset = 0;

    outMesh->indexSize = sizeof(u32);
    outMesh->indexCount = ArrayCount(indices);
    outMesh->indexOffset = 0;

    GAPI_Mesh_Create(vertices, indices, outMesh);

    outMesh->material = &gRendererState.materialAABB;
    outMesh->material->diffuseColor = F32x4(0.0f, 0.0f, 0.0f, 1.0f);
    outMesh->material->specularColor = F32x4(1.0f, 1.0f, 1.0f, 1.0f);

    outMesh->material->diffuseTex = Converter_Get_Asset_Handle(LitToStr("negro.bmp"));
    outMesh->material->specularTex = Converter_Get_Asset_Handle(LitToStr("negro.bmp"));
    outMesh->material->normalTex = Converter_Get_Asset_Handle(LitToStr("negro.bmp"));

    GAPI_Material_Create(PI_Wireframe, outMesh->material);
}

Public void Create_Quad(f32x4 inColor, Mesh* outQuad, Material* outMaterial) {
    Vertex_Data vertices[] = {
        { F32x3(-1.0f,-1.0f, 0.0f), F32x3(1.0f), F32x2(0.0f,0.0f) },
        { F32x3( 1.0f,-1.0f, 0.0f), F32x3(1.0f), F32x2(1.0f,0.0f) },
        { F32x3(-1.0f, 1.0f, 0.0f), F32x3(1.0f), F32x2(0.0f,1.0f) },
        { F32x3( 1.0f, 1.0f, 0.0f), F32x3(1.0f), F32x2(1.0f,1.0f) },
    };

    u32 indices[] = {
        2, 0, 1,
        1, 3, 2,
    };

    outQuad->vertexSize = sizeof(Vertex_Data);
    outQuad->vertexCount = ArrayCount(vertices);
    outQuad->vertexOffset = 0;

    outQuad->indexSize = sizeof(u32);
    outQuad->indexCount = ArrayCount(indices);
    outQuad->indexOffset = 0;

    outQuad->material = outMaterial;

    GAPI_Mesh_Create(vertices, indices, outQuad);

    outMaterial->diffuseColor = inColor;
    outMaterial->diffuseTex = Converter_Get_Asset_Handle(LitToStr("white.tga"));
    outMaterial->normalTex  = Converter_Get_Asset_Handle(LitToStr("white.tga"));

    GAPI_Material_Create(PI_UI, outQuad->material);
}

Public void Renderer_Init() {
    //gRendererState.colors.count = 4096;
    //gRendererState.colors.E = ArenaPushArray(&dyMem.perma.arena, Material, gRendererState.colors.count);
}

// TODO(JENH): Implement reference count for material destroy.
Public Material* Renderer_Material_From_Color(f32x4 inColor, u32 inPipelineID) {
    Foreach (Material, material, gRendererState.colors.E, gRendererState.colors.count) {
        if ( inPipelineID == material->pipelineID && F32x4_Eq(material->diffuseColor, inColor) ) {
            return material;
        }
    }

    // create new material.
    Material* newMaterial = Array_Push(gRendererState.colors.E, &gRendererState.colors.count);

    newMaterial->diffuseColor = inColor;
    newMaterial->specularColor = inColor;

    newMaterial->diffuseTex = Converter_Get_Asset_Handle(LitToStr("white.tga"));
    newMaterial->normalTex = Converter_Get_Asset_Handle(LitToStr("white.tga"));
    newMaterial->specularTex = Converter_Get_Asset_Handle(LitToStr("white.tga"));

    GAPI_Material_Create(inPipelineID, newMaterial);

    return newMaterial;
}

Public void Renderer_Move_Draw_Data_To_Top(Renderer_Draw_Data* inDrawData) {
    if ( gRendererState.opaqueObjects <= inDrawData && inDrawData <= (gRendererState.opaqueObjects + gRendererState.opaqueObjectCount) ) {

        Renderer_Draw_Data copyDrawData = *inDrawData;
#if 0
        Mem_Copy_Forward(inDrawData, inDrawData + 1, (u32)((byte*)(&gRendererState.opaqueObjects[gRendererState.opaqueObjectCount])
                                                           - (byte*)inDrawData));
#endif

        for (Renderer_Draw_Data* drawData = inDrawData; drawData < (gRendererState.opaqueObjects + gRendererState.opaqueObjectCount); ++drawData) {
            drawData = drawData + 1;
        }

        gRendererState.opaqueObjects[gRendererState.opaqueObjectCount] = copyDrawData;
    } else if ( gRendererState.transparentObjects <= inDrawData &&
                inDrawData <= (gRendererState.transparentObjects + gRendererState.transparentObjectCount) ) {

#if 0
        Mem_Copy_Forward(inDrawData, inDrawData + 1, (u32)((byte*)(&gRendererState.transparentObjects[gRendererState.transparentObjectCount])
                                                           - (byte*)inDrawData));

#endif
        Renderer_Draw_Data copyDrawData = *inDrawData;
        for (Renderer_Draw_Data* drawData = inDrawData;
             drawData < (gRendererState.transparentObjects + gRendererState.transparentObjectCount);
             ++drawData) {
            drawData = drawData + 1;
        }

        gRendererState.transparentObjects[gRendererState.transparentObjectCount] = copyDrawData;
    } NO_ELSE
}

Public void Renderer_Draw_Mesh_UI(Mesh* inMesh, Transform* inTransform) {
    f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PM = gRendererState.projection * CreateSRTModel(inTransform);

    Texture* tex = Asset_System_Get_Texture(inMesh->material->diffuseTex);

    Renderer_Draw_Data* drawData = ( tex->hasTransparency ) ? &gRendererState.transparentObjects[gRendererState.transparentObjectCount++]
                                                            : &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

    drawData->PVM  = PM;
    drawData->mesh = inMesh;
    drawData->material = inMesh->material;
    drawData->pipelineID = PI_Material;
}

Public void Renderer_Draw_Line(Line* inLine) {
    Line* line = Array_Push(gRendererState.lines.E, &gRendererState.lines.count);
    *line = *inLine;
}

Public void Renderer_Draw_Raw(Mesh* inMesh, Transform* inTransform) {
    f32x4x4* PVM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PVM = gRendererState.projection * gRendererState.view * CreateSRTModel(inTransform);

    Texture* tex = Asset_System_Get_Texture(inMesh->material->diffuseTex);

    Renderer_Draw_Data* drawData = ( tex->hasTransparency ) ? &gRendererState.transparentObjects[gRendererState.transparentObjectCount++]
                                                            : &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

    drawData->PVM  = PVM;
    drawData->mesh = inMesh;
    drawData->material = inMesh->material;
    drawData->pipelineID = inMesh->material->pipelineID;
}

Public void Renderer_Draw_Model(Asset_Handle inModel, Transform* inTransform) {
    Model* model = Asset_System_Get_Model(inModel);

    f32x4x4* PVM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PVM = gRendererState.projection * gRendererState.view * CreateSRTModel(inTransform);

    for (u32 i = 0; i < model->meshCount; ++i) {
        Mesh* mesh = &model->meshes[i];

        Texture* tex = Asset_System_Get_Texture(mesh->material->diffuseTex);

        Renderer_Draw_Data* drawData = ( tex->hasTransparency ) ? &gRendererState.transparentObjects[gRendererState.transparentObjectCount++]
                                                                : &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

        drawData->PVM  = PVM;
        drawData->mesh = mesh;
        drawData->material = mesh->material;
        drawData->pipelineID = PI_Material;

        if ( gRendererState.shouldRenderAABBs ) {
            Transform trans;
            trans.position = mesh->center;
            trans.scale = mesh->halfDim;
            trans.rotation = F32x3(0.0f, 0.0f, 0.0f);

            f32x4x4* wireframePVM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
            *wireframePVM = (*PVM) * CreateSRTModel(&trans);

            Renderer_Draw_Data* drawDataAABB = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

            drawDataAABB->PVM  = wireframePVM;
            drawDataAABB->mesh = &gRendererState.cubeAABB;
            drawDataAABB->material = &gRendererState.materialAABB;
            drawDataAABB->pipelineID = PI_Wireframe;
        }
    }
}

Public void Renderer_Draw_UI(Asset_Handle inModel, Transform* inTransform) {
    Model* model = Asset_System_Get_Model(inModel);

    f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PM = gRendererState.uiProjection * CreateSRTModel(inTransform);

    for (u32 i = 0; i < model->meshCount; ++i) {
        Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];

        Mesh* mesh = &model->meshes[i];

        drawData->PVM  = PM;
        drawData->mesh = mesh;
        drawData->material = mesh->material;
        drawData->pipelineID = PI_UI;
    }
}

Public void Change_Material_Color(f32x4 inColor, Material* outMaterial) {
    outMaterial->diffuseColor = inColor;
    outMaterial->upToDate = JENH_FALSE;
}

Public void Renderer_Render_UI_Window(UI_Window* inWindow) {
    f32x4 colorTable[] {
        F32x4(1.0f, 0.0f, 0.0f, 1.0f),
        F32x4(0.0f, 1.0f, 0.0f, 1.0f),
        F32x4(0.5f, 0.0f, 5.0f, 1.0f),
        F32x4(0.0f, 0.0f, 1.0f, 1.0f),
    };

    f32x4 color = F32x4(0.0f, 0.0f, 0.0f, 1.0f);

    Transform trans = inWindow->control->rendererTrans;

#if 1
    for (u32 i = 0; i < 4; ++i) {
        UI_Window_List* list = &gGameState.currentWindow->adjacents[i];

        //Material* color = &gUIWindowState.materialTable[i];
        u32 counter = 1;
        for (UI_Window_Node* node = list->first; node; node = node->next, ++counter) {
            UI_Window* adjacentWindow = node->window;

            if ( adjacentWindow == inWindow ) {
                color = colorTable[i];
                //material = Renderer_Material_From_Color(colorTable[i], PI_UI);
                trans.position.z += 2.0f;

#if 1
                UI_Text* text = adjacentWindow->number;
                //Change_Material_Color(F32x4(1.0f, 1.0f, 1.0f, 1.0f), &text->material);
                Change_Material_Color(colorTable[i], &text->material);
                U32_To_Str(counter, &adjacentWindow->numberString);
                UI_Text_Change_Text(adjacentWindow->numberString, adjacentWindow->number);
#endif

                goto adjacent_found;
            }
        }
    }
#endif

    /* if adjacent_found */ {
        Change_Material_Color(F32x4(0.0f, 0.0f, 0.0f, 0.0f), &inWindow->number->material);
    }
adjacent_found:

    //UI_Control_Draw_OutLine(inWindow->control, -5.0f, color);

#if 0
    f32x4x4* outlinePM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *outlinePM = gRendererState.uiProjection * CreateSRTModel(&trans);

    Renderer_Draw_Data* drawDataOutline = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
    drawDataOutline->PVM  = outlinePM;
    drawDataOutline->mesh = &gRendererState.cubeAABB;
    drawDataOutline->material = material;
    drawDataOutline->pipelineID = PI_Wireframe;
#endif

    if ( inWindow->hasScene ) { return; }

    f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];
    *PM = gRendererState.uiProjection * CreateSRTModel(&trans);

    Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
    drawData->PVM = PM;
    drawData->mesh = &inWindow->mesh;
    drawData->material = inWindow->mesh.material;
    drawData->pipelineID = PI_UI;

}

Public void Renderer_Render_UI() {
#if 0
    Foreach (UI_Text, text, gUIState.texts, gUIState.textCount) {
        Assert( text->control->type == UT_Text );

        UI_Control* control = &text->control;
        if ( !control->isActive ) { continue; }

        f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

        Transform trans = control->rendererTrans;

        f32x2 center = F32x2_Elem_Prod(control->center, control->rendererTrans.scale.xy);
        F32x2_Sub_Equal(&trans.position.xy, center);

        *PM = gRendererState.uiProjection * CreateSRTModel(&trans);

        Renderer_Draw_Data* drawData = &gRendererState.transparentObjects[gRendererState.transparentObjectCount++];
        drawData->PVM = PM;
        drawData->mesh = &text->mesh;
        drawData->material = text->mesh.material;
        drawData->pipelineID = PI_UI;

        if ( text == gGameState.textToWrite ) {
            f32x4x4* outlinePM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

            Transform textTrans = { 0 };
            textTrans.position.xy = control->center;
            textTrans.scale.xy = control->halfDims;

            *outlinePM = gRendererState.uiProjection * CreateSRTModel(&trans) * CreateSRTModel(&textTrans);

            Renderer_Draw_Data* drawDataOutline = Array_Push(gRendererState.opaqueObjects, &gRendererState.opaqueObjectCount);
            drawDataOutline->PVM  = outlinePM;
            drawDataOutline->mesh = &gRendererState.cubeAABB;
            drawDataOutline->material = &gUIWindowState.red;
            drawDataOutline->pipelineID = PI_Wireframe;
        }
    }

    Foreach (UI_Panel, panel, gUIState.panels, gUIState.panelCount) {
        Assert( panel->control->type == UT_Panel );

        UI_Control* control = &panel->control;
        if ( !control->isActive ) { continue; }

        f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

        // TODO(JENH): Add the center and halfdims transform logic here.
        *PM = gRendererState.uiProjection * CreateSRTModel(&control->rendererTrans);

        Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
        drawData->PVM = PM;
        drawData->mesh = &panel->mesh;
        drawData->material = panel->mesh.material;
        drawData->pipelineID = PI_UI;
    }

    Foreach (UI_Button, button, gUIState.buttons, gUIState.buttonCount) {
        Assert( button->control->type == UT_Button );

        UI_Control* control = &button->control;
        if ( !control->isActive ) { continue; }

        f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

        // TODO(JENH): Add the center and halfdims transform logic here.
        *PM = gRendererState.uiProjection * CreateSRTModel(&control->rendererTrans);

        Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
        drawData->PVM = PM;
        drawData->mesh = &button->mesh;
        drawData->material = button->mesh.material;
        drawData->pipelineID = PI_UI;
    }

    Foreach (UI_Window, window, gUIWindowState.windows, gUIWindowState.windowCount) {
        Renderer_Render_UI_Window(window);
    }
#else
    Foreach (UI_Control, control, gUIState.controls, gUIState.controlCount) {
        if ( !control->isActive ) { continue; }

        switch ( control->type ) {
            case UT_Text: {
                UI_Text* text = (UI_Text*)UI_Control_Get_Data(control);

                f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

                Transform trans = control->rendererTrans;

                f32x2 center = F32x2_Elem_Prod(control->center, control->rendererTrans.scale.xy);
                F32x2_Sub_Equal(&trans.position.xy, center);

                *PM = gRendererState.uiProjection * CreateSRTModel(&trans);

                Renderer_Draw_Data* drawData = &gRendererState.transparentObjects[gRendererState.transparentObjectCount++];
                drawData->PVM = PM;
                drawData->mesh = &text->mesh;
                drawData->material = text->mesh.material;
                drawData->pipelineID = PI_UI;
            } break;

            case UT_Panel: {
                UI_Panel* panel = (UI_Panel*)UI_Control_Get_Data(control);

                f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

                // TODO(JENH): Add the center and halfdims transform logic here.
                *PM = gRendererState.uiProjection * CreateSRTModel(&control->rendererTrans);

                Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
                drawData->PVM = PM;
                drawData->mesh = &panel->mesh;
                drawData->material = panel->mesh.material;
                drawData->pipelineID = PI_UI;
            } break;

            case UT_Button: {
                UI_Button* button = (UI_Button*)UI_Control_Get_Data(control);

                f32x4x4* PM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

                // TODO(JENH): Add the center and halfdims transform logic here.
                *PM = gRendererState.uiProjection * CreateSRTModel(&control->rendererTrans);

                Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[gRendererState.opaqueObjectCount++];
                drawData->PVM = PM;
                drawData->mesh = &button->mesh;
                drawData->material = button->mesh.material;
                drawData->pipelineID = PI_UI;
            } break;

            case UT_Window: {
#if 0
                UI_Window* window = (UI_Window*)UI_Control_Get_Data(control);

                Renderer_Render_UI_Window(window);
#endif
            } break;

            case UI_TYPE_COUNT:
            NO_DEFAULT
        }
    }
#endif

    if ( gUIState.selected ) { UI_Control_Draw_OutLine(gUIState.selected, -15.0f, F32x4(0.0f, 0.0f, 1.0f, 1.0f)); }
    if ( gUIState.underCursor ) { UI_Control_Draw_OutLine(gUIState.underCursor, -15.0f, F32x4(1.0f, 0.0f, 0.0f, 1.0f)); }

    if ( gUIState.isSomeEdgeLineSelected ) {
        Renderer_Draw_Line(&gUIState.edgeLineSelected);
    }

#if 0
    f32x4x4* outlinePM = &gRendererState.modelMatrices[gRendererState.modelMatrixCount++];

    Transform trans;
    trans = gUIState.underCursor->rendererTrans;

    f32x2 center = F32x2_Elem_Prod(gUIState.underCursor->center, gUIState.underCursor->rendererTrans.scale.xy);
    F32x2_Sub_Equal(&trans.position.xy, center);

    Transform textTrans = { 0 };
    textTrans.position.xy = gUIState.underCursor->center;
    textTrans.scale.xy = gUIState.underCursor->halfDims;

    *outlinePM = gRendererState.uiProjection * CreateSRTModel(&trans) * CreateSRTModel(&textTrans);

    Renderer_Draw_Data* drawDataOutline = Array_Push(gRendererState.opaqueObjects, &gRendererState.opaqueObjectCount);
    drawDataOutline->PVM  = outlinePM;
    drawDataOutline->mesh = &gRendererState.cubeAABB;
    drawDataOutline->material = &gUIWindowState.red;
    drawDataOutline->pipelineID = PI_Wireframe;
#endif

#if 0
    if ( !(gUIState.edgeLine.a.x == 0.0f && gUIState.edgeLine.a.x == 0.0f && gUIState.edgeLine.a.x == 0.0f &&
           gUIState.edgeLine.a.x == 0.0f && gUIState.edgeLine.a.x == 0.0f && gUIState.edgeLine.a.x == 0.0f) ) {
        Renderer_Draw_Line();
    }
#endif
}

Public void Renderer_Render_Frame() {
    VK_Draw_Call_Begin();
#if 1

    VK_Set_Line_Width(1.0f);

    if ( gGameState.isUIActive ) {
        UI_Update();
        Renderer_Render_UI();
    }

    for (u32 i = 0; i < gRendererState.opaqueObjectCount; ++i) {
        Renderer_Draw_Data* drawData = &gRendererState.opaqueObjects[i];

        VK_Pipeline* pipeline = VK_Pipeline_Get(drawData->pipelineID);
        VK_Pipeline_Bind(pipeline);

        u32 samplerCount = ( drawData->pipelineID == PI_Wireframe ) ? 0U : 2U;

        Texture* textures[2];

        if ( samplerCount == 2 ) {
            textures[0] = Asset_System_Get_Texture(drawData->material->diffuseTex);
            textures[1] = Asset_System_Get_Texture(drawData->material->normalTex);
        }

        VK_Update_Material_Uniforms(pipeline, textures, drawData->material, samplerCount);
        VK_Update_Draw_Call_Uniforms(pipeline, *drawData->PVM);

        VK_Mesh_Draw(drawData->mesh);

        VK_Pipeline_Unbind();
    }

    for (u32 i = 0; i < gRendererState.transparentObjectCount; ++i) {
        Renderer_Draw_Data* drawData = &gRendererState.transparentObjects[i];

        VK_Pipeline* pipeline = VK_Pipeline_Get(drawData->pipelineID);
        VK_Pipeline_Bind(pipeline);

        u32 samplerCount = ( drawData->pipelineID == PI_Wireframe ) ? 0U : 2U;

        Texture* textures[2];

        if ( samplerCount == 2 ) {
            textures[0] = Asset_System_Get_Texture(drawData->material->diffuseTex);
            textures[1] = Asset_System_Get_Texture(drawData->material->normalTex);
        }

        VK_Update_Material_Uniforms(pipeline, textures, drawData->mesh->material, samplerCount);
        VK_Update_Draw_Call_Uniforms(pipeline, *drawData->PVM);

        VK_Mesh_Draw(drawData->mesh);

        VK_Pipeline_Unbind();
    }

    VK_Set_Line_Width(2.0f);

    VK_Pipeline* pipeline = VK_Pipeline_Get(PI_Wireframe);
    VK_Pipeline_Bind(pipeline);

#if 1
    //Foreach (Line, line, gUIState.lines.E, gUIState.lines.count) {
    for (u32 i = 0; i < gRendererState.lines.count; ++i) {
        Line* line = &gRendererState.lines.E[i];
        Mesh* mesh = &gRendererState.tempMeshLines[i];

        u32 samplerCount = 0U;

        Texture* textures[2];

        Vertex_Data vertices[] = {
            { line->a, F32x3(0.0f, 0.0f, 0.0f), F32x2(0.0f, 0.0f) },
            { line->b, F32x3(0.0f, 0.0f, 0.0f), F32x2(0.0f, 0.0f) },
        };

        u32* indices = 0;

        mesh->vertexCount = ArrayCount(line->points);
        mesh->vertexSize = sizeof(Vertex_Data);

        mesh->indexCount = 0;
        mesh->indexSize = 0;

        GAPI_Mesh_Create(vertices, indices, mesh);

        VK_Update_Material_Uniforms(pipeline, textures, line->material, samplerCount);

        f32x4x4 PVM = gRendererState.uiProjection;
        VK_Update_Draw_Call_Uniforms(pipeline, PVM);

        VK_Mesh_Draw(mesh);

        VK_Pipeline_Unbind();
    }

    Foreach (Mesh, mesh, gRendererState.tempMeshLines, gRendererState.lines.count) {
        GAPI_Mesh_Destroy(mesh);
    }
#endif

#endif
    gRendererState.lines.count = 0;
    gRendererState.opaqueObjectCount = 0;
    gRendererState.transparentObjectCount = 0;
    gRendererState.modelMatrixCount = 0;

    VK_Draw_Call_End();
}

#if 0
#define RenderCmdPush(cmdStack, cmd) (Render_Cmd_##cmd *)RenderCmdPush_(cmdStack, sizeof(Render_Cmd_##cmd), RCI_##cmd)
Intern inline void *RenderCmdPush_(Render_Cmd_Stack *cmdStack, u32 size, Render_Cmd_ID cmdID) {
    Render_Cmd_ID *newCmdID = (Render_Cmd_ID *)ArenaPushMem(&cmdStack->mem, size);
    *newCmdID = cmdID;
    ++cmdStack->cmdsCount;
    return (void *)newCmdID;
}

Intern inline void RenderMesh(Render_Cmd_Stack *cmdStack, f32x4x4 *model, f32x4x4 *view, f32x4x4 *projection,
                       Asset_Mesh *asset, f32x4 color) {
    Render_Cmd_Mesh *mesh = RenderCmdPush(cmdStack, Mesh);

    mesh->MVP = (*projection) * (*view) * (*model);

    mesh->mesh = LoadAssetMesh(asset);

    mesh->color = color;
}

Intern inline void RenderCube(Render_Cmd_Stack *cmdStack, f32x3 scale, f32x3 rotation, f32x3 position,
                       f32x4x4 *view, f32x4x4 *projection, f32x4 color) {
    Render_Cmd_Cube *cube = RenderCmdPush(cmdStack, Cube);

    cube->MVP = (*projection) * (*view) * CreateSRTModel(scale, rotation, position);
    cube->color = color;
}

Intern inline void RenderBasis(Render_Cmd_Stack *cmdStack, f32 axesDist, f32 cubeScale, Basis_3D *basis,
                        f32x4x4 *model, f32x4x4 *view, f32x4x4 *proj) {
    Render_Cmd_Basis *cmd = RenderCmdPush(cmdStack, Basis);

    cmd->cubeScale = cubeScale;
    cmd->axesDist  = axesDist;

    f32x3 xAxis = basis->axes.x;
    f32x3 yAxis = basis->axes.y;
    f32x3 zAxis = basis->axes.z;

    f32x4x4 matFromBasis = {
        F32x4(cubeScale * xAxis, 0.0f),
        F32x4(cubeScale * yAxis, 0.0f),
        F32x4(cubeScale * zAxis, 0.0f),
        F32x4(basis->o, 1.0f)
    };

    cmd->MVP = (*proj) * (*view) * (*model) * matFromBasis;
}

Intern inline void RenderClear(Render_Cmd_Stack *cmdStack, f32x4 color) {
    Render_Cmd_Clear *clear = RenderCmdPush(cmdStack, Clear);
    clear->color = color;
}

Intern Render_Cmd_Stack CreateRenderCmdStack(Memory_Arena *arena, u32 capacity, Basis_3D *defaultBasis, f32 worldToPixels) {
    Render_Cmd_Stack ret;

    ret.cmdsCount = 0;
    InitArena(&ret.mem, ArenaPushMem(arena, capacity), capacity);

    return ret;
}

Intern void Render(Render_Cmd_Stack *cmdStack, Window_Buffer *windowBuffer) {
    byte *scan = (byte *)cmdStack->mem.base;

    for (u32 i = 0; i < cmdStack->cmdsCount; ++i) {
        Render_Cmd_ID *cmdID = (Render_Cmd_ID *)scan;

        switch (*cmdID) {
            case RCI_Clear: {
                Render_Cmd_Clear *cmd = (Render_Cmd_Clear *)scan;

                //ClearScreen(windowBuffer, cmd->color);

                scan += sizeof(Render_Cmd_Clear);
            } break;

            case RCI_Bitmap: {
                scan += sizeof(Render_Cmd_Bitmap);
            } break;

            case RCI_Rect: {
                Render_Cmd_Rect *cmd = (Render_Cmd_Rect *)scan;

#if 0
                f32x2 o     = cmdStack->WToP * F32x2(cmd->basis.o.x,      -cmd->basis.o.y);;
                f32x2 xAxis = cmdStack->WToP * F32x2(cmd->basis.axes.x.x, -cmd->basis.axes.x.y);
                f32x2 yAxis = cmdStack->WToP * F32x2(cmd->basis.axes.y.x, -cmd->basis.axes.y.y);

                DrawRectBasis(windowBuffer, o, xAxis, yAxis, cmd->color);

                f32x2 dims = { 5, 5 };
                f32x4 color = F32x4(0.0f, 0.0f, 1.0f, 1.0f);

                f32x2 p1 = o;
                DrawRectangle(windowBuffer, p1, p1 + dims, color);
                f32x2 p2 = o + xAxis;
                DrawRectangle(windowBuffer, p2, p2 + dims, color);
                f32x2 p3 = o + yAxis;
                DrawRectangle(windowBuffer, p3, p3 + dims, color);
                f32x2 p4 = o + xAxis + yAxis;
                DrawRectangle(windowBuffer, p4, p4 + dims, color);
#endif

                scan += sizeof(Render_Cmd_Rect);
            } break;

            case RCI_Tri: {
                Render_Cmd_Tri *cmd = (Render_Cmd_Tri *)scan;

#if 0
                f32 WToP = cmdStack->WToP;

                Basis_3D *basis = &cmd->basis;

                f32x3 a = basis->o + (basis->axes.x * cmd->a.x) + (basis->axes.y * cmd->a.y) + (basis->axes.z * cmd->a.z);
                f32x3 b = basis->o + (basis->axes.x * cmd->b.x) + (basis->axes.y * cmd->b.y) + (basis->axes.z * cmd->b.z);
                f32x3 c = basis->o + (basis->axes.x * cmd->c.x) + (basis->axes.y * cmd->c.y) + (basis->axes.z * cmd->c.z);

                f32x2 pa = F32x2(a.x * (PROJECTION_NEAR / a.z), a.y * (PROJECTION_NEAR / a.z));
                f32x2 pb = F32x2(b.x * (PROJECTION_NEAR / b.z), b.y * (PROJECTION_NEAR / b.z));
                f32x2 pc = F32x2(c.x * (PROJECTION_NEAR / c.z), c.y * (PROJECTION_NEAR / c.z));

                f32 halfDimSx = windowBuffer->dims.x * 0.5f;
                f32 halfDimSy = windowBuffer->dims.y * 0.5f;

                pa = F32x2(WToP * pa.x, WToP * -pa.y) + F32x2(halfDimSx, halfDimSy);
                pb = F32x2(WToP * pb.x, WToP * -pb.y) + F32x2(halfDimSx, halfDimSy);
                pc = F32x2(WToP * pc.x, WToP * -pc.y) + F32x2(halfDimSx, halfDimSy);

                DrawTriangle(windowBuffer, pa, pb, pc, cmd->color);
#endif

                scan += sizeof(*cmd);
            } break;

            case RCI_Mesh: {
                Render_Cmd_Mesh *cmd = (Render_Cmd_Mesh *)scan;

#if 0
                Array_f32x3 vertices = cmd->mesh->vertices;

                foreach (Tri_Indices, face, cmd->mesh->indices) {
                    TriPrimitive(windowBuffer, vertices.A[face->i1], vertices.A[face->i2], vertices.A[face->i3],
                                 &cmd->MVP, ToF32(windowBuffer->dims), cmd->color);
                }
#endif

                scan += sizeof(*cmd);
            } break;

            case RCI_Cube: {
                Render_Cmd_Cube *cmd = (Render_Cmd_Cube *)scan;

#if 0
                f32x3 vertices[] = {
                    {-0.5f, -0.5f, -0.5f},
                    {0.5f, -0.5f, -0.5f},
                    {0.5f,  0.5f, -0.5f},
                    {0.5f,  0.5f, -0.5f},
                    {-0.5f,  0.5f, -0.5f},
                    {-0.5f, -0.5f, -0.5f},

                    {-0.5f, -0.5f,  0.5f},
                    {0.5f, -0.5f,  0.5f},
                    {0.5f,  0.5f,  0.5f},
                    {0.5f,  0.5f,  0.5f},
                    {-0.5f,  0.5f,  0.5f},
                    {-0.5f, -0.5f,  0.5f},

                    {-0.5f,  0.5f,  0.5f},
                    {-0.5f,  0.5f, -0.5f},
                    {-0.5f, -0.5f, -0.5f},
                    {-0.5f, -0.5f, -0.5f},
                    {-0.5f, -0.5f,  0.5f},
                    {-0.5f,  0.5f,  0.5f},

                    {0.5f,  0.5f,  0.5f},
                    {0.5f,  0.5f, -0.5f},
                    {0.5f, -0.5f, -0.5f},
                    {0.5f, -0.5f, -0.5f},
                    {0.5f, -0.5f,  0.5f},
                    {0.5f,  0.5f,  0.5f},

                    {-0.5f, -0.5f, -0.5f},
                    {0.5f, -0.5f, -0.5f},
                    {0.5f, -0.5f,  0.5f},
                    {0.5f, -0.5f,  0.5f},
                    {-0.5f, -0.5f,  0.5f},
                    {-0.5f, -0.5f, -0.5f},

                    {-0.5f,  0.5f, -0.5f},
                    {0.5f,  0.5f, -0.5f},
                    {0.5f,  0.5f,  0.5f},
                    {0.5f,  0.5f,  0.5f},
                    {-0.5f,  0.5f,  0.5f},
                    {-0.5f,  0.5f, -0.5f},
                };

                for (u32 i = 0; i < ArrayCount(vertices); i += 3) {
                    TriPrimitive(windowBuffer, vertices[i+0], vertices[i+1], vertices[i+2], &cmd->MVP,
                                 ToF32(windowBuffer->dims), cmd->color);
                }
#endif

                scan += sizeof(Render_Cmd_Cube);
            } break;

            case RCI_Basis: {
                Render_Cmd_Basis *cmd = (Render_Cmd_Basis *)scan;

#if 0
                f32x4x4 xLineM = MAT4_IDENTITY;
                Translate(&xLineM, (0.5f * cmd->axesDist * F32x3(1.0f, 0.0f, 0.0f)));

                Scale(&xLineM, F32x3(cmd->axesDist, 0.3f, 0.3f));
                f32x4x4 xLineMVP = cmd->MVP * xLineM;

                DrawCube(windowBuffer, &xLineMVP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));

                f32x4x4 yLineM = MAT4_IDENTITY;
                Translate(&yLineM, (0.5f * cmd->axesDist * F32x3(0.0f, 1.0f, 0.0f)));

                Scale(&yLineM, F32x3(0.3f, cmd->axesDist, 0.3f));
                f32x4x4 yLineMVP = cmd->MVP * yLineM;

                DrawCube(windowBuffer, &yLineMVP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));

                f32x4x4 zLineM = MAT4_IDENTITY;
                Translate(&zLineM, (0.5f * cmd->axesDist * F32x3(0.0f, 0.0f, 1.0f)));

                Scale(&zLineM, F32x3(0.3f, 0.3f, cmd->axesDist));
                f32x4x4 zLineMVP = cmd->MVP * zLineM;

                DrawCube(windowBuffer, &zLineMVP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));

#if 0
                f32x4x4 yLineM = MAT4_IDENTITY;
                Translate(&yLineM, (0.5f * cmd->axesDist * F32x3(0.0f, 1.0f, 0.0f)));

                yLineM.c1.xyz = cmd->basis.axes.x;
                yLineM.c2.xyz = cmd->basis.axes.y;
                yLineM.c3.xyz = cmd->basis.axes.z;

                Scale(&yLineM, F32x3(cmd->cubeScale * 0.3f, cmd->axesScale * Len(cmd->basis.axes.y), cmd->cubeScale * 0.3f));
                f32x4x4 yLineMVP = cmd->MVP * yLineM;

                DrawCube(windowBuffer, &yLineMVP, cmdStack->WToP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));

                f32x4x4 zLineM = MAT4_IDENTITY;
                Translate(&zLineM, (0.5f * cmd->axesDist * F32x3(0.0f, 0.0f, 1.0f)));

                zLineM.c1.xyz = cmd->basis.axes.x;
                zLineM.c2.xyz = cmd->basis.axes.y;
                zLineM.c3.xyz = cmd->basis.axes.z;

                Scale(&zLineM, F32x3(cmd->cubeScale * 0.3f, cmd->cubeScale * 0.3f, cmd->axesScale * Len(cmd->basis.axes.z)));
                f32x4x4 zLineMVP = cmd->MVP * zLineM;

                DrawCube(windowBuffer, &zLineMVP, cmdStack->WToP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));
#endif
                DrawCube(windowBuffer, &cmd->MVP, F32x4(1.0f, 1.0f, 1.0f, 1.0f));

                f32x4x4 xM = MAT4_IDENTITY;
                Translate(&xM, F32x3(1.0f, 0.0f, 0.0f) * cmd->axesDist);

                f32x4x4 xMVP = cmd->MVP * xM;

                DrawCube(windowBuffer, &xMVP, F32x4(1.0f, 0.0f, 0.0f, 1.0f));

                f32x4x4 yM = MAT4_IDENTITY;
                Translate(&yM, F32x3(0.0f, 1.0f, 0.0f) * cmd->axesDist);

                f32x4x4 yMVP = cmd->MVP * yM;

                DrawCube(windowBuffer, &yMVP, F32x4(0.0f, 1.0f, 0.0f, 1.0f));

                f32x4x4 zM = MAT4_IDENTITY;
                Translate(&zM, F32x3(0.0f, 0.0f, 1.0f) * cmd->axesDist);

                f32x4x4 zMVP = cmd->MVP * zM;

                DrawCube(windowBuffer, &zMVP, F32x4(0.0f, 0.0f, 1.0f, 1.0f));
#endif

                scan += sizeof(Render_Cmd_Basis);
            } break;

            case RCI_Line: {
                scan += sizeof(Render_Cmd_Line);
            } break;

            NO_DEFAULT
        }
    }
}
#endif
