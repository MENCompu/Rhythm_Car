#define UI_STRING_TABLE_SIZE KiB(64)
#define MAX_UI_LINE 4094

typedef struct {
    u32 controlCount;
    UI_Control controls[MAX_CONTROL_COUNT];

    u32 textCount;
    UI_Text texts[MAX_TEXT_COUNT];

    u32 panelCount;
    UI_Panel panels[MAX_PANEL_COUNT];

    u32 buttonCount;
    UI_Button buttons[MAX_BUTTON_COUNT];

    char textInsertionBuffer[MiB(2)];

    char stringTable[UI_STRING_TABLE_SIZE];
    Free_List freeList;

    u32 updateEventCount;
    UI_Update_Event updateEvents[256];

    UI_Control* underCursor;
    UI_Control* selected;

    Direction selectedLineDirection;

    b8 isSomeEdgeLineSelected;
    Line edgeLineSelected;

    b8 updated;
} UI_State;

Private Global UI_State gUIState;

Private void UI_Text_Create_Draw_Data(UI_Text* ioText);
Private void UI_Panel_Create_Draw_Data(UI_Panel* ioPanel);
Private void* UI_Control_Get_Data(UI_Control* inControl);

Public void UI_Init() {
    Free_List_Init(ArrayCount(gUIState.stringTable), &gUIState.freeList);
}

Public void UI_Cleanup() {
    Foreach (UI_Control, control, gUIState.controls, gUIState.controlCount) {
#if 1
        if ( control->isActive ) {
            switch ( control->type ) {
                case UT_Text: {
                    UI_Text* text = (UI_Text*)UI_Control_Get_Data(control);

                    GAPI_Mesh_Destroy(&text->mesh);
                    GAPI_Material_Destroy(&text->material);
                } break;

#if 1
                case UT_Window: {
                    UI_Window* window = (UI_Window*)UI_Control_Get_Data(control);

                    GAPI_Mesh_Destroy(&window->mesh);
                    GAPI_Material_Destroy(&window->material);
                } break;
#endif

                case UT_Button: {
                    UI_Button* button = (UI_Button*)UI_Control_Get_Data(control);

                    GAPI_Mesh_Destroy(&button->mesh);
                    GAPI_Material_Destroy(&button->material);
                } break;

                case UT_Panel: {
                    UI_Panel* panel = (UI_Panel*)UI_Control_Get_Data(control);

                    GAPI_Mesh_Destroy(&panel->mesh);
                    GAPI_Material_Destroy(&panel->material);
                } break;

                case UI_TYPE_COUNT:
                NO_DEFAULT
            }
        }
#endif
    }

    Mem_Zero_Array(gUIState.controls);
    Mem_Zero_Array(gUIState.texts);
}

Public void UI_Update() {
    for (u32 i = 0; i < gUIState.updateEventCount; ++i) {
        UI_Update_Event* event = &gUIState.updateEvents[i];

        switch ( event->control->type ) {
            case UT_Text: {
                UI_Text* text = (UI_Text*)UI_Control_Get_Data(event->control);
                UI_Text_Create_Draw_Data(text);
            } break;

            case UT_Window:
            case UT_Panel:
            case UT_Button:
            case UI_TYPE_COUNT:
            NO_DEFAULT
        }

        UI_Control_Set_Relative(event->control);

        event->control->updateEventAlreadyPush = JENH_FALSE;
    }

    gUIState.updateEventCount = 0;
}

Public void UI_On_Resize(UI_Control* inControl) {
    inControl->On_Resize(inControl, 0);

    for (UI_Control* node = inControl->next; node; node = node->next) {
        UI_Control_Set_Relative(node);
    }
}

