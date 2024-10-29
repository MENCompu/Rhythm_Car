typedef struct UI_Window_State {
#if 1
    u32 windowCount;
    UI_Window windows[UI_WINDOW_MAX_COUNT];
#endif

    UI_Window_Node_Mem nodeMem;
} UI_Window_State;

Private Global UI_Window_State gUIWindowState;

Private void UI_Window_Set_Text_Pos(UI_Window* inWindow);

Public UI_Control* UI_Window_Get_Last_Child_Control(UI_Window* inWindow) {
    UI_Control* retControl;

    for (retControl = inWindow->control; retControl->next; retControl = retControl->next);

    return retControl;
}

Private UI_Window* UI_Window_Create(f32x4 inColor, Fn_UI_On_Resize* inOnResizeFunc,
                                    Fn_UI_Cursor_Over* inUI_Cursor_Over, Fn_UI_Cursor_Click* inUI_Cursor_Click) {
    UI_Window* retWindow = Array_Push(gUIWindowState.windows, &gUIWindowState.windowCount);

    Transform windowTrans = { 0 };
    retWindow->control = UI_Control_Create(UT_Window, retWindow, &windowTrans, 0, 0, 0, 0);
    retWindow->control->rendererTrans.position.z = -30.0f;

    retWindow->color = inColor;
    Create_Quad(retWindow->color, &retWindow->mesh, &retWindow->material);

    retWindow->control->center = F32x2(0.0f, 0.0f);
    retWindow->control->halfDims = F32x2(1.0f, 1.0f);

    retWindow->control->On_Resize = ( inOnResizeFunc ) ? inOnResizeFunc : Stub_On_Resize;

#if 1
    retWindow->numberString = Str(retWindow->numberBuffer, 1);

    Transform absTrans = { 0 };
    absTrans.position = F32x3(0.00f, 0.00f, -20.0f);
    //absTrans.position = F32x3(0.01f, -0.01f, -20.0f);
    absTrans.scale = F32x3(0.05f, 0.05f, 1.00f);

    Transform relTrans = { 0 };
    relTrans.position.xy = F32x2(-1.0f, 1.0f);

    // UI_Window_Get_Last_Child_Control(windowHandle)
    retWindow->number = UI_Text_Create(retWindow->numberString, JENH_FALSE, Converter_Get_Asset_Handle(LitToStr("font.fnt")),
                                       &absTrans, retWindow->control, &relTrans, URWO_Align_Top_Left, 0, UI_Text_Cursor_Click);
#endif

    return retWindow;
}

Public UI_Window* UI_Window_Init(f32x2 inInitialScale, f32x4 inColor, Fn_UI_On_Resize* inOnResizeFunc) {
    UI_Window* retWindow = UI_Window_Create(inColor, inOnResizeFunc, 0, 0);

    retWindow->hasScene = JENH_TRUE;

    retWindow->control->rendererTrans.position.xy = F32x2(0.0f, 0.0f);
    retWindow->control->rendererTrans.scale = F32x3(inInitialScale.width, inInitialScale.height, 1.0f);

    UI_On_Resize(retWindow->control);

    //UI_Control_Set_Relative(retWindow->control);

    return retWindow;
}

Private Direction Opposite_Direction(Direction inDirection) {
    Direction retDirection = (Direction)( inDirection ^ 0b10 );
    return retDirection;
}

Public Axis Axis_Form_Direction(Direction inDirection) {
    return ( inDirection & 0b1 ) ? Axis_Y : Axis_X;
}

Private UI_Window_Node* UI_Window_Node_Create() {
    UI_Window_Node_Mem* nodeMem = &gUIWindowState.nodeMem;

    UI_Window_Node* newNode;
    if ( nodeMem->freeList ) {
        newNode = nodeMem->freeList;
        nodeMem->freeList = nodeMem->freeList->next;
    } else {
        Assert( nodeMem->nodeCount < ArrayCount(nodeMem->pool) );
        newNode = &nodeMem->pool[nodeMem->nodeCount++];
    }

    newNode->next = 0;

    return newNode;
}

Private inline void UI_Window_Node_Destroy(UI_Window_Node* inNode) {
    UI_Window_Node_Mem* nodeMem = &gUIWindowState.nodeMem;

    inNode->next = nodeMem->freeList;
    nodeMem->freeList = inNode;
}

Private void UI_Window_List_Destroy(UI_Window_List* inList) {
    UI_Window_Node* node = inList->list;
    Assert( node );

    UI_Window_Node* next;

    do {
         next = node->next;
         UI_Window_Node_Destroy(node);
         node = next;
    } while ( next );

    inList->list = 0;
    inList->tail = 0;
}

Private void OneAdjacentCaseDeleteNodeBySubWindow(UI_Window* subWindowToDelete, UI_Window_List* adjacentList) {
    Assert(adjacentList->list);

    if (adjacentList->first->window == subWindowToDelete) {
        UI_Window_Node* nodeToDelete = adjacentList->first;

        adjacentList->list = adjacentList->first->next;
        if (!adjacentList->first) { adjacentList->tail = 0; }

        UI_Window_Node_Destroy(nodeToDelete);
        return;
    }

    UI_Window_Node* traverse = adjacentList->list;

    while (traverse->next->window != subWindowToDelete) {
	    traverse = traverse->next;
    }

    if (traverse->next == adjacentList->tail) { adjacentList->tail = traverse; }

    UI_Window_Node* nodeToDelete = traverse->next;

    traverse->next = traverse->next->next;

    UI_Window_Node_Destroy(nodeToDelete);
}

