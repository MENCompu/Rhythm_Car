typedef struct {
    b8 textMode;

    Input_Mapping mapping;

    Game_Actions gameActions;
    Msgs_Actions msgsActions;

    u32 mouseCooldown;

    union {
        struct {
            Action_Pool poolA;
            Action_Pool poolB;
        };
        Action_Pool pools[2];
    };
} Input_System;

Private Global Input_System gInputSystem;

Private void InputRecordsInitState(Memory_Arena *arena, Input_Records_State *initState, Input_Mapping *inputMapping);
Private void BindingAppend(Memory_Arena *arena, Input_Records *inputRecords, Action_Index actionIndex, Array_Input_Index keys);
Private void SaveToGameInput(Input_Records *inputRecords);

Public b8 Input_Text_Mode_Enabled() {
    return gInputSystem.textMode;
}

Intern inline Action_ID ActionIndexToActionID(Action_Index actionIndex, u64 actionFlag) {
    Action_ID ret = (Action_ID)((u64)(actionIndex)|actionFlag);
    return ret;
}

Intern inline b8 IsExtendedInput(Input_Index inputIndex) {
    b8 ret = (inputIndex >= EXTENDED_INPUT_INDEX_START);
    return ret;
}

Public inline u32 ExtendedInput(Input_Index inputIndex) {
    u32 ret = (u32)(inputIndex - EXTENDED_INPUT_INDEX_START);
    return ret;
}

Intern inline b8 IsExtendedAction(Action_Index actionIndex) {
    b8 ret = (actionIndex >= EXTENDED_ACTION_INDEX_START);
    return ret;
}

