#ifndef JENH_VECTORS_H
#define JENH_VECTORS_H

typedef enum {
    Axis_X,
    Axis_Y,
    Axis_Z,
} Axis;

#define VNF(type, count) P2(type, v, count)

#define VECTOR_LIST \
    Vec_X(u16, 2, Vec_Mem(x) Vec_Mem(y), \
        struct { \
            u16 x; \
            u16 y; \
        }; \
        struct { \
            u16 width; \
            u16 height; \
        }; \
    ) \
    Vec_X(s16, 2, Vec_Mem(x) Vec_Mem(y), \
        struct { \
            s16 x; \
            s16 y; \
        }; \
        struct { \
            s16 width; \
            s16 height; \
        }; \
    ) \
    Vec_X(s32, 2, Vec_Mem(x) Vec_Mem(y), \
        struct { \
            s32 x; \
            s32 y; \
        }; \
        struct { \
            s32 width; \
            s32 height; \
        }; \
    ) \
    Vec_X(u32, 2, Vec_Mem(x) Vec_Mem(y), \
        struct { \
            u32 x; \
            u32 y; \
        }; \
        struct { \
            u32 width; \
            u32 height; \
        }; \
    ) \
    Vec_X(f32, 2, Vec_Mem(x) Vec_Mem(y), \
        struct { \
            f32 x; \
            f32 y; \
        }; \
        struct { \
            f32 width; \
            f32 height; \
        }; \
    ) \
    Vec_X(s32, 3, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z), \
        struct { \
            s32 x; \
            s32 y; \
            s32 z; \
        }; \
        struct { \
            s32 r; \
            s32 g; \
            s32 b; \
        }; \
        struct { \
            s32 width; \
            s32 height; \
            s32 depth; \
        }; \
    ) \
    Vec_X(u32, 3, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z), \
        struct { \
            u32 x; \
            u32 y; \
            u32 z; \
        }; \
        struct { \
            u32 r; \
            u32 g; \
            u32 b; \
        }; \
        struct { \
            u32 width; \
            u32 height; \
            u32 depth; \
        }; \
    ) \
    Vec_X(f32, 3, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z), \
        struct { \
            f32 x; \
            f32 y; \
            f32 z; \
        }; \
        struct { \
            f32 r; \
            f32 g; \
            f32 b; \
        }; \
        struct { \
            f32 width; \
            f32 height; \
            f32 depth; \
        }; \
    ) \
    Vec_X(s32, 4, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z) Vec_Mem(w), \
        struct { \
            s32 x; \
            s32 y; \
            s32 z; \
            s32 w; \
        }; \
        struct { \
            s32 r; \
            s32 g; \
            s32 b; \
            s32 a; \
        }; \
        struct { \
            s32 width; \
            s32 height; \
            s32 depth; \
        }; \
    ) \
    Vec_X(u32, 4, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z) Vec_Mem(w), \
        struct { \
            s32 x; \
            s32 y; \
            s32 z; \
            s32 w; \
        }; \
        struct { \
            s32 r; \
            s32 g; \
            s32 b; \
            s32 a; \
        }; \
        struct { \
            s32 width; \
            s32 height; \
            s32 depth; \
        }; \
    ) \
    Vec_X(f32, 4, Vec_Mem(x) Vec_Mem(y) Vec_Mem(z) Vec_Mem(w), \
        struct { \
            f32 x; \
            f32 y; \
            f32 z; \
            f32 w; \
        }; \
        struct { \
            f32 r; \
            f32 g; \
            f32 b; \
            f32 a; \
        }; \
        struct { \
            f32 width; \
            f32 height; \
            f32 depth; \
        }; \
        f32v3 xyz; \
        f32v2 xy; \
        f32v3 rgb; \
        f32v2 rg; \
    ) \

#define Vec_X(type, count, mems, def) \
    typedef union VNF(type, count) { \
        def \
        type E[count]; \
    } VNF(type, count);