Private void UI_Window_Update_Adjacents_Reference(UI_Window* inWindow, Direction adjacentsDirection) {
    UI_Window_List* list = &inWindow->adjacents[adjacentsDirection];
    u32 oppositeDirection = Opposite_Direction(adjacentsDirection);

    if ( list->first == list->tail ) { return; }

    list->first->window->adjacents[oppositeDirection].tail->window = inWindow;

    for (UI_Window_Node* traverse = list->first->next; traverse != list->tail; traverse = traverse->next) {
        traverse->window->adjacents[oppositeDirection].first->window = inWindow;
    }

    list->tail->window->adjacents[oppositeDirection].first->window = inWindow;
}

Private void OneAdjacentCaseReplaceSubWindow(UI_Window* newWindow, UI_Window* oldWindow,
		                              u32 directionAdjacentSW, u32 oppositeDirectionAdjacentSW) {
    u32 direction = directionAdjacentSW;
    u32 opposite  = oppositeDirectionAdjacentSW;

    UI_Window_Node* node = newWindow->adjacents[direction].first->window->adjacents[opposite].list;

    while ( node->window != oldWindow ) {
	    node = node->next;
    }

    node->window = newWindow;
}

Private b32 CanBeMerged(UI_Window_List* adjacentList, UI_Window* subWindow, u32 axis) {
    b32 result;

    f32 beginVerticeSubWindow = subWindow->control->rendererTrans.position.E[axis];
    f32 endVerticeSubWindow   = beginVerticeSubWindow + subWindow->control->rendererTrans.scale.E[axis];

    f32 beginVerticeList = adjacentList->first->window->control->rendererTrans.position.E[axis];
    f32 endVerticeSubWindowList = adjacentList->tail->window->control->rendererTrans.position.E[axis] + adjacentList->tail->window->control->rendererTrans.scale.E[axis];

    result = (beginVerticeSubWindow == beginVerticeList && endVerticeSubWindow == endVerticeSubWindowList);

    return result;
}

Private b32 AreAlingedAtTheEnd(UI_Window* subWindow1, UI_Window* subWindow2, u32 axis) {
    b32 result;

    result = subWindow1->control->rendererTrans.position.E[axis] + subWindow1->control->rendererTrans.scale.E[axis] ==
             subWindow2->control->rendererTrans.position.E[axis] + subWindow2->control->rendererTrans.scale.E[axis];

    return result;
}

Private b32 AreAlinged(UI_Window* subWindow1, UI_Window* subWindow2, u32 axis) {
    b32 result;

    result = subWindow1->control->rendererTrans.position.E[axis] == subWindow2->control->rendererTrans.position.E[axis];

    return result;
}

//TODO(JENH): Is still not updating correctly?.
Private void UpdateOppositeMergeAxisSubWindowInDelete(UI_Window* subWindowDelete, UI_Window* subWindowReplace, u32 mergeDirection,
                                                      u32 mergeAxis, u32 mergeOppositeAxis, u32 oppMergeOppAxis) {
    if ( subWindowDelete->adjacents[mergeOppositeAxis].first ) {
        return;
    }

    UI_Window_List* newWindowList;
    UI_Window_List* oldWindowList;

    b32 areAlinged;

    if ( mergeDirection <= Direction_Down ) {
        newWindowList = &subWindowReplace->adjacents[mergeOppositeAxis];
        oldWindowList = &subWindowDelete->adjacents[mergeOppositeAxis];
        areAlinged = AreAlinged(subWindowDelete, subWindowDelete->adjacents[mergeOppositeAxis].first->window, mergeAxis);
    } else {
        newWindowList = &subWindowDelete->adjacents[mergeOppositeAxis];
        oldWindowList = &subWindowReplace->adjacents[mergeOppositeAxis];
        areAlinged = AreAlinged(subWindowReplace, subWindowReplace->adjacents[mergeOppositeAxis].first->window, mergeAxis);
    }

    // NOTE(JENH): Here may be the problem?.
    if ( areAlinged ) {
        UI_Window_List* ToDeleteAdjacentList = &subWindowDelete->adjacents[mergeOppositeAxis];

        if ( mergeDirection <= Direction_Down ) {
            ToDeleteAdjacentList->first->window->adjacents[oppMergeOppAxis].first->window = subWindowReplace;
        } else {
            ToDeleteAdjacentList->first->window->adjacents[oppMergeOppAxis].tail->window = subWindowReplace;
        }
    } else {
        OneAdjacentCaseDeleteNodeBySubWindow(subWindowDelete, &oldWindowList->first->window->adjacents[oppMergeOppAxis]);
    }

    UI_Window_Update_Adjacents_Reference(subWindowReplace, (Direction)mergeOppositeAxis);

    UI_Window_Node* listToAppend = oldWindowList->list;

    if ( !areAlinged ) {
        listToAppend = listToAppend->next;
        UI_Window_Node_Destroy(oldWindowList->first);
    }

    newWindowList->tail->next = listToAppend;

    if ( newWindowList->tail->next ) {
        newWindowList->tail = oldWindowList->tail;
    }

    subWindowReplace->adjacents[mergeOppositeAxis] = *newWindowList;
}