Public void Input_Update_Actions(Input_Index inInputIndex, b8 inIsPress, f32 inTimeStamp) {
    Input_Mapping *inputMapping = &gInputSystem.mapping;
    Bit_Flags_Inputs_State *inputsState = &inputMapping->inputsState;

    if ( inIsPress && Bit_Flags_Is_Set(inputsState, inInputIndex) ) { return; }

    Action action = { 0 };

    Action_Index actionIndex = AI_Nul;

    action.ID = A_Nul;
    action.timeStamp = inTimeStamp;

    Assert(inInputIndex < INPUT_INDEX_COUNT);
    Input_Binding_Indices_Collection bindingsIndices = inputMapping->inputToBindings[inInputIndex];

    if ( inIsPress ) {
        Bit_Flags_Set(inputsState, inInputIndex);
    }

    Map_Action_Bindings *mapActionBindings = inputMapping->bindingsPerAction;

    u32 higherInputsMatchCount = 0;
    u8  savedBindingIndex = 0;

    Bit_Flags_Bindings_State *bindingsState = inputMapping->bindingsState;

    foreach (Binding_Indices, indices, bindingsIndices) {
        u8 actionType = mapActionBindings[indices->action].actionType;

        Array_Input_Index inputs = mapActionBindings[indices->action].bindings.A[indices->binding].inputs;

        foreach (Input_Index, bInputIndex, inputs) {
            if ( !Bit_Flags_Is_Set(inputsState, *bInputIndex) ) {
                goto binding_dont_match;
            }
        }

#if 1
        if ( inputs.size > higherInputsMatchCount ) {
            higherInputsMatchCount = inputs.size;
            savedBindingIndex = indices->binding;
            actionIndex = (Action_Index)indices->action;
        }
#endif

binding_dont_match:;
    }

    if ( !inIsPress ) {
        Bit_Flags_Unset(inputsState, inInputIndex);
    }

    if (actionIndex == AI_Nul) { return; }

#if 1
    if ( inIsPress ) {
        if (Bit_Flags_Is_Empty(&bindingsState[actionIndex])) {
            action.ID = ActionIndexToActionID(actionIndex, ACTION_BEGIN_MASK);

            Action_Pool* inputPool = gInputSystem.msgsActions.pool;
            inputPool->A[inputPool->size++] = action;

            if ( IsExtendedAction(actionIndex) ) {
                Array_Input_Index inputs = mapActionBindings[actionIndex].bindings.A[savedBindingIndex].inputs;
                foreach (Input_Index, bInputIndex, inputs) {
                    if ( IsExtendedInput(*bInputIndex) ) {
                        Extended_Input* extendedInput = &inputMapping->extendedInputs[ExtendedInput(*bInputIndex)];
                        extendedInput->actionIDAttached = ActionIndexToActionID(actionIndex, ACTION_HELD_MASK);
                        break;
                    }
                }
            } else {
                // NOTE(JENH): Insert action from held buffer.
                Action heldAction = {0};
                heldAction.ID = ActionIndexToActionID(actionIndex, ACTION_HELD_MASK);

                Held_Action_Buffer *heldBuffer = &gInputSystem.msgsActions.heldBuffer;
                heldBuffer->A[heldBuffer->size++] = heldAction;
            }
        }

        Bit_Flags_Set(&bindingsState[actionIndex], savedBindingIndex);
    } else {
        Bit_Flags_Unset(&bindingsState[actionIndex], savedBindingIndex);

        if ( Bit_Flags_Is_Empty(&bindingsState[actionIndex]) ) {
            action.ID = ActionIndexToActionID(actionIndex, ACTION_END_MASK);

            Action_Pool *inputPool = gInputSystem.msgsActions.pool;
            inputPool->A[inputPool->size++] = action;

            if (IsExtendedAction(actionIndex)) {
                Array_Input_Index inputs = mapActionBindings[actionIndex].bindings.A[savedBindingIndex].inputs;
                foreach (Input_Index, bInputIndex, inputs) {
                    if (IsExtendedInput(*bInputIndex)) {
                        Extended_Input *extendedInput = &inputMapping->extendedInputs[ExtendedInput(*bInputIndex)];
                        extendedInput->actionIDAttached = A_Nul;
                        break;
                    }
                }
            } else {
                // NOTE(JENH): Delete action from held buffer.
                Held_Action_Buffer *heldBuffer = &gInputSystem.msgsActions.heldBuffer;

                Action_ID heldActionID = ActionIndexToActionID(actionIndex, ACTION_HELD_MASK);

                if (heldBuffer->size == 0) { return; }

                // NOTE(JENH): Compress the pool.
                u32 actionToDelIndex = 0;
                for (;actionToDelIndex < heldBuffer->size; ++actionToDelIndex) {
                    if (heldBuffer->A[actionToDelIndex].ID == heldActionID) { break; }
                }

                for (;actionToDelIndex < (u16)(heldBuffer->size - 1); ++actionToDelIndex) {
                    heldBuffer->A[actionToDelIndex] = heldBuffer->A[actionToDelIndex + 1];
                }

                --heldBuffer->size;
            }
        }
    }
#endif
}

Public void UpdateActionPools() {
    Game_Actions* gameActions = &gInputSystem.gameActions;
    Msgs_Actions* msgsActions = &gInputSystem.msgsActions;

    Swap(&gameActions->pool, &msgsActions->pool, Action_Pool*);
    msgsActions->pool->size = 0;
    //Mem_Zero_Type(&msgsActions->pool->A);

    foreach (Action, heldAction, msgsActions->heldBuffer) {
        for (s32 gameActionsPoolIndex = (s32)(gameActions->pool->size - 1);
            gameActionsPoolIndex >= 0; --gameActionsPoolIndex) {
            Action_ID gameActionID = gameActions->pool->A[gameActionsPoolIndex].ID;

            if ( gameActionID == ((Action_ID)((u64)heldAction->ID) | ACTION_BEGIN_MASK) ) {
                goto begin_actions_dont_held_the_same_frame;
            }
        }

        gameActions->pool->A[gameActions->pool->size++] = *heldAction;
begin_actions_dont_held_the_same_frame:;
    }

    for (u32 i = 0; i < EXTENDED_INPUT_INDEX_COUNT; ++i) {
        Extended_Input *extendedInput = &gInputSystem.mapping.extendedInputs[i];
        if ( extendedInput->actionIDAttached != A_Nul ) {
            Action action;
            action.ID = extendedInput->actionIDAttached;
            action.timeStamp = 0.0f;
            action.data = extendedInput->data;

            // NOTE(JENH): adding a held type action.
            gameActions->pool->A[gameActions->pool->size++] = action;

            u32 pene = EXTENDED_INPUT_INDEX_START;
            u32 hola = (u32)(II_Ms_Movement - EXTENDED_INPUT_INDEX_START);
            if ( i == II_Ms_Movement - EXTENDED_INPUT_INDEX_START ) {
                Mem_Zero_Type(&extendedInput->data);
            }
        }
    }
}

