#include "ppm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "imagefunc.h"

/**
 * @file
 * @brief A kép módosításához használt függvények
 */

/**
 * @brief beállítja egy filter értékét amit a convolve használ
 * @param[in] *filter a Filter egy példánya pointerként
 * @param[in] *filt a betöltendő adatok
 * @param[in] mult a filter szorzásához szükséges konstans
 * @param[in] size_x filter oszlopainak száma
 * @param[in] size_y filter sorainak száma
*/
void setfilter(Filter *filter, int *filt, double mult, int size_x, int size_y) {
    filter->filt = allocatefilter(3, 3);

        filter->mult = mult;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                filter->filt[i][j] = filt[i*size_x+j];
            }
        }
        filter->size_x = size_x;
        filter->size_y = size_y;
}
/**
 * @brief felszabadítja a filtert amit a convolve használ
 * @param[in] filter a felszabadítandó filter
*/
void freefilter(Filter filter) {
    for (int i = 0; i < filter.size_y; i++) {
        free(filter.filt[i]);
    }
    free(filter.filt);
}

/**
 * @brief lefoglalja a Filter 2 dimenziós tömbjét
 * @param[in] size_x a filter oszlopainak száma
 * @param[in] size_y a filter sorainak száma
*/
int **allocatefilter(int size_x, int size_y) {
    int **filter = (int **) malloc(size_y * sizeof(int *));

    for (int i=0; i<size_y; i++)
         filter[i] = (int *) malloc(size_x * sizeof(int));
    return filter;
}

/**
 * @brief RGB színskála átalakítása HSL skálára
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @param[out] hslpixel a pixel adatai HSL struktúraként
 * @see pszeudókód innen: http://marcocorvi.altervista.org/games/imgpr/rgb-hsl.htm
 */

HSL rgb2hsl(unsigned char pixel[]) {
    double H = 0;
    double S = 0;
    double L = 0;

    double r = pixel[0]/255.0;
    double g = pixel[1]/255.0;
    double b = pixel[2]/255.0;

    double Xmin = min((double[]) {r, g, b}, 3);
    double Xmax = max((double[]) {r, g, b}, 3);
    L = (Xmin + Xmax)/2.0;
    if (Xmin != Xmax) {
        if (L < 0.5)
            S = (Xmax - Xmin)/(Xmax + Xmin);
        else
            S = (Xmax - Xmin)/(2 - Xmax - Xmin);
        if (r == Xmax)
            H = (g-b)/(Xmax - Xmin) + ((g < b) ? 6 : 0);
        else if (g == Xmax)
            H = 2 + (b-r)/(Xmax - Xmin);
        else if (b == Xmax)
            H = 4 + (r-g)/(Xmax - Xmin);

        if (H < 0.0)
            H += 6;
        H /= 6.0;
    }

    HSL hsl;
    hsl.h = H;
    hsl.s = S;
    hsl.l = L;
    return hsl;
}

double hsl2rgbcolor(double temp1, double temp2, double temp3) {
    if (temp3 < (1/6.0))
        return temp1+(temp2-temp1)*6*temp3;
    if (temp3 < (1/2.0))
        return temp2;
    if (temp3 < (2/3.0))
        return temp1+(temp2-temp1)*(2/3.0 - temp3)*6;
    return temp1;
}

/**
 * @brief HSL színskála átalakítása RGB skálára
 * @param[in] pixel egy pixel adatai HSL struktúrában
 * @param[out] rgbpixel a pixel RGB struktúraként
 * @see pszeudókód innen: http://marcocorvi.altervista.org/games/imgpr/rgb-hsl.htm
 * @see hsl2rgbcolor
 */

RGB hsl2rgb(HSL pixel) {
    double H = pixel.h;
    double S = pixel.s;
    double L = pixel.l;

    double r = 0;
    double g = 0;
    double b = 0;

    RGB rgbpixel;

    double temp1, temp2, temp3;

    if (S == 0.0) {
        rgbpixel.r = L*255;
        rgbpixel.g = L*255;
        rgbpixel.b = L*255;

    }
    else {
        if (L < 0.5)
            temp2 = L*(1.0+S);
        else
            temp2 = L + S - L*S;

        temp1 = 2.0 * L - temp2;

        // red
        temp3=H+1/3.0;
        if (temp3 > 1)
            temp3 = temp3 - 1.0;
        r = hsl2rgbcolor(temp1, temp2, temp3);

        // green
        temp3 = H;
        g = hsl2rgbcolor(temp1, temp2, temp3);

        // blue
        temp3 = H-1/3.0;
        if (temp3 < 0.0)
            temp3 = temp3 + 1.0;
        b = hsl2rgbcolor(temp1, temp2, temp3);

        rgbpixel.r = r*255;
        rgbpixel.g = g*255;
        rgbpixel.b = b*255;
    }

    return rgbpixel;
}

