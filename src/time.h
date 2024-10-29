#ifndef JENH_TIME_H
#define JENH_TIME_H

typedef struct {
    f32 targetTimePerFrameSec;

    s64 counterLastFrameOS;
    u64 counterLastFrameCPU;
    //s64 prevCounterOS;
    //u64 prevCounterCPU;

    f32 timeSinceStartup;

    s32 refreshRateHz;
    s32 gameUpdateRateHz;
} Time_State;

Public void Time_Init();
Public void Time_Cleanup();

Public s32 Time_Get_Update_Rate();
Public s64 Time_OS_Last_Counter();

Public u64 Time_CPU_Counter();

Public s64 Time_OS_Counter();
Public f32 Time_OS_Seconds_Elapsed(s64 begin, s64 end);

Public f32 Time_Sec_To_Ms(f32 secs);

#endif // JENH_TIME_H