Public Input_Mapping* Input_Get_Mappings() {
    return &gInputSystem.mapping;
}

Public void Input_Init() {
    gInputSystem.gameActions.pool = &gInputSystem.poolA;
    gInputSystem.msgsActions.pool = &gInputSystem.poolB;
    //gInputSystem.msgsActions.heldBuffer.size = 0;

    Input_Mapping *mapping = &gInputSystem.mapping;

    Input_Records inputRecords = {0};
    Memory_Arena *arena = AllocTempArena(MiB(1));
    InputRecordsInitState(arena, &inputRecords.initState, mapping);

    // TODO(JENH): input bindings should be in a config file.
    Local_Array_Init(Input_Index, splitLeft, II_Kbd_Ctrl_L, II_Kbd_J);
    BindingAppend(arena, &inputRecords, AI_Split_Left, splitLeft);

    Local_Array_Init(Input_Index, splitUp, II_Kbd_Ctrl_L, II_Kbd_I);
    BindingAppend(arena, &inputRecords, AI_Split_Up, splitUp);

    Local_Array_Init(Input_Index, splitRight, II_Kbd_Ctrl_L, II_Kbd_L);
    BindingAppend(arena, &inputRecords, AI_Split_Right, splitRight);

    Local_Array_Init(Input_Index, splitDown, II_Kbd_Ctrl_L, II_Kbd_K);
    BindingAppend(arena, &inputRecords, AI_Split_Down, splitDown);

    Local_Array_Init(Input_Index, windowLeft, II_Kbd_J);
    BindingAppend(arena, &inputRecords, AI_Window_Left, windowLeft);

    Local_Array_Init(Input_Index, windowUp, II_Kbd_I);
    BindingAppend(arena, &inputRecords, AI_Window_Up, windowUp);

    Local_Array_Init(Input_Index, windowRight, II_Kbd_L);
    BindingAppend(arena, &inputRecords, AI_Window_Right, windowRight);

    Local_Array_Init(Input_Index, windowDown, II_Kbd_K);
    BindingAppend(arena, &inputRecords, AI_Window_Down, windowDown);

    // player movement
    Local_Array_Init(Input_Index, playerMove, II_Xbox_360_LS);
    BindingAppend(arena, &inputRecords, AI_Player_Move, playerMove);

#if 1
    Local_Array_Init(Input_Index, moveLeft, II_Kbd_Left);
    BindingAppend(arena, &inputRecords, AI_Player_Left, moveLeft);

    Local_Array_Init(Input_Index, moveRight, II_Kbd_Right);
    BindingAppend(arena, &inputRecords, AI_Player_Right, moveRight);
#endif
    //

    Local_Array_Init(Input_Index, sizeLeft, II_Kbd_Left);
    BindingAppend(arena, &inputRecords, AI_Size_Left, sizeLeft);

    Local_Array_Init(Input_Index, sizeDown, II_Kbd_Down);
    BindingAppend(arena, &inputRecords, AI_Size_Down, sizeDown);

    Local_Array_Init(Input_Index, sizeRight, II_Kbd_Right);
    BindingAppend(arena, &inputRecords, AI_Size_Right, sizeRight);

    Local_Array_Init(Input_Index, sizeUp, II_Kbd_Up);
    BindingAppend(arena, &inputRecords, AI_Size_Up, sizeUp);


    Local_Array_Init(Input_Index, saveConfig, II_Kbd_1);
    BindingAppend(arena, &inputRecords, AI_Save_Config, saveConfig);

    Local_Array_Init(Input_Index, loadConfig, II_Kbd_2);
    BindingAppend(arena, &inputRecords, AI_Load_Config, loadConfig);

    Local_Array_Init(Input_Index, createText, II_Kbd_T);
    BindingAppend(arena, &inputRecords, AI_Create_Text, createText);

    Local_Array_Init(Input_Index, textMode, II_Kbd_Y);
    BindingAppend(arena, &inputRecords, AI_Text_Mode, textMode);


    Local_Array_Init(Input_Index, mouseUI, II_Ms_Movement);
    BindingAppend(arena, &inputRecords, AI_UI_Cursor, mouseUI);

    Local_Array_Init(Input_Index, clickUI, II_Ms_Button1);
    BindingAppend(arena, &inputRecords, AI_UI_Click, clickUI);

    Local_Array_Init(Input_Index, createUI, II_Kbd_N);
    BindingAppend(arena, &inputRecords, AI_UI_Create, createUI);

    Local_Array_Init(Input_Index, moveUI, II_Ms_Movement, II_Ms_Button1);
    BindingAppend(arena, &inputRecords, AI_UI_Move, moveUI);

    Local_Array_Init(Input_Index, modeUI, II_Kbd_U);
    BindingAppend(arena, &inputRecords, AI_UI_Mode, modeUI);


    Local_Array_Init(Input_Index, moveCurve, II_Ms_Movement, II_Ms_Button1);
    BindingAppend(arena, &inputRecords, AI_Move_Curve, moveCurve);

    Local_Array_Init(Input_Index, debugMode, II_Kbd_Q);
    BindingAppend(arena, &inputRecords, AI_Debug_Mode, debugMode);

    Local_Array_Init(Input_Index, renderAABBs, II_Kbd_R);
    BindingAppend(arena, &inputRecords, AI_Render_AABB, renderAABBs);

    Local_Array_Init(Input_Index, insertCube, II_Ms_Button1);
    BindingAppend(arena, &inputRecords, AI_Insert_Road, insertCube);

    Local_Array_Init(Input_Index, camera, II_Ms_Movement);
    BindingAppend(arena, &inputRecords, AI_Move_Cam, camera);

    Local_Array_Init(Input_Index, debugPointer, II_Ms_Movement);
    BindingAppend(arena, &inputRecords, AI_Debug_Pointer, debugPointer);

    Local_Array_Init(Input_Index, sprint, II_Kbd_Shift_L);
    BindingAppend(arena, &inputRecords, AI_Sprint, sprint);

    Local_Array_Init(Input_Index, w, II_Kbd_W);
    BindingAppend(arena, &inputRecords, AI_Move_Forward, w);
    //Local_Array_Init(Input_Index, up, II_Kbd_Up);
    //BindingAppend(arena, &inputRecords, AI_Move_Forward, up);

    Local_Array_Init(Input_Index, s, II_Kbd_S);
    BindingAppend(arena, &inputRecords, AI_Move_Backward, s);
    //Local_Array_Init(Input_Index, down, II_Kbd_Down);
    //BindingAppend(arena, &inputRecords, AI_Move_Backward, down);

    Local_Array_Init(Input_Index, a, II_Kbd_A);
    BindingAppend(arena, &inputRecords, AI_Move_Left, a);
    //Local_Array_Init(Input_Index, left, II_Kbd_Left);
    //BindingAppend(arena, &inputRecords, AI_Move_Left, left);

    Local_Array_Init(Input_Index, d, II_Kbd_D);
    BindingAppend(arena, &inputRecords, AI_Move_Right, d);
    //Local_Array_Init(Input_Index, right, II_Kbd_Right);
    //BindingAppend(arena, &inputRecords, AI_Move_Right, right);

    Local_Array_Init(Input_Index, space, II_Kbd_Space);
    BindingAppend(arena, &inputRecords, AI_Move_Up, space);

    Local_Array_Init(Input_Index, c, II_Kbd_Ctrl_L);
    BindingAppend(arena, &inputRecords, AI_Move_Down, c);

    Local_Array_Init(Input_Index, fullScreen, II_Kbd_F);
    BindingAppend(arena, &inputRecords, AI_FullScreen, fullScreen);

    Local_Array_Init(Input_Index, close, II_Kbd_Alt_L, II_Kbd_F4);
    BindingAppend(arena, &inputRecords, AI_Close, close);
    Local_Array_Init(Input_Index, close2, II_Kbd_Esc);
    BindingAppend(arena, &inputRecords, AI_Close, close2);

    Local_Array_Init(Input_Index, drive, II_Kbd_V);
    BindingAppend(arena, &inputRecords, AI_Drive, drive);
    Local_Array_Init(Input_Index, drive2, II_Xbox_360_Y);
    BindingAppend(arena, &inputRecords, AI_Drive, drive2);

    Local_Array_Init(Input_Index, sponza, II_Kbd_Z);
    BindingAppend(arena, &inputRecords, AI_Sponza, sponza);

    SaveToGameInput(&inputRecords);

    FreeTempArena(arena);
}

