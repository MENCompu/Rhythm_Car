#ifndef JENH_MATRICES_H
#define JENH_MATRICES_H

typedef struct {
    f32v3 position;
    f32v3 rotation;
    f32v3 scale;
} Transform;

typedef union {
    union {
        struct {
            f32v4 c1;
            f32v4 c2;
            f32v4 c3;
            f32v4 c4;
        };
        f32v4 C[4];
    };
    f32 E[4][4];
} f32m4x4;

typedef union {
    union {
        struct {
            f32v4 c1;
            f32v4 c2;
        };
        f32v4 C[2];
    };
    f32 E[3][2];
} f32x2x2;

typedef struct {
    f32 right;
    f32 left;
    f32 top;
    f32 bottom;
    f32 nearPlane;
    f32 farPlane;
} Frustum;

#define f32m4x4_IDENTITY (f32m4x4){ \
        .c1 = { 1.0f, 0.0f, 0.0f, 0.0f }, \
        .c2 = { 0.0f, 1.0f, 0.0f, 0.0f }, \
        .c3 = { 0.0f, 0.0f, 1.0f, 0.0f }, \
        .c4 = { 0.0f, 0.0f, 0.0f, 1.0f }  \
    }

// TODO(JENH): This should be the other way around.
Public f32v4 f32m4x4_Mul_Vec(f32m4x4 mat, f32v4 vec) {
    f32v4 ret;

    for (u32 i = 0; i < 4; ++i) {
        ret.E[i] = (vec.x * mat.c1.E[i])
                 + (vec.y * mat.c2.E[i])
                 + (vec.z * mat.c3.E[i])
                 + (vec.w * mat.c4.E[i]);
    }

    return ret;
}

Public f32m4x4 f32m4x4_Mul(f32m4x4 mat1, f32m4x4 mat2) {
    f32m4x4 ret;

    for (u32 i = 0; i < 4; ++i) {
        for (u32 j = 0; j < 4; ++j) {
            ret.E[i][j] = (mat1.E[0][j] * mat2.E[i][0])
                        + (mat1.E[1][j] * mat2.E[i][1])
                        + (mat1.E[2][j] * mat2.E[i][2])
                        + (mat1.E[3][j] * mat2.E[i][3]);
        }
    }

    return ret;
}

Public void f32m4x4_Translate(f32m4x4 *mat, f32v3 trans) {
    f32m4x4 transMat = {
        .c1 = { 1.0f, 0.0f, 0.0f, 0.0f },
        .c2 = { 0.0f, 1.0f, 0.0f, 0.0f },
        .c3 = { 0.0f, 0.0f, 1.0f, 0.0f },
        .c4 = { trans.x, trans.y, trans.z, 1.0f },
    };

    (*mat) = f32m4x4_Mul(*mat, transMat);
}

Public void f32m4x4_Scale(f32m4x4 *mat, f32v3 scale) {
    f32m4x4 scaleMat = {
        .c1 = { scale.x, 0.0f, 0.0f, 0.0f },
        .c2 = { 0.0f, scale.y, 0.0f, 0.0f },
        .c3 = { 0.0f, 0.0f, scale.z, 0.0f },
        .c4 = { 0.0f, 0.0f, 0.0f, 1.0f },
    };

    (*mat) = f32m4x4_Mul(*mat, scaleMat);
}

Public void RotateY(f32m4x4 *mat, f32 angle) {
    f32m4x4 rotYMat = {
        .c1 = { F32_Cos(angle), 0.0f, F32_Sin(angle), 0.0f },
        .c2 = { 0.0f, 1.0f, 0.0f, 0.0f },
        .c3 = { -F32_Sin(angle), 0.0f, F32_Cos(angle), 0.0f },
        .c4 = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    (*mat) = f32m4x4_Mul(*mat, rotYMat);
}
Public void Frustum_Create(Frustum* frustum, f32 FOV, f32 whRatio, f32 nearPlane, f32 farPlane) {
    f32 halfHeight = nearPlane * F32_Tan(FOV / 2.0f);
    f32 halfWidth = halfHeight * whRatio;

    frustum->right = halfWidth;
    frustum->left = -halfWidth;
    frustum->top = halfHeight;
    frustum->bottom = -halfHeight;
    frustum->nearPlane = nearPlane;
    frustum->farPlane = farPlane;
}

// NOTE(JENH): Perspective matrix explanation: https://www.songho.ca/opengl/gl_projectionmatrix.html
Public f32m4x4 Perspective(Frustum* frustum) {
    f32 r = frustum->right;
    f32 l = frustum->left;

    f32 t = frustum->top;
    f32 b = frustum->bottom;

    f32 n = frustum->nearPlane;
    f32 f = frustum->farPlane;

    f32m4x4 ret = {
        .c1 = { (2.0f * n) / (r - l), 0.0f, 0.0f, 0.0f },
        .c2 = { 0.0f, (2.0f * n) / (t - b), 0.0f, 0.0f },
        .c3 = { (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1.0f },
        .c4 = { 0.0f, 0.0f, (-2.0f * f * n) / (f - n), 0.0f },
    };

    return ret;
}

#define Symmetric_Orthographic(h, v, n, f) Orthographic(-h, h, -v, v, n, f)
Public f32m4x4 Orthographic(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    f32m4x4 ret = {
        .c1 = { 1.0f / r, 0.0f, 0.0f, 0.0f },
        .c2 = { 0.0f, 1.0f / t, 0.0f, 0.0f },
        .c3 = { 0.0f, 0.0f, (-2.0f / (f - n)), 0.0f },
        .c4 = { 0.0f, 0.0f, -(((f + n) / (f - n))), 1.0f },
    };

    return ret;
}

#endif // JENH_MATRICES_H