/**
 * @brief A pixel értékének invertálása
 *
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 *
 * A pixelből kivonunk 255-öt majd az abszolút értékét vesszük, és így kapjuk meg az invertált értéket.
 */

void invert(unsigned char pixel[]) {
    for (int color = 0; color < 3; color++) {
        pixel[color] = (unsigned char) fabs((double) pixel[color]-255);
    }
}

/**
 * @brief Megváltoztatja a pixel kontrasztját
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @param[in] level a változtatás mértéke (a 259 nem megengedett)
 * @see pszeudokód innen: https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-5-contrast-adjustment/
 */
void contrast(unsigned char pixel[], double level) {
    if (level == 259) // a 0-val való osztás elkerülése miatt
        return;
    double factor = (259 * (level + 255)) / (255.0 * (259 - level));
    pixel[0] = (unsigned char) clamp(factor * (pixel[0] - 128) + 128, 0, 255);
    pixel[1] = (unsigned char) clamp(factor * (pixel[1] - 128) + 128, 0, 255);
    pixel[2] = (unsigned char) clamp(factor * (pixel[2] - 128) + 128, 0, 255);
}

/**
 * @brief fekete-fehérré alakít egy képet
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @see használt értékek innen: https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-3-greyscale-conversion/
 */
void grayscale(unsigned char pixel[]) {
    unsigned char value = pixel[0] * 0.2989 + pixel[1] * 0.5870 + pixel[2] * 0.1140;
    for (int i = 0; i < 3; i++)
        pixel[i] = value;
}

/**
 * @brief a pixel intenzitása (a három szín átlaga) alapján a pixel értékéhez legközelebbi szélsőértékhez igazítja a pixel értékét (128 alatt 0, 128 felett 255)
 *
 * @param[in] pixel[] a módositandó pixel
 */
void sharp_grayscale(unsigned char pixel[]) {
    int avg = (pixel[0] + pixel[1] + pixel[2])/3;
    for (int color = 0; color < 3; color++) {
        pixel[color] = (avg < 128) ? 0 : 255;
    }
}

/**
 * @brief a treshold feletti értéket fehérré változtatja, az alatta lévőt feketévé
 *
 * @param[in] pixel[] a módositandó pixel
 * @param[in] treshold ez az érték felett kell fehérré ez alatt feketévé
 */
void set_white(unsigned char pixel[], int treshold) {
    for (int color = 0; color < 3; color++) {
        if (pixel[color] > treshold)
            pixel[color] = 255;
        else
            pixel[color] = 0;
    }
}

/**
 * @brief megkeresi a képen található objektumok függőleges széleit
 *
 * @param[in] ***image a kép amin keresni kell
 * @param[in] size_x a kép oszlopainak száma
 * @param[in] size_y a kép sorainak száma
 *
 * Mivel egy teljesen új képet hozunk létre, először lemásoljuk a képet.
 * Ezek után sorrendben a következő műveleteket hajtjuk végre:
 * - 3x3 Gauss-blur - ez előkészíti a következő művelethez a képet, mivel így sokkal kevesebb él lesz a képen.
 * - 3x3 vertical-line filter - a függőleges éleket keresi meg, az élek fehérek, minden más fekete
 * - set_white a kicsit sötét színek kivételével (9 felett) teljesen fehérré (255) változtatjuk a pixeleket, a többi fekete (0) lesz, így élesebbek lesznek az élek
 * - 3x3 Gauss-blur tompítjuk az éleket, a következő művelethez
 * - sharp_grayscale ami a pixel értékéhez legközelebbi szélsőértékhez igazítja a pixel értékét (128 alatt 0, 128 felett 255), így nagyon vékony élek keletkeznek, mivel az előző művelet összemossa a fehér és fekete színeket, így csak a legbelső élek maradnak meg.
 */