typedef enum {
    IACT_Move_To_Top = 0x00,
    IACT_Move_To_Btm = 0x01,
} Input_Action_Change_Type;

typedef struct {
    Input_Action_Change_Type type;
    Action_Index action;
} Input_Action_Change_Entry;

Public void Input_Change_Actions(Input_Action_Change_Entry* inChanges, u32 inChangeCount) {
    Input_Mapping* mapping = &gInputSystem.mapping;

    Foreach (Input_Action_Change_Entry, change, inChanges, inChangeCount) {
        Assert( change->action < ACTION_COUNT );
        Array_Binding bindings = mapping->bindingsPerAction[change->action].bindings;

        Foreach (Binding, binding, bindings.A, bindings.size) {
            Foreach (Input_Index, input, binding->inputs.A, binding->inputs.size) {
                Input_Binding_Indices_Collection* bindingIndices = &mapping->inputToBindings[*input];

                for (u32 actionRefIndex = 0; actionRefIndex < bindingIndices->size; ++actionRefIndex) {
                    Binding_Indices* indices = &bindingIndices->A[actionRefIndex];

                    if ( indices->action == change->action ) {
                        switch ( change->type ) {
                            case IACT_Move_To_Top: {
                                if ( actionRefIndex == 0 ) { break; }

                                Binding_Indices temp = *indices;
                                Mem_Shift_Right_Array(bindingIndices->A, 0, actionRefIndex - 1, 1);
                                bindingIndices->A[0] = temp;
                            } break;

                            case IACT_Move_To_Btm: {
                                if ( actionRefIndex == (bindingIndices->size - 1) ) { break; }

                                Binding_Indices temp = *indices;
                                Mem_Shift_Left_Array(bindingIndices->A, actionRefIndex + 1, bindingIndices->size - 1, 1);

                                bindingIndices->A[bindingIndices->size - 1] = temp;
                            } break;

                            NO_DEFAULT
                        }

                        goto action_find;
                    }
                }

                /* if not action_find */ {
                    LogFatal("Failed to find the action reference in the input mappings");
                }
                action_find:;
            }
        }
    }
}

