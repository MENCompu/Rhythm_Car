#ifndef JENH_TEST_UTILS_H
#define JENH_TEST_UTILS_H

#define Test_Check(inExpresion) Assert( inExpresion )

#define CMD_LINE "cl -Wall -WX -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4200 -wd4324 -wd4996 -wd4820 -wd5246 -wd5029 -wd4815 -wd5220 -wd4514 -wd4577 -wd5045 -wd4471 -MDd -FC -Z7 -Oi -Od -GR- -EHa- -nologo R:\\src\\tests\\test_free_list.c -link -OUT:test2.exe -opt:ref -subsystem:console,5.02"

Public void Rebuild_If_Necessary(CString inBinName) {
    u64 binWriteTime  = File_Get_Write_Time(inBinName);
    u64 codeWriteTime = File_Get_Write_Time(__FILE__);

    if ( binWriteTime >= codeWriteTime ) { return; }

    STARTUPINFO startupInfo = { 0 };
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION procInfo;

    Win32_Check( CreateProcessA(0, CMD_LINE, 0, 0, FALSE, 0, 0, 0, &startupInfo, &procInfo) );

    WaitForSingleObject(procInfo.hProcess, INFINITE);

    Win32_Check( CreateProcessA("test2.exe", 0, 0, 0, FALSE, 0, 0, 0, &startupInfo, &procInfo) );

    exit(0);
}

#endif // JENH_TEST_UTILS_H
