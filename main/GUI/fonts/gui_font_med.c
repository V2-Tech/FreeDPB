/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#define LGFX_USE_V1
#include "../../components/lvgl/lvgl.h"

#ifndef FUTURASTDCONDENSEDLIGHT
#define FUTURASTDCONDENSEDLIGHT 1
#endif

#if FUTURASTDCONDENSEDLIGHT

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0x10,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x14, 0x28, 0x51, 0x47, 0xe5, 0xa, 0x7e, 0x50,
    0xa1, 0x45, 0x0,

    /* U+0024 "$" */
    0x21, 0x1d, 0x8, 0x43, 0xc, 0x30, 0x42, 0x11,
    0xb8, 0x80,

    /* U+0025 "%" */
    0xe2, 0xa4, 0xa4, 0xa8, 0xa8, 0xf0, 0x16, 0x29,
    0x29, 0x29, 0x49, 0x46,

    /* U+0026 "&" */
    0x30, 0x91, 0x22, 0x43, 0x4, 0x14, 0x48, 0x89,
    0x1b, 0x33, 0xa0,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x29, 0x29, 0x24, 0x91, 0x22,

    /* U+0029 ")" */
    0x89, 0x22, 0x49, 0x25, 0x28,

    /* U+002A "*" */
    0x20, 0x8f, 0x94, 0x90,

    /* U+002B "+" */
    0x10, 0x20, 0x40, 0x8f, 0xe2, 0x4, 0x8,

    /* U+002C "," */
    0x49, 0x0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x1, 0x2, 0x2, 0x4, 0x4, 0x8, 0x8, 0x8,
    0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,

    /* U+0030 "0" */
    0x72, 0xa3, 0x18, 0xc6, 0x31, 0x8c, 0x54, 0xe0,

    /* U+0031 "1" */
    0x64, 0x92, 0x49, 0x24, 0x90,

    /* U+0032 "2" */
    0x74, 0x62, 0x10, 0x88, 0x44, 0x22, 0x11, 0xf0,

    /* U+0033 "3" */
    0x74, 0x62, 0x10, 0xb8, 0x61, 0xc, 0x62, 0xe0,

    /* U+0034 "4" */
    0x11, 0x8c, 0xa5, 0x2a, 0x52, 0xf8, 0x84, 0x20,

    /* U+0035 "5" */
    0x7a, 0x10, 0x84, 0x38, 0x61, 0x8, 0x67, 0xe0,

    /* U+0036 "6" */
    0x11, 0x8, 0x84, 0x3a, 0x31, 0x8c, 0x72, 0xe0,

    /* U+0037 "7" */
    0xf8, 0x84, 0x22, 0x10, 0x8c, 0x42, 0x31, 0x0,

    /* U+0038 "8" */
    0x74, 0x63, 0x18, 0xbb, 0x31, 0x8c, 0x76, 0xe0,

    /* U+0039 "9" */
    0x74, 0xe3, 0x18, 0xcd, 0xc2, 0x11, 0x8, 0x80,

    /* U+003A ":" */
    0x81,

    /* U+003B ";" */
    0x40, 0x5, 0x60,

    /* U+003C "<" */
    0x2, 0x18, 0xc6, 0xc, 0x4, 0x6, 0x3,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0x80, 0x80, 0xc0, 0x60, 0x43, 0x18, 0x40,

    /* U+003F "?" */
    0x69, 0x91, 0x16, 0x44, 0x40, 0x4,

    /* U+0040 "@" */
    0xf, 0x83, 0x4, 0x67, 0x24, 0xc9, 0x88, 0x99,
    0x11, 0x91, 0x19, 0x12, 0x51, 0x66, 0xff, 0x30,
    0x60, 0xf8,

    /* U+0041 "A" */
    0x30, 0xc3, 0xc, 0x51, 0x64, 0x92, 0x4b, 0xec,
    0xe1,

    /* U+0042 "B" */
    0xe4, 0xa5, 0x29, 0x4b, 0x93, 0x8c, 0x63, 0xe0,

    /* U+0043 "C" */
    0x3a, 0x21, 0x8, 0x42, 0x10, 0x84, 0x10, 0x70,

    /* U+0044 "D" */
    0xf4, 0xa3, 0x18, 0xc6, 0x31, 0x8c, 0x65, 0xc0,

    /* U+0045 "E" */
    0xf8, 0x88, 0x8f, 0x88, 0x88, 0x8f,

    /* U+0046 "F" */
    0xf8, 0x88, 0x8f, 0x88, 0x88, 0x88,

    /* U+0047 "G" */
    0x72, 0x61, 0x8, 0x42, 0xf1, 0x8c, 0x72, 0xe0,

    /* U+0048 "H" */
    0x8c, 0x63, 0x18, 0xfe, 0x31, 0x8c, 0x63, 0x10,

    /* U+0049 "I" */
    0xff, 0xf0,

    /* U+004A "J" */
    0x24, 0x92, 0x49, 0x24, 0xf0,

    /* U+004B "K" */
    0x94, 0xa9, 0x4e, 0x63, 0x14, 0xa5, 0xa5, 0x20,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0x88, 0x88, 0x8f,

    /* U+004D "M" */
    0x62, 0x66, 0x66, 0x66, 0x66, 0x66, 0x69, 0xd9,
    0x99, 0x99, 0x99, 0x91,

    /* U+004E "N" */
    0x8e, 0x73, 0x9c, 0xd6, 0xb5, 0x9c, 0xe7, 0x10,

    /* U+004F "O" */
    0x39, 0x28, 0x61, 0x86, 0x18, 0x61, 0x86, 0x14,
    0x9e,

    /* U+0050 "P" */
    0xe9, 0x99, 0x9e, 0x88, 0x88, 0x88,

    /* U+0051 "Q" */
    0x79, 0x28, 0x61, 0x86, 0x18, 0x61, 0x86, 0x54,
    0x8f, 0x4,

    /* U+0052 "R" */
    0xe9, 0x99, 0x9e, 0xca, 0xaa, 0x99,

    /* U+0053 "S" */
    0x76, 0x21, 0xc, 0x30, 0xc3, 0x8, 0x67, 0xe0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x84, 0x21, 0x8, 0x40,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x62, 0xe0,

    /* U+0056 "V" */
    0x87, 0x14, 0xd2, 0x49, 0x26, 0x8a, 0x30, 0xc3,
    0x4,

    /* U+0057 "W" */
    0x91, 0x99, 0x99, 0x99, 0x99, 0x9a, 0x6a, 0x6a,
    0x66, 0x66, 0x66, 0x64,

    /* U+0058 "X" */
    0x45, 0x34, 0x8a, 0x30, 0xc3, 0xc, 0x29, 0x24,
    0xd1,

    /* U+0059 "Y" */
    0x8c, 0x54, 0xa5, 0x30, 0x84, 0x21, 0x8, 0x40,

    /* U+005A "Z" */
    0x78, 0xc4, 0x21, 0x10, 0x84, 0x42, 0x11, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+005C "\\" */
    0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10,
    0x8, 0x8, 0x4, 0x4, 0x4, 0x2, 0x2, 0x1,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x21, 0x14, 0xa5, 0x46, 0x20,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x24,

    /* U+0061 "a" */
    0x79, 0x99, 0x99, 0x97,

    /* U+0062 "b" */
    0x88, 0x88, 0x8e, 0x99, 0x99, 0x99, 0xe0,

    /* U+0063 "c" */
    0x79, 0x88, 0x88, 0xd7,

    /* U+0064 "d" */
    0x11, 0x11, 0x17, 0x99, 0x99, 0x99, 0x70,

    /* U+0065 "e" */
    0x69, 0x9f, 0x89, 0x96,

    /* U+0066 "f" */
    0x69, 0x2e, 0x92, 0x49, 0x20,

    /* U+0067 "g" */
    0x79, 0x99, 0x99, 0x97, 0x19, 0x70,

    /* U+0068 "h" */
    0x88, 0x88, 0x8f, 0x99, 0x99, 0x99, 0x90,

    /* U+0069 "i" */
    0x9f, 0xe0,

    /* U+006A "j" */
    0x9f, 0xfc,

    /* U+006B "k" */
    0x84, 0x21, 0x8, 0x4a, 0x9c, 0xc6, 0x29, 0x29,
    0x80,

    /* U+006C "l" */
    0xff, 0xf8,

    /* U+006D "m" */
    0xef, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9,

    /* U+006E "n" */
    0xf9, 0x99, 0x99, 0x99,

    /* U+006F "o" */
    0x76, 0xe3, 0x18, 0xc7, 0x6e,

    /* U+0070 "p" */
    0xe9, 0x99, 0x99, 0x9e, 0x88, 0x80,

    /* U+0071 "q" */
    0x79, 0x99, 0x99, 0x97, 0x11, 0x10,

    /* U+0072 "r" */
    0xea, 0xaa,

    /* U+0073 "s" */
    0x79, 0x84, 0x21, 0x1e,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x92, 0x48,

    /* U+0075 "u" */
    0x99, 0x99, 0x99, 0x96,

    /* U+0076 "v" */
    0x8e, 0x54, 0xa5, 0x28, 0x84,

    /* U+0077 "w" */
    0x99, 0xd9, 0x5a, 0x5a, 0x6a, 0x6a, 0x24, 0x24,

    /* U+0078 "x" */
    0x4a, 0x94, 0x43, 0x29, 0x59,

    /* U+0079 "y" */
    0x8e, 0x56, 0xa5, 0x28, 0xc4, 0x21, 0x10,

    /* U+007A "z" */
    0x78, 0x84, 0x42, 0x21, 0x1f,

    /* U+007B "{" */
    0x69, 0x24, 0xa2, 0x49, 0x26,

    /* U+007C "|" */
    0xff, 0xff,

    /* U+007D "}" */
    0xc9, 0x24, 0x8a, 0x49, 0x2c,

    /* U+007E "~" */
    0x73, 0x18
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 53, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 88, .box_w = 1, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 79, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 5, .adv_w = 106, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 106, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 133, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 135, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 57, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 50, .adv_w = 77, .box_w = 3, .box_h = 13, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 55, .adv_w = 77, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 60, .adv_w = 106, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 64, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 53, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 73, .adv_w = 40, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 74, .adv_w = 53, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 119, .box_w = 8, .box_h = 16, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 91, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 106, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 53, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 53, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 172, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 114, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 183, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 205, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 93, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 102, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 94, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 105, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 87, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 88, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 105, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 111, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 49, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 67, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 94, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 75, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 111, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 114, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 92, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 114, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 341, .adv_w = 98, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 100, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 80, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 110, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 100, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 155, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 103, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 100, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 86, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 77, .box_w = 3, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 423, .adv_w = 57, .box_w = 8, .box_h = 16, .ofs_x = -2, .ofs_y = -3},
    {.bitmap_index = 439, .adv_w = 77, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 445, .adv_w = 114, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 450, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 451, .adv_w = 72, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 452, .adv_w = 89, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 89, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 71, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 467, .adv_w = 89, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 88, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 53, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 91, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 489, .adv_w = 91, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 40, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 40, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 500, .adv_w = 90, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 40, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 143, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 91, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 91, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 89, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 533, .adv_w = 89, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 539, .adv_w = 52, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 71, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 54, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 90, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 81, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 129, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 566, .adv_w = 86, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 571, .adv_w = 85, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 578, .adv_w = 85, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 77, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 588, .adv_w = 57, .box_w = 1, .box_h = 16, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 590, .adv_w = 77, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 595, .adv_w = 114, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    34, 53,
    34, 55,
    34, 56,
    34, 58,
    34, 87,
    34, 88,
    34, 90,
    39, 13,
    39, 15,
    39, 34,
    45, 53,
    45, 55,
    45, 56,
    45, 58,
    45, 90,
    49, 13,
    49, 15,
    49, 34,
    51, 53,
    51, 55,
    51, 56,
    51, 58,
    53, 13,
    53, 14,
    53, 15,
    53, 27,
    53, 28,
    53, 34,
    53, 66,
    53, 68,
    53, 70,
    53, 80,
    53, 83,
    53, 84,
    53, 86,
    53, 88,
    53, 90,
    55, 13,
    55, 14,
    55, 15,
    55, 27,
    55, 28,
    55, 34,
    55, 66,
    55, 70,
    55, 74,
    55, 80,
    55, 83,
    55, 86,
    56, 13,
    56, 14,
    56, 15,
    56, 27,
    56, 28,
    56, 34,
    56, 66,
    56, 70,
    56, 74,
    56, 80,
    56, 83,
    56, 86,
    56, 90,
    58, 13,
    58, 14,
    58, 15,
    58, 27,
    58, 28,
    58, 34,
    58, 66,
    58, 70,
    58, 80,
    58, 81,
    58, 82,
    58, 86,
    58, 87,
    83, 13,
    83, 15,
    83, 71,
    83, 73,
    83, 78,
    83, 79,
    83, 83,
    83, 85,
    83, 86,
    83, 87,
    83, 88,
    83, 90,
    83, 91,
    87, 13,
    87, 15,
    88, 13,
    88, 15,
    90, 13,
    90, 15
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -9, -8, -8, -14, -3, -5, -3, -28,
    -28, -9, -9, -9, -9, -13, -5, -33,
    -33, -9, -5, -2, -2, -5, -15, -8,
    -15, -15, -15, -9, -13, -13, -13, -13,
    -15, -13, -15, -13, -13, -20, -2, -20,
    -5, -5, -8, -8, -8, 2, -8, -5,
    -5, -19, -2, -19, -3, -3, -8, -6,
    -6, 2, -6, -3, -3, -2, -20, -19,
    -20, -9, -9, -14, -14, -17, -14, -9,
    -14, -9, -5, -11, -11, 2, 2, 2,
    2, 2, 2, 2, 5, 5, 5, 2,
    -14, -14, -9, -9, -14, -14
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 94,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t gui_font_med = {
#else
lv_font_t gui_font_med = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FUTURASTDCONDENSEDLIGHT*/

