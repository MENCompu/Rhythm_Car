#ifndef JENH_HID_H
#define JENH_HID_H

#pragma pack(push, 1)
typedef struct {
    u8 pad; // ???

    s16 LSX;
    u16 LSY;

    u16 RSX;
    u16 RSY;

    u8  LT;
    u8  RT;

    u8  A : 1;
    u8  B : 1;
    u8  X : 1;
    u8  Y : 1;

    u8  LB : 1;
    u8  RB : 1;

    u8  back  : 1;
    u8  start : 1;

    u8  LSB : 1;
    u8  RSB : 1;

    u8  DPad : 4;
} Xbox_360_Controller;
#pragma pack(pop)

#endif // JENH_HID_H
