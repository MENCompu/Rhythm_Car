#pragma warning(push, 1)
#include <shlwapi.h>
#include <DSound.h>
#include <hidsdi.h>
#pragma warning(pop)

Private Global OS_State gStateOS;

typedef struct {
    u32 volatile size;
    u32 volatile index;

    Job_Entry Q[256];
} Job_Queue;

Private Global HANDLE dirThread;
Private Global HANDLE semaphoreDir;

Private Global HANDLE threadOS;
Private Global HANDLE semaphoreOS;
Private Global Job_Queue queueOS;

typedef HRESULT WINAPI Fn_DirectSoundCreate(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);

Public s32 OS_Get_Refresh_Rate() {
    HDC refreshDC = GetDC(gStateOS.window);
    s32 actualRefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
    ReleaseDC(gStateOS.window, refreshDC);

    return ( actualRefreshRate > 1 ) ? actualRefreshRate : 60;
}

Private void InitDirectSound(u32 samplesPerSecond, u32 bufferSize) {
    HMODULE directSoundLibrary = LoadLibraryA("dsound.dll");

    if (directSoundLibrary == INVALID_HANDLE_VALUE) {
        Win32_LogError("Failed to load direct sound library");
        return;
    }

    Fn_DirectSoundCreate* DirectSoundCreate = (Fn_DirectSoundCreate*)(void*)GetProcAddress(directSoundLibrary, "DirectSoundCreate");

	LPDIRECTSOUND directSound;
	if (!DirectSoundCreate || !SUCCEEDED(DirectSoundCreate(0, &directSound, 0))) {
        Win32_LogError("Failed to create direct sound");
        return;
    }

    WAVEFORMATEX waveFormat = {0};

    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (WORD)((waveFormat.nChannels * waveFormat.wBitsPerSample) / 8);
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    if (!SUCCEEDED(directSound->SetCooperativeLevel(gStateOS.window, DSSCL_PRIORITY))) {
        Win32_LogError("Failed to set cooperative level");
        return;
    }

    DSBUFFERDESC bufferDescription1 = {0};
    bufferDescription1.dwSize = sizeof(bufferDescription1);
    bufferDescription1.dwFlags = DSBCAPS_PRIMARYBUFFER;

    LPDIRECTSOUNDBUFFER primaryBuffer;
    if (!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription1, &primaryBuffer, 0))) {
        Win32_LogError("Failed to create primary buffer");
        return;
    }

    if (!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
        Win32_LogError("Failed to set the format of the primary buffer");
        return;
    }

    DSBUFFERDESC bufferDescription2 = {0};
    bufferDescription2.dwSize = sizeof(bufferDescription2);
    bufferDescription2.dwBufferBytes = bufferSize;
    bufferDescription2.lpwfxFormat = &waveFormat;

    if (!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription2, &gStateOS.sound.secondaryBuffer, 0))) {
        Win32_LogError("Failed to create the secondary buffer");
        return;
    }
}

Private void Sound_Clear_Buffer() {
    Win32_Sound* sound = &gStateOS.sound;

    VOID* region1;
    DWORD sizeRegion1;
    VOID* region2;
    DWORD sizeRegion2;

    if (SUCCEEDED(sound->secondaryBuffer->Lock(0, sound->bufferSize, &region1, &sizeRegion1, &region2, &sizeRegion2, 0))) {
        byte* byteToClear = (byte*)region1;
        for (DWORD sampleIndex = 0; sampleIndex < sizeRegion1; sampleIndex++) {
           *byteToClear++ = 0;
        }

        byteToClear = (byte*)region2;
        for (DWORD sampleIndex = 0; sampleIndex < sizeRegion2; sampleIndex++) {
           *byteToClear++ = 0;
        }

        HRESULT result = sound->secondaryBuffer->Unlock(region1, sizeRegion1, region2, sizeRegion2);
    }
}

Private void Sound_Fill_Buffer(s16* bufferSource, DWORD byteToLock, DWORD bytesToWrite) {
    Win32_Sound* sound = &gStateOS.sound;

    VOID* region1;
    DWORD sizeRegion1;
    VOID* region2;
    DWORD sizeRegion2;

    if (SUCCEEDED(sound->secondaryBuffer->Lock(byteToLock, bytesToWrite, &region1, &sizeRegion1, &region2, &sizeRegion2, 0))) {
        s16* dst = (s16*)region1;
        s16* src = bufferSource;

        DWORD region1SampleCount = sizeRegion1 / sound->bytesPerSample;
        for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; sampleIndex++) {
           *dst++ = *src++;
           *dst++ = *src++;
           sound->runningSampleIndex++;
        }

        dst = (s16*)region2;
        DWORD region2SampleCount = sizeRegion2 / sound->bytesPerSample;
        for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++) {
           *dst++ = *src++;
           *dst++ = *src++;
           sound->runningSampleIndex++;
        }

        sound->secondaryBuffer->Unlock(region1, sizeRegion1, region2, sizeRegion2);
    }
}