unsigned char ***detect_edges(unsigned char ***image, int size_x, int size_y) {
    unsigned char ***edgeimage = allocateimage (size_x, size_y);

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            for (int k = 0; k < 3; k++) {
                edgeimage[i][j][k] = image[i][j][k];
            }
        }
    }

    Filter blur;
    setfilter(&blur, (int[]) {1, 2, 1, 2, 4, 2, 1, 2, 1}, 1/16.0, 3, 3);
    Filter vertical_line;
    setfilter(&vertical_line, (int[]) {-1, 2, -1, -1, 2, -1, -1, 2, -1}, 1, 3, 3);

    convolve(edgeimage, size_x, size_y, blur, 1);
    convolve(edgeimage, size_x, size_y, vertical_line, 1);

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            set_white (edgeimage[i][j], 9);
        }
    }

    convolve(edgeimage, size_x, size_y, blur, 1);

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            sharp_grayscale (edgeimage[i][j]);
        }
    }
    freefilter (blur);
    freefilter (vertical_line);
    return edgeimage;
}

/**
 * @brief egy pixel fényességének megváltoztatása
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @param[in] percent mennyi százalékkal
 *
 * A pixel értéket először HSL színskálára alakítjuk át, majd ehhez hozzáadjuk a változtatás mértékét, végül korlátozzuk 0, 1 intervallumra és visszaalakítjuk RGB színskálára.
 */
void change_light(unsigned char pixel[], int percent) {
    double value = percent/100.0;
    HSL hsl = rgb2hsl(pixel);
    RGB rgbpixel;

    hsl.l = clamp(hsl.l + value, 0.0, 1.0);
    rgbpixel = hsl2rgb(hsl);

    pixel[0] = rgbpixel.r;
    pixel[1] = rgbpixel.g;
    pixel[2] = rgbpixel.b;
}

/**
 * @brief az átló mentén tükrözi a képet
 * @param[in] ***matrix a tükrözendő matrix (többnyire a kép)
 * @param[in] size_x a mátrix oszlopainak száma
 * @param[in] size_y a mátrix sorainak száma
 *
 * A bal felső sarokból kezdve, a pixeleket a jobb alsó sarokba helyezi át. Azért csak a sze_y/2-ig mert ha ennél tovább menne akkor visszacserélné az egész mátrixot.
 */
void mirror_diagonal(unsigned char ***matrix, int size_x, int size_y) {
    for (int line = 0; line < size_y / 2; line++) {
        for (int row = 0; row < size_x; row++) {
            unsigned char *temp = matrix[line][row];
            matrix[line][row] = matrix[size_y-1-line][size_x-1-row];
            matrix[size_y-1-line][size_x-1-row] = temp;
        }
    }
}

/**
 * @brief a függöleges tengelyre tükrözi a képet
 * @param[in] ***matrix a tükrözendő matrix (többnyire a kép)
 * @param[in] size_x a mátrix oszlopainak száma
 * @param[in] size_y a mátrix sorainak száma
 *
 * A sor első elemét a végére helyezi. Azért csak size_x/2-ig megy, mert ha ennél tovább menne akkor visszacserélné az egész mátrixot.
 */
void mirror_vertical(unsigned char ***matrix, int size_x, int size_y) {
    for (int line = 0; line < size_y; line++) {
        for (int row = 0; row < size_x/2; row++) {
            unsigned char *temp = matrix[line][row];
            matrix[line][row] = matrix[line][size_x-1-row];
            matrix[line][size_x-1-row] = temp;
        }
    }
}

/**
 * @brief a vízszintes tengelyre tükrözi a képet
 * @param[in] ***matrix a tükrözendő matrix (többnyire a kép)
 * @param[in] size_x a mátrix oszlopainak száma
 * @param[in] size_y a mátrix sorainak száma
 *
 * Az oszlop első elemét a végére helyezi. Azért csak size_y/2-ig megy, mert ha ennél tovább menne akkor visszacserélné az egész mátrixot.
 */
void mirror_horizontal(unsigned char ***matrix, int size_x, int size_y) {
    for (int line = 0; line < size_y / 2; line++) {
        for (int row = 0; row < size_x; row++) {
            unsigned char *temp = matrix[line][row];
            matrix[line][row] = matrix[size_y-1-line][row];
            matrix[size_y-1-line][row] = temp;
        }
    }
}

/**
 * @brief végrehajtja a konvolúciót, ami a blur és sharpen lépésekhez kell
 * @see pszeudokód és működési elv itt: https://en.wikipedia.org/wiki/Kernel_(image_processing)
 * @see Filter
 *
 * @param[in] ***original módosítandó kép
 * @param[in] size_x a kép oszlopainak száma
 * @param[in] size_y a kép sorainak száma
 * @param[in] filter a használandó filter
 */
