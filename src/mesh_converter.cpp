#if 1
Private u32 Asset_Loader_Load_Material_MTL(String inDirPath, String inFileName, File_Handle inDstFile, String* outMaterialNames) {
    Local_Str(filePath, MAX_FILE_PATH);
    CatStr(&filePath, inDirPath, inFileName);
    filePath.str[filePath.size++] = '\0';

    File_Handle file = File_Open(filePath.str);

    u32 fileSize = File_Get_Size(file);
    if (fileSize == MAX_U32) {
        LogWarn("Failed to load obj file");
        return JENH_FALSE;
    }

    char* mem = (char*)ArenaPushMem(&dyMem.temp.arena, fileSize);
    if (!File_Read(file, fileSize, mem)) {
        LogWarn("Failed to load obj file");
        return JENH_FALSE;
    }

    File_Close(file);

    u32 materialCount = 0;
    RCAF_Material material = {0};

    // "- sizeof(RCAF_Material)" is to compenssate the first newmtl.
    u64 currentMaterialFileOffset = FieldOffset(RCAF_Model, materials) - sizeof(RCAF_Material);
    b8 firstMaterial = JENH_TRUE;

    for (String_Scan fileScan = StrScan(mem, mem + fileSize); !StrScanEOS(fileScan);) {
        StrScanAdvance(&fileScan, FindAnyCharForward(fileScan.scan, fileScan.end, LitToStr("\r\n")));

        String_Scan line = StrToStrScan(fileScan.data);

        StrScan_Consume_White_Space(&fileScan);

        StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));

        if (StrScanEOS(line)) { continue; }

        if (Str_Equal(line.data, LitToStr("#"))) {
            continue;
        } else if (Str_Equal(line.data, LitToStr("Kd"))) {
            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            material.diffuseColor.x = StrToF32(line.data);

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            material.diffuseColor.y = StrToF32(line.data);

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));
            material.diffuseColor.z = StrToF32(line.data);
        } else if (Str_Equal(line.data, LitToStr("Ks"))) {
            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            material.specularColor.x = StrToF32(line.data);

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            material.specularColor.y = StrToF32(line.data);

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));
            material.specularColor.z = StrToF32(line.data);
        } else if (Str_Equal(line.data, LitToStr("map_Kd"))) {
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            String texFileName = Str_Get_File_Name_In_Path(line.data);
            material.diffuseTex = Converter_Get_Asset_Handle(texFileName);
        } else if (Str_Equal(line.data, LitToStr("map_Ks"))) {
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            String texFileName = Str_Get_File_Name_In_Path(line.data);
            material.specularTex = Converter_Get_Asset_Handle(texFileName);
        } else if (Str_Equal(line.data, LitToStr("map_bump")) || Str_Equal(line.data, LitToStr("map_Disp"))) {
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            String texFileName = Str_Get_File_Name_In_Path(line.data);
            material.normalTex = Converter_Get_Asset_Handle(texFileName);
        } else if (Str_Equal(line.data, LitToStr("newmtl"))) {
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            if ( !firstMaterial ) {
                Assert( File_Write_At(inDstFile, currentMaterialFileOffset, sizeof(RCAF_Material), &material) );
            }

            currentMaterialFileOffset += sizeof(RCAF_Material);
            outMaterialNames[materialCount++] = line.data;

            Mem_Zero_Type(&material);
            material.diffuseColor  = F32x4(1.0f, 1.0f, 1.0f, 1.0f);
            material.specularColor = F32x4(1.0f, 1.0f, 1.0f, 1.0f);

            firstMaterial = JENH_FALSE;
            //outMaterial = &outMaterials[materialCount];
        }
    }

    Assert( File_Write_At(inDstFile, currentMaterialFileOffset, sizeof(RCAF_Material), &material) );

    return materialCount;
}
#endif

typedef struct {
    Material material;
    u32 meshID;
} Object;