Private UI_Window_Node*
ConectOppMergeWithMergeSubWindow(UI_Window_Node* traverseMerge, UI_Window* oppMergeSubWindow,
                                 UI_Window_Node* startAppendOppMerge, u32 mergeOppositeAxis,
                                 u32 mergeDirection, u32 oppositeMergeDirection,
                                 b32 lastTraverseMergeSubWindowWasAligned) {
    Assert(traverseMerge);

    UI_Window_Node* lastMergeNodeAdded = traverseMerge;

    UI_Window_Node* lastAppendOppMerge = startAppendOppMerge;

    b32 firstTraverseMergeNodeWasUsed = !lastTraverseMergeSubWindowWasAligned;

    UI_Window_List* traverseMergeList = &traverseMerge->window->adjacents[oppositeMergeDirection];
    UI_Window_List* traverseOppMergeList = &oppMergeSubWindow->adjacents[mergeDirection];

    startAppendOppMerge->window = traverseMerge->window;

    if (!firstTraverseMergeNodeWasUsed) {
        traverseMergeList->first->window = oppMergeSubWindow;
        firstTraverseMergeNodeWasUsed = false;
    } else {
        UI_Window_Node* newNode = UI_Window_Node_Create();
        newNode->window = oppMergeSubWindow;
        traverseMergeList->tail->next = newNode;
        traverseMergeList->tail = traverseMergeList->tail->next;
    }

    traverseMerge = traverseMerge->next;

    while (traverseMerge && oppMergeSubWindow->control->rendererTrans.position.E[mergeOppositeAxis] +
           oppMergeSubWindow->control->rendererTrans.scale.E[mergeOppositeAxis] >
           traverseMerge->window->control->rendererTrans.position.E[mergeOppositeAxis]) {

        traverseMergeList = &traverseMerge->window->adjacents[oppositeMergeDirection];

        UI_Window_Node* newNode;

	newNode = UI_Window_Node_Create();
	newNode->window = traverseMerge->window;
	newNode->next = lastAppendOppMerge->next;
	lastAppendOppMerge->next = newNode;
        lastAppendOppMerge = lastAppendOppMerge->next;

	if ( !newNode->next ) { traverseOppMergeList->tail = newNode; }

	traverseMergeList->first->window = oppMergeSubWindow;

        lastMergeNodeAdded = traverseMerge;
        traverseMerge = traverseMerge->next;
    }

    return lastMergeNodeAdded;
}

