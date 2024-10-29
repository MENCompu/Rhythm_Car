typedef struct {
    b8 isRunning;
    b8 isFullScreen;
} State_Application;

Private Global State_Application gStateApp;

Public void Program_Close() {
    gStateApp.isRunning = JENH_FALSE;
}

Public void Window_Toggle_Fullsreen() {
    if ( gStateApp.isFullScreen ) {
        Window_Normal_Size();
        gStateApp.isFullScreen = JENH_FALSE;
    } else {
        Window_Fullsreen_Size();
        gStateApp.isFullScreen = JENH_TRUE;
    }
}