Private void InputRecordsInitState(Memory_Arena *arena, Input_Records_State *initState, Input_Mapping *inputMapping) {
    for (u32 i = 0; i < ACTION_COUNT; ++i) {
        Array_Binding bindings = inputMapping->bindingsPerAction[i].bindings;

        if (bindings.size == 0) { continue; }

        List_Node_Binding *bindingsInit = &initState->bindings[i];
        bindingsInit->size = bindings.size;
        bindingsInit->list = ArenaPushArray(arena, Node_Binding, bindings.size);

        for (u32 nodeIndex = 0; nodeIndex < bindings.size; ++nodeIndex) {
            Node_Binding *node = &bindingsInit->list[nodeIndex];
            Binding *binding = &bindings.A[nodeIndex];

            // NOTE(JENH): Copying binding to the arena.
            node->binding.inputs.size = binding->inputs.size;
            node->binding.inputs.A = ArenaPushArray(arena, Input_Index, binding->inputs.size);;
            ArrayCopy(node->binding.inputs.A, binding->inputs.A, binding->inputs.size);

            node->next = &bindingsInit->list[nodeIndex + 1];
        }

        // NOTE(JENH): Nulling last node next.
        bindingsInit->list[bindings.size - 1].next = 0;
    }

    for (u32 i = 0; i < INPUT_INDEX_COUNT; ++i) {
        Input_Binding_Indices_Collection bindingIndices = inputMapping->inputToBindings[i];

        if (bindingIndices.size == 0) { continue; }

        List_Node_Binding_Indices *keyboardInit = &initState->inputs[i];
        keyboardInit->size = bindingIndices.size;
        keyboardInit->list = ArenaPushArray(arena, Node_Binding_Indices, bindingIndices.size);

        for (u32 nodeIndex = 0; nodeIndex < bindingIndices.size; ++nodeIndex) {
            Node_Binding_Indices *node = &keyboardInit->list[nodeIndex];

            node->indices = bindingIndices.A[nodeIndex];
            node->next = &keyboardInit->list[nodeIndex + 1];
        }

        // NOTE(JENH): Nulling last node next.
        keyboardInit->list[bindingIndices.size - 1].next = 0;
    }
}