Private void UI_Window_Delete(UI_Window* subWindow) {
    UI_Window_List* traverseList = subWindow->adjacents;

    u32 mergeDirection = Direction_Left;

    for (;!traverseList->list || !CanBeMerged(traverseList, subWindow, (mergeDirection + 1) % 2); ++traverseList, ++mergeDirection);

    UI_Window_List* mergeList = traverseList;

#if 0
    ChangeSubWindow(state, mergeList->first->adjacent);
    state->savedSubWindow = mergeList->first->adjacent;
#endif

    u32 mergeAxis = mergeDirection % 2;

    u32 mergeOppositeAxis = ( mergeAxis == AXIS_HOR ) ? Direction_Up : Direction_Left;

    UI_Window* subWindowReplace = mergeList->first->window;

    UpdateOppositeMergeAxisSubWindowInDelete(subWindow, subWindowReplace, mergeDirection, mergeAxis, mergeOppositeAxis, mergeOppositeAxis + 2);

    subWindowReplace = mergeList->tail->window;

    UpdateOppositeMergeAxisSubWindowInDelete(subWindow, subWindowReplace, mergeDirection, mergeAxis, mergeOppositeAxis + 2, mergeOppositeAxis);

    u32 oppositeMergeDirection = mergeDirection + 2 - (4 * (mergeDirection / 2));

    UI_Window_List* oppositeMergeList = &subWindow->adjacents[oppositeMergeDirection];

    if (!oppositeMergeList->first) {
        for (UI_Window_Node* node = mergeList->list; node; node = node->next) {
            UI_Window_List_Destroy(&node->window->adjacents[oppositeMergeDirection]);
        }
    } else if (!oppositeMergeList->first->next) {
        UI_Window* oppMergeSubWindow = oppositeMergeList->first->window;
        UI_Window_List* oppositeMergeToDelSWList = &oppMergeSubWindow->adjacents[mergeDirection];

        UI_Window_Node* traverseOppMerge = oppositeMergeToDelSWList->list;

	    while (traverseOppMerge->window != subWindow) { traverseOppMerge = traverseOppMerge->next; }

        traverseOppMerge->window = mergeList->first->window;
	    mergeList->first->window->adjacents[oppositeMergeDirection].first->window = oppMergeSubWindow;

        UI_Window_Node* traverseMerge = mergeList->first->next;

	    b32 isTail = traverseOppMerge == oppositeMergeToDelSWList->tail;

        while (traverseMerge) {
            traverseMerge->window->adjacents[oppositeMergeDirection].first->window = oppMergeSubWindow;

            UI_Window_Node* newNode = UI_Window_Node_Create();
            newNode->window = traverseMerge->window;
            newNode->next = traverseOppMerge->next;
            traverseOppMerge->next = newNode;
            traverseOppMerge = traverseOppMerge->next;

                if (isTail) { oppositeMergeToDelSWList->tail = newNode; }

                traverseMerge = traverseMerge->next;
        }
    } else {
        UI_Window_Node* traverseMerge = mergeList->list;
        UI_Window_Node* traverseOppMerge = oppositeMergeList->list;

        UI_Window_Node* startAppendOppMerge = oppositeMergeList->first->window->adjacents[mergeDirection].tail;

        b32 lastTraverseMergeSubWindowWasAligned = true;

        while ( INFINITE_LOOP ) {
                traverseMerge = ConectOppMergeWithMergeSubWindow(traverseMerge, traverseOppMerge->window, startAppendOppMerge,
                                                                 mergeOppositeAxis, mergeDirection, oppositeMergeDirection,
                                                                 lastTraverseMergeSubWindowWasAligned);

            if (AreAlingedAtTheEnd(traverseMerge->window, traverseOppMerge->window, mergeOppositeAxis)) {
                traverseMerge = traverseMerge->next;
                lastTraverseMergeSubWindowWasAligned = true;
            } else {
                lastTraverseMergeSubWindowWasAligned = false;
            }

            traverseOppMerge = traverseOppMerge->next;

            if (!traverseOppMerge) { break; }

            startAppendOppMerge = traverseOppMerge->window->adjacents[mergeDirection].first;
        }
    }

    f32x2 glyphDimensions = { 2.0f, 2.0f };

    if ( mergeDirection <= Direction_Down ) {
        for (UI_Window_Node* traverse = mergeList->list; traverse; traverse = traverse->next) {
            UI_Window *adjacent = traverse->window;

            adjacent->control->rendererTrans.scale.E[mergeAxis] += subWindow->control->rendererTrans.scale.E[mergeAxis];

            //adjacent->dimensionsInGlyphs = Tou32(HadamardDiv(adjacent->control->rendererTrans.scale, glyphDimensions));
        }
    } else {
        for (UI_Window_Node* traverse = mergeList->list; traverse; traverse = traverse->next) {
            UI_Window *adjacent = traverse->window;

            adjacent->control->rendererTrans.position.E[mergeAxis] -= subWindow->control->rendererTrans.scale.E[mergeAxis];
            adjacent->control->rendererTrans.scale.E[mergeAxis] += subWindow->control->rendererTrans.scale.E[mergeAxis];

            //adjacent->dimensionsInGlyphs = Tou32(HadamardDiv(adjacent->control->rendererTrans.scale, glyphDimensions));
        }
    }

    UI_Window_List_Destroy(mergeList);

    if ( oppositeMergeList->list ) {
        UI_Window_List_Destroy(oppositeMergeList);
    }

    return;
}

Private b8 UpdateOtherSplitAxisAdjacentSubWindows(UI_Window_List* adjacentsNewSubWindow, UI_Window* oldWindow,
                                                  UI_Window_List* adjacentsOldSubWindow, u32 axis) {
    *adjacentsNewSubWindow = *adjacentsOldSubWindow;

    if ( !adjacentsOldSubWindow->list ) { return false; }

    f32 oldWindowEdgePos = oldWindow->control->rendererTrans.position.E[axis] - oldWindow->control->rendererTrans.scale.E[axis];

    UI_Window_Node* node = adjacentsOldSubWindow->list;
    for (UI_Window_Node* next = node->next; next; node = node->next, next = next->next) {
        f32 nextWindowEdgePos = next->window->control->rendererTrans.position.E[axis] - next->window->control->rendererTrans.scale.E[axis];
        f32 epsilon = 0.0001f;

        if ( (nextWindowEdgePos - epsilon) <= oldWindowEdgePos && oldWindowEdgePos <= (nextWindowEdgePos + epsilon) ) {
            b8 windowsAreAling = ( oldWindowEdgePos == nextWindowEdgePos );

            if ( windowsAreAling ) {
                adjacentsOldSubWindow->list = next;
            } else {
                UI_Window_Node* copyNode = UI_Window_Node_Create();
                *copyNode = *node;
                adjacentsOldSubWindow->list = copyNode;
            }

            node->next = 0;
            adjacentsNewSubWindow->tail = node;

            Assert( adjacentsNewSubWindow->list );

	        return windowsAreAling;
    	}
    }

    UI_Window_Node* copyNode = UI_Window_Node_Create();
    *copyNode = *node;
    adjacentsOldSubWindow->list = copyNode;
    adjacentsOldSubWindow->tail = copyNode;

    Assert( adjacentsNewSubWindow->list );

    return false;
}

