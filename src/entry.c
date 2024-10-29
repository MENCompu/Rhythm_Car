#include "dll_api.h"

Import void RC_Main(void);

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE hinstprev, LPSTR cmdline, int cmdshow) {
    RC_Main();
}

#else

int main(int argc, char *argv[]) {
    RC_Main();
}

#endif
