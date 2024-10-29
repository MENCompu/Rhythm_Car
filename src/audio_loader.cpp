//#define Variable_Lenght_Array

#if 0
typedef enum {
    WM_Riff = Magic_4("RIFF"),
    WM_Wave = Magic_4("WAVE"),
    WM_Fmt  = Magic_4("fmt "),
    WM_Data = Magic_4("data"),
} Wav_Marks;
#endif

#pragma pack(push, 1)
typedef struct {
    u32 mark;
    u32 size;
} Riff_Chunk_Header;

typedef struct {
    u32 mark;
    u32 size;
    u32 waveMark;
} Wav_Header;

typedef struct {
    //Riff_Chunk_Header chunkHeader;
	u16 formatTag;
    u16 channelCount;
    u32 sampleRate;
    u32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
#if 0
    u16 extensionSize;
    u16
wValidBitsPerSample	2	Number of valid bits
dwChannelMask	4	Speaker position mask
SubFormat	16
#endif
} Wav_Format;

typedef struct {
    //Riff_Chunk_Header chunkHeader;
    byte mem[];
} Wav_Data;
#pragma pack(pop)

typedef struct {
    //Riff_Chunk_Header chunkHeader;
    u32 mark;
    u32 size;
    byte chunk[];
} Riff_Iter;

Private Riff_Iter* Riff_Iter_Next_Chuck(Riff_Iter* iter) {
    Riff_Iter* retIter;
    retIter = (Riff_Iter*)((byte*)iter + sizeof(Riff_Iter) + iter->size);
    return retIter;
}

Public void Min_Max_Amplitude(Sound* inSound) {
    inSound->minAmplitude = MAX_S16;
    inSound->maxAmplitude = MIN_S16;

    Foreach (s16, sample, inSound->samples, inSound->sampleCount) {
        inSound->minAmplitude = Min(inSound->minAmplitude, *sample);
        inSound->maxAmplitude = Max(inSound->maxAmplitude, *sample);
    }
}

Public b8 Asset_Loader_Load_Wav(String path, Sound* outSound) {
    File_Handle file = File_Open(path.str);

    u32 fileSize = File_Get_Size(file);
    if ( fileSize == MAX_U32 ) {
        LogWarn("Failed to load wav file");
        return JENH_FALSE;
    }

    // TODO(JENH): Remove all "OS_Alloc_Mem".
    byte* mem = (byte*)OS_Alloc_Mem(fileSize);
    if ( !File_Read(file, fileSize, mem) ) {
        LogWarn("Failed to load obj file");
        return JENH_FALSE;
    }

    File_Close(file);

    Wav_Header* header = (Wav_Header*)mem;
    //Riff_Iter* iter = (Riff_Iter*)
    Assert( header->mark == Magic_4("RIFF") );

    for (Riff_Iter* iter = (Riff_Iter*)(header + 1); iter < (Riff_Iter*)(mem + fileSize); iter = Riff_Iter_Next_Chuck(iter)) {
        switch ( iter->mark ) {
            case Magic_4("fmt "): {
                Wav_Format* fmt = (Wav_Format*)&iter->chunk;

                outSound->channelCount = fmt->channelCount;
                outSound->sampleRate = fmt->sampleRate;
            } break;

            case Magic_4("data"): {
                Wav_Data* data = (Wav_Data*)&iter->chunk;

                outSound->sampleCount = iter->size / sizeof(s16);
                outSound->samples = (s16*)&data->mem;

                Min_Max_Amplitude(outSound);
            } break;
        }
    }

    return JENH_TRUE;
}