Private void OneAdjacentSubWindowCaseOppSplitAxis(UI_Window* newWindow, UI_Window* oldWindow, u32 directionAdjacentSW,
                                                  u32 oppositeDirectionAdjacentSW) {
    u32 direction = directionAdjacentSW;
    u32 opposite  = oppositeDirectionAdjacentSW;

    UI_Window_Node* newNode = UI_Window_Node_Create();

    newNode->window = newWindow;

    UI_Window_Node* node = newWindow->adjacents[direction].tail->window->adjacents[opposite].list;

    if ( node->window == oldWindow ) {
        newWindow->adjacents[direction].tail->window->adjacents[opposite].list = newNode;
	    newNode->next = node;
    } else {
        UI_Window_Node* next = node->next;

        while ( next->window != oldWindow ) {
            node = node->next;
            next = next->next;
        }

        node->next = newNode;
        newNode->next = next;
    }

    if ( !newNode->next ) {
	    newWindow->adjacents[direction].first->window->adjacents[opposite].tail = newNode;
    }
}

Public UI_Window* UI_Window_Split(UI_Window* inWindow, Direction inDirection, f32x4 inColor, Fn_UI_On_Resize* inOnResizeFunc) {
    UI_Window* retWindow = UI_Window_Create(inColor, inOnResizeFunc, 0, 0);

    UI_Window* newWindow = retWindow;
    UI_Window* oldWindow = inWindow;

    u32 axis = Axis_Form_Direction(inDirection);
    u32 otherAxis = ( axis == AXIS_HOR ) ? Direction_Down : Direction_Left;

    f32 sign = ( inDirection & 0b10 ) ? +1.0f : -1.0f;

    newWindow->control->rendererTrans.rotation = F32x3(0.0f, 0.0f, 0.0f);

    newWindow->control->rendererTrans.scale = oldWindow->control->rendererTrans.scale;
    newWindow->control->rendererTrans.scale.E[axis] /= 2.0f;
    oldWindow->control->rendererTrans.scale.E[axis] -= newWindow->control->rendererTrans.scale.E[axis];

    newWindow->control->rendererTrans.position = oldWindow->control->rendererTrans.position;
    newWindow->control->rendererTrans.position.E[axis] += sign * newWindow->control->rendererTrans.scale.E[axis];
    oldWindow->control->rendererTrans.position.E[axis] += sign * -1.0f * oldWindow->control->rendererTrans.scale.E[axis];

    //UI_Control_Set_Relative(newWindow->control);

    UI_On_Resize(oldWindow->control);
    UI_On_Resize(newWindow->control);

    UI_Window* window1 = newWindow;
    UI_Window* window2 = oldWindow;

    if ( inDirection & 0b10 ) {
        window1 = oldWindow;
        window2 = newWindow;

        for (u32 i = 0; i < Field_Array_Count(UI_Window, adjacents); ++i) {
            window2->adjacents[i] = window1->adjacents[i];
            window1->adjacents[i] = { 0 };

            Direction dir = (Direction)i;
            Direction opp = Opposite_Direction(dir);

            if ( window2->adjacents[dir].first ) {
                if ( window2->adjacents[dir].first != window2->adjacents[dir].tail ) {
                    UI_Window_Update_Adjacents_Reference(window2, dir);
                } else { // One adjacent window case.
                    UI_Window_Node* node = window2->adjacents[dir].first->window->adjacents[opp].list;
                        while ( node->window != oldWindow ) {
                        node = node->next;
                    }

                    node->window = window2;
                }
            }
        }
    }

    // At opposite split axis nodes.
    UI_Window_List* firstOtherSplitAxisSide  = &window2->adjacents[otherAxis];
    UI_Window_List* secondOtherSplitAxisSide = firstOtherSplitAxisSide + 2;

    b32 areAlinged;

    areAlinged = UpdateOtherSplitAxisAdjacentSubWindows(&window1->adjacents[otherAxis], window2, &window2->adjacents[otherAxis], axis);

    if ( window1->adjacents[otherAxis].first ) {
        if ( areAlinged ) {
            window1->adjacents[otherAxis].first->window->adjacents[otherAxis + 2].tail->window = window1;
        } else {
            OneAdjacentSubWindowCaseOppSplitAxis(window1, window2, otherAxis, otherAxis + 2);
        }

        UI_Window_Update_Adjacents_Reference(window1, (Direction)otherAxis);
    }

    areAlinged = UpdateOtherSplitAxisAdjacentSubWindows(&window1->adjacents[otherAxis + 2], window2, &window2->adjacents[otherAxis + 2], axis);

    if ( window1->adjacents[otherAxis + 2].first ) {
        if ( areAlinged ) {
            window1->adjacents[otherAxis + 2].first->window->adjacents[otherAxis].tail->window = window1;
        } else {
            OneAdjacentSubWindowCaseOppSplitAxis(window1, window2, otherAxis + 2, otherAxis);
        }

        UI_Window_Update_Adjacents_Reference(window1, (Direction)(otherAxis + 2));
    }

    // At split axis nodes.
    window1->adjacents[axis] = window2->adjacents[axis];
    window2->adjacents[axis] = { 0 };

    if ( window1->adjacents[axis].first ) {
        if ( window1->adjacents[axis].first != window1->adjacents[axis].tail ) {
            UI_Window_Update_Adjacents_Reference(window1, (Direction)axis);
        } else { // One adjacent window case.
            UI_Window_Node* node = window1->adjacents[axis].first->window->adjacents[axis + 2].list;
            while ( node->window != window2 ) {
                node = node->next;
            }

            node->window = window1;
        }
    }

    // Between old window and new window nodes.
    UI_Window_Node* nodeAtSplit1 = UI_Window_Node_Create();
    nodeAtSplit1->window = window2;
    window1->adjacents[axis + 2].first = nodeAtSplit1;
    window1->adjacents[axis + 2].tail = nodeAtSplit1;

    UI_Window_Node* nodeAtSplit2 = UI_Window_Node_Create();
    nodeAtSplit2->window = window1;
    window2->adjacents[axis].first = nodeAtSplit2;
    window2->adjacents[axis].tail = nodeAtSplit2;

    return retWindow;
}