Public void UpdateAudio() {
#if 0
    f32 fromBeginToAudioSeconds = Time_OS_Seconds_Elapsed(gTimeState.counterLastFrameOS, Time_OS_Counter());
    Win32_Sound* sound = &gStateOS.sound;

    DWORD playCursor;
    DWORD writeCursor;
    DWORD byteToLock = 0;
    DWORD bytesToWrite = 0;

    if (!sound->secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK) {
        sound->isValid = false;
        return;
    }

    if (!sound->isValid) {
        sound->runningSampleIndex = (u32)(writeCursor / sound->bytesPerSample);
        sound->isValid = true;
    }

    byteToLock = (sound->runningSampleIndex * sound->bytesPerSample) % sound->bufferSize;

    DWORD expectedSoundBytesPerFrame = (DWORD)(sound->bufferSize / gTimeState.gameUpdateRateHz);

    f32 secondsLeftUntilFlip = gTimeState.targetTimePerFrameSec - fromBeginToAudioSeconds;

    DWORD expectedBytesUntilFlip = (DWORD)((secondsLeftUntilFlip / gTimeState.targetTimePerFrameSec) * (f32)expectedSoundBytesPerFrame);
    DWORD expectedFrameBoundaryByte = playCursor + expectedBytesUntilFlip;

    DWORD safeWriteCursor = writeCursor;
    if (safeWriteCursor < playCursor) {
        safeWriteCursor += sound->bufferSize;
    }
    Assert(playCursor < safeWriteCursor);

    DWORD safeByteToLockCursor = byteToLock;
    if (safeByteToLockCursor < playCursor) {
        safeByteToLockCursor += sound->bufferSize;
    }
    //Assert(safeWriteCursor <= safeByteToLockCursor);

    safeWriteCursor += sound->safetyBytes;

    b8 audioCardIsLowLatency = safeWriteCursor < expectedFrameBoundaryByte;
    //Assert(audioCardIsLowLatency);

    DWORD targetCursor = 0;
    //targetCursor = writeCursor + expectedSoundBytesPerFrame + sound->safetyBytes;
#if 1
    if (audioCardIsLowLatency) {
        targetCursor = expectedFrameBoundaryByte + expectedSoundBytesPerFrame;
    } else {
        targetCursor = writeCursor + expectedSoundBytesPerFrame + sound->safetyBytes;
    }
#endif

    targetCursor %= sound->bufferSize;

    // safeByteToLockCursor
    if (byteToLock > targetCursor) {
        bytesToWrite = (sound->bufferSize - byteToLock) + targetCursor;
    } else {
        bytesToWrite = targetCursor - byteToLock;
    }

    Sound_Buffer soundBuffer = {0};
    soundBuffer.samplesPerSecond = sound->samplesPerSecond;
    soundBuffer.sampleCount = bytesToWrite / sound->bytesPerSample;
    soundBuffer.samples = sound->samples;

    GetSoundSamples(&soundBuffer);
    Sound_Fill_Buffer(soundBuffer.samples, byteToLock, bytesToWrite);
#endif
}

Public s32x2 OS_Window_Get_Dims() {
    s32x2 ret = {0};

    RECT rect;
    GetClientRect(gStateOS.window, &rect);

    ret.width  = rect.right - rect.left;
    ret.height = rect.bottom - rect.top;

    return ret;
}

Private inline void Window_Fullsreen_Size() {
    HWND window = gStateOS.window;

    // HWND_TOPMOST
    LONG windowStyle = GetWindowLong(window, GWL_STYLE);
    gStateOS.windowMaximizedFlag = windowStyle & WS_MAXIMIZE;
    SetWindowLongPtrA(window, GWL_STYLE, windowStyle & ~(WS_OVERLAPPEDWINDOW|gStateOS.windowMaximizedFlag));
    SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    ShowWindow(window, SW_SHOWMAXIMIZED);
}

