#ifndef RC_SOFTWARE_RENDERER_H
#define RC_SOFTWARE_RENDERER_H

Intern void DrawBMPImage(Window_Buffer *windowBuffer, BMPImage *bmpImage, f32x2 topLeftF) {
    s32x2 topLeft = RoundF32ToS32_x2(topLeftF);
    s32x2 bottomRight = topLeft + bmpImage->dims;

    topLeft = ClipTop_s32x2(S32x2(0, 0), topLeft);
    bottomRight = ClipBottom_s32x2(windowBuffer->dims, bottomRight);

    u8 *destRow = (u8 *)windowBuffer->memory + topLeft.x * windowBuffer->pitch + topLeft.y * windowBuffer->bytesPerPixel;
    u32 *sourceRow = bmpImage->pixels + ((bmpImage->dims.height - 1) * bmpImage->dims.width);

    for (s32 y = topLeft.y; y < bottomRight.y; y++) {
        u32 *dest = (u32 *)destRow;
        u32 *src  = sourceRow;

        for (s32 x = topLeft.x; x < bottomRight.x; x++) {
            u8 alphaChannel = (u8)(*src >> 24);

            if (alphaChannel) { *dest = *src; }

            ++dest;
            ++src;
        }

        sourceRow -= bmpImage->dims.width;
        destRow += windowBuffer->pitch;
    }
}