Public b8 Model_Converter_OBJ_To_RCAF(void* mem, u32 size, File_Handle inDstFile, String inDirPath, Memory_Arena* inArena) {
    // TODO(JENH): Temporary.
    // TODO(JENH): This is bad, better memory handling.
    u32 materialCount = 0;
    RCAF_Material materials[256];
    Mem_Zero_Array(materials);
    String materialNames[256];

    u32 meshCount = 0;
    RCAF_Mesh meshes[512];
    RCAF_Mesh* mesh = 0;

    mesh = &meshes[meshCount++];

    mesh->indexSize = sizeof(u32);
    mesh->vertexSize = sizeof(Vertex_Data);
    mesh->indexCount = 0;
    mesh->vertexCount = 0;

    u32 firstFaceIndex = FindByteBackwards((byte*)mem + size - 1, size, 'f');
    String firstFace = Str(((char*)mem + size - 1) - firstFaceIndex + 2, firstFaceIndex);
    String_Scan firstFaceScan = StrToStrScan(firstFace);

    Tri_Indices attribCounts;

    StrScanAdvance(&firstFaceScan, FindCharForward(firstFaceScan.scan, firstFaceScan.end, '/'));
    attribCounts.i1 = (u32)Str_To_S32(firstFaceScan.data);

    StrScanAdvance(&firstFaceScan, FindCharForward(firstFaceScan.scan, firstFaceScan.end, '/'));
    attribCounts.i2 = (u32)Str_To_S32(firstFaceScan.data);

    StrScanAdvance(&firstFaceScan, FindCharForward(firstFaceScan.scan, firstFaceScan.end, ' '));
    attribCounts.i3 = (u32)Str_To_S32(firstFaceScan.data);

    u32 positionCount = 1;
    f32x3* positionData = (f32x3*)OS_Alloc_Mem((WordSize)(sizeof(f32x3) * (u32)((f32)attribCounts.i1 * 1.3f)));

    u32 texCoordsCount = 1;
    f32x2* texCoordsData = (f32x2*)OS_Alloc_Mem((WordSize)(sizeof(f32x2) * (u32)((f32)attribCounts.i2 * 1.3f)));

    u32 normalCount = 1;
    f32x3* normalData = (f32x3*)OS_Alloc_Mem((WordSize)(sizeof(f32x3) * (u32)((f32)attribCounts.i3 * 1.3f)));

    b8 isFirstMesh = JENH_TRUE;

    // TODO(JENH): This is bad, better memory handling.
    Tri_Indices* mulIndexData = (Tri_Indices*)OS_Alloc_Mem(size);

    u32 dstFileSize = sizeof(RCAF_Model);

    b8 mtllibIsFirstFound = JENH_FALSE;

    u32 lineCount = 1;

    for (String_Scan fileScan = StrScan((char*)mem, (char*)mem + size); !StrScanEOS(fileScan);) {
        StrScanAdvance(&fileScan, FindAnyCharForward(fileScan.scan, fileScan.end, LitToStr("\r\n")));
        String_Scan line = StrToStrScan(fileScan.data);

        if (fileScan.scan[0] == '\n') { ++fileScan.scan; }

        StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));

        if (StrScanEOS(line)) { continue; }

        if (Str_Equal(line.data, LitToStr("#"))) {
            continue;
        } else if (Str_Equal(line.data, LitToStr("mtllib"))) {
            mtllibIsFirstFound = JENH_TRUE;

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            String materialName = Str_Get_File_Name_In_Path(line.data);

            Local_Str(name, 256);
            Mem_Zero(name.str, 256);
            Mem_Copy_Forward(name.str, materialName.str, materialName.size);
            name.size = materialName.size;

            materialCount += Asset_Loader_Load_Material_MTL(inDirPath, name, inDstFile, materialNames);

            dstFileSize += (materialCount * sizeof(RCAF_Material));
        } else if (Str_Equal(line.data, LitToStr("usemtl"))) {
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            for (u32 i = 0; i < materialCount; ++i) {
                if ( Str_Equal(materialNames[i], line.data) ) {
                    mesh->materialID = i;

                    //if ( mesh->material.diffuseTex.assetID  == 0 ) { DEBUG_Breakpoint; }
                    //if ( mesh->material.specularTex.assetID == 0 ) { DEBUG_Breakpoint; }
                    //if ( mesh->material.normalTex.assetID   == 0 ) { DEBUG_Breakpoint; }

                    break;
                }
            }
        } else if (Str_Equal(line.data, LitToStr("v"))) {
            Assert ( mtllibIsFirstFound );

            f32x3* pos = &positionData[positionCount++];

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            pos->x = StrToF32(line.data);

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            pos->y = StrToF32(line.data);

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));
            pos->z = StrToF32(line.data);
        } else if (Str_Equal(line.data, LitToStr("vn"))) {
            Assert ( mtllibIsFirstFound );

            f32x3* normal = &normalData[normalCount++];

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            normal->x = StrToF32(line.data);

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            normal->y = StrToF32(line.data);

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));
            normal->z = StrToF32(line.data);
        } else if (Str_Equal(line.data, LitToStr("vt"))) {
            Assert ( mtllibIsFirstFound );

            f32x2* texCoord = &texCoordsData[texCoordsCount++];

            StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
            texCoord->x = StrToF32(line.data);

            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr(" \r\n")));
            texCoord->y = StrToF32(line.data);
        } else if (Str_Equal(line.data, LitToStr("f"))) {
            Assert ( mtllibIsFirstFound );

            for (u32 i = 0; i < 3; ++i) {
                Tri_Indices *indices = &mulIndexData[mesh->indexCount++];

                StrScanAdvance(&line, FindCharForward(line.scan, line.end, '/'));
                indices->i1 = (u32)Str_To_S32(line.data);

                StrScanAdvance(&line, FindCharForward(line.scan, line.end, '/'));
                indices->i2 = (u32)Str_To_S32(line.data);

                StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr(" \r\n")));
                indices->i3 = (u32)Str_To_S32(line.data);
            }
        } else if (Str_Equal(line.data, LitToStr("g"))) {
            Assert ( mtllibIsFirstFound );

            if ( !isFirstMesh ) {
                Vertex_Data* vertices = ArenaPushArray(inArena, Vertex_Data, mesh->indexCount);
                u32* indices = ArenaPushArray(inArena, u32, mesh->indexCount * 3);

                mesh->vertexCount = 0;

                f32x3 minPoint = F32x3(MAX_F32, MAX_F32, MAX_F32);
                f32x3 maxPoint = F32x3(-MAX_F32, -MAX_F32, -MAX_F32);

                for (u32 i = 0; i < mesh->indexCount; ++i) {
                    Tri_Indices* mulIndex = &mulIndexData[i];

                    Vertex_Data vertex;
                    vertex.pos = positionData[mulIndex->i1];
                    vertex.tex = texCoordsData[mulIndex->i2];
                    vertex.normal = normalData[mulIndex->i3];

                    u32* index = &indices[i];

                    for (u32 j = 0; j < mesh->vertexCount; ++j) {
                        Vertex_Data* savedVertex = &vertices[j];
                        if ( Mem_Type_Equal(&vertex, savedVertex) ) {
                            *index = j;
                            goto duplicate_found;
                        }
                    }

                    minPoint.x = Min(minPoint.x, vertex.pos.x);
                    minPoint.y = Min(minPoint.y, vertex.pos.y);
                    minPoint.z = Min(minPoint.z, vertex.pos.z);

                    maxPoint.x = Max(maxPoint.x, vertex.pos.x);
                    maxPoint.y = Max(maxPoint.y, vertex.pos.y);
                    maxPoint.z = Max(maxPoint.z, vertex.pos.z);

                    *index = mesh->vertexCount;
                    vertices[mesh->vertexCount++] = vertex;

                    duplicate_found:;
                }

                mesh->halfDim.x = (maxPoint.x - minPoint.x) * 0.5f;
                mesh->halfDim.y = (maxPoint.y - minPoint.y) * 0.5f;
                mesh->halfDim.z = (maxPoint.z - minPoint.z) * 0.5f;

                mesh->center = minPoint + mesh->halfDim;

                mesh->vertexOffset = dstFileSize;
                Assert( File_Write_At(inDstFile, mesh->vertexOffset, mesh->vertexCount * mesh->vertexSize, vertices) );
                dstFileSize += mesh->vertexCount * mesh->vertexSize;

                mesh->indexOffset = dstFileSize;
                Assert( File_Write_At(inDstFile, mesh->indexOffset, mesh->indexCount * mesh->indexSize, indices) );
                dstFileSize += mesh->indexCount * mesh->indexSize;

                Arena_Clear(inArena);

                mesh = &meshes[meshCount++];

                mesh->indexSize = sizeof(u32);
                mesh->vertexSize = sizeof(Vertex_Data);
                mesh->indexCount = 0;
                mesh->vertexCount = 0;
            }

            isFirstMesh = JENH_FALSE;
        }

        ++lineCount;
    }

    // The same that the 'g' input.
    Vertex_Data* vertices = ArenaPushArray(inArena, Vertex_Data, mesh->indexCount);
    u32* indices = ArenaPushArray(inArena, u32, mesh->indexCount * 3);

    mesh->vertexCount = 0;

    f32x3 minPoint = F32x3(MAX_F32, MAX_F32, MAX_F32);
    f32x3 maxPoint = F32x3(-MAX_F32, -MAX_F32, -MAX_F32);

    for (u32 i = 0; i < mesh->indexCount; ++i) {
        Tri_Indices* mulIndex = &mulIndexData[i];

        Vertex_Data vertex;
        vertex.pos = positionData[mulIndex->i1];
        vertex.tex = texCoordsData[mulIndex->i2];
        vertex.normal = normalData[mulIndex->i3];

        u32* index = &indices[i];

        for (u32 j = 0; j < mesh->vertexCount; ++j) {
            Vertex_Data* savedVertex = &vertices[j];
            if ( Mem_Type_Equal(&vertex, savedVertex) ) {
                *index = j;
                goto duplicate_found_2;
            }
        }

        minPoint.x = Min(minPoint.x, vertex.pos.x);
        minPoint.y = Min(minPoint.y, vertex.pos.y);
        minPoint.z = Min(minPoint.z, vertex.pos.z);

        maxPoint.x = Max(maxPoint.x, vertex.pos.x);
        maxPoint.y = Max(maxPoint.y, vertex.pos.y);
        maxPoint.z = Max(maxPoint.z, vertex.pos.z);

        *index = mesh->vertexCount;
        vertices[mesh->vertexCount++] = vertex;

        duplicate_found_2:;
    }

    mesh->halfDim.x = (maxPoint.x - minPoint.x) * 0.5f;
    mesh->halfDim.y = (maxPoint.y - minPoint.y) * 0.5f;
    mesh->halfDim.z = (maxPoint.z - minPoint.z) * 0.5f;

    mesh->center = minPoint + mesh->halfDim;

    mesh->vertexOffset = dstFileSize;
    Assert( File_Write_At(inDstFile, mesh->vertexOffset, mesh->vertexCount * mesh->vertexSize, vertices) );
    dstFileSize += mesh->vertexCount * mesh->vertexSize;

    mesh->indexOffset = dstFileSize;
    Assert( File_Write_At(inDstFile, mesh->indexOffset, mesh->indexCount * mesh->indexSize, indices) );
    dstFileSize += mesh->indexCount * mesh->indexSize;

    Arena_Clear(inArena);
    //

    RCAF_Model model;
    model.meshCount = meshCount;
    model.meshesOffset = dstFileSize;
    model.materialCount = materialCount;

    File_Write_At(inDstFile, 0, sizeof(RCAF_Model), &model);
    File_Write_At(inDstFile, model.meshesOffset, model.meshCount * sizeof(RCAF_Mesh), meshes);

    OS_Free_Mem(mulIndexData);
    OS_Free_Mem(positionData);
    OS_Free_Mem(texCoordsData);
    OS_Free_Mem(normalData);

    return JENH_TRUE;
}