Public void Window_Normal_Size() {
    HWND window = gStateOS.window;

    LONG windowStyle = GetWindowLong(window, GWL_STYLE);
    LONG newStyle = windowStyle|WS_OVERLAPPEDWINDOW;
    SetWindowLongPtrA(window, GWL_STYLE, newStyle);
    SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    ShowWindow(window, SW_RESTORE);
    SetWindowLongPtrA(window, GWL_STYLE, (newStyle & ~WS_MAXIMIZE)|gStateOS.windowMaximizedFlag);
}

Public u32 OS_Get_Page_Size() {
    SYSTEM_INFO systemInfo;

    GetSystemInfo(&systemInfo);

    u32 pageSize = (u32)systemInfo.dwPageSize;

    return pageSize;
}

#if 0
Private inline void ToggleTopMostIfWindowIsActive(HWND windowHandle, b32 isBeingActivated) {
    if (gStateApp.isFullScreen) {
        if (isBeingActivated) {
            SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
        } else {
            SetWindowPos(windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
        }
    }
}
#endif

// TODO(JENH): Should not crash when failing to register devices.
Private void Win32_Init_Raw_Input(HWND window) {
    RAWINPUTDEVICE devices[3];
    RAWINPUTDEVICE *keyboard = &devices[0];
    RAWINPUTDEVICE *mouse = &devices[1];
    RAWINPUTDEVICE *controller = &devices[2];

    // NOTE(JENH): HID Clients Supported in Windows.
    keyboard->usUsagePage = 0x0001;
    keyboard->usUsage = 0x0006;
    keyboard->dwFlags = RIDEV_NOLEGACY;
    keyboard->hwndTarget = window;

#if 1
    mouse->usUsagePage = 0x0001;
    mouse->usUsage = 0x0002;
    mouse->dwFlags = RIDEV_NOLEGACY;
    mouse->hwndTarget = window;

    controller->usUsagePage = 0x0001;
    controller->usUsage = 0x0005;
    controller->dwFlags = 0;
    controller->hwndTarget = window;

    s32 displayCounter = ShowCursor(false);
    Assert(displayCounter < 0);
#endif

    Win32_Check( RegisterRawInputDevices(devices, ArrayCount(devices), sizeof(RAWINPUTDEVICE)), > 0 );
}

Fn_Job(TSetMouseRelative) {
    // TODO(JENH): Shuold set mouse position to the center of the screen for absolute movement of mouse in raw input.
    RAWINPUTDEVICE mouse;

    s32 displayCounter;
    while((displayCounter = ShowCursor(false)) >= 0);
    Assert(displayCounter < 0);

    mouse.usUsagePage = 0x0001;
    mouse.usUsage = 0x0002;
    mouse.dwFlags = RIDEV_NOLEGACY;
    mouse.hwndTarget = gStateOS.window;

    Win32_Check( RegisterRawInputDevices(&mouse, 1, sizeof(RAWINPUTDEVICE)), > 0 );
}

Fn_Job(TSetMouseAbsolute) {
    RAWINPUTDEVICE mouse;

    while(ShowCursor(true) < 0);
    //Assert(displayCounter >= 0);

    mouse.usUsagePage = 0x0001;
    mouse.usUsage = 0x0002;
    mouse.dwFlags = RIDEV_REMOVE;
    mouse.hwndTarget = 0;

    Win32_Check( RegisterRawInputDevices(&mouse, 1, sizeof(RAWINPUTDEVICE)), > 0 );
}

#if 0
Intern Array_CString ParseCommands(Memory_Arena *arena, CString cmdline) {
    Array_CString cmds = {0};
    cmds.A = ArenaPushArray(arena, CString, UNDETERMINED_SIZE);

    char *scan = cmdline;
    char *end  = (char *)((byte *)arena->base + arena->size);

    while (INFINITE_LOOP) {
        ++cmds.size;

        scan = FindAnyDiffCharForward(scan, (u32)arena->size, LitToStr(" \t\0"), 0);

	    if (*scan == '\"') {
            CString *cmd = ArenaPushType(arena, CString);
            *cmd = ++scan;

            scan += FindAnyCharForward(scan, end, LitToStr("\"\0"));

	        *scan++ = '\0';

	        if (*scan == '\0') { break; }
	    } else {
            CString *cmd = ArenaPushType(arena, CString);
            *cmd = scan;

            scan += FindAnyCharForward(scan, end, LitToStr(" \t\0"));

            if (*scan == '\0') { break; }

            *scan++ = '\0';
        }
    }

    return cmds;
}
#endif

Private b8 Controller_Button_Change(u8 inButton, Input_Index inInputIndex, Input_Index* outInputIndex, b8* outIsPress) {
    Input_Mapping* mapping = Input_Get_Mappings();
    Bit_Flags_Inputs_State *inputsState = &mapping->inputsState;

    if ( inButton && !Bit_Flags_Is_Set(inputsState, inInputIndex) ) {
        *outInputIndex = inInputIndex;
        *outIsPress = true;
        return true;
    } else if ( !inButton && Bit_Flags_Is_Set(inputsState, inInputIndex) ) {
        *outInputIndex = inInputIndex;
        *outIsPress = false;
        return true;
    }

    return false;
}

Intern inline void WIN32_ProcessPendingEvents(MSG *event, u32* outSleepTime) {
	switch (event->message) {
	    case WM_QUIT: {
            Program_Close();
	    } break;

#if 1
        case WM_INPUT: {

            //u32 sizeRawInputBuffer;
            //Win32_Check(GetRawInputBuffer(0, &sizeRawInputBuffer, sizeof(RAWINPUTHEADER)) != (UINT)-1);

            // TODO(JENH): Change this to GetRawInputBuffer call for high update rate devices.

            RAWINPUT* rawInput;
            UINT dataSize;
            Win32_Check( GetRawInputData((HRAWINPUT)event->lParam, RID_INPUT, 0, &dataSize, sizeof(RAWINPUTHEADER)), != (UINT)-1 );


#if 1
            void* arenaMem = OS_Alloc_Mem(KiB(256));
            Memory_Arena arena;
            Arena_Create(&arena, arenaMem, KiB(256));

            rawInput = (RAWINPUT*)ArenaPushMem(&arena, dataSize);
#else
            // TODO(JENH): There is a bug with the temp arena and windows.
            rawInput = (RAWINPUT*)ArenaPushMem(&dyMem.temp.arena, dataSize);
#endif

            u32 bytesWritten;
            Win32_Check( (bytesWritten = GetRawInputData((HRAWINPUT)event->lParam, RID_INPUT, rawInput,
                                                          &dataSize, sizeof(RAWINPUTHEADER))), != (UINT)-1 && bytesWritten == dataSize );

            // TODO(JENH): Handle error scan codes (0x00, 0xff) and other protocol scan codes.
            //             https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

            Input_Index inputIndex = II_Nul;
            b8 isPress = false;

            f32 timeStamp = Time_OS_Seconds_Elapsed(Time_OS_Last_Counter(), Time_OS_Counter());

            Input_Mapping* mapping = Input_Get_Mappings();
            Bit_Flags_Inputs_State *inputsState = &mapping->inputsState;

#if 0
            // TODO(JENH): This should go away when handling exteded inputs releases properly.
            Bit_Flags_Set(inputsState, II_Ms_Movement);
            //
#endif

            u8 type = (u8)rawInput->header.dwType;
            switch (type) {
                case RIM_TYPEMOUSE: {
                    RAWMOUSE *mouse = &rawInput->data.mouse;

                    u32 buttonFlags = (u32)mouse->usButtonFlags;
                    u32 flags = (u32)mouse->usFlags;

                    Assert(!Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_1_DOWN|RI_MOUSE_BUTTON_1_UP));
                    if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_1_DOWN)) {
                        inputIndex = II_Ms_Button1;
                        isPress = true;
                    } else if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_1_UP)) {
                        inputIndex = II_Ms_Button1;
                        isPress = false;
                    }

                    Assert(!Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_2_DOWN|RI_MOUSE_BUTTON_2_UP));
                    if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_2_DOWN)) {
                        inputIndex = II_Ms_Button2;
                        isPress = true;
                    } else if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_2_UP)) {
                        inputIndex = II_Ms_Button2;
                        isPress = false;
                    }

                    Assert(!Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_3_DOWN|RI_MOUSE_BUTTON_3_UP));
                    if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_3_DOWN)) {
                        inputIndex = II_Ms_Button3;
                        isPress = true;
                    } else if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_3_UP)) {
                        inputIndex = II_Ms_Button3;
                        isPress = false;
                    }

                    Assert(!Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_4_DOWN|RI_MOUSE_BUTTON_4_UP));
                    if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_4_DOWN)) {
                        inputIndex = II_Ms_Button4;
                        isPress = true;
                    } else if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_4_UP)) {
                        inputIndex = II_Ms_Button4;
                        isPress = false;
                    }

                    Assert(!Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_5_DOWN|RI_MOUSE_BUTTON_5_UP));
                    if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_5_DOWN)) {
                        inputIndex = II_Ms_Button5;
                        isPress = true;
                    } else if (Flags_Has_All(buttonFlags, RI_MOUSE_BUTTON_5_UP)) {
                        inputIndex = II_Ms_Button5;
                        isPress = false;
                    }

                    if (Flags_Has_All(buttonFlags, RI_MOUSE_WHEEL)) {
                        inputIndex = II_Ms_Wheel_V;
                        //data.move = mouse->usButtonData;
                        isPress = true;
                    }

                    if (Flags_Has_All(buttonFlags, RI_MOUSE_HWHEEL)) {
                        inputIndex = II_Ms_Wheel_H;
                        //data.move = mouse->usButtonData;
                        isPress = true;
                    }

                    if (Flags_Has_All(flags, MOUSE_MOVE_ABSOLUTE)) {
                        INVALID_PATH("Suppoting relative mouse only.");

                        RECT rect;
                        if (Flags_Has_All(flags, MOUSE_VIRTUAL_DESKTOP)) {
                            rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
                            rect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
                            rect.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                            rect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
                        } else {
                            rect.left = 0;
                            rect.top = 0;
                            rect.right = GetSystemMetrics(SM_CXSCREEN);
                            rect.bottom = GetSystemMetrics(SM_CYSCREEN);
                        }

                        //s32 absoluteX = mouse->lLastX, rect.right, 65535 + rect.left;
                        //s32 absoluteY = mouse->lLastY, rect.bottom, 65535 + rect.top;
                    } else if (mouse->lLastX != 0 || mouse->lLastY != 0) {
                        gInputSystem.mouseCooldown = 10;
                        *outSleepTime = gInputSystem.mouseCooldown;

                        Extended_Input* extendedInput = &mapping->extendedInputs[ExtendedInput(II_Ms_Movement)];

                        extendedInput->actionIDAttached = A_H_Move_Cam;

                        if ( extendedInput->actionIDAttached != A_Nul ) {
                            extendedInput->data.moveX += mouse->lLastX;
                            extendedInput->data.moveY += mouse->lLastY;
                        }

                        inputIndex = II_Ms_Movement;
                        isPress = true;
                    }
                } break;

                case RIM_TYPEKEYBOARD: {
                    RAWKEYBOARD* keyboard = &rawInput->data.keyboard;

                    u32 scanCode = (u32)keyboard->MakeCode;
                    Assert( scanCode < 0xff );

                    if ( Flags_Has_All(keyboard->Flags, RI_KEY_BREAK) ) {
                        isPress = false;
                    } else if ( Flags_Has_All(keyboard->Flags, RI_KEY_MAKE) ) {
                        isPress = true;
                    } NO_ELSE

                    if ( Input_Text_Mode_Enabled() && isPress ) {
            		    BYTE keyboardState[256];

                        Win32_Check( GetKeyboardState(keyboardState), > 0 );

                        WORD keyWord;
                        b32 isChar = ToAscii((UINT)keyboard->VKey, scanCode, keyboardState, &keyWord, false);

                        if ( keyboard->VKey == 0x30 ) {
                            inputIndex = II_Kbd_Y;
                            goto desable_text_mode;
                        }

                        char charPressed = 0;

                        if ( isChar ) {
                            charPressed = (char)keyWord;
                        } NO_ELSE

                        UI_Text_Write(gGameState.textToWrite, charPressed);
                        goto hola;
                    } else {
                        if ( Flags_Has_Any(keyboard->Flags, RI_KEY_E0|RI_KEY_E1) ) {
                            scanCode |= KBD_EXT_INDEX_MASK;
                        }

                        inputIndex = (Input_Index)scanCode;
                    }

                } break;

                case RIM_TYPEHID: {
                    RAWHID* hid = &rawInput->data.hid;

                    Xbox_360_Controller* controller = (Xbox_360_Controller*)hid->bRawData;

                    if ( Controller_Button_Change(controller->A, II_Xbox_360_A, &inputIndex, &isPress) ) {
                        LogInfo("A: %s", (isPress) ? "Press" : "Release");
                    }

                    if ( Controller_Button_Change(controller->B, II_Xbox_360_B, &inputIndex, &isPress) ) {
                        LogInfo("B: %s", (isPress) ? "Press" : "Release");
                    }

                    if ( Controller_Button_Change(controller->X, II_Xbox_360_X, &inputIndex, &isPress) ) {
                        LogInfo("X: %s", (isPress) ? "Press" : "Release");
                    }

                    if ( Controller_Button_Change(controller->Y, II_Xbox_360_Y, &inputIndex, &isPress) ) {
                        LogInfo("Y: %s", (isPress) ? "Press" : "Release");
                    }

                    s16 LSX = (s16)(controller->LSX - 0x8000);
                    s16 LSY = (s16)(controller->LSY - 0x8000);

                    if ( (LSX <= -DEADZONE || DEADZONE <= LSX) ||
                         (LSY <= -DEADZONE || DEADZONE <= LSY) ) {
                        Extended_Input* extendedInput = &mapping->extendedInputs[ExtendedInput(II_Xbox_360_LS)];

                        extendedInput->actionIDAttached = A_H_Player_Move;

                        if ( extendedInput->actionIDAttached != A_Nul ) {
                            extendedInput->data.moveX = LSX;
                            extendedInput->data.moveY = LSY;
                        }

                        inputIndex = II_Xbox_360_LS;
                        isPress = true;
                    } else if ( Bit_Flags_Is_Set(inputsState, II_Xbox_360_LS) ) {
                        Extended_Input* extendedInput = &mapping->extendedInputs[ExtendedInput(II_Xbox_360_LS)];

                        extendedInput->actionIDAttached = A_Nul;

                        extendedInput->data.moveX = 0;
                        extendedInput->data.moveY = 0;

                        inputIndex = II_Xbox_360_LS;
                        isPress = false;
                    }

                    //LogInfo("LSX: %hd", LSX);
                } break;

                NO_DEFAULT
            }

            desable_text_mode:
            Input_Update_Actions(inputIndex, isPress, timeStamp);