Intern void DrawRectBasis(Window_Buffer *windowBuffer, f32x2 o, f32x2 xAxis, f32x2 yAxis, f32x4 color) {
    f32x2 points[4] = {o, o + xAxis, o + yAxis, o + xAxis + yAxis};

    s32x2 topLeft = windowBuffer->dims;
    s32x2 bottomRight = {0};

    for (u32 pIndex = 0; pIndex < ArrayCount(points); ++pIndex) {
        f32x2 p = points[pIndex];

        topLeft.x = Min(FloorF32ToS32(p.x), topLeft.x);
        topLeft.y = Min(FloorF32ToS32(p.y), topLeft.y);
        bottomRight.x = Max(CeilF32ToS32(p.x), bottomRight.x);
        bottomRight.y = Max(CeilF32ToS32(p.y), bottomRight.y);
    }

    //- S32x2(1, 1)
    topLeft = ClipBottom_s32x2(S32x2(0, 0), topLeft);
    bottomRight = ClipTop_s32x2(windowBuffer->dims, bottomRight);

    //BBGGRRAA
    u32 colorU32 = (u32)(((u32)(color.alpha * 255) << 24) |
                         ((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );

    u8 *row = (u8 *)windowBuffer->memory + topLeft.y * windowBuffer->pitch + topLeft.x * windowBuffer->bytesPerPixel;

    for (f32x2 p = S32ToF32_x2(topLeft); p.y < bottomRight.y; ++(p.y)) {
        u32 *pixel = (u32 *)row;

        for (p.x = (f32)topLeft.x; p.x < bottomRight.x; ++(p.x)) {
            b8 pCheck1 = (Dot_f32x2(p - (o), Perp_f32x2(xAxis)) < 0.0f);
            b8 pCheck2 = (Dot_f32x2(p - (o + xAxis), Perp_f32x2(yAxis)) < 0.0f);
            b8 pCheck3 = (Dot_f32x2(p - (o + yAxis), -Perp_f32x2(yAxis)) < 0.0f);
            b8 pCheck4 = (Dot_f32x2(p - (o + xAxis + yAxis), -Perp_f32x2(xAxis)) < 0.0f);

            if (pCheck1 && pCheck2 && pCheck3 && pCheck4) {
                *pixel = colorU32;
            }

            ++pixel;
        }

        row += windowBuffer->pitch;
    }
}

Intern void DrawTriangle(Window_Buffer *windowBuffer, f32x2 a, f32x2 b, f32x2 c, f32x4 color) {
    f32x2 points[] = {a, b, c};

    s32x2 topLeft = windowBuffer->dims;
    s32x2 bottomRight = {0};

    for (u32 pIndex = 0; pIndex < ArrayCount(points); ++pIndex) {
        f32x2 p = points[pIndex];

        topLeft.x = Min(FloorF32ToS32(p.x), topLeft.x);
        topLeft.y = Min(FloorF32ToS32(p.y), topLeft.y);
        bottomRight.x = Max(CeilF32ToS32(p.x), bottomRight.x);
        bottomRight.y = Max(CeilF32ToS32(p.y), bottomRight.y);
    }

    topLeft = ClipBottom_s32x2(S32x2(0, 0), topLeft);
    bottomRight = ClipTop_s32x2(windowBuffer->dims, bottomRight);

    //BBGGRRAA
    u32 colorU32 = (u32)(((u32)(color.alpha * 255) << 24) |
                         ((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );

    f32x2 testp = a + ((b - a) * 0.5f);
    testp = testp + ((c - testp) * 0.5f);

    f32x2 perp1 = (Dot_f32x2(testp - a, Perp_f32x2(b - a)) < 0.0f) ? Perp_f32x2(b - a) : -Perp_f32x2(b - a);
    f32x2 perp2 = (Dot_f32x2(testp - b, Perp_f32x2(c - b)) < 0.0f) ? Perp_f32x2(c - b) : -Perp_f32x2(c - b);
    f32x2 perp3 = (Dot_f32x2(testp - c, Perp_f32x2(a - c)) < 0.0f) ? Perp_f32x2(a - c) : -Perp_f32x2(a - c);

    u8 *row = (u8 *)windowBuffer->memory + topLeft.y * windowBuffer->pitch + topLeft.x * windowBuffer->bytesPerPixel;

    for (f32x2 p = S32ToF32_x2(topLeft); p.y < bottomRight.y; ++(p.y)) {
        u32 *pixel = (u32 *)row;

        for (p.x = (f32)topLeft.x; p.x < bottomRight.x; ++(p.x)) {
            b8 pCheck1 = (Dot_f32x2(p - a, perp1) < 0.0f);
            b8 pCheck2 = (Dot_f32x2(p - b, perp2) < 0.0f);
            b8 pCheck3 = (Dot_f32x2(p - c, perp3) < 0.0f);

            if (pCheck1 && pCheck2 && pCheck3) {
                *pixel = colorU32;
            }

            ++pixel;
        }

        row += windowBuffer->pitch;
    }
}

Intern void DrawLine(Window_Buffer *windowBuffer, f32x2 p1f, f32x2 p2f, f32x4 color) {
    s32x2 p1 = S32(p1f);
    s32x2 p2 = S32(p2f);

    if (p1.x > p2.x) { Swap(&p1, &p2, s32x2); }

    p1 = ClipTop_s32x2(S32x2(0, 0), p1);
    p1 = ClipBottom_s32x2(windowBuffer->dims, p1);
    p2 = ClipTop_s32x2(S32x2(0, 0), p2);
    p2 = ClipBottom_s32x2(windowBuffer->dims, p2);

    f32 slope;
    if (p2.x == p1.x) {
        if (p1.y < p2.y) {
            slope = 999999999999.0f;
        } else  {
            slope = -999999999999.0f;
        }
    } else {
        slope = (f32)((p2.y - p1.y) / (p2.x - p1.x));
    }

    u32 *buffer = (u32 *)windowBuffer->memory;

    u32 colorU32 = (u32)(((u32)(color.alpha * 255) << 24) |
                         ((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );

    for (s32 x = p1.x; x < p2.x; ++x) {
        s32 y = (s32)(slope * (x - p1.x) + p1.y);
        if (((f32)y + 0.5f) < 0.0f) {
            buffer[(y + 1) * windowBuffer->dims.width + x] = colorU32;
        } else {
            buffer[y * windowBuffer->dims.width + x] = colorU32;
        }
    }
}

Intern void DrawRectangle(Window_Buffer *windowBuffer, f32x2 topLeft, f32x2 bottomRight, f32x4 color) {
    s32x2 top = S32x2(RoundF32ToS32(topLeft.x), RoundF32ToS32(topLeft.y));
    s32x2 bottom = S32x2(RoundF32ToS32(bottomRight.x), RoundF32ToS32(bottomRight.y));

    top = ClipBottom_s32x2(S32x2(0, 0), top);
    bottom = ClipTop_s32x2(windowBuffer->dims, bottom);

    //BBGGRRAA
    u32 colorU32 = (u32)(((u32)(color.alpha * 255) << 24) |
                         ((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );

    u8 *row = (u8 *)windowBuffer->memory + top.y * windowBuffer->pitch + top.x * windowBuffer->bytesPerPixel;

    for (s32 y = top.y; y < bottom.y; y++) {
        u32 *pixel = (u32 *)row;

        for (s32 x = top.x; x < bottom.x; x++) {
            *pixel++ = colorU32;
        }

        row += windowBuffer->pitch;
    }
}

Intern void ClearScreen(Window_Buffer *windowBuffer, f32x4 color) {
    f32x2 topLeft = {0.0f, 0.0f};
    f32x2 bottomRight = S32ToF32_x2(windowBuffer->dims);
    DrawRectangle(windowBuffer, topLeft, bottomRight, color);
}

Intern b8 ClipVertex(f32x4 vec) {
    b8 ret = (-vec.w < vec.x && vec.x < vec.w) &&
             (-vec.w < vec.y && vec.y < vec.w) &&
             (-vec.w < vec.z && vec.z < vec.w);
    return ret;
}

Intern f32x3 NDC(f32x4 vec) {
    f32x3 ret = F32x3(vec.x / vec.w, vec.y / vec.w, vec.z / vec.w);
    return ret;
}

Intern void TriPrimitive(Window_Buffer *windowBuffer, f32x3 a, f32x3 b, f32x3 c, f32x4x4 *MVP, f32x2 winDims, f32x4 color) {

    f32x4 pa = (*MVP) * ToF32x4(a, 1.0f);
    f32x4 pb = (*MVP) * ToF32x4(b, 1.0f);
    f32x4 pc = (*MVP) * ToF32x4(c, 1.0f);

    if (ClipVertex(pa) && ClipVertex(pb) && ClipVertex(pc)) {
        f32x3 an = NDC(pa);
        f32x3 bn = NDC(pb);
        f32x3 cn = NDC(pc);

        an = F32x3(ElemProd((0.5f * (F32x2(1.0f, 1.0f) + an.xy)), winDims), an.z);
        bn = F32x3(ElemProd((0.5f * (F32x2(1.0f, 1.0f) + bn.xy)), winDims), bn.z);
        cn = F32x3(ElemProd((0.5f * (F32x2(1.0f, 1.0f) + cn.xy)), winDims), cn.z);

        DrawTriangle(windowBuffer, an.xy, bn.xy, cn.xy, color);
    }
}

Intern void DrawCube(Window_Buffer *windowBuffer, f32x4x4 *MVP, f32x4 color) {
    f32x3 vertices[] = {
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f,  0.5f, -0.5f},
        {0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},

        {-0.5f, -0.5f,  0.5f},
        {0.5f, -0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f},

        {-0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},

        {0.5f,  0.5f,  0.5f},
        {0.5f,  0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f},

        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f,  0.5f},
        {0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f, -0.5f},

        {-0.5f,  0.5f, -0.5f},
        {0.5f,  0.5f, -0.5f},
        {0.5f,  0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f, -0.5f},
    };

    for (u32 i = 0; i < ArrayCount(vertices); i += 3) {
        TriPrimitive(windowBuffer, vertices[i+0], vertices[i+1], vertices[i+2], MVP, ToF32(windowBuffer->dims), color);
    }
}

#if 0
#define PROJECTION_NEAR 50.0f

Intern f32x2 MVP(f32x3 p, f32x4x4 *model, f32x4x4 *view, f32x2 windowDims, f32 WtoP) {
    f32x2 ret;

    f32x4 mv = (*view) * (*model) * ToF32x4(p, 1.0f);

    f32x2 pro = F32x2(mv.x * -(PROJECTION_NEAR / mv.z), mv.y * -(PROJECTION_NEAR / mv.z));

    f32 halfDimSx = windowDims.x * 0.5f;
    f32 halfDimSy = windowDims.y * 0.5f;

    ret = WtoP * pro + F32x2(halfDimSx, halfDimSy);

    return ret;
}

Intern f32x2 MVP2(f32x3 p, f32x4x4 *model, f32x4x4 *view, f32x2 windowDims, f32 WtoP) {
    f32x2 ret;

    f32x4 mv = (*view) * (*model) * ToF32x4(p, 1.0f);

    //f32x2 pro = F32x2(p.x * (PROJECTION_NEAR / p.z), p.y * (PROJECTION_NEAR / p.z));

    f32 halfDimSx = windowDims.x * 0.5f;
    f32 halfDimSy = windowDims.y * 0.5f;

    ret = WtoP * mv.xyz.xy + F32x2(halfDimSx, halfDimSy);

    return ret;
}
#endif

#endif //RC_SOFTWARE_RENDERER_H