Public UI_Window* UI_Config_Callback() {
    UI_Window* retWindow = 0;

#if 1
    Foreach (UI_Control, control, gUIState.controls, gUIState.controlCount) {
        if ( !control->isActive ) { continue; }

        control->On_Resize = Stub_On_Resize;

        switch ( control->type ) {
            case UT_Text: {
                UI_Text* text = (UI_Text*)UI_Control_Get_Data(control);

                text->mesh.material = &text->material;
                GAPI_Material_Create(PI_UI, &text->material);

                text->mesh.vertexOffset = 0;
                text->mesh.indexOffset = 0;

                if ( !text->text.str ) {
                    text->text.size = 1;
                    text->text.str = S(" ");
                }

                UI_Text_Create_Draw_Data(text);
            } break;

            case UT_Window: {
                UI_Window* window = (UI_Window*)UI_Control_Get_Data(control);

                window->numberString.str = window->numberBuffer;

                if ( !retWindow ) {
                    control->On_Resize = Scene_Window_On_Resize;
                    retWindow = window;
                }

                Create_Quad(window->color, &window->mesh, &window->material);
            } break;

            case UT_Button: {
                UI_Button* button = (UI_Button*)UI_Control_Get_Data(control);

                Create_Quad(button->color, &button->mesh, &button->material);
            } break;

            case UT_Panel: {
                UI_Panel* panel = (UI_Panel*)UI_Control_Get_Data(control);

                Create_Quad(panel->color, &panel->mesh, &panel->material);
            } break;

            case UI_TYPE_COUNT:
            NO_DEFAULT
        }

        control->On_Resize(control, 0);

        Assert( control->next != control && control->prev != control );
    }
#else
    Foreach (UI_Text, text, gUIState.texts, gUIState.textCount) {
        Assert( text->control->type == UT_Text );
        Assert( text->control->next != &text->control && text->control.prev != &text->control );

        text->mesh.material = &text->material;
        GAPI_Material_Create(PI_UI, &text->material);

        text->mesh.vertexOffset = 0;
        text->mesh.indexOffset = 0;

        if ( !text->text.str ) {
            text->text.size = 1;
            text->text.str = S(" ");
        }

        UI_Text_Create_Draw_Data(text);
    }

    Foreach (UI_Window, window, gUIWindowState.windows, gUIWindowState.windowCount) {
        Assert( window->control.type == UT_Window );
        Assert( window->control.next != &window->control && window->control.prev != &window->control );

        window->numberString.str = window->numberBuffer;

        if ( retWindow ) {
            window->control.On_Resize = Stub_On_Resize;
        } else {
            window->control.On_Resize = Scene_Window_On_Resize;
            retWindow = window;
        }

        UI_On_Resize(&window->control);
        Create_Quad(window->color, &window->mesh, &window->material);
    }
#endif

    Assert( retWindow );
    return retWindow;
}

Public void UI_Text_Write(UI_Text* inText, char inChar) {
    File_String_2 newText;
    newText.size = inText->text.size + 1;
    newText.str = (char*)((byte*)gUIState.stringTable + Free_List_Alloc(&gUIState.freeList, newText.size));
    Mem_Copy_Forward(newText.str, inText->text.str, inText->text.size);
    newText.str[newText.size - 1] = inChar;

    Mem_Zero(inText->text.str, inText->text.size);
    u64 oldTextOffset = (u64)((byte*)inText->text.str - (byte*)gUIState.stringTable);
    Free_List_Free(&gUIState.freeList, oldTextOffset, inText->text.size);

    inText->text = newText;

    UI_Control_Update(inText->control, UUET_Text);
}

Public void UI_Control_Set_Relative(UI_Control* inControl) {
    gUIState.updated = JENH_TRUE;

    if ( !inControl->prev ) {
        inControl->rendererTrans = inControl->absTrans;
        return;
    }

    inControl->pivot = F32x2(-1.0f, 1.0f);

    Transform* trans = &inControl->rendererTrans;

    Transform* relTrans = &inControl->prevRelTrans;
    Transform* absTrans = &inControl->absTrans;

    Transform* prevTrans = &inControl->prev->rendererTrans;

    f32x2 prevHalfDims = F32x2_Elem_Prod(inControl->prev->halfDims, prevTrans->scale.xy);

    f32x2 halfDims = F32x2_Elem_Prod(inControl->halfDims, absTrans->scale.xy);

    trans->position.xy = prevTrans->position.xy;
    F32x2_Add_Equal(&trans->position.xy, F32x2_Elem_Prod(prevHalfDims, relTrans->position.xy));
    F32x2_Sub_Equal(&trans->position.xy, F32x2_Elem_Prod(halfDims, inControl->pivot));

    trans->scale = absTrans->scale;
#if 0
    if ( inControl->relOp == URWO_Align_Top_Left ) {
        trans->position.x += halfDims.width;
        trans->position.y -= halfDims.height;
    }
#endif

    trans->position.z = -20.0f;
}