hola:
            if (GET_RAWINPUT_CODE_WPARAM(event->wParam) == RIM_INPUT) {
                DefWindowProcA(event->hwnd, event->message, event->wParam, event->lParam);
            }

            OS_Free_Mem(arenaMem);
        } break;

        case WM_MOUSEMOVE: {
            s32 x = (s32)LOWORD(event->lParam);
            s32 y = (s32)HIWORD(event->lParam);

            gInputSystem.mouseCooldown = 10;
            *outSleepTime = gInputSystem.mouseCooldown;

            Input_Mapping* mapping = Input_Get_Mappings();

            Extended_Input* extendedInput = &mapping->extendedInputs[ExtendedInput(II_Ms_Movement)];
            extendedInput->data.moveX = x;
            extendedInput->data.moveY = y;

            Input_Update_Actions(II_Ms_Movement, true, 0.0f);

            //extendedInput->actionIDAttached = A_H_UI_Cursor;
        } break;

        case WM_LBUTTONDOWN: {
            Input_Mapping* mapping = Input_Get_Mappings();
            Bit_Flags_Inputs_State *inputsState = &mapping->inputsState;

            Assert(!Bit_Flags_Is_Set(inputsState, II_Ms_Button1));

            Input_Update_Actions(II_Ms_Button1, true, 0.0f);
        } break;

        case WM_LBUTTONUP: {
            Input_Mapping* mapping = Input_Get_Mappings();
            Bit_Flags_Inputs_State *inputsState = &mapping->inputsState;

            Input_Update_Actions(II_Ms_Button1, false, 0.0f);
        } break;