void convolve(unsigned char ***original, int size_x, int size_y, Filter filter, int times) {
    double sur, sug, sub;
    /* lefoglalunk egy új képet ahova az új értékeket írjuk */
    unsigned char ***newmatrix = allocateimage(size_x, size_y);

    int filterCenterX = filter.size_x / 2;
    int filterCenterY = filter.size_y / 2;

    for (int ttimes = 0; ttimes < times; ttimes++) {
        for (int i = 0; i < size_y; i++) { /* sorok */
            for(int j = 0; j < size_x; j++) { /* oszlopok */

                sur = 0;
                sug = 0;
                sub = 0;

                for (int k = 0; k < filter.size_y; k++) { /* a filter sorai */
                    int kk = filter.size_y - 1 - k;      /* a megfordított filter sorai */

                    for(int l = 0; l < filter.size_x; l++) { /* a filter oszlopai */
                        int ll = filter.size_x - 1 - l; /* a megfordított filter oszlopai */

                        /* a vizsgálandó pixel koordinátái */
                        int ii = i + (filterCenterY - kk);
                        int jj = j + (filterCenterX - ll);

                        /* ha a koordináták a képen belül vannak */
                        if( ii >= 0 && ii < size_y && jj >= 0 && jj < size_x ) {
                            sur += original[ii][jj][0] * filter.mult*filter.filt[kk][ll];
                            sug += original[ii][jj][1] * filter.mult*filter.filt[kk][ll];
                            sub += original[ii][jj][2] * filter.mult*filter.filt[kk][ll];
                        }
                        /* ha kívül, akkor a hozzá legközelebb eső legszélső pixelt használjuk */
                        else {
                            /* az ii mindenképpen a két határon kívül van, tehát az értéke vagy a legfelső sor, vagy a legalsó */
                            int newline = clamp(ii, 0, size_y-1);
                            /* az jj mindenképpen a két határon kívül van, tehát az értéke vagy a legelső bal oldali oszlop, vagy a legszélső jobboldali */
                            int newcol = clamp(jj, 0, size_x-1);
                            sur += original[newline][newcol][0] * filter.mult*filter.filt[kk][ll];
                            sug += original[newline][newcol][1] * filter.mult*filter.filt[kk][ll];
                            sub += original[newline][newcol][2] * filter.mult*filter.filt[kk][ll];
                        }
                    }
                }
                /* végül átírjuk az üres képbe az adatokat úgy hogy levágjuk a 0 és 255 közötti intervallumra */
                newmatrix[i][j][0] = (unsigned char) clamp(sur, 0, 255);
                newmatrix[i][j][1] = (unsigned char) clamp(sug, 0, 255);
                newmatrix[i][j][2] = (unsigned char) clamp(sub, 0, 255);
            }
        }

        /* átmásoljuk az új képet az eredetibe */
        for (int y = 0; y < size_y; y++) {
            for (int x = 0; x < size_x; x++) {
                original[y][x][0] = newmatrix[y][x][0];
                original[y][x][1] = newmatrix[y][x][1];
                original[y][x][2] = newmatrix[y][x][2];
            }
        }
    }
    /* felszabadítjuk az új képet */
    freeimage(newmatrix, size_x, size_y);
}

/**
 * A pixlsort segédfüggvénye ami rendezi és helyére rakja a pixeleket
 * @param[in] partline[] a rendezésre kiválasztott pixelek
 * @param[in] ***image a kép ahova vissza kell írni a pixeleket
 * @param[in] line a kép aktuálisan módosítandó sora
 * @param[in] start a kezdés pozíciója, ahonnan el kell kezdeni a másolást
 * @param[in] end ameddig másolni kell
 * @param[in] dir a rendezés iránya: 0 balról jobbra 1: jobbról balra
 *
 * Először rendezzük a pixeleket. Ezután egy ideiglenes tömbbe másoljuk a rendezett elemeknek megfelelő adatokat és utána az iránynak megfelelően visszaírjuk őket az eredeti képbe.
 *
*/
void sortcopy(Sort partline[], unsigned char ***image, int line, int start, int end, int dir) {
    int size = end-start;
    quicksort(partline, 0, size-1);
    unsigned char partlinesorted[size][3];
    for (int i = 0; i < size; i++) {
        for (int color = 0; color < 3; color++) {
            partlinesorted[i][color] = image[line][partline[i].idx][color];
        }
    }
    int reverse_i;
    if (dir == 1)
        reverse_i = size-1;
    else
        reverse_i = 0;
    for (int i = start; i < end; i++) {
        for (int color = 0; color < 3; color++) {
            image[line][i][color] = partlinesorted[reverse_i][color];
        }
        if (dir == 1)
            reverse_i--;
        else
            reverse_i++;
    }
}
/**
 * @brief A pixelsort segédfüggvénye. A cím szerint megadott *treshold paraméterbe állítja be a felső tresholdot az alapján hogy a pixelsort milyen paraméterket kapott
 * @param[in] *treshold a paraméter amibe vissza kell írni az értéket
 * @param[in] options a pixelsort által kapott tulajdonságok
 * @see PsOptions
 * @see ps_option_type
 *
 * Ha a ps_option_type ran akkor egy véletlenszerű értéket választunk a felső és alsó korlát között. Ha man akkor csak az alsó korlátot vesszük figyelembe.
 */