Public UI_Control* UI_Control_Create(UI_Type inType, void* inData, Transform* inAbsTrans, UI_Control* inPrevControl, Transform* inRelTrans,
                                     Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click) {
    gUIState.updated = JENH_TRUE;

    UI_Control* retControl = Array_Push(gUIState.controls, &gUIState.controlCount);

    retControl->isActive = JENH_TRUE;
    retControl->type = inType;
    retControl->data = inData;
    retControl->absTrans = *inAbsTrans;

    if ( inRelTrans ) {
        retControl->prevRelTrans = *inRelTrans;
    }

    retControl->prev = inPrevControl;

    if ( retControl->prev ) {
        retControl->next = retControl->prev->next;
        if ( retControl->prev->next ) { retControl->prev->next->prev = retControl; }

        retControl->prev->next = retControl;
    }

    retControl->Cursor_Over  = ( inUI_Cursor_Over )  ? inUI_Cursor_Over  : Stub_Cursor_Over;
    retControl->Cursor_Click = ( inUI_Cursor_Click ) ? inUI_Cursor_Click : Stub_Cursor_Click;

    return retControl;
}

Public UI_Text* UI_Text_Create(String inText, b8 saveText, Asset_Handle inFont, Transform* inAbsTrans, UI_Control* inPrevControl,
                               Transform* inRelTrans, UI_Relative_Op inRelOp, Fn_UI_Cursor_Over* inUI_Cursor_Over,
                               Fn_UI_Cursor_Click* inUI_Cursor_Click) {
    UI_Text* retText = Array_Push(gUIState.texts, &gUIState.textCount);
    retText->control = UI_Control_Create(UT_Text, retText, inAbsTrans, inPrevControl, inRelTrans, inUI_Cursor_Over, inUI_Cursor_Click);

    retText->control->relOp = inRelOp;

    if ( saveText ) {
        File_String_2 textCopy;
        textCopy.size = inText.size;

        textCopy.str = (char*)((byte*)gUIState.stringTable + Free_List_Alloc(&gUIState.freeList, textCopy.size));
        Mem_Copy_Forward(textCopy.str, inText.str, textCopy.size);

        retText->text = textCopy;
    } else {
        File_String_2 string;
        string.size = inText.size;
        string.str = inText.str;
        retText->text = string;
    }

    retText->font = inFont;

    retText->mesh.vertexSize = sizeof(Vertex_Data);
    retText->mesh.vertexOffset = 0;

    retText->mesh.indexSize = sizeof(u32);
    retText->mesh.indexOffset = 0;

    retText->mesh.material = &retText->material;

    retText->material.diffuseColor  = F32x4(1.0f, 1.0f, 1.0f, 1.0f);
    retText->material.specularColor = F32x4(1.0f, 1.0f, 1.0f, 1.0f);
    GAPI_Material_Create(PI_UI, &retText->material);

    UI_Text_Create_Draw_Data(retText);

    UI_Control_Set_Relative(retText->control);

    return retText;
}

Public void UI_Text_Destroy(UI_Text* inText) {
    inText->control->isActive = JENH_FALSE;

    GAPI_Mesh_Destroy(&inText->mesh);
    GAPI_Material_Destroy(&inText->material);
}

Public void UI_Text_Change_Text(String inNewText, UI_Text* inText) {
    GAPI_Mesh_Destroy(&inText->mesh);

    File_String_2 string;
    string.size = inNewText.size;
    string.str = inNewText.str;
    inText->text = string;

    UI_Text_Create_Draw_Data(inText);
}

Public void UI_Text_Change_Font(Asset_Handle inFont, UI_Text* inText) {
    inText->font = inFont;
    UI_Text_Create_Draw_Data(inText);
}