#endif

	    default: {
		    DispatchMessageA(event);
	    } break;
	}
}

LRESULT CALLBACK WIN32_EventCallback(HWND windowHandle, UINT event, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (event) {
        case WM_CLOSE: {
            Program_Close();
        } break;

        case WM_DESTROY: {
            Program_Close();
        } break;

#if 0
        //TODO(JENH): Double clicking in the title bar doesn't work.
        case WM_SYSCOMMAND: {
        } break;

        case WM_ENTERSIZEMOVE: {
        } break;
#endif

        case WM_SIZE: {
#if 0
            s32x2 winDims = S32x2(LOWORD(lParam), HIWORD(lParam));

            if (winDims.width > 0 && winDims.height > 0) {
                RecreateSwapChain();
            }
#endif
        } break;

        case WM_SETCURSOR: {
            (void)SetCursor(LoadCursor(0, IDC_ARROW));
        } break;

        case WM_ACTIVATEAPP: {
            b32 isBeingActivated = (b32)wParam;

            //ToggleTopMostIfWindowIsActive(windowHandle, isBeingActivated);
        } break;

        default: {
            result = DefWindowProcA(windowHandle, event, wParam, lParam);
        } break;
    }

    return result;
}

Public void Thread_Sleep(u32 milliseconds) {
    if (milliseconds > 0) {
        Sleep((DWORD)milliseconds);
    }
}