static void checktreshold_top(int *treshold, PsOptions options) {
    if (options.treshold == ran)
        *treshold = (rand()%(options.treshold_top_max - options.treshold_top_min + 1) + options.treshold_top_min);
    else
        *treshold = options.treshold_top_min;
}

/**
 * @brief A pixelsort segédfüggvénye. A cím szerint megadott *treshold paraméterbe állítja be az alsó tresholdot az alapján hogy a pixelsort milyen paraméterket kapott
 * @param[in] *treshold a paraméter amibe vissza kell írni az értéket
 * @param[in] options a pixelsort által kapott tulajdonságok
 * @see PsOptions
 * @see ps_option_type
 *
 * Ha a ps_option_type ran akkor egy véletlenszerű értéket választunk a felső és alsó korlát között. Ha man akkor csak az alsó korlátot vesszük figyelembe.
 */
static void checktreshold_bottom(int *treshold, PsOptions options) {
    if (options.treshold == ran)
        *treshold = (rand()%(options.treshold_bottom_max - options.treshold_bottom_min + 1) + options.treshold_bottom_min);
    else
        *treshold = options.treshold_bottom_min;
}

/**
 * @brief A pixelsort segédfüggvénye. A cím szerint megadott *interval paraméterbe állítja be, hogy mekkora legyen az a környezet amin el kell végezni a rendezést, az alapján hogy a pixelsort milyen paraméterket kapott
 * @param[in] *interval a paraméter amibe vissza kell írni az értéket
 * @param[in] options a pixelsort által kapott tulajdonságok
 * @param[in] elem az éppen aktuálisan vizsgált pixel oszlopszáma
 * @see PsOptions
 * @see ps_option_type
 *
 * Ha a ps_option_type ran akkor egy véletlenszerű értéket választunk a felső és alsó korlát között. Ha man akkor csak az alsó korlátot vesszük figyelembe.
 */
static void checkinterval(int *interval, PsOptions options, int elem) {
    if (options.interval == ran)
        *interval = rand()%(options.interval_max - options.interval_min + 1) + options.interval_min;
    else
        *interval = options.interval_min;
    if (*interval < 0)
        *interval = elem;
}