Private void UI_Text_Create_Draw_Data(UI_Text* ioText) {
    Font* font = Asset_Get_Font(ioText->font);

    ioText->mesh.vertexCount = ioText->text.size * 4;
    ioText->mesh.indexCount  = ioText->text.size * 6;

    Vertex_Data* vertices = ArenaPushArray(&dyMem.temp.arena, Vertex_Data, ioText->mesh.vertexCount);
    u32* indices = ArenaPushArray(&dyMem.temp.arena, u32, ioText->mesh.indexCount);

    f32 xPos = 0.0f;
    f32 yPos = 0.0f;
    f32 scale = 10.0f;

    char* scan = ioText->text.str;

    f32x2 minPoint = F32x2(MAX_F32, MAX_F32);
    f32x2 maxPoint = F32x2(-MAX_F32, -MAX_F32);

    for (u32 i = 0; i < ioText->text.size; ++i, ++scan) {
        RCAF_Code_Point_Data* data = 0;

        switch ( *scan ) {
            case '\n': {
                yPos -= ((f32)font->lineHeight / (f32)font->bitmapWidth) * scale;
                xPos = 0.0f;
                continue;
            } break;
        }

        for (u32 j = 0; j < font->codePointCount; ++j) {
            data = &font->table[j];

            if ( data->codePoint == *scan ) {
                data = &font->table[j];
                break;
            }
        }

        Assert( data != 0 );

        f32 xAdvance = ((f32)data->xAdvance / (f32)font->bitmapWidth);

        f32 normWidth =  ((f32)data->width / (f32)font->bitmapWidth);
        f32 normHeight = ((f32)data->height / (f32)font->bitmapHeight);

        f32 texCoordMinX = (f32)data->xPos / (f32)font->bitmapWidth;
        f32 texCoordMaxX = texCoordMinX + normWidth;
        f32 texCoordMinY = (f32)(font->bitmapHeight - data->yPos) / (f32)font->bitmapHeight;
        f32 texCoordMaxY = texCoordMinY - normHeight;

        f32 xOffset = (f32)data->xOffset / (f32)font->bitmapWidth;
        f32 yOffset = (f32)(data->yOffset) / (f32)font->bitmapHeight;

        f32 minX = xPos + (xOffset * scale);
        f32 maxX = minX + (normWidth * scale);
        f32 maxY = yPos - (yOffset * scale);
        f32 minY = maxY - (normHeight * scale);

        vertices[(i*4) + 0] = { F32x3(minX, minY, 0.0f), F32x3(1.0f), F32x2(texCoordMinX, texCoordMaxY) };
        vertices[(i*4) + 1] = { F32x3(maxX, minY, 0.0f), F32x3(1.0f), F32x2(texCoordMaxX, texCoordMaxY) };
        vertices[(i*4) + 2] = { F32x3(minX, maxY, 0.0f), F32x3(1.0f), F32x2(texCoordMinX, texCoordMinY) };
        vertices[(i*4) + 3] = { F32x3(maxX, maxY, 0.0f), F32x3(1.0f), F32x2(texCoordMaxX, texCoordMinY) };

        indices[(i*6) + 0] = (i*4) + 2;
        indices[(i*6) + 1] = (i*4) + 0;
        indices[(i*6) + 2] = (i*4) + 1;
        indices[(i*6) + 3] = (i*4) + 1;
        indices[(i*6) + 4] = (i*4) + 3;
        indices[(i*6) + 5] = (i*4) + 2;

        if ( *scan != ' ' ) {
            minPoint.x = Min(minPoint.x, minX);
            minPoint.y = Min(minPoint.y, minY);

            maxPoint.x = Max(maxPoint.x, maxX);
            maxPoint.y = Max(maxPoint.y, maxY);
        }

        xPos += xAdvance * scale;
    }

    ioText->control->halfDims = F32x2((maxPoint.x - minPoint.x) * 0.5f, (maxPoint.y - minPoint.y) * 0.5f);
    ioText->control->center = F32x2(minPoint.x + ioText->control->halfDims.x, minPoint.y + ioText->control->halfDims.y);

    if ( ioText->mesh.vertexOffset != 0 ) {
        GAPI_Mesh_Destroy(&ioText->mesh);
    }

    GAPI_Mesh_Create(vertices, indices, &ioText->mesh);

    ioText->material.diffuseTex = font->bitmap;
    ioText->material.normalTex  = font->bitmap;
}

Public UI_Panel* UI_Panel_Create(f32x4 inColor, Asset_Handle inTex, Transform* inAbsTrans, UI_Control* inPrevControl,
                                 Transform* inRelTrans, Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click) {
    UI_Panel* retPanel = Array_Push(gUIState.panels, &gUIState.panelCount);
    retPanel->control = UI_Control_Create(UT_Button, retPanel, inAbsTrans, inPrevControl, inRelTrans, inUI_Cursor_Over, inUI_Cursor_Click);

    retPanel->color = inColor;
    retPanel->tex = inTex;
    Create_Quad(inColor, &retPanel->mesh, &retPanel->material);

    retPanel->control->center   = F32x2(0.0f, 0.0f);
    retPanel->control->halfDims = F32x2(1.0f, 1.0f);

    UI_Control_Set_Relative(retPanel->control);

    return retPanel;
}