VECTOR_LIST
#undef Vec_X

#define Vec_X(type, count, mems, def) \
    Array(VNF(type, count)); \
    Slice(VNF(type, count));
VECTOR_LIST
#undef Vec_X

typedef struct {
    f32v3 o;
    struct {
        f32v3 x;
        f32v3 y;
        f32v3 z;
    } axes;
} Basis_3D;

typedef struct {
    f32v2 o;
    struct {
        f32v2 x;
        f32v2 y;
    } axes;
} Basis_2D;

#define Vec_Mem(mem) ret.mem = a.mem + b.mem;
#define Vec_X(type, count, mems, def) \
Public VNF(type, count) VNF(type, count)_Add(VNF(type, count) a, VNF(type, count) b) { \
    VNF(type, count) ret; mems return ret; \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem

#define Vec_Mem(mem) dst->mem += arg.mem;
#define Vec_X(type, count, mems, def) \
Public void VNF(type, count)_AddEq(VNF(type, count)* dst, VNF(type, count) arg) { \
    mems \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem

#if 0
#define Vec_Mem(mem) ret.mem = -arg.mem;
#define Vec_X(type, count, mems, def) \
Public VNF(type, count) VNF(type, count)_Neg(VNF(type, count) arg) { \
    VNF(type, count) ret; mems return ret; \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem
#endif

#define Vec_Mem(mem) ret.mem = a.mem - b.mem;
#define Vec_X(type, count, mems, def) \
Public VNF(type, count) VNF(type, count)_Sub(VNF(type, count) a, VNF(type, count) b) { \
    VNF(type, count) ret; mems return ret; \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem

#define Vec_Mem(mem) dst->mem -= arg.mem;
#define Vec_X(type, count, mems, def) \
Public void VNF(type, count)_SubEq(VNF(type, count)* dst, VNF(type, count) arg) { \
    mems \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem

#define Vec_Mem(mem) ret.mem = vec.mem * scalar;
#define Vec_X(type, count, mems, def) \
Public VNF(type, count) VNF(type, count)_Mul(VNF(type, count) vec, type scalar) { \
    VNF(type, count) ret; mems return ret; \
}
VECTOR_LIST
#undef Vec_X
#undef Vec_Mem

Public f32 f32v2_Dot(f32v2 a, f32v2 b) {
    return ( a.x * b.x + a.y * b.y );
}

Public f32v2 f32v2_ElemWise_Mul(f32v2 a, f32v2 b) {
    return (f32v2){ a.x * b.x, a.y * b.y };
}

Public f32v2 f32v2_ElemWise_Div(f32v2 a, f32v2 b) {
    return (f32v2){ a.x / b.x, a.y / b.y };
}

Public f32v3 f32v3_ElemWise_Mul(f32v3 a, f32v3 b) {
    return (f32v3){ a.x * b.x, a.y * b.y, a.z * b.z };
}

Public f32v3 f32v3_ElemWise_Div(f32v3 a, f32v3 b) {
    return (f32v3){ a.x / b.x, a.y / b.y, a.z / b.z };
}

#if 0
Public inline f32v2 CW_Perp(f32v2 vec) {
    return (f32v2){ vec.y, -vec.x };
}

Public inline f32v2 CCW_Perp(f32v2 vec) {
    return (f32v2){ -vec.y, vec.x };
}
#else
#define CW_Perp(vec) { (vec).y, -(vec).x }

#define CCW_Perp(vec) { -(vec).y, (vec).x }
#endif

Public inline f32 s32v2_Length(s32v2 vec) {
    return F32_Sqrt((f32)( (vec.x * vec.x) + (vec.y * vec.y) ));
}

Public inline s32v2 s32v2_Normalize(s32v2 vec) {
    return (s32v2){ (s32)( (f32)vec.x / s32v2_Length(vec) ), (s32)( (f32)vec.y / s32v2_Length(vec) ) };
}

Public inline f32 f32v2_Length(f32v2 vec) {
    return F32_Sqrt(F32_Sq(vec.x) + F32_Sq(vec.y));
}

Public inline f32 f32v3_Length(f32v3 vec) {
    return F32_Sqrt(F32_Sq(vec.x) + F32_Sq(vec.y) + F32_Sq(vec.z));
}

Public inline f32v3 f32v3_Normalize(f32v3 vec) {
    return f32v3_Mul(vec, 1.0f / f32v3_Length(vec));
}

Public f32v4 f32v4_ElemWise_Mul(f32v4 a, f32v4 b) {
    return (f32v4){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

Public f32v2 f32v2_Lerp(f32 t, f32v2 p0, f32v2 p1) {
    return f32v2_Add(f32v2_Mul(p0, 1.0f - t), f32v2_Mul(p1, t));
}

Public f32v2 s32v2_To_f32v2(s32v2 vec) {
    return (f32v2){ (f32)vec.x, (f32)vec.y };
}

Public u32v2 s32v2_To_u32v2(s32v2 vec) {
    return (u32v2){ (u32)vec.x, (u32)vec.y };
}

Public s32v2 f32v2_To_s32v2(f32v2 vec) {
    return (s32v2){ (s32)vec.x, (s32)vec.y };
}

Public u32v2 f32v2_To_u32v2(f32v2 vec) {
    return (u32v2){ (u32)vec.x, (u32)vec.y };
}

Public s32v2 s32v2_Neg(s32v2 vec) {
    return (s32v2){ -vec.x, -vec.y };
}

#if 0
Intern s32x2 operator*(s32x2 a, f32 b) {
    s32x2 ret;
    ret.x = (s32)((f32)a.x * b);
    ret.y = (s32)((f32)a.y * b);
    return ret;
}

Intern s32x2 operator*(f32 a, s32x2 b) {
    s32x2 ret;
    ret.x = (s32)(a * (f32)b.x);
    ret.y = (s32)(a * (f32)b.y);
    return ret;
}

Intern s32x2 &operator*=(s32x2 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V2 u32
//

Intern u32x2 U32x2(u32 a, u32 b) {
    u32x2 ret;
    ret.x = a;
    ret.y = b;
    return ret;
}

Intern u32x2 operator+(u32x2 a, u32x2 b) {
    u32x2 ret;
    ret.x = a.x + b.x;
    ret.y = a.y + b.y;
    return ret;
}

Intern u32x2 &operator+=(u32x2 &_this, u32x2 a) {
    _this = _this + a;
    return _this;
}

Intern s32x2 operator-(u32x2 a) {
    s32x2 ret;
    ret.x = -(s32)a.x;
    ret.y = -(s32)a.y;
    return ret;
}

Intern u32x2 operator-(u32x2 a, u32x2 b) {
    u32x2 ret;
    ret.x = a.x - b.x;
    ret.y = a.y - b.y;
    return ret;
}

Intern u32x2 &operator-=(u32x2 &_this, u32x2 a) {
    _this = _this - a;
    return _this;
}

Intern u32x2 operator*(u32x2 a, f32 b) {
    u32x2 ret;
    ret.x = (u32)((f32)a.x * b);
    ret.y = (u32)((f32)a.y * b);
    return ret;
}

Intern u32x2 operator*(f32 a, u32x2 b) {
    u32x2 ret;
    ret.x = (u32)(a * (f32)b.x);
    ret.y = (u32)(a * (f32)b.y);
    return ret;
}

Intern u32x2 &operator*=(u32x2 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V2 f32
//

Intern f32x2 F32x2(f32 a, f32 b) {
    f32x2 ret;
    ret.x = a;
    ret.y = b;
    return ret;
}

Intern f32x2 operator+(f32x2 a, f32x2 b) {
    f32x2 ret;
    ret.x = a.x + b.x;
    ret.y = a.y + b.y;
    return ret;
}

Intern f32x2 &operator+=(f32x2 &_this, f32x2 a) {
    _this = _this + a;
    return _this;
}

Intern f32x2 operator-(f32x2 a) {
    f32x2 ret;
    ret.x = -a.x;
    ret.y = -a.y;
    return ret;
}

Intern f32x2 operator-(f32x2 a, f32x2 b) {
    f32x2 ret;
    ret.x = a.x - b.x;
    ret.y = a.y - b.y;
    return ret;
}

Intern f32x2 &operator-=(f32x2 &_this, f32x2 a) {
    _this = _this - a;
    return _this;
}

Intern f32x2 operator*(f32x2 a, f32 b) {
    f32x2 ret;
    ret.x = a.x * b;
    ret.y = a.y * b;
    return ret;
}

Intern f32x2 operator*(f32 a, f32x2 b) {
    f32x2 ret;
    ret.x = a * b.x;
    ret.y = a * b.y;
    return ret;
}

Intern f32x2 &operator*=(f32x2 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V3
//

Intern f32x3 F32x3(f32 a, f32 b, f32 c) {
    f32x3 ret;
    ret.x = a;
    ret.y = b;
    ret.z = c;
    return ret;
}

Intern f32x3 F32x3(f32x2 xy, f32 z) {
    f32x3 ret;
    ret.x = xy.x;
    ret.y = xy.y;
    ret.z = z;
    return ret;
}

Intern f32x3 F32x3(f32 a) {
    f32x3 ret;
    ret.x = a;
    ret.y = a;
    ret.z = a;
    return ret;
}

Intern f32x3 operator+(f32x3 a, f32x3 b) {
    f32x3 ret;
    ret.x = a.x + b.x;
    ret.y = a.y + b.y;
    ret.z = a.z + b.z;
    return ret;
}

Intern f32x3 &operator+=(f32x3 &_this, f32x3 a) {
    _this = _this + a;
    return _this;
}

Intern f32x3 operator-(f32x3 a) {
    f32x3 ret;
    ret.x = -a.x;
    ret.y = -a.y;
    ret.z = -a.z;
    return ret;
}

Intern f32x3 operator-(f32x3 a, f32x3 b) {
    f32x3 ret;
    ret.x = a.x - b.x;
    ret.y = a.y - b.y;
    ret.z = a.z - b.z;
    return ret;
}

Intern f32x3 &operator-=(f32x3 &_this, f32x3 a) {
    _this = _this - a;
    return _this;
}

Intern f32x3 operator*(f32x3 a, f32 b) {
    f32x3 ret;
    ret.x = a.x * b;
    ret.y = a.y * b;
    ret.z = a.z * b;
    return ret;
}

Intern f32x3 operator*(f32 a, f32x3 b) {
    f32x3 ret;
    ret.x = a * b.x;
    ret.y = a * b.y;
    ret.z = a * b.z;
    return ret;
}

Intern f32x3 &operator*=(f32x3 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V4 F32
//

Intern f32x4 F32x4(f32 a, f32 b, f32 c, f32 d) {
    f32x4 ret;
    ret.x = a;
    ret.y = b;
    ret.z = c;
    ret.w = d;
    return ret;
}

Intern f32x4 F32x4(f32x3 xyz, f32 w) {
    f32x4 ret;
    ret.x = xyz.x;
    ret.y = xyz.y;
    ret.z = xyz.z;
    ret.w = w;
    return ret;
}

Intern f32x4 operator+(f32x4 a, f32x4 b) {
    f32x4 ret;
    ret.x = a.x + b.x;
    ret.y = a.y + b.y;
    ret.z = a.z + b.z;
    ret.w = a.w + b.w;
    return ret;
}

Intern f32x4 &operator+=(f32x4 &_this, f32x4 a) {
    _this = _this + a;
    return _this;
}

Intern f32x4 operator-(f32x4 a) {
    f32x4 ret;
    ret.x = -a.x;
    ret.y = -a.y;
    ret.z = -a.z;
    ret.w = -a.w;
    return ret;
}

Intern f32x4 operator-(f32x4 a, f32x4 b) {
    f32x4 ret;
    ret.x = a.x - b.x;
    ret.y = a.y - b.y;
    ret.z = a.z - b.z;
    ret.w = a.w - b.w;
    return ret;
}

Intern f32x4 &operator-=(f32x4 &_this, f32x4 a) {
    _this = _this - a;
    return _this;
}

Intern f32x4 operator*(f32x4 a, f32 b) {
    f32x4 ret;
    ret.x = a.x * b;
    ret.y = a.y * b;
    ret.z = a.z * b;
    ret.w = a.w * b;
    return ret;
}

Intern f32x4 operator*(f32 a, f32x4 b) {
    f32x4 ret;
    ret.x = a * b.x;
    ret.y = a * b.y;
    ret.z = a * b.z;
    ret.w = a * b.w;
    return ret;
}

Intern f32x4 &operator*=(f32x4 &_this, f32 a) {
    _this = _this * a;
    return _this;
}
// functions

#define ClipTop_f32x2(clip, val)    (F32x2(Min((clip).x, (val).x), Min((clip).y, (val).y)))
#define ClipBottom_f32x2(clip, val) (F32x2(Max((clip).x, (val).x), Max((clip).y, (val).y)))

#define ClipTop_s32x2(clip, val)    (S32x2(Min((clip).x, (val).x), Min((clip).y, (val).y)))
#define ClipBottom_s32x2(clip, val) (S32x2(Max((clip).x, (val).x), Max((clip).y, (val).y)))

inline f32x2 F32x2(s32x2 a) {
    f32x2 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    return ret;
}

inline f32x2 Perp_f32x2(f32x2 a) {
    f32x2 ret = { -a.y, a.x };
    return ret;
}

inline f32x2 Perp(f32x2 a) {
    f32x2 ret = { -a.y, a.x };
    return ret;
}

inline f32 Dot_f32x2(f32x2 a, f32x2 b) {
    f32 ret = { (a.x * b.x) + (a.y * b.y) };
    return ret;
}

inline f32 Dot(f32x2 a, f32x2 b) {
    f32 ret = { (a.x * b.x) + (a.y * b.y) };
    return ret;
}

inline f32 Dot(f32x3 a, f32x3 b) {
    f32 ret = { (a.x * b.x) + (a.y * b.y) + (a.z * b.z)};
    return ret;
}

inline f32x3 Cross_f32x3(f32x3 a, f32x3 b) {
    f32x3 ret = { (a.y * b.z) - (a.z * b.y),
                  (a.z * b.x) - (a.x * b.z),
                  (a.x * b.y) - (a.y * b.x) };
    return ret;
}

inline f32x3 Cross(f32x3 a, f32x3 b) {
    f32x3 ret = { (a.y * b.z) - (a.z * b.y),
                  (a.z * b.x) - (a.x * b.z),
                  (a.x * b.y) - (a.y * b.x) };
    return ret;
}

Intern inline s32x2 S32(f32x2 a) {
    s32x2 ret;
    ret.x = (s32)a.x;
    ret.y = (s32)a.y;
    return ret;
}

inline f32 Len(f32x3 a) {
    f32 len = Sqrt32(Sq32(a.x) + Sq32(a.y) + Sq32(a.z));
    return len;
}

inline f32 Len(f32x2 a) {
    f32 len = Sqrt32(Sq32(a.x) + Sq32(a.y));
    return len;
}

inline f32x3 Normalize(f32x3 a) {
    f32x3 ret = a * (1.0f / Len(a));
    return ret;
}

inline f32x2 Normalize(f32x2 a) {
    f32x2 ret = a * (1.0f / Len(a));
    return ret;
}

Public inline f32x3 BezierCurve3(f32 t, f32x3 p0, f32x3 p1, f32x3 p2, f32x3 p3) {
    f32x3 ret =       p0 +
             t  * (-3*p0 +3*p1) +
        Sq32(t) * (+3*p0 -6*p1 +3*p2) +
        Cb32(t) * (-1*p0 +3*p1 -3*p2 +1*p3);

    return ret;
}

Public inline f32x3 BezierCurve3Tan(f32 t, f32x3 p0, f32x3 p1, f32x3 p2, f32x3 p3) {
    f32x3 ret = p0 * ( -Sq32(t) + 2*t - 1) +
                p1 * (3*Sq32(t) - 4*t + 1) +
                p2 * (-3*Sq32(t) + 2*t) +
                p3 * (Sq32(t));

    return Normalize(ret);
}

inline f32x3 Normalize_f32x3(f32x3 a) {
    f32 len = Sqrt32(Sq32(a.x) + Sq32(a.y) + Sq32(a.z));
    f32x3 ret = a * (1.0f / len);
    return ret;
}

inline f32x4 ElemProd(f32x4 a, f32x4 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

inline f32x3 ElemProd(f32x3 a, f32x3 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline f32x2 ElemProd(f32x2 a, f32x2 b) {
    return {a.x * b.x, a.y * b.y};
}

inline f32x2 ElemDiv(f32x2 a, f32x2 b) {
    return {a.x / b.x, a.y / b.y};
}

inline f32x2 ElemProd_f32x2(f32x2 a, f32x2 b) {
    return {a.x * b.x, a.y * b.y};
}

inline f32x2 ElemDiv_f32x2(f32x2 a, f32x2 b) {
    return {a.x / b.x, a.y / b.y};
}

inline u32x2 ElemProd_u32x2(u32x2 a, u32x2 b) {
    return {a.x * b.x, a.y * b.y};
}

inline u32x2 ElemDiv_u32x2(u32x2 a, u32x2 b) {
    return {a.x / b.x, a.y / b.y};
}

inline f32x3 HadamardProd(f32x3 a, f32x3 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline f32x3 HadamardDiv(f32x3 a, f32x3 b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}

inline f32x4 HadamardProd(f32x4 a, f32x4 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

inline f32x4 HadamardDiv(f32x4 a, f32x4 b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

inline s32x2 RoundF32ToS32_x2(f32x2 a) {
    s32x2 ret = S32x2(RoundF32ToS32(a.x), RoundF32ToS32(a.y));
    return ret;
}

inline f32x2 ToF32(s32x2 a) {
    f32x2 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    return ret;
}

inline f32x2 S32ToF32_x2(s32x2 a) {
    f32x2 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    return ret;
}

inline f32x4 ToF32x4(f32x3 xyz, f32 w) {
    f32x4 ret;
    ret.x = xyz.x;
    ret.y = xyz.y;
    ret.z = xyz.z;
    ret.w = w;
    return ret;
}

inline s32x2 Tos32(f32x2 a) {
    s32x2 ret;
    ret.x = (s32)a.x;
    ret.y = (s32)a.y;
    return ret;
}

inline s32x2 Tos32(u32x2 a) {
    s32x2 ret;
    ret.x = (s32)a.x;
    ret.y = (s32)a.y;
    return ret;
}

inline s32x3 Tos32(f32x3 a) {
    s32x3 ret;
    ret.x = (s32)a.x;
    ret.y = (s32)a.y;
    ret.y = (s32)a.z;
    return ret;
}

inline u32x2 Tou32(s32x2 a) {
    u32x2 ret;
    ret.x = (u32)a.x;
    ret.y = (u32)a.y;
    return ret;
}

inline u32x2 Tou32(f32x2 a) {
    u32x2 ret;
    ret.x = (u32)a.x;
    ret.y = (u32)a.y;
    return ret;
}

inline f32x2 Tof32(s32x2 a) {
    f32x2 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    return ret;
}

inline f32x2 Tof32(u32x2 a) {
    f32x2 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    return ret;
}

inline f32x3 Tof32(s32x3 a) {
    f32x3 ret;
    ret.x = (f32)a.x;
    ret.y = (f32)a.y;
    ret.y = (f32)a.z;
    return ret;
}

//

// F32x2

Public inline f32x2 F32x2_Add(f32x2 a, f32x2 b) {
    return { a.x + b.x, a.y + b.y };
}

Public inline f32x2 F32x2_Sub(f32x2 a, f32x2 b) {
    return { a.x - b.x, a.y - b.y };
}

Public inline f32x2 F32x2_Neg(f32x2 a) {
    return { -a.x, -a.y };
}

Public inline void F32x2_Add_Equal(f32x2* a, f32x2 b) {
    a->x += b.x;
    a->y += b.y;
}

Public inline void F32x2_Sub_Equal(f32x2* a, f32x2 b) {
    a->x -= b.x;
    a->y -= b.y;
}

Public inline void F32x2_Set(f32x2* a, f32x2 b) {
    a->x = b.x;
    a->y = b.y;
}

Public inline f32x2 F32x2_Elem_Prod(f32x2 a, f32x2 b) {
    return {a.x * b.x, a.y * b.y};
}

// F32x3

Public inline f32x3 F32x3_Add(f32x3 a, f32x3 b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Public inline f32x3 F32x3_Sub(f32x3 a, f32x3 b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Public inline f32x3 F32x3_Neg(f32x3 a) {
    return { -a.x, -a.y, -a.z };
}

Public inline void F32x3_Add_Equal(f32x3* a, f32x3 b) {
    a->x += b.x;
    a->y += b.y;
    a->z += b.z;
}

Public inline void F32x3_Sub_Equal(f32x3* a, f32x3 b) {
    a->x -= b.x;
    a->y -= b.y;
    a->z -= b.z;
}

Public inline void F32x3_Set(f32x3* a, f32x3 b) {
    a->x = b.x;
    a->y = b.y;
    a->z = b.z;
}

Public inline b8 F32x3_Eq(f32x3 a, f32x3 b) {
    return ( a.x == b.x &&
             a.y == b.y &&
             a.z == b.z);
}

Public inline b8 F32x4_Eq(f32x4 a, f32x4 b) {
    return ( a.x == b.x &&
             a.y == b.y &&
             a.z == b.z &&
             a.w == b.w );
}

Public inline f32x3 F32x3_Elem_Prod(f32x3 a, f32x3 b) {
    return { a.x * b.x, a.y * b.y, a.z * b.z };
}

Public inline b8 F32_Eq_Epsilon(f32 a, f32 b, f32 epsilon) {
    return ( (b - epsilon) <= a && a <= (b + epsilon) );
}

Public inline f32x3 Intersection_Two_Lines(f32x3 a0, f32x3 a1, f32x3 b0, f32x3 b1) {
    f32 matrix[2][3] = {
        a1.x, -b1.x, (b0.x - a0.x),
        a1.z, -b1.z, (b0.z - a0.z),
    };

    for (u32 i = 0; i < 2; ++i) {
        f32 inv = 1 / matrix[i][i];

        for (u32 j = 0; j < 3; ++j) {
            matrix[i][j] *= inv;
        }

        for (u32 j = 0; j < 2; ++j) {
            if ( j == i ) { continue; }

            f32 inv2 = -matrix[j][i];
            for (u32 k = 0; k < 3; ++k) {
                matrix[j][k] += matrix[i][k] * inv2;
            }
        }
    }

    f32x3 interA = F32x3(matrix[0][2] * a1.x + a0.x, a0.y, matrix[0][2] * a1.z + a0.z);
    f32x3 interB = F32x3(matrix[1][2] * b1.x + b0.x, b0.y, matrix[1][2] * b1.z + b0.z);

    //Assert( F32_Eq_Epsilon(interA.x, interB.x, 0.1f) && F32_Eq_Epsilon(interA.z, interB.z, 0.1f) );

    return interA;
}

#endif
#endif // JENH_VECTORS_H