typedef struct {
    HANDLE handle;
    u32 ID;
} Thread_Info;

Private Global Thread_Info threadPool[64];
Private Global u32 threadPoolCount;
Private Global Job_Queue globalQueue;
Private Global HANDLE globalSemaphore;

Intern b8 GetNextJob(Job_Queue *queue, Job_Entry *jobEntry) {
    u32 currentJobIndex = queue->index;
    u32 nextJobIndex = currentJobIndex + 1;

    if (queue->index < queue->size) {
        u32 index = (u32)InterlockedCompareExchange(&queue->index, nextJobIndex, currentJobIndex);
        if (index == currentJobIndex) {
            *jobEntry = queue->Q[index];
            return 1;
        }
    }

    return 0;
}

Intern void ThreadPoolAddJob(Job_Queue *queue, HANDLE semaphore, Fn_Ptr_Job func, void *args) {
    Job_Entry *job = &queue->Q[queue->size];

    job->func = func;
    job->args = args;

    (void)Atomic_Inc_U32(&queue->size);

    Win32_Check( ReleaseSemaphore(semaphore, 1, 0), > 0 );
}

Export Fn_Prot_Thread_Function(OS_Thread_Home) {
    u32 reloadIndex = 0;
    u32 semaphoreIndex = 1;
    u32 msgIndex = 2;

    // TODO(JENH) This shouldn't be here.
    while ( semaphoreOS == 0 );

    u32 sleepTime = INFINITE;

    const HANDLE handles[] = { reloadSemaphore, semaphoreOS };

    while ( INFINITE_LOOP ) {
        u32 handleIndex;
        Win32_Check( (handleIndex = (u32)MsgWaitForMultipleObjects(ArrayCount(handles), handles, false, sleepTime, QS_ALLINPUT)), != WAIT_FAILED );

        if ( handleIndex == WAIT_TIMEOUT ) {
            sleepTime = INFINITE;

            Input_Index inputIndex = II_Ms_Movement;
            b8 isPress = false;
            f32 timeStamp = Time_OS_Seconds_Elapsed(Time_OS_Last_Counter(), Time_OS_Counter());

            Input_Update_Actions(inputIndex, isPress, timeStamp);
            continue;
        }

        if ( handleIndex == reloadIndex ) { return; }

        if ( handleIndex == semaphoreIndex ) {
            Job_Entry job;
            GetNextJob(&queueOS, &job);
            job.func(job.args);
        } else if ( handleIndex == msgIndex ) {
            MSG event;

#if 0
            //Process and remove all messages before WM_INPUT
            while(PeekMessage(&event, 0, 0, WM_INPUT - 1, PM_REMOVE)) {
                WIN32_ProcessPendingEvents(&event);
            }
            //Process and remove all messages after WM_INPUT
            while(PeekMessage(&event, 0, WM_INPUT + 1, MAX_U32, PM_REMOVE)) {
                WIN32_ProcessPendingEvents(&event);
            }
#endif

            while (PeekMessageA(&event, 0, 0, 0, PM_REMOVE)) {
                WIN32_ProcessPendingEvents(&event, &sleepTime);
            }
        }
    }
}

