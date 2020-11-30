#ifndef PPM
#define PPM

#include <stdbool.h>

/**
 * @brief a PPM fájl tárolására használt struktúra
 */
typedef struct PPM_Image {
    unsigned char ***image_data; /**< 3 dimenziós tömb amiben a kép pixeleinek az értékeit tároljuk */
    int size_x; /**< a kép oszlopainak száma */
    int size_y; /**< a kép sorainak száma */
    char magic[2+1]; /**< a kép két karakterből álló magic-je */
    int maxval; /**< a kép maxval-ja */
} PPM_Image;

unsigned char getpixelcolor(unsigned char *image, int x, int y, int z, int size_x);
void setpixelcolor(unsigned char *image, int x, int y, int z, int size_x, unsigned char value);
unsigned char *allocateimage1d(int size_x, int size_y);
void freeimage(unsigned char ***image, int size_x, int size_y);
unsigned char ***allocateimage(int size_x, int size_y);
bool parsepixels(char word[], PPM_Image *image, int *iter_h, int *iter_w, int *rgb);
void parseword(char word[], PPM_Image *image,  int *iter_h, int *iter_w, int *rgb);
void checktype(char line[], PPM_Image *image, int *iter_h, int *iter_w, int *rgb);
PPM_Image PPM_Parser(char filename[]);
void PPM_Writer(char filename[], PPM_Image *image);

#endif
