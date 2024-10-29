Public u64 Time_CPU_Counter(void) {
    return (u64)__rdtsc();
}

Public f32 Time_Sec_To_Ms(f32 secs) {
    return (1000.0f * secs);
}

#ifdef _WIN32
Private Global Time_OS gTimeOS;

Private void Time_OS_Init(void) {
    LARGE_INTEGER frecuency;
    QueryPerformanceFrequency(&frecuency);
    gTimeOS.counterFrecuency = frecuency.QuadPart;

    u32 desiredSchedulerMs = 1;
    gTimeOS.sleepIsGranular = timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR;

    if ( gTimeOS.sleepIsGranular ) {
        gTimeOS.schedulerMs = desiredSchedulerMs;
    } NO_ELSE
}

Public s64 Time_OS_Counter(void) {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return (s64)result.QuadPart;
}

Public f32 Time_OS_Seconds_Elapsed(s64 begin, s64 end) {
    f32 secondsElapsed = (f32)(end - begin) / (f32)gTimeOS.counterFrecuency;
    return secondsElapsed;
}

Private void Time_OS_Cleanup(void) {
    timeEndPeriod(gTimeOS.schedulerMs);
}

Public Time Time_Mask_Days(Time time) {
    return ( time / TIME_DAY ) * TIME_DAY;
}

Public Time Time_Mask_Hrs(Time time) {
    return ( time / TIME_HR  ) * TIME_HR;
}

Public Time Time_Mask_Mins(Time time) {
    return ( time / TIME_MIN ) * TIME_MIN;
}

Public Time Time_Mask_Secs(Time time) {
    return ( time / TIME_SEC ) * TIME_SEC;
}

Public Time Time_Mask_Mils(Time time) {
    return ( time / TIME_MIL ) * TIME_MIL;
}

Public Time OS_FILETIME_To_Time(FILETIME inFileTime) {
    Time retFileTime;

    ULARGE_INTEGER ularge;
    ularge.LowPart  = inFileTime.dwLowDateTime;
    ularge.HighPart = inFileTime.dwHighDateTime;

    retFileTime = (Time)ularge.QuadPart;

    return retFileTime;
}

Public Time Time_Get(void) {
    SYSTEMTIME timeSys;
    GetSystemTime(&timeSys);

    FILETIME timeOS;
    Win32_Check( SystemTimeToFileTime(&timeSys, &timeOS), > 0 );

    Time ret = OS_FILETIME_To_Time(timeOS) / 1000;
    return ret;
}
#endif
