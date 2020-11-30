#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "ppm.h"

/**
 * @file
 * @brief PPM fájl beolvasása
 */

/**
 * A kép tárolására használt 3 dimenziós tömb felszabadítása
 * @param[in] ***image felszabadítandó kép
 * @param[in] size_x a kép oszlopainak száma
 * @param[in] size_y a kép sorainak száma
 */
void freeimage(unsigned char ***image, int size_x, int size_y) {
    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            free(image[i][j]);
        }
        free(image[i]);
    }
    free(image);
}

/**
 * @brief A kép tárolására használt 3 dimenziós tömb lefoglalása
 * @param[in] size_x a kép oszlopainak száma
 * @param[in] size_y a kép sorainak száma
 * @param[out] image a létrehozott 3 dimenziós tömb
 */
unsigned char ***allocateimage(int size_x, int size_y) {
    unsigned char ***image;

    if ((image = (unsigned char***) malloc(size_y * sizeof(unsigned char**))) == NULL) {
        return NULL;
    }
    for (int i=0; i < size_y; i++)
        image[i] = NULL;
    for (int i = 0; i < size_y; i++) {
        if ((image[i] = (unsigned char**) malloc(size_x * sizeof(unsigned char*))) == NULL)
            return NULL;
    }

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            image[i][j] = NULL;
        }
    }

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            // azért calloc mert feketére kell állítani, ha nincs elég pixel a fájlban
            if ((image[i][j] = (unsigned char*) calloc(3, sizeof(unsigned char))) == NULL)
                return NULL;
        }
    }
    return image;
}

/**
 * @brief egy pixel értékeinek beállítása a kapott szó alapján
 *
 * A megkapott szót unsigned char-rá alakítja és beírja a képbe. Növeli az aktuális oszlop számát, és ha eléri a sorszélességet az aktuális sor számát is növeli. Az rgb értéket mindig növeli, de ha eléri a 3-at, átírja 0-ra. Ha az aktuális sor értéke megegyezik a kép sorainak számával az azt jelenti, hogy betelt a kép és megszakad a beolvasás.
 *
 * @param[in] word[] a szó amit pixellé kell átalakítani
 * @param[in] *image PPM_Image kép aminek az image_data részébe kell beírni a pixelt
 * @param[in] *iter_h a sor, amibe írni kell a pixelt
 * @param[in] *iter_w az oszlop, amibe írni kell a pixelt
 * @param[in] *rgb megadja, hogy ez melyik szín
 * @param[out] ongoing true ha még mindig fér a képbe, false ha betelt
 *
 * @see PPM_Image
 */
bool parsepixels(char word[], PPM_Image *image, int *iter_h, int *iter_w, int *rgb) {
    int temp;
    sscanf (word, "%d", &temp);
    // csak 8 bites képeket kezelünk. Mindent mást át kell alakítani.
    image->image_data[*iter_h][*iter_w][*rgb] = (unsigned char) temp*(255.0f/image->maxval);

    *rgb += 1;
    if (*rgb == 3) {
        *iter_w += 1;
        *rgb = 0;
    }
    if (*iter_w == image->size_x) {
        *iter_w = 0;
        *iter_h += 1;
    }
    if (*iter_h == image->size_y)
        return false;
    return true;
}

/**
 * @brief Eldönti, hogy mit kell kezdeni a kapott szóval.
 * Először azt nézi meg, hogy van-e magic. Ha nincs ezt kell beállítani. Majd sorban a sorszámot, oszlopszámot és a maxvalt. Ha megvan az oszlopszám, le lehet foglalni a kép tömbjét. Ezek után a maradék biztosan pixel érték lesz.
 *
 * @param[in] word[] a szó amit le kell ellenőrizni
 * @param[in] *image PPM_Image kép aminek az image_data részébe kell beírni a pixelt
 * @param[in] *iter_h a sor, amibe írni kell a pixelt
 * @param[in] *iter_w az oszlop, amibe írni kell a pixelt
 * @param[in] *rgb megadja, hogy ez melyik szín
 *
 * @see allocateimage
 * @see parsepixels
 */