Public void UI_Window_Resize_Update_Opposite_Axis(UI_Window* inWindow, f32 inAmount, Direction inDirection, Direction inOppAxisDirection) {
    f32 halfAmount = inAmount * 0.5f;
    f32 sign = ( inDirection & 0b10 ) ? +1.0f : -1.0f;
    Axis axis = Axis_Form_Direction(inDirection);

    f32 prevEdgePosition = inWindow->control->rendererTrans.position.E[axis] + (sign * inWindow->control->rendererTrans.scale.E[axis]);
    f32 edgePosition = prevEdgePosition + (sign * inAmount);

    List_Element listElement = ( inDirection & 0b1 ) ? LE_First : LE_Tail;

    UI_Window* prevWindow;
    UI_Window* window = 0;

    for (prevWindow = inWindow; prevWindow->adjacents[inOppAxisDirection].list; prevWindow = window) {
        window = prevWindow->adjacents[inOppAxisDirection].LE[listElement]->window;

        if ( !window->adjacents[inDirection].list ) { break; }

        // This condition tells that there are windows align.
        List_Element alignElement1 = ( inOppAxisDirection & 0b10 ) ? LE_First : LE_Tail;
        List_Element alignElement2 = ( inOppAxisDirection & 0b10 ) ? LE_Tail : LE_First;
        if ( prevWindow->adjacents[inDirection].LE[alignElement2]->window != window->adjacents[inDirection].LE[alignElement1]->window ) {
            f32 windowEdge = window->control->rendererTrans.position.E[axis] + (sign * window->control->rendererTrans.scale.E[axis]);
            f32 epsilon = 0.0001f;

            // TODO(JENH): All this cases should handle windows in between.
            //             |  x  |x|x |x | x |     |
            //             |   |      <-     |     |

            if ( (windowEdge - epsilon) <= prevEdgePosition && prevEdgePosition <= (windowEdge + epsilon) ) {
                if ( sign == -1.0f ) {
                    UI_Window* alignWindow = window->adjacents[inDirection].LE[(List_Element)!listElement]->window;

                    UI_Window_Node* newNode = UI_Window_Node_Create();
                    newNode->window = prevWindow;

                    alignWindow->adjacents[Opposite_Direction(inOppAxisDirection)].tail->next = newNode;
                    alignWindow->adjacents[Opposite_Direction(inOppAxisDirection)].tail = newNode;

                    newNode = UI_Window_Node_Create();
                    newNode->window = alignWindow;

                    newNode->next = prevWindow->adjacents[inOppAxisDirection].first;
                    prevWindow->adjacents[inOppAxisDirection].first = newNode;
                } else { // sign == +1.0f
                    UI_Window* alignWindow = window->adjacents[inDirection].LE[listElement]->window;

                    UI_Window_Node* newNode = UI_Window_Node_Create();
                    newNode->window = alignWindow;

                    prevWindow->adjacents[inOppAxisDirection].tail->next = newNode;
                    prevWindow->adjacents[inOppAxisDirection].tail = newNode;

                    newNode = UI_Window_Node_Create();
                    newNode->window = prevWindow;

                    newNode->next = alignWindow->adjacents[Opposite_Direction(inOppAxisDirection)].first;
                    alignWindow->adjacents[Opposite_Direction(inOppAxisDirection)].first = newNode;
                }
            } else if ( (windowEdge - epsilon) <= edgePosition && edgePosition <= (windowEdge + epsilon) ) {
                List_Element listElement2 = ( inOppAxisDirection & 0b10 ) ? LE_Tail : LE_First;

                if ( sign == -1.0f ) {
                    UI_Window* alignWindow = prevWindow->adjacents[inDirection].LE[listElement2]->window;

                    UI_Window_List* alignWindowList = &alignWindow->adjacents[inOppAxisDirection];
                    UI_Window_List* windowList = &window->adjacents[Opposite_Direction(inOppAxisDirection)];

                    // Destroy the tail node of align window.
                    UI_Window_Node* prevLastOne;
                    for (prevLastOne = alignWindowList->list; prevLastOne->next != alignWindowList->tail; prevLastOne = prevLastOne->next);
                    UI_Window_Node_Destroy(alignWindowList->tail);
                    prevLastOne->next = 0;
                    alignWindowList->tail = prevLastOne;

                    // Destroy the first node of prev window.
                    UI_Window_Node* nodeToDelete;
                    nodeToDelete = windowList->first;
                    windowList->first = windowList->first->next;
                    UI_Window_Node_Destroy(nodeToDelete);
                } else { // sign == +1.0f
                    UI_Window* alignWindow = prevWindow->adjacents[inDirection].LE[listElement2]->window;

                    UI_Window_List* alignWindowList = &alignWindow->adjacents[inOppAxisDirection];
                    UI_Window_List* windowList = &window->adjacents[Opposite_Direction(inOppAxisDirection)];

                    // Destroy the tail node of prev window.
                    UI_Window_Node* prevLastOne;
                    for (prevLastOne = windowList->list; prevLastOne->next != windowList->tail; prevLastOne = prevLastOne->next);
                    UI_Window_Node_Destroy(windowList->tail);
                    prevLastOne->next = 0;
                    windowList->tail = prevLastOne;

                    // Destroy the first node of align window.
                    UI_Window_Node* nodeToDelete;
                    nodeToDelete = alignWindowList->first;
                    alignWindowList->first = alignWindowList->first->next;
                    UI_Window_Node_Destroy(nodeToDelete);
                }
            }

            return;
        }

        window->control->rendererTrans.scale.E[axis] += halfAmount;
        window->control->rendererTrans.position.E[axis]  = window->control->rendererTrans.position.E[axis] + (sign * halfAmount);
        UI_On_Resize(window->control);

        UI_Window_List* list = &window->adjacents[inDirection];
        UI_Window_Node* nodeToSkip = list->LE[( inOppAxisDirection & 0b10 ) ? LE_First : LE_Tail];
        for (UI_Window_Node* node = list->first; node; node = node->next) {
            if ( node == nodeToSkip ) { continue; }

            UI_Window* adjacentWindow = node->window;

            adjacentWindow->control->rendererTrans.scale.E[axis] -= halfAmount;
            adjacentWindow->control->rendererTrans.position.E[axis]  = adjacentWindow->control->rendererTrans.position.E[axis] + (sign * halfAmount);
            UI_On_Resize(adjacentWindow->control);
        }
    }
}