typedef struct {
    String str;
} Thread_String_Data;

Intern Fn_Job(PrintThreadString) {
    String *msg = (String *)args;
    LogDebug("thread: %s", msg->str);
}

Export Fn_Prot_Thread_Function(Job_Home) {
    // TODO(JENH) This shouldn't be here.
    while ( globalSemaphore == 0 );

    HANDLE handles[] = { reloadSemaphore, globalSemaphore };

    while ( INFINITE_LOOP ) {
        Job_Entry job = {0};

        if ( GetNextJob(&globalQueue, &job) ) {
            job.func(job.args);
        } else {
            DWORD handleIndex = WaitForMultipleObjects(ArrayCount(handles), handles, false, INFINITE);

            if ( handleIndex == 0 ) { return; }

            //Win32_Check( ret != WAIT_FAILED );
            //Assert( ret == WAIT_OBJECT_0 );
        }
    }
}

Intern inline void SetMouseRelative() {
    ThreadPoolAddJob(&queueOS, semaphoreOS, TSetMouseRelative, 0);
}

Intern inline void SetMouseAbsolute() {
    ThreadPoolAddJob(&queueOS, semaphoreOS, TSetMouseAbsolute, 0);
}

Intern Fn_Job(WindowCreate) {
    (void)args;

    HINSTANCE instance = (HINSTANCE)GetModuleHandleA(0);

    s32x2 windowDims = { 960, 540 };

    RECT clientRect = {0, 0, windowDims.width, windowDims.height};

    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

    s32x2 windowDimsWithBorder;
    windowDimsWithBorder.x = (clientRect.right + -clientRect.left);
    windowDimsWithBorder.y = (clientRect.bottom += -clientRect.top);

    WNDCLASSA windowClass = {0};

    windowClass.style = CS_VREDRAW|CS_HREDRAW;
    windowClass.lpfnWndProc = WIN32_EventCallback;
    windowClass.hInstance = instance;
    // windowClass.hIcon = ;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    //gStateOS.cursor = LoadCursor(hinst, MAKEINTRESOURCE(230))

    windowClass.lpszClassName = "Rhythm_Car";

    ATOM windowClassAtom = RegisterClassA(&windowClass);
    if (!windowClassAtom) {
        LogError("The window class did not register successfully");
    }

    HWND window = CreateWindowExA(0, windowClass.lpszClassName, "Rhythm Car", WS_VISIBLE|WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                  CW_USEDEFAULT, windowDimsWithBorder.width, windowDimsWithBorder.height, 0, 0, instance, 0);

    if (!window) {
        LogError("Failed at creating window");
    }

#if 0
    LONG lStyle = GetWindowLong(window, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLong(window, GWL_STYLE, lStyle);
#endif

    Win32_Init_Raw_Input(window);
#if 0
    if (!Win32_Init_Raw_Input(window)) {
        LogError("Failed register input devices");
    }
#endif

    gStateOS.windowMaximizedFlag = 0;
    gStateOS.windowClassAtom = windowClassAtom;
    gStateOS.window = window;
    gStateOS.instance = instance;
}

Public b8 OS_Init() {
    // for sparce files.
#if 0
    DWORD VolumeSerialNumber;
    DWORD MaximumComponentLength;
    DWORD FileSystemFlags;

    char hola[256];

    BOOL resCaca = GetVolumeInformationA(0, 0, 0, &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, hola, ArrayCount(hola));
#endif

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    //threadPoolCount = systemInfo.dwNumberOfProcessors - 3;
    threadPoolCount = 1;

    globalSemaphore = CreateSemaphoreA(0, 0, MAX_S32, 0);
    Assert(globalSemaphore);

    for (u32 i = 0; i < threadPoolCount; ++i) {
        Thread_Create(Job_Home, 0);
    }

#if 1
    u32 StringsCount = 16;
    String *jobStrings = ArenaPushArray(&dyMem.perma.strings, String, StringsCount);

    for (u32 i = 0; i < StringsCount; ++i) {
        char stringBuffer[KiB(4)];

        String *jobStr = &jobStrings[i];
        jobStr->size = (u32)sprintf_s(stringBuffer, ArrayCount(stringBuffer), "soy weon %u", i+1);
        jobStr->str  = ArenaPushArray(&dyMem.perma.strings, char, jobStr->size + 1);
        Mem_Copy_Forward(jobStr->str, stringBuffer, jobStr->size);
        jobStr->str[jobStr->size] = '\0';

        ThreadPoolAddJob(&globalQueue, globalSemaphore, PrintThreadString, jobStr);
    }
#endif

    semaphoreOS = CreateSemaphoreA(0, 0, MAX_S32, 0);
    Assert(semaphoreOS);

    Thread_Create(OS_Thread_Home, 0);

    ThreadPoolAddJob(&queueOS, semaphoreOS, WindowCreate, 0);

    Win32_Sound* sound = &gStateOS.sound;

    sound->samplesPerSecond = 48000;
    sound->bytesPerSample = sizeof(s16) * 2;
    sound->bufferSize  = sound->bytesPerSample * sound->samplesPerSecond;
    sound->safetyBytes = (u32)(((f32)sound->bufferSize / (f32)Time_Get_Update_Rate()) / 3.0f);

    while (gStateOS.window == 0);

    InitDirectSound(sound->samplesPerSecond, sound->bufferSize);
    Sound_Clear_Buffer();
    sound->secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

    sound->samples = ArenaPushArray(&dyMem.perma.arena, s16, sound->samplesPerSecond);

    return 0;
}

Public void OS_Cleanup() {
}