Private void UI_Panel_Create_Draw_Data(UI_Panel* ioPanel) {
    f32 minX = -1.0f;
    f32 maxX =  1.0f;
    f32 minY = -1.0f;
    f32 maxY =  1.0f;

    f32 texCoordMinX = 0.0f;
    f32 texCoordMaxX = 1.0f;
    f32 texCoordMinY = 0.0f;
    f32 texCoordMaxY = 1.0f;

#if 1
    Vertex_Data vertices[] = {
        { F32x3(-1.0f,-1.0f, 0.0f), F32x3(1.0f), F32x2(0.0f,0.0f) },
        { F32x3( 1.0f,-1.0f, 0.0f), F32x3(1.0f), F32x2(1.0f,0.0f) },
        { F32x3(-1.0f, 1.0f, 0.0f), F32x3(1.0f), F32x2(0.0f,1.0f) },
        { F32x3( 1.0f, 1.0f, 0.0f), F32x3(1.0f), F32x2(1.0f,1.0f) },
    };
#endif

    u32 indices[6];

    indices[0] = 2;
    indices[1] = 0;
    indices[2] = 1;
    indices[3] = 1;
    indices[4] = 3;
    indices[5] = 2;

    if ( ioPanel->mesh.vertexOffset != 0 ) {
        GAPI_Mesh_Destroy(&ioPanel->mesh);
    }

    GAPI_Mesh_Create(vertices, indices, &ioPanel->mesh);

    ioPanel->material.diffuseColor = ioPanel->color;
    ioPanel->material.diffuseTex = ioPanel->tex;
    ioPanel->material.normalTex  = ioPanel->tex;
}

Public UI_Button* UI_Button_Create(f32x4 inColor, Asset_Handle inTex, Transform* inAbsTrans, UI_Control* inPrevControl,
                                   Transform* inRelTrans, Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click) {
    UI_Button* retButton = Array_Push(gUIState.buttons, &gUIState.buttonCount);
    retButton->control = UI_Control_Create(UT_Button, retButton, inAbsTrans, inPrevControl, inRelTrans, inUI_Cursor_Over, inUI_Cursor_Click);

    retButton->color = inColor;
    retButton->tex = inTex;
    Create_Quad(inColor, &retButton->mesh, &retButton->material);

    retButton->control->center   = F32x2(0.0f, 0.0f);
    retButton->control->halfDims = F32x2(1.0f, 1.0f);

    UI_Control_Set_Relative(retButton->control);

    return retButton;
}

Public b8 UI_Control_Is_Point_Inside(UI_Control* inControl, f32x2 inPoint) {
    Coll_Rect_2D rect;
    Coll_Rect_2D_Create(inControl->rendererTrans.position.xy, F32x2_Elem_Prod(inControl->halfDims, inControl->rendererTrans.scale.xy), &rect);

    return Coll_2D_Is_Intersecting_Rect_Point(&rect, inPoint);
}

Public inline void UI_Cursor_Click() {
    if ( !gUIState.underCursor ) {
        gUIState.selected = 0;
    } else {
        gUIState.selected = gUIState.underCursor;
        gUIState.selected->Cursor_Click(gUIState.underCursor);
    }
}

Public inline void UI_Cursor_Over(UI_Control* inControl) {
    inControl->Cursor_Over(inControl);
}

Public b8 UI_Check_Edge_Coll(UI_Control* inControl, f32x2 inPoint, Line* outLine) {
    Coll_Rect_2D rect;

    Transform* trans = &inControl->rendererTrans;

    f32x2 halfDims = F32x2_Elem_Prod(inControl->halfDims, trans->scale.xy);

    Coll_Rect_2D_Create(F32x2(trans->position.x, trans->position.y + halfDims.height), F32x2(halfDims.width, 0.01f), &rect);
    if ( Coll_2D_Is_Intersecting_Rect_Point(&rect, inPoint) ) {
        gUIState.selectedLineDirection = Direction_Up;
        outLine->a.xy = F32x2(trans->position.x - halfDims.width, trans->position.y + halfDims.height);
        outLine->b.xy = F32x2(trans->position.x + halfDims.width, trans->position.y + halfDims.height);
        return JENH_TRUE;
    }

    Coll_Rect_2D_Create(F32x2(trans->position.x, trans->position.y - halfDims.height), F32x2(halfDims.width, 0.01f), &rect);
    if ( Coll_2D_Is_Intersecting_Rect_Point(&rect, inPoint) ) {
        gUIState.selectedLineDirection = Direction_Down;
        outLine->a.xy = F32x2(trans->position.x - halfDims.width, trans->position.y - halfDims.height);
        outLine->b.xy = F32x2(trans->position.x + halfDims.width, trans->position.y - halfDims.height);
        return JENH_TRUE;
    }

    Coll_Rect_2D_Create(F32x2(trans->position.x + halfDims.width, trans->position.y), F32x2(0.01f, halfDims.height), &rect);
    if ( Coll_2D_Is_Intersecting_Rect_Point(&rect, inPoint) ) {
        gUIState.selectedLineDirection = Direction_Right;
        outLine->a.xy = F32x2(trans->position.x + halfDims.width, trans->position.y - halfDims.height);
        outLine->b.xy = F32x2(trans->position.x + halfDims.width, trans->position.y + halfDims.height);
        return JENH_TRUE;
    }

    Coll_Rect_2D_Create(F32x2(trans->position.x - halfDims.width, trans->position.y), F32x2(0.01f, halfDims.height), &rect);
    if ( Coll_2D_Is_Intersecting_Rect_Point(&rect, inPoint) ) {
        gUIState.selectedLineDirection = Direction_Left;
        outLine->a.xy = F32x2(trans->position.x - halfDims.width, trans->position.y - halfDims.height);
        outLine->b.xy = F32x2(trans->position.x - halfDims.width, trans->position.y + halfDims.height);
        return JENH_TRUE;
    }

    return JENH_FALSE;
}