/**
 * @brief Végrehajtja a pixelsort-ot.
 * @param[in] ***image a módosítandó kép
 * @param[in] size_x a kép oszlopainak száma
 * @param[in] size_y a kép sorainak száma
 * @param[in] options a pixelsort tulajdonságai a PsOptions struktúraként
 * @see PsOptions
 * @see sortcopy
 *
 * Az algoritmus lényege, hogy a megadott típus alapján (HSL lightness, RGB intesity) minden sorban keres egy olyan pixelt ami belefér a megadott treshold-ba (bottom, top). Az edges típusnál a kép objektumainak függőleges széleit keresi meg és ezek a határok között rendez.
 * A megtalált pixelnek egy valamilyen a PsOptions-ban meghatározott környezetét vesszük és ezen a környezeten sorba rendezzük a pixeleket a megadott típus alapján, a szintén megadott irányba.
 *
*/
void pixelsort(unsigned char ***image, int size_x, int size_y, PsOptions options) {
    int times = 0;

    /* edges típusú pixelsort. Ez adja a legjobb eredményt.*/
    if (options.pstype == edges) {
        time_t seconds;
        seconds = time(NULL);
        int lastelem;
        int interval = 0;
        unsigned char ***edgeimage = detect_edges (image, size_x, size_y);
        for (int line = 0; line < size_y; line++) {
            lastelem = 0;
            for (int elem = 0; elem < size_x; elem++) {
                if (edgeimage[line][elem][0] == 255) { /* a fehér szín egy edge*/
                    interval = elem-lastelem;
                    int size = (elem < interval) ? elem : interval; /* ha az aktuális pixel közelebb van a kép széléhez mint a megválasztott környezet akkor ennek megfelelő méretet kell választani */
                    int start = (int) max((double[]){elem-interval, 0}, 2); /* az előzőhöz hasonlóan */
                    Sort partline[size];
                    int i = 0; /* az új tömb számlálója */
                    for (int interval_i = start; interval_i < elem; interval_i++) {
                        partline[i].idx = interval_i;
                        partline[i].value = rgb2hsl(image[line][interval_i]).l; /* HSL Lightness alapján rendezünk*/
                        i++;
                    }
                    sortcopy(partline, image, line, start, elem, 0); // többnyire jobb az eredmény ha világostól sötét fele rendezünk (dir=0), mert többnyire arra számítunk, hogy egy objektum sötétebb, mint a háttér
                    times++;
                    lastelem = elem;
                    //break;
                }
            }
        }
        freeimage(edgeimage, size_x, size_y);
        printf("Edges pixelsort %d alkalommal végrehajtva, %ld másodperc alatt\n", times, time(NULL)-seconds);
        return;
    }

    int interval;
    int treshold_top, treshold_bottom;

    for (int line = 0; line < size_y; line++) {
        for (int elem = 0; elem < size_x; elem++) {
            if (options.pstype == hsl_l) { /* ha HSL Lightness alapján kell sortolni */
                checktreshold_top (&treshold_top, options); /* beállítjuk a tresholdot az alapján hogy mi a beállítás */
                checktreshold_bottom (&treshold_bottom, options); /* beállítjuk a tresholdot az alapján hogy mi a beállítás */

                HSL hsl = rgb2hsl(image[line][elem]); /* megnézzük az aktuális pixel HSL értékeit */
                if (hsl.l*100 >= treshold_bottom && hsl.l*100 <= treshold_top) { /* ha tresholdon belül van */

                    checkinterval(&interval, options, elem); /* beállítjuk azt a környezetet amin belül rendezni kell */
                    // rendezésre kijelölt elemek átmásolása egy új tömbbe
                    int size = (elem < interval) ? elem : interval; /* ha az aktuális pixel közelebb van a kép széléhez mint a megválasztott környezet akkor ennek megfelelő méretet kell választani */
                    int start = (int) max((double[]){elem-interval, 0}, 2); /* az előzőhöz hasonlóan */
                    Sort partline[size];
                    int i = 0; /* az új tömb számlálója */
                    for (int interval_i = start; interval_i < elem; interval_i++) {
                        partline[i].idx = interval_i;
                        partline[i].value = rgb2hsl(image[line][interval_i]).l;
                        i++;
                    }
                    sortcopy(partline, image, line, start, elem, 0); //(elem < size_x/2) ? 0 : 1);

                    elem += interval*options.merge;

                    times++;

                }
            }
            else if (options.pstype == rgb_sum) {
                checktreshold_top (&treshold_top, options); /* beállítjuk a tresholdot az alapján hogy mi a beállítás */
                checktreshold_bottom (&treshold_bottom, options); /* beállítjuk a tresholdot az alapján hogy mi a beállítás */

                int rgbsum = image[line][elem][0] + image[line][elem][1] + image[line][elem][2];
                if (rgbsum >= treshold_bottom && rgbsum <= treshold_top) {
                    checkinterval(&interval, options, elem);
                    // copying below treshold elements to a new array
                    int i = 0;
                    int size = (elem < interval) ? elem : interval;
                    int start = (int) max((double[]){elem-interval, 0}, 2);
                    Sort partline[size];
                    for (int interval_i = start; interval_i < elem; interval_i++) {
                        partline[i].idx = i;
                        partline[i].value = image[line][i][0] + image[line][i][1] + image[line][i][2];
                        i++;
                    }
                    sortcopy(partline, image, line, start, elem, 0);
                    // merge size smaller the value more sorts
                    elem += interval*options.merge;

                    times++;
                }

            }
        }
    }
    printf("Pixelsort végrehajtva %d alkalommal\n", times);
}

/**
 * @brief A HSL színskála Hue értékét tolja el
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @param[in] value mekkora értékkel (-100)-100
 *
 * A pixel értékét átalakítja HSL színskálára, majd a Hue értékhez hozzáadja a választott értéket. Nincs határ megszabva, de mivel a Hue 0 és 1 közötti értéket vesz fel, így a -100-nál kisebb és a 100 nagyobb értékek mod 100-zal lesznek számolva
 */

