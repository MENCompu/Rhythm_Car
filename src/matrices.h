#ifndef JENH_MATRICES_H
#define JENH_MATRICES_H

typedef struct {
    f32x3 position;
    f32x3 rotation;
    f32x3 scale;
} Transform;

typedef _cache_align union {
    union {
        struct {
            f32x4 c1;
            f32x4 c2;
            f32x4 c3;
            f32x4 c4;
        };
        f32x4 C[4];
    };
    f32 E[4][4];
} f32x4x4;

typedef union {
    union {
        struct {
            f32x4 c1;
            f32x4 c2;
        };
        f32x4 C[2];
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

    //f32 nearPlaneWidth;
    //f32 nearPlaneHeight;
} Frustum;

#ifdef JENH_MATRICES_IMPL

#define MAT4_IDENTITY               \
    {F32x4(1.0f, 0.0f, 0.0f, 0.0f), \
     F32x4(0.0f, 1.0f, 0.0f, 0.0f), \
     F32x4(0.0f, 0.0f, 1.0f, 0.0f), \
     F32x4(0.0f, 0.0f, 0.0f, 1.0f)} \

Intern f32x4x4 Mat4Identity() {
    f32x4x4 ret = MAT4_IDENTITY;
    return ret;
}

Intern void F32x4x4(f32 c11, f32 c12, f32 c13, f32 c14,
                    f32 c21, f32 c22, f32 c23, f32 c24,
                    f32 c31, f32 c32, f32 c33, f32 c34,
                    f32 c41, f32 c42, f32 c43, f32 c44) {
}

// TODO(JENH): This should be the other way around.
Intern f32x4 operator*(f32x4 vec, f32x4x4 mat) {
    f32x4 ret;

    for (u32 i = 0; i < 4; ++i) {
        ret.E[i] = (vec.x * mat.C[i].x)
                 + (vec.y * mat.C[i].y)
                 + (vec.z * mat.C[i].z)
                 + (vec.w * mat.C[i].w);
    }

    return ret;
}

Intern f32x4 operator*(f32x4x4 mat, f32x4 vec) {
    f32x4 ret;

    for (u32 i = 0; i < 4; ++i) {
        ret.E[i] = (vec.x * mat.c1.E[i])
                 + (vec.y * mat.c2.E[i])
                 + (vec.z * mat.c3.E[i])
                 + (vec.w * mat.c4.E[i]);
    }

    return ret;
}

Intern f32x4x4 operator*(f32x4x4 mat1, f32x4x4 mat2) {
    f32x4x4 ret;

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

Intern void Translate(f32x4x4 *mat, f32x3 trans) {
    f32x4x4 transMat = {
        F32x4(1.0f, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, 1.0f, 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, 1.0f, 0.0f),
        F32x4(trans.x, trans.y, trans.z, 1.0f)
    };

    (*mat) = (*mat) * transMat;
}

Intern void Scale(f32x4x4 *mat, f32x3 scale) {
    f32x4x4 scaleMat = {
        F32x4(scale.x, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, scale.y, 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, scale.z, 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f)
    };

    (*mat) = (*mat) * scaleMat;
}

Intern void RotateY(f32x4x4 *mat, f32 angle) {
    f32x4x4 rotYMat = {
        F32x4(Cos32(angle), 0.0f, Sin32(angle), 0.0f),
        F32x4(0.0f, 1.0f, 0.0f, 0.0f),
        F32x4(-Sin32(angle), 0.0f, Cos32(angle), 0.0f),
        F32x4(0.0f, 0.0f, 0.0f, 1.0f)
    };

    (*mat) = (*mat) * rotYMat;
}

Intern void CreateFrustum(Frustum *frustum, f32 FOV, f32 whRatio, f32 nearPlane, f32 farPlane) {
    f32 halfHeight = nearPlane * Tan32(FOV / 2.0f);
    f32 halfWidth = halfHeight * whRatio;

    frustum->right = halfWidth;
    frustum->left = -halfWidth;
    frustum->top = halfHeight;
    frustum->bottom = -halfHeight;
    frustum->nearPlane = nearPlane;
    frustum->farPlane = farPlane;
}

// NOTE(JENH): Perspective matrix explanation: https://www.songho.ca/opengl/gl_projectionmatrix.html
Intern f32x4x4 Perspective(Frustum *frustum) {
    f32 r = frustum->right;
    f32 l = frustum->left;

    f32 t = frustum->top;
    f32 b = frustum->bottom;

    f32 n = frustum->nearPlane;
    f32 f = frustum->farPlane;

    f32x4x4 ret = {
        F32x4((2.0f * n) / (r - l), 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, (2.0f * n) / (t - b), 0.0f, 0.0f),
        F32x4((r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1.0f),
        F32x4(0.0f, 0.0f, (-2.0f * f * n) / (f - n), 0.0f)
    };

    return ret;
}

Intern f32x4x4 Orthographic(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
#if 0
    f32x4x4 ret = {
        F32x4((2.0f / (r - l)), 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, (2.0f / (t - b)), 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, (-2.0f / (f - n)), 0.0f),
        F32x4(-((r + l / (r - l))), -((t + b / (t - b))), -((f + n / (f - n))), 1.0f)
    };
#else
    f32x4x4 ret = {
        F32x4(1.0f / r, 0.0f, 0.0f, 0.0f),
        F32x4(0.0f, 1.0f / t, 0.0f, 0.0f),
        F32x4(0.0f, 0.0f, (-2.0f / (f - n)), 0.0f),
        F32x4(0.0f , 0.0f, -(((f + n) / (f - n))), 1.0f)
    };
#endif

    return ret;
}

#endif // JENH_MATRICES_IMPL

#endif // JENH_MATRICES_H