// TODO(JENH): Should clamp "inAmount" to 1.0f or -1.0f.
Public void UI_Window_Resize(UI_Window* inWindow, f32 inAmount, Direction inDirection) {
    if ( !inWindow->adjacents[inDirection].first ) { return; }

    // NOTE(JENH): Because it is the half dimension that we are modifing, we need to take half the amount as well.
    f32 halfAmount = inAmount * 0.5f;
    Axis axis = Axis_Form_Direction(inDirection);
    f32 sign = ( inDirection & 0b10 ) ? +1.0f : -1.0f;

    // Update adjacent window at "inDirection".
    for (UI_Window_Node* node = inWindow->adjacents[inDirection].first; node; node = node->next) {
        UI_Window* window = node->window;

        window->control->rendererTrans.scale.E[axis] -= halfAmount;
        window->control->rendererTrans.position.E[axis]  = window->control->rendererTrans.position.E[axis] + (sign * halfAmount);

        UI_On_Resize(window->control);
    }

    Direction direction2 = ( inDirection & 0b1 ) ? Direction_Left : Direction_Down;

    UI_Window_Resize_Update_Opposite_Axis(inWindow, inAmount, inDirection, direction2);
    UI_Window_Resize_Update_Opposite_Axis(inWindow, inAmount, inDirection, Opposite_Direction(direction2));

    inWindow->control->rendererTrans.scale.E[axis] += halfAmount;
    inWindow->control->rendererTrans.position.E[axis]  = inWindow->control->rendererTrans.position.E[axis] + (sign * halfAmount);

    UI_On_Resize(inWindow->control);
}

#if 0
MACRO_SetSubWindowsForResizing(SetSubWindowsForResizing) {
    v2_f32 minTopLeftPadding = gUIWindowState.minTopLeftPadding;
    v2_f32 windowDimensions = gUIWindowState.windowDimensions;
    windowDimensions.height -= miniBufferSubWindow->scale.height;

    UI_Window* subWindows = gUIWindowState.subWindows;
    UI_Window* firstSubWindowEmpty = gUIWindowState.firstSubWindowEmpty;

    for (UI_Window* subWindow = subWindows; subWindow < firstSubWindowEmpty; subWindow++) {
        subWindow->relTopLeftPosition = HadamardDiv(subWindow->position, windowDimensions);

        subWindow->relDimensions = HadamardDiv(subWindow->scale, windowDimensions);

        v2_u32 cursorPosition   = subWindow->cursor.position;
        v2_u32 screenCharOffset = subWindow->screenCharOffsetRelToText;

        v2_u32 cursorRelPosition = cursorPosition - screenCharOffset;
        v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(subWindow->dimensionsInGlyphs));

        subWindow->cursor.relPositionForResizing = relPositionForResizing;
    }
}

