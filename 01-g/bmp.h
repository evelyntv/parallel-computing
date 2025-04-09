#ifndef BMP
#define BMP

#include <stdint.h>

struct bmp_id {
    uint_t magic1;
    uint_t magic2;
}

struct bmp_header {
    uint32_t file_size;
    uint16_t 
}