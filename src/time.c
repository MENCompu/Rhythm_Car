Private Global Time_State gTimeState;

Private void Time_OS_Cleanup();

Public u64 Time_CPU_Counter() {
    return (u64)__rdtsc();
}

Public f32 Time_Sec_To_Ms(f32 secs) {
    return (1000.0f * secs);
}

Public void Time_Init() {
    // TODO(JENH): This should be computed better.
    gTimeState.targetTimePerFrameSec = 1.0f / (f32)gTimeState.gameUpdateRateHz;

    gTimeState.counterLastFrameOS  = Time_OS_Counter();
    gTimeState.counterLastFrameCPU = Time_CPU_Counter();

    gTimeState.refreshRateHz = OS_Get_Refresh_Rate();
    gTimeState.gameUpdateRateHz = gTimeState.refreshRateHz / 2;

    Time_OS_Init();
}

Public void Time_Cleanup() {
    Time_OS_Cleanup();
}

Public s32 Time_Get_Update_Rate() {
    return gTimeState.gameUpdateRateHz;
}

Public s64 Time_OS_Last_Counter() {
    return gTimeState.counterLastFrameOS;
}

#ifdef _WIN32
typedef struct {
    b8  sleepIsGranular;
    u32 schedulerMs;

    s64 counterFrecuency;
} Time_OS;

Private Global Time_OS gTimeOS;

Public void Time_OS_Init() {
    LARGE_INTEGER frecuency;
    QueryPerformanceFrequency(&frecuency);
    gTimeOS.counterFrecuency = frecuency.QuadPart;

    u32 desiredSchedulerMs = 1;
    gTimeOS.sleepIsGranular = timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR;

    if ( gTimeOS.sleepIsGranular ) {
        gTimeOS.schedulerMs = desiredSchedulerMs;
    }
}

Public s64 Time_OS_Counter() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return (s64)result.QuadPart;
}

Public f32 Time_OS_Seconds_Elapsed(s64 begin, s64 end) {
    f32 secondsElapsed = (f32)(end - begin) / (f32)gTimeOS.counterFrecuency;
    return secondsElapsed;
}

Private void Time_OS_Cleanup() {
    timeEndPeriod(gTimeOS.schedulerMs);
}
#endif
