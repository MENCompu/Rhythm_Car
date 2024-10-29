#ifndef JENH_PROFILER_H
#define JENH_PROFILER_H

typedef struct {
    CString file;
    CString function;
    u32 lineNum;

    u64 timeStampCPU;
} Profile_Entry;

#define Profile_Block_Begin(inID, outTimeStamp) Profile_Block_Begin_(inID, __FILE__, __func__, __LINE__, outTimeStamp)

#endif // JENH_PROFILER_H