void hue_shift(unsigned char pixel[], double value) {
    HSL hsl = rgb2hsl(pixel);
    RGB rgbpixel;

    hsl.h = (abs((int) (hsl.h*100 + value))%100)/100.0;
    rgbpixel = hsl2rgb(hsl);

    pixel[0] = rgbpixel.r;
    pixel[1] = rgbpixel.g;
    pixel[2] = rgbpixel.b;
}

/**
 * @brief A szinusz függvény alapján eltolja a színeket, RGB színcsatornánként
 * @param[in] pixel[] egy pixel RGB adatai nyers állapotban (Red, Green, Blue)
 * @param[in] amplifier a szinusz függvény amplitúdóját adja meg
 * @param[in] freq a szinusz függvény frekvenciáját adja meg. Ajánlott értékei: ]0.004, 0.05[ vagy ]10, 50[. Nagyon nagy frekvencia zajos képet eredményez, mivel egymáshoz nagyon közeli színek is teljesen különböző értéket kapnak.
 * @param[in] phase a szinusz fáziseltolása
 * @param[in] bias megmondja hogy minimum mennyivel kell szorozni a pixel értékét. Ajánlott ]0.7, 1[ között, különben elkezd sötétedni a kép
 *
 * A pixelt csatornánként felbontjuk, majd az értékét megszorozzuk a szinusz(pixelcsatorna) értékével. Végül korlátozzuk a 0, 255 intervallumra, mivel az amplitúdó és a bias miatt kicsúszhat a határból.
 * @see https://github.com/ImageMagick/ImageMagick/blob/f842055ddb5936c1cacbbcfc8c8e06a7be69d102/MagickCore/accelerate-kernels-private.h CLQuantum ApplyFunction (line 1707)
 */

void sinecolor_shift(unsigned char pixel[], double amplifier, double freq,  double phase, double bias) {
    pixel[0] = clamp(pixel[0]*fabs(amplifier*sin(2.0f*3.14*(freq*pixel[0] + phase/360.0f)) + bias), 0, 255);
    pixel[1] = clamp(pixel[1]*fabs(amplifier*sin(2.0f*3.14*(freq*pixel[1] + phase/360.0f)) + bias), 0, 255);
    pixel[2] = clamp(pixel[2]*fabs(amplifier*sin(2.0f*3.14*(freq*pixel[2] + phase/360.0f)) + bias), 0, 255);
}

/**
 * @brief Megforgatja a paraméterként kapott sort.
 * @param[in] **line a forgatandó sor
 * @param[in] size a sor mérete
 * @param[in] color melyik színt kell áthelyezni
 * @param[in] rot mennyivel kell forgatni
 */
static void rotate(unsigned char **line, int size, int color, int rot) {
    unsigned char temp, tempn;
    int dir = (rot < 0) ? 1 : 0; // 1 balra, 0 jobbra
    rot = (rot < 0) ? -rot : rot;
    for (int i = 0; i < rot; i++) {
        temp = line[0][color]; /**> az első lefutásnál a 2. helyre így az első kerül, majd így tovább*/
        for (int j = 0; j < size; j++) {
            int ind = (dir == 1) ? (size-(j+1)) : (j+1)%size; /**> ha balra forgatunk akkor a méretből kell elvenni hogy melyik kell, ha jobbra akkor csak eggyel növelünk*/
            tempn = line[ind][color]; /**> azért kell mod size-ot venni, mert így a végén lévő utolsó elem a legelejére kerül*/
            line[ind][color] = temp;
            temp = tempn;
        }
    }
}

/**
 * @brief Oszlopokat forgat meg.
 * @param[in] *image a teljes kép ahonnan ki lehet venni az oszlopokat
 * @param[in] col melyik oszlopot kell
 * @param[in] color melyik színt kell forgatni
 * @param[in] rot mennyivel kell forgatni
 */
static void rotate_vertical(PPM_Image *image, int col, int color, int rot) {
    // kigyűjtjük a forgatandó elemeket
    unsigned char **temp = (unsigned char **) malloc(image->size_y * sizeof(unsigned char *));

    for (int i = 0; i < image->size_y; i++)
         temp[i] = (unsigned char *) malloc(3 * sizeof(unsigned char));

    int tempi = 0;
    for (int line = 0; line < image->size_y; line++) {
        for (int color = 0; color < 3; color++)
            temp[tempi][color] = image->image_data[line][col][color];
        tempi++;
    }

    // megforgatás
    rotate(temp, image->size_y, color, rot);

    // visszaírás
    tempi = 0;
    for (int line = 0; line < image->size_y; line++) {
        for (int color = 0; color < 3; color++)
            image->image_data[line][col][color] = temp[tempi][color];
        tempi++;
    }

    //felszabadítás
    for (int i = 0; i < image->size_y; i++) {
        free(temp[i]);
    }
    free(temp);
}

