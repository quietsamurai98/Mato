#ifndef CEXT_TYPES_H
#define CEXT_TYPES_H
typedef unsigned int  Uint32;
typedef unsigned char byte;
typedef union {
    Uint32 m_color;
    struct {
        byte b: 8;
        byte g: 8;
        byte r: 8;
        byte a: 8;
    }      ch;
}                     Color;
#endif //CEXT_TYPES_H
