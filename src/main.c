#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include "ppm.h"
#include "imagefunc.h"

/**
 * @file
 * @brief a fő fájl, ami a parancssort kezeli
 * @details debugmalloc helyett a Valgrindot használtam a tesztelésre és nem jelzett hibát
*/

int main (int argc, char *argv[]) {

    typedef struct CmdOptions {
        int lightness;
        int contrast;
        bool grayscale;
        int hue_shift;
        double sinecolor_shft;
        bool invert;
        mirror_type mirror;
        RGB_SHIFT rgbshft;
        pixelsort_preset ps_preset;
        int blur;
        int sharpen;
        bool edge;
        bool corrupt;
        bool a3d;
    } CmdOptions;

    CmdOptions options = {0, 0, false, 0, 0, false, none, {0,0, 0,0, 0,0}, psnone, 0, 0, false, false, false};

    char *inn_fname;
    char *outt_fname;

    int c;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",   no_argument,  0,  'h' },
            {"input",   required_argument,  0,  'i' },
            {"output",  required_argument,  0,  'o' },
            {"lightness",  required_argument,  0,  0 },
            {"contrast",  required_argument,  0,  1 },
            {"grayscale",   no_argument,        0,   2  },
            {"hue-shift",  required_argument,  0,  3 },
            {"sinecolor-shift",  required_argument,  0,  4 },
            {"invert",      no_argument,        0,   5  },
            {"mirror",  required_argument,  0,  6 },
            {"rgb-shift",  required_argument,  0,  7 },
            {"pixelsort",  required_argument,  0,  8 },
            {"blur",  required_argument,  0,  9 },
            {"sharpen",  required_argument,  0,  10 },
            {"edge-detect",  no_argument,  0,  11 },
            {"corrupt",      no_argument,        0,   12  },
            {"3d",      no_argument,        0,   13  }
        };

        c = getopt_long(argc, argv, "i:o:h", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
               options.lightness = atoi(optarg);
               break;
            case 1:
               options.contrast = atoi(optarg);
               break;
            case 2:
               options.grayscale = true;
               break;
            case 3:
               options.hue_shift = atoi(optarg);
               break;
            case 4:
               options.sinecolor_shft = atof(optarg);
               break;
            case 5:
               options.invert = true;
               break;
            case 6:
                if (strcmp(optarg, "diagonal") == 0)
                    options.mirror = diagonal;
                else if (strcmp(optarg, "vertical") == 0)
                    options.mirror = vertical;
                else if (strcmp(optarg, "horizontal") == 0)
                    options.mirror = horizontal;
               break;
            case 7:
                ;
                int shfts[6];
                char *token;
                token = strtok(optarg, ",");

                int i = 0;
                while( token != NULL ) {
                    shfts[i++] = atoi(token);
                    token = strtok(NULL, ",");
                }
                options.rgbshft.red_x = shfts[0];
                options.rgbshft.red_y = shfts[1];
                options.rgbshft.green_x = shfts[2];
                options.rgbshft.green_y = shfts[3];
                options.rgbshft.blue_x = shfts[4];
                options.rgbshft.blue_y = shfts[5];
               break;
            case 8:
                if (strcmp(optarg, "all-random") == 0)
                    options.ps_preset = allrandom;
                else if (strcmp(optarg, "landscape") == 0)
                    options.ps_preset = landscape;
                else if (strcmp(optarg, "macro") == 0)
                    options.ps_preset = macro;
                else if (strcmp(optarg, "dark") == 0)
                    options.ps_preset = dark;
                else if (strcmp(optarg, "fewcolors") == 0)
                    options.ps_preset = fewcolors;
                else if (strcmp(optarg, "edges") == 0)
                    options.ps_preset = edge;
               break;
            case 9:
               options.blur = atoi(optarg);
               break;
            case 10:
               options.sharpen = atoi(optarg);
               break;
            case 11:
               options.edge = true;
               break;
            case 12:
               options.corrupt = true;
               break;
            case 13:
               options.a3d = true;
               break;
            case 'i':
                inn_fname = strdup(optarg);
                break;
            case 'o':
                outt_fname = strdup(optarg);
                break;
            case 'h':
                printf("-h, --help\t\t\tezen menü megjelenítése és kilépés\n");
                printf("-i, --input fájl\t\tbemenetként használt képfájl útvonala\n");
                printf("-o, --output fájl\t\tkimeneti kép útvonala\n");
                printf("--lightness ±érték\t\ta kép fényességének változtatása,\n\t\t\t\t+fényesebb, -sötétebb\n");
                printf("--contrast ±érték\t\ta kép kontrasztjának állítása, +nagyobb\n\t\t\t\tkontraszt, -kisebb kontraszt\n");
                printf("--hue-shift ±érték\t\ta kép HSL hue értékének eltolása a megadott\n\t\t\t\tértékkel\n");
                printf("--invert\t\t\ta kép negatívvá tétele\n");
                printf("--sinecolor-shift frekvencia\ta kép színeit a szinusz függvény alapján torzítja\n");
                printf("--mirror típus\t\t\ta kép tükrözése, típus: diagonal,\n\t\t\t\thorizontal, vertical irányokban\n");
                printf("--rgb-shift ± rvalue, ± gvalue, ± bvalue\n\t\t\t\tRGB shift alkalmazása a képen, a színek\n\t\t\t\tértékeit a megadott értékekkel csúsztatja\n\t\t\t\tel a megfelelő irányba\n");
                printf("--pixelsort preset\t\tpixelsort algoritmus végrehajtása a képen a\n\t\t\t\tmegadott preset alapján\n\t\t\t\t preset:\n\t\t\t\t  edges: megkeresi a kép objektumainak a szélét\n\t\t\t\t  és ezek között rendez\n\t\t\t\t  all-random: teljesen véletlenszerű\n\t\t\t\t  beállítások\n\t\t\t\t  landscape: tájképekhez és nagy tárgyakhoz\n\t\t\t\t  macro: részletes képekhez használható\n\t\t\t\t  fewcolors: kevés színt tartalmazó képekhez\n\t\t\t\t  dark: sötét területek kiemelése\n");
                printf("--blur érték\t\t\ta kép elmosása a megadott értékkel arányosan, kis\n\t\t\t\térték kis elmosás, nagy érték nagy elmosás\n");
                printf("--sharpen érték\t\t\ta kép élesebbé tétele a megadott értékkel\n\t\t\t\tarányosan, kis érték kis élesítés, nagy érték\n\t\t\t\tnagy élesítés\n");
                printf("--corrupt\t\t\tteljesen véletlenszerűen tönkreteszi a képet\n");
                printf("--grayscale\t\t\ta kép fekete-fehérre változtatása\n");
                printf("--3d\t\t\t\ta képet vörös-cián 3D képpé alakítja\n");
                printf("--edge-detect\t\t\ta kép objektumainak függőleges széleit mutató\n\t\t\t\tképet adja vissza\n");
                return 0;
            case '?':
                break;

           default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    time_t seconds;
    seconds = time(NULL);

    srand(seconds);
    Filter blur;
    Filter sharpen;
    if (options.blur > 0)
        setfilter(&blur, (int[]) {1, 2, 1, 2, 4, 2, 1, 2, 1}, 1/16.0, 3, 3);

    if (options.sharpen > 0)
        setfilter(&sharpen, (int[]) {0, -1, 0, -1, 5, -1, 0, -1, 0}, 1, 3, 3);

    if (inn_fname == NULL || outt_fname == NULL) {
        printf("nincs bemeneti, vagy kimeneti kép\n");
        return 1;
    }

    PPM_Image image = PPM_Parser(inn_fname);
    free(inn_fname);

    for (int i = 0; i < image.size_y; i++) {
        for (int j = 0; j < image.size_x; j++) {
            if (options.lightness != 0)
                change_light(image.image_data[i][j], options.lightness);
            if (options.contrast != 0)
                contrast(image.image_data[i][j], options.contrast);
            if (options.hue_shift != 0)
                hue_shift(image.image_data[i][j], options.hue_shift);
            if (options.invert)
                invert(image.image_data[i][j]);
            if (options.sinecolor_shft != 0)
            /*amplitude, frequency, phase, bias*/
            sinecolor_shift (image.image_data[i][j], 0.5, options.sinecolor_shft, 90, 1);
        }
    }
    if (options.mirror != none) {
        switch (options.mirror) {
            case diagonal:
                mirror_diagonal (image.image_data, image.size_x, image.size_y);
                break;
            case vertical:
                mirror_vertical (image.image_data, image.size_x, image.size_y);
                break;
            case horizontal:
                mirror_horizontal (image.image_data, image.size_x, image.size_y);
                break;
            case none:
                break;
        }
    }

    rgb_shift (&image, options.rgbshft);
    PsOptions preset;

    if (options.ps_preset == allrandom) {
        preset.pstype = hsl_l;
        preset.treshold = ran;
        preset.treshold_bottom_min = 0;
        preset.treshold_bottom_max = 100;
        preset.treshold_top_min = 0;
        preset.treshold_top_max = 100;
        preset.interval = ran;
        preset.interval_min = image.size_x/40;
        preset.interval_max = image.size_x/5;
        preset.merge = 1.0/(rand()%5)*(0.5+(rand()%10)/10);
    }

    if (options.ps_preset == landscape) {
        preset.pstype = hsl_l;
        preset.treshold = ran;
        preset.treshold_bottom_min = 0;
        preset.treshold_bottom_max = 10;
        preset.treshold_top_min = 0;
        preset.treshold_top_max = 70;
        preset.interval = ran;
        preset.interval_min = image.size_x/10;
        preset.interval_max = image.size_x/5;
        preset.merge = 1;
    }

    if (options.ps_preset == macro) {
        preset.pstype = hsl_l;
        preset.treshold = ran;
        preset.treshold_bottom_min = 0;
        preset.treshold_bottom_max = 70;
        preset.treshold_top_min = 0;
        preset.treshold_top_max = 100;
        preset.interval = ran;
        preset.interval_min = image.size_x/40;
        preset.interval_max = image.size_x/35;
        preset.merge = 1;
    }

    if (options.ps_preset == fewcolors) {
        preset.pstype = hsl_l;
        preset.treshold = ran;
        preset.treshold_bottom_min = 0;
        preset.treshold_bottom_max = 10;
        preset.treshold_top_min = 0;
        preset.treshold_top_max = 70;
        preset.interval = ran;
        preset.interval_min = image.size_x/30;
        preset.interval_max = image.size_x/20;
        preset.merge = 1/2.0;
    }

    if (options.ps_preset == dark) {
        preset.pstype = rgb_sum;
        preset.treshold = man;
        preset.treshold_bottom_min = 1;
        preset.treshold_bottom_max = 1;
        preset.treshold_top_min = 50;
        preset.treshold_top_max = 10;
        preset.interval = ran;
        preset.interval_min = image.size_x/30;
        preset.interval_max = image.size_x/20;
        preset.merge = 1;
    }
    if (options.ps_preset == edge) {
        preset.pstype = edges;
    }

    if (options.ps_preset != psnone)
        pixelsort(image.image_data, image.size_x, image.size_y, preset);

    convolve(image.image_data, image.size_x, image.size_y, blur, options.blur);

    convolve(image.image_data, image.size_x, image.size_y, sharpen, options.sharpen);

    if (options.corrupt)
        corrupt(&image);

    for (int i = 0; i < image.size_y; i++) {
        for (int j = 0; j < image.size_x; j++) {
            if (options.grayscale)
                grayscale(image.image_data[i][j]);
        }
    }
    if (options.a3d)
        anaglyph3d(&image);

    if (options.edge)
        image.image_data = detect_edges (image.image_data, image.size_x, image.size_y);
    PPM_Writer(outt_fname, &image);
    free(outt_fname);

    if (options.blur > 0)
        freefilter(blur);

    if (options.sharpen > 0)
        freefilter(sharpen);

    freeimage(image.image_data, image.size_x, image.size_y);

    printf("A program %ld másodperc alatt végzett\n", time(NULL)-seconds);

    return 0;
}