/**
 * @brief Elmozdítja az RGB színskála értékeit a megadott irányba
 *
 * @param[in] *image a kép amin alkalmazni kell
 * @param[in] options melyik színt milyen irányba, mennyivel (RGB_SHIFT)
 *
 * Először a soronként mozgat a rotate segítségével, majd oszloponként a rotate_vertical segítségével.
 *
 * @see RGB_SHIFT
 * @see rotate
 * @see rotate_vertical
 */

void rgb_shift(PPM_Image *image, RGB_SHIFT options) {
    for (int line = 0; line < image->size_y; line++) {
        for (int color = 0; color < 3; color++) {
            switch (color) {
                case 0: // red
                    rotate(image->image_data[line], image->size_x, color, options.red_x);
                    break;
                case 1: // green
                    rotate(image->image_data[line], image->size_x, color, options.green_x);
                    break;
                case 2: // blue
                    rotate(image->image_data[line], image->size_x, color, options.blue_x);
                    break;
            }
        }
    }
    for (int col = 0; col < image->size_x; col++) {
        for (int color = 0; color < 3; color++) {
            switch (color) {
                case 0: // red
                    if (options.red_y != 0)
                        rotate_vertical (image, col, color, options.red_y);
                    break;
                case 1: // green
                    if (options.green_y != 0)
                        rotate_vertical (image, col, color, options.green_y);
                    break;
                case 2: // blue
                    if (options.blue_y != 0)
                        rotate_vertical (image, col, color, options.blue_y);
                    break;
            }
        }
    }
}

/**
 * @brief Vörös-Cián 3D képpé alakítja a bemenetet
 * @param[in] *image átalakítandó kép (PPM_Image)
 *
 * A lényege, hogy a vörös csatornát el kell tolni balra valamennyivel. Ez az érték a szélesség nagyjából 0,925%-a (1080 pixel szélességnél kb. 10 pixel).
 * A kép úgy fog kinézni, mintha be lenne süllyesztve.
 * @see PPM_Image
 * @see rotate
 */
void anaglyph3d(PPM_Image *image) {
    for (int line = 0; line < image->size_y; line++) {
        rotate(image->image_data[line], image->size_x, 0, -image->size_x*0.00925);
    }
}

/**
 * @brief Véletlenszerűen tönkreteszi a képet.
 *
 * @param[in] ***image a módosítandó kép
 *
 * Először kiválasztunk 3 értéket amivel sorrendben a: fényességet (change_light), a kontrasztot (contrast) és a HSL Hue -t (hue_shift) módosítjuk.
 * Ezek után a kép bizonyos részeit tükrözzük, ha páratlan számú alkalommal, akkor a kép egy része fordítva lesz, ha páros számú alkalommal, akkor az eredeti irányban.
 * Ez után az RGB színeit kell shiftelni véletlenszerű irányba és értékkel.
 * Ezután a lehető legbővebb véletlenszerű beállítással végrehajtjuk a pixelsort -ot
 */
void corrupt(PPM_Image *image) {
    int light = rand()%(30-(-25)) +(-25);
    int cont = rand()%51 +(-25);
    int hue = rand()%201 +(-100);
    for (int i = 0; i < image->size_y; i++) {
        for (int j = 0; j < image->size_x; j++) {
            change_light(image->image_data[i][j], light);
            contrast(image->image_data[i][j], cont);
            hue_shift(image->image_data[i][j], hue);
        }
    }

    for (int i = 0; i < 3; i++) {
        mirror_diagonal(image->image_data, (int) (image->size_x*((rand()%(30 - 10 + 1) + 10)/100.0)), (int) (image->size_y*((rand()%(30 - 10 + 1) + 10)/100.0)));
    }

    RGB_SHIFT rgbshft = {rand()%(image->size_x/5),rand()%(image->size_y/3), rand()%(image->size_x/5),rand()%(image->size_y/3), rand()%(image->size_x/5),rand()%(image->size_y/3)};
    rgb_shift (image, rgbshft);

    int interval_min = image->size_x/((rand()%(20 - 5 + 1)+5));
    int interval_max = clamp(image->size_x/((rand()%(20 - 5 + 1)+5)), interval_min+1, image->size_x);
    PsOptions rando = {hsl_l, ran, 1, 100, 1, 100, ran, interval_min, interval_max, (rand()%100)/100.0};
    pixelsort(image->image_data, image->size_x, image->size_y, rando);
}
