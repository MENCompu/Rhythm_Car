#if 0
Public inline void Rect_From_Center_Scale(f32x3 inCenter, f32x3 inScale, Coll_Rect_3D* outRect) {
    outRect->min = inCenter - inScale;
    outRect->max = inCenter + inScale;
}
#endif

Public inline b8 Coll_2D_Is_Intersecting_Rect_Point(Coll_Rect_2D* inRect, f32x2 inPoint) {
    return ( inRect->min.x <= inPoint.x && inPoint.x <= inRect->max.x &&
             inRect->min.y <= inPoint.y && inPoint.y <= inRect->max.y );
}

Public inline b8 Coll_3D_Is_Intersecting_Rect_Point(Coll_Rect_3D* inRect, f32x3 inPoint) {
    return ( inRect->min.x <= inPoint.x && inPoint.x <= inRect->max.x &&
             inRect->min.y <= inPoint.y && inPoint.y <= inRect->max.y &&
             inRect->min.z <= inPoint.z && inPoint.z <= inRect->max.z );
}

Public inline void Coll_Rect_2D_Create(f32x2 inCenter, f32x2 inHalfDims, Coll_Rect_2D* outRect) {
    outRect->min = F32x2_Sub(inCenter, inHalfDims);
    outRect->max = F32x2_Add(inCenter, inHalfDims);
}

Public inline void Coll_Rect_3D_Create(f32x3 inCenter, f32x3 inHalfDims, Coll_Rect_3D* outRect) {
    outRect->min = F32x3_Sub(inCenter, inHalfDims);
    outRect->max = F32x3_Add(inCenter, inHalfDims);
}
