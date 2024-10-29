#include "dll_api.h"

void Entry(void);
//Import void Entry(void);

#ifdef _WIN32

#pragma warning(push, 1)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

#undef FALSE
#undef TRUE
#undef IN
#undef OUT

#pragma warning(pop)

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE hinstprev, LPSTR cmdline, int cmdshow) {
    Entry();
}

#else

int main(int argc, char *argv[]) {
    Entry();
}

#endif