Public void UI_Select_From_Cursor() {
    gUIState.selected = gUIState.underCursor;
}

Public void UI_Cursor_Update(f32x2 newCursorPos) {
    gUIState.isSomeEdgeLineSelected = JENH_FALSE;
    gUIState.underCursor = 0;

    for (u32 i = 0; i < gUIState.controlCount; ++i) {
        UI_Control* control = &gUIState.controls[i];

        if ( control->type != UT_Window && UI_Check_Edge_Coll(control, newCursorPos, &gUIState.edgeLineSelected) ) {
            gUIState.edgeLineSelected.a.z = -10.0f;
            gUIState.edgeLineSelected.b.z = -10.0f;

            gUIState.edgeLineSelected.width = 2.0f;
            gUIState.edgeLineSelected.material = Renderer_Material_From_Color(F32x4(0.5f, 0.0f, 0.5f, 1.0f), PI_Wireframe);

            gUIState.isSomeEdgeLineSelected = JENH_TRUE;
            goto is_under_cursor;
        }

        if ( control->type != UT_Window && UI_Control_Is_Point_Inside(control, newCursorPos) ) {
is_under_cursor:
            UI_Cursor_Over(control);
            gUIState.underCursor = control;
            break;
        }
    }
}

Public void* UI_Control_Get_Data(UI_Control* inControl) {
    return inControl->data;
}

Private void UI_Control_Draw_Line(UI_Control* inControl, f32 inPos, Axis inAxis, f32 inLayer, f32x4 inColor) {
    Axis oppAxis = ( inAxis == Axis_X ) ? Axis_Y : Axis_X;

    Transform* trans = &inControl->rendererTrans;
    f32x2 halfDims = F32x2_Elem_Prod(inControl->halfDims, trans->scale.xy);

    f32 relPos = trans->position.E[inAxis] + (inPos * halfDims.E[inAxis]);

    Line line;
    line.a.z = inLayer;
    line.b.z = inLayer;

    line.a.E[inAxis] = relPos;
    line.a.E[oppAxis] = trans->position.E[oppAxis] - halfDims.E[oppAxis];

    line.b.E[inAxis] = relPos;
    line.b.E[oppAxis] = trans->position.E[oppAxis] + halfDims.E[oppAxis];

    line.width = 2.0f;
    line.material = Renderer_Material_From_Color(inColor, PI_Wireframe);

    Renderer_Draw_Line(&line);
}

Public inline void UI_Control_Draw_OutLine(UI_Control* inControl, f32 inLayer, f32x4 inColor) {
    UI_Control_Draw_Line(inControl, -1.0f, Axis_X, inLayer, inColor);
    UI_Control_Draw_Line(inControl, +1.0f, Axis_X, inLayer, inColor);
    UI_Control_Draw_Line(inControl, -1.0f, Axis_Y, inLayer, inColor);
    UI_Control_Draw_Line(inControl, +1.0f, Axis_Y, inLayer, inColor);
}

Public inline void UI_Control_Update(UI_Control* inControl, UI_Update_Event_Type inType) {
    if ( inControl->updateEventAlreadyPush ) { return; }

    gUIState.updated = JENH_TRUE;

    UI_Update_Event* event = Array_Push(gUIState.updateEvents, &gUIState.updateEventCount);
    inControl->updateEventAlreadyPush = JENH_TRUE;
    event->control = inControl;
}