Intern void SaveToGameInput(Input_Records *inputRecords) {
    Memory_Arena *arena = &dyMem.perma.bindings;
    arena->used = 0;

    Input_Mapping *mapping = &gInputSystem.mapping;

    for (u32 i = 0; i < ACTION_COUNT; ++i) {
        Array_Binding *bindingsPerma = &mapping->bindingsPerAction[i].bindings;
        List_Node_Binding *bindingsTemp = &inputRecords->initState.bindings[i];

        bindingsPerma->size = (u8)bindingsTemp->size;

        if ( bindingsPerma->size == 0 ) {
            bindingsPerma->A = 0;
            continue;
        }

        bindingsPerma->A = ArenaPushArray(arena, Binding, bindingsTemp->size);

        Node_Binding *node = bindingsTemp->list;
        for (u32 nodeIndex = 0; nodeIndex < bindingsTemp->size; ++nodeIndex, node = node->next) {
            Binding *binding = &bindingsPerma->A[nodeIndex];

            // NOTE(JENH): Copying binding to the perma arena.
            binding->inputs.size = node->binding.inputs.size;
            binding->inputs.A = ArenaPushArray(arena, Input_Index, binding->inputs.size);
            ArrayCopy(binding->inputs.A, node->binding.inputs.A, binding->inputs.size);
        }
    }

    for (u32 i = 0; i < INPUT_INDEX_COUNT; ++i) {
        Input_Binding_Indices_Collection *bindingIndicesPerma = &mapping->inputToBindings[i];
        List_Node_Binding_Indices *bindingIndicesTemp = &inputRecords->initState.inputs[i];

#if 0
        bindingIndicesPerma->maxCount = (u8)bindingIndicesTemp->size;
        bindingIndicesPerma->size = bindingIndicesPerma->maxCount;
#else
        bindingIndicesPerma->size = (u8)bindingIndicesTemp->size;
#endif

        if ( bindingIndicesPerma->size == 0 ) {
            bindingIndicesPerma->A = 0;
            continue;
        }

        bindingIndicesPerma->A = ArenaPushArray(arena, Binding_Indices, bindingIndicesTemp->size);

        Node_Binding_Indices *node = bindingIndicesTemp->list;
        for (u32 nodeIndex = 0; nodeIndex < bindingIndicesTemp->size; ++nodeIndex, node = node->next) {
            bindingIndicesPerma->A[nodeIndex] = node->indices;
        }
    }
}