MACRO_UpdateSubWindowsSize(UpdateSubWindowsSize) {
    State *state = (State *)memory->permanent;

    v2_f32 glyphDimensions = state->characterInfo.glyphDimensions;

    v2_f32 minTopLeftPadding = state->subWindowGridSystem.minTopLeftPadding;
    v2_f32 minBottomRightPadding = state->subWindowGridSystem.minBottomRightPadding;

    UI_Window* miniBufferSubWindow = &state->miniBufferSW;

    v2_f32 windowDimensionsWithPadding = newWindowDimensions - minTopLeftPadding;

    miniBufferSubWindow->position.height = windowDimensionsWithPadding.height -
	                                             miniBufferSubWindow->scale.height;
    miniBufferSubWindow->scale.width = windowDimensionsWithPadding.width;

    v2_f32 fixedScreenDimensions = windowDimensionsWithPadding;
    fixedScreenDimensions.height -= miniBufferSubWindow->scale.height;

    UI_Window* subWindows = state->subWindowGridSystem.subWindows;
    UI_Window* firstSubWindowEmpty = state->subWindowGridSystem.firstSubWindowEmpty;

    for (UI_Window* subWindow = subWindows; subWindow < firstSubWindowEmpty; ++subWindow) {
        subWindow->position = HadamardProd(fixedScreenDimensions, subWindow->relTopLeftPosition);

        v2_u32 startScreenInChars = subWindow->screenCharOffsetRelToText;
        v2_u32 oldDimensionsInGlyphs = subWindow->dimensionsInGlyphs;

        subWindow->scale = HadamardProd(fixedScreenDimensions, subWindow->relDimensions);

        v2_f32 absDimensionsWithPadding = HadamardProd(fixedScreenDimensions - minBottomRightPadding,
                                               subWindow->relDimensions);

        subWindow->dimensionsInGlyphs = Tou32(HadamardDiv(absDimensionsWithPadding, glyphDimensions));

        v2_u32 newDimensionsInGlyphs = subWindow->dimensionsInGlyphs;

	v2_f32 cursorRelPosition = subWindow->cursor.relPositionForResizing;

        v2_i32 oldDimensionsInGlyphsFromCursorToTopLeft = Toi32(HadamardProd(cursorRelPosition,
			                                                     Tof32(oldDimensionsInGlyphs)));

        v2_i32 oldDimensionsInGlyphsFromCursorToBottomRight = Toi32(oldDimensionsInGlyphs) -
		                                              oldDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 newDimensionsInGlyphsFromCursorToTopLeft = Toi32(HadamardProd(cursorRelPosition,
			                                                     Tof32(newDimensionsInGlyphs)));

        v2_i32 newDimensionsInGlyphsFromCursorToBottomRight = Toi32(newDimensionsInGlyphs) -
		                                              newDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 screenOffsetTopLeft = oldDimensionsInGlyphsFromCursorToTopLeft -
		                     newDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 newScreenOffsetBottomRight = oldDimensionsInGlyphsFromCursorToBottomRight -
		                            newDimensionsInGlyphsFromCursorToBottomRight;

	if (startScreenInChars.x == 0 && screenOffsetTopLeft.x < 0) {
	    v2_u32 cursorRelPosition = subWindow->cursor.position - startScreenInChars;
	    v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(newDimensionsInGlyphs));

	    subWindow->cursor.relPositionForResizing = relPositionForResizing;
            screenOffsetTopLeft.x = 0;
	}

	if (startScreenInChars.y == 0 && screenOffsetTopLeft.y < 0) {
	    v2_u32 cursorRelPosition = subWindow->cursor.position - startScreenInChars;
	    v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(newDimensionsInGlyphs));

	    subWindow->cursor.relPositionForResizing = relPositionForResizing;
            screenOffsetTopLeft.y = 0;
	}

        MoveScreen(subWindow, -screenOffsetTopLeft, screenOffsetTopLeft);
    }

    state->subWindowGridSystem.windowDimensions = windowDimensionsWithPadding;
}
#endif

#if 0
Private inline void ChangeSubWindow(State *state, UI_Window *newWindow) {
    UI_Window *oldWindow = state->currentSubWindow;

    state->currentSubWindow = newWindow;
    state->currentBuffer = newWindow->displayedBuffer;
}

Private void MoveToSubWindow(State *state, u32 direction) {
    UI_Window* currentSubWindow = state->currentSubWindow;

    UI_Window_Node *adjacentList = currentSubWindow->adjacents[direction].list;

    u32 otherAxis = (direction + 1) % 2;

    if (adjacentList) {
	f32 glyphDimension  = state->characterInfo.glyphDimensions.E[otherAxis];
	u32 cursorPosInAxis = currentSubWindow->cursor.position.E[otherAxis];

	f32 cursorOtherAxisInWindowCoords = currentSubWindow->position.E[otherAxis] +
		                            (cursorPosInAxis * glyphDimension);

        UI_Window_Node* traverse = adjacentList;
        UI_Window_Node* next = traverse->next;

	while (next && next->window->position.E[otherAxis] <= cursorOtherAxisInWindowCoords) {
	    traverse = traverse->next;
	    next = next->next;
	}

        ChangeSubWindow(state, traverse->window);
	state->savedSubWindow = state->currentSubWindow;
    }
}
#endif