void parseword(char word[], PPM_Image *image,  int *iter_h, int *iter_w, int *rgb) {

    if (image->magic[0] == '0') {
        strcpy(image->magic, word);
        return;
    }
    if (strcmp(image->magic, "P3") == 0) {
        //P3 filetype
        if (image->size_x == 0) {
            sscanf (word, "%d", &image->size_x);
            return;
        }
        if (image->size_y == 0) {
            sscanf (word, "%d", &image->size_y);
            image->image_data = allocateimage(image->size_x, image->size_y);
            return;
        }
        if (image->maxval == 0) {
            sscanf (word, "%d", &image->maxval);
            return;
        }
        if (!parsepixels(word, image, iter_h, iter_w, rgb)) {
            return;
        }
    }
}

/**
 * @brief Eldönti, hogy a megadott sor mit tartalmaz és mit kell vele csinálni
 * Ha comment akkor kimarad, egyébként szóközönként fel kell bontani. Ezek lesznek a szavak, amiket utána tovább kell kategorizálni.
 * @param[in] line[] a sor amit vizsgálni kell
 * @param[in] *image PPM_Image kép aminek az image_data részébe kell beírni a pixelt
 * @param[in] *iter_h a sor, amibe írni kell a pixelt
 * @param[in] *iter_w az oszlop, amibe írni kell a pixelt
 * @param[in] *rgb megadja, hogy ez melyik szín
 *
 * @see parseword
 */

void checktype(char line[], PPM_Image *image, int *iter_h, int *iter_w, int *rgb) {
    if (line[0] == '#')
        // comment
        return;
    char *token;

    token = strtok(line, " ");

    while( token != NULL ) {
        if (strcmp(token, "#") == 0 || strstr(token, "#") != NULL)
            // sorközi komment
            return;
        parseword(token, image, iter_h, iter_w, rgb);
        token = strtok(NULL, " ");
   }
}

/**
 * @brief beolvas egy képet
 * Megnyitja a fájlt, majd létrehozza a PPM_Image struktúra egy példányát és beállítja a kezdő értékeit. Ezek után maximum 4096 karakter hosszú soronként beolvassa a fájlt és átadja a checktype -nak. Végül bezárja a fájlt.
 * @param[in] filename[] ezt a fájlt fogja megnyitni
 *
 * @see checktype
 * @see PPM_Image
 */

PPM_Image PPM_Parser(char filename[]) {
    FILE *fp;
    fp = fopen(filename, "r");

    PPM_Image image;

    if (fp == NULL) {
        perror("error reading file");
        abort();
        return image;
    }

    strcpy(image.magic, "00");
    image.size_x = 0;
    image.size_y = 0;
    image.maxval = 0;
    int iter_h = 0;
    int iter_w = 0;
    int rgb = 0;
    char line[4096]; /* a specifikáció szerint maximum 70 karakter lehetne egy sorban, de az ImageMagick többet tesz bele, ezért maximum 4096 lesz a mérete */

    while((fscanf(fp, "%[^\n]", line)) != EOF) {
        fgetc(fp);
        checktype(line, &image, &iter_h, &iter_w, &rgb);
    }


    //printf("magic: %s\n", image.magic);
    printf("Szélesség: %d\n", image.size_x);
    printf("Magasság: %d\n", image.size_y);
    //printf("maxval: %u\n", image.maxval);

    fclose(fp);
    return image;
}

/**
 * @brief Fájlba írja a PPM_Image tartalmát
 * A megnyitott fájlba először kiírjuk sorrendben a magic-et az oszlopok számát, a sorok számát, a maxvalt. Ezek után a pixelek adatait írjuk ki, egy sorba egy pixelt, tehát három számot. Végül bezárjuk a fájlt.
 * @param[in] filename[] a kimenti fájl neve
 * @param[in] *image a kiírandó kép
 */
void PPM_Writer(char filename[], PPM_Image *image) {
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("error reading file");
        abort();
    }

    fprintf(fp, "%s\n", image->magic);
    fprintf(fp, "%d %d\n", image->size_x, image->size_y);
    fprintf(fp, "%d\n", image->maxval);

    for (int line = 0; line < image->size_y; line++) {
        for (int col = 0; col < image->size_x; col++) {
            for (int color = 0; color < 3; color++) {
                fprintf(fp, "%d ", (image->image_data[line][col][color]));
            }
            fprintf(fp, "\n");
        }
    }
    fclose(fp);
}