Intern void BindingAppend(Memory_Arena *arena, Input_Records *inputRecords, Action_Index actionIndex, Array_Input_Index keys) {
    // TODO(JENH): adding a new binding should consider binding types too?

    Node_Binding *nodeBinding = ArenaPushType(arena, Node_Binding);

    nodeBinding->binding.inputs.size = keys.size;
    nodeBinding->binding.inputs.A = ArenaPushArray(arena, Input_Index, keys.size);
    ArrayCopy(nodeBinding->binding.inputs.A, keys.A, keys.size);

    // NOTE(JENH): Inserting node.
    List_Node_Binding *bindingsList = &inputRecords->initState.bindings[actionIndex];

    if (bindingsList->list == 0) {
        bindingsList->list = nodeBinding;
    } else {
        Node_Binding *tail = bindingsList->list;
        for (u32 nodeIndex = 0; nodeIndex < (bindingsList->size - 1); ++nodeIndex, tail = tail->next);
        tail->next = nodeBinding;
    }

    ++bindingsList->size;

    foreach (Input_Index, input, keys) {
        List_Node_Binding_Indices *keyboardList = &inputRecords->initState.inputs[*input];

        Node_Binding_Indices *nodeKeyboard = ArenaPushType(arena, Node_Binding_Indices);

        // TODO(JENH): Depending on the total actions in the game, action and binding indices could share room to give
        //             more capacity for total bindings.
        nodeKeyboard->indices.action = actionIndex;
        nodeKeyboard->indices.binding = (u8)(bindingsList->size - 1);

        nodeKeyboard->next = 0;

        if (keyboardList->list == 0) {
            keyboardList->list = nodeKeyboard;
        } else {
            Node_Binding_Indices *tail = keyboardList->list;
            for (u32 nodeIndex = 0; nodeIndex < (keyboardList->size - 1); ++nodeIndex, tail = tail->next);
            tail->next = nodeKeyboard;
        }

        ++keyboardList->size;
    }

    // NOTE(JENH): undo/redo thing.
#if 0
    List_Records_Bindings *recordList = &inputRecords->records[actionIndex];

    Node_Records_Bindings *recordNode = ArenaPushType(arena, Node_Records_Bindings);
    recordNode->type = RT_Add;
    recordNode->record = binding;
    recordNode->next = 0;

    if (recordList->tail == 0) {
        recordList->list = recordNode;
    } else {
        recordList->tail->next = recordNode;
    }

    recordList->tail = recordNode;
#endif
}

// NOTE(JENH): undo/redo thing.
#if 0
Intern void EvaluteRecords(Memory_Arena *arena, Input_Records *inputRecords) {
    for (u32 i = 0; i < INPUT_INDEX_COUNT; ++i) {
        List_Records_Bindings *records = &inputRecords.records[i];

        if (records->list == 0) { continue; }

        for (Node_Records_Bindings *node = records->list; node != records->tail; node = node->next) {
            Bindings *binding = node->record;

            switch (node->type) {
                case RT_Append: {
                    Node_Binding *newNodeBindings = ArenaPushType(arena, Node_Binding);
                    newNodeBindings->binding = binding;
                    newNodeBindings->next = 0;

                    List_Node_Binding *bindingsList = inputRecords.initState.bindings[i];
                    if (bindingsList->list == 0) {
                        bindingsList->list = newNodeBindings;
                    } else {
                        Node_Binding *tail = bindingsList->list;
                        for (u32 nodeIndex = 0; nodeIndex < (bindingsList->size - 1); ++nodeIndex, tail = tail->next);
                        tail->next = newNodeBindings;
                    }

                    foreach (Input_Index, input, binding->inputs) {
                        Node_Binding *newNodeKeyboard = ArenaPushType(arena, Node_Binding);
                        newNodeKeyboard->binding = binding;
                        newNodeKeyboard->next = 0;

                        List_Node_Binding *keyboardList = inputRecords.initState.inputs[i];
                        if (keyboardList->list == 0) {
                            keyboardList->list = newNodeKeyboard;
                        } else {
                            Node_Binding *tail = keyboardList->list;
                            for (u32 nodeIndex = 0; nodeIndex < (keyboardList->size - 1); ++nodeIndex, tail = tail->next);
                            tail->next = newNodeKeyboard;
                        }
                    }
                } break;
            }
        }
    }
}
#endif
