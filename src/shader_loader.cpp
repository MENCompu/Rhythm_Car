typedef struct {
    u32 size;
    byte* code;
} Shader;

Public b8 Asset_Loader_Load_SPIR_V(CString path, Shader* shader) {
    File_Handle file = File_Open(path);

    u32 fileSize = File_Get_Size(file);
    if (fileSize == MAX_U32) {
        LogWarn("Failed to load shader file");
        return JENH_FALSE;
    }

    // TODO(JENH): Here is a memory leek.
    char* mem = (char*)ArenaPushMem(&dyMem.temp.arena, fileSize);
    //

    if (!File_Read(file, fileSize, mem)) {
        LogWarn("Failed to load shader file");
        return JENH_FALSE;
    }

    shader->size = fileSize;
    shader->code = (byte*)mem;

    return JENH_TRUE;
}
