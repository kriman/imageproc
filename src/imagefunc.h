#ifndef IMAGEFUNC
#define IMAGEFUNC

#include "addmath.h"
#include "ppm.h"
/**
 * @brief HSL színskála struktúrája
 */
typedef struct HSL {
    double h; /**< HSL Hue */
    double s; /**< HSL Saturation */
    double l; /**< HSL Lightness */
} HSL;

/**
 * @brief RGB színskála struktúrája
 */
typedef struct RGB {
    unsigned char r; /**< RGB Red */
    unsigned char g; /**< RGB Green */
    unsigned char b; /**< RGB Blue */
} RGB;

/**
 * @brief A konvolúcióhoz használt filter tulajdonságai
 */
typedef struct Filter {
    int **filt; /**< 2 dimenziós tömb a filter értékeivel */
    int size_x; /**< Filter oszlopainak száma */
    int size_y; /**< Filter sorainak száma */
    double mult;/**< Filter értékeinek szorzásához használt konstans */
} Filter;

/**
 * @brief Az RGB shifteléshez használt értékek
 */
typedef struct RGB_SHIFT {
  int red_x; /**< Vörös szín eltolásásnak x értéke */
  int red_y; /**< Vörös szín eltolásásnak y értéke */
  int green_x; /**< Zöld szín eltolásásnak x értéke */
  int green_y; /**< Zöld szín eltolásásnak y értéke */
  int blue_x; /**< Kék szín eltolásásnak x értéke */
  int blue_y; /**< Kék szín eltolásásnak y értéke */
} RGB_SHIFT;

/**
 * @brief Pixelsort preset típusa
 */

typedef enum pixelsort_preset {
  psnone, /**< Semmilyen rendezés */
  landscape, /**< Nagy objektumokhoz használt rendezés */
  macro, /**< Kis objektumokhoz használt rendezés */
  fewcolors, /**< Kevés színhez használt rendezés */
  allrandom, /**< Teljesen véletlenszerű rendezés */
  dark, /**< Sötét képekhez használt rendezés */
  edge /**< A képen található objktumok széle alapján rendezés */
} pixelsort_preset;

/**
 * @brief A tükrözés típusa
 */
typedef enum mirror_type {
  none,
  diagonal,
  vertical,
  horizontal
} mirror_type;

/**
 * @brief A pixelsort típusa
 * @see pixelsort
 */
typedef enum ps_type {
  rgb_sum,  /**< RGB intensity */
  hsl_l,     /**< HSL Lightness */
  edges, /**< Megkeresi az objektumok szélét és onnan kezdi */
} ps_type;

/**
 * @brief A pixelsort treshold és interval értékének kiválasztása
 */
typedef enum ps_option_type {
  ran,  /**< véletlenszerű az alsó és felső határok között */
  man   /**< csak az alsó határ */
} ps_option_type;


/**
 * @brief A pixelsort beállításai
 */
typedef struct PsOptions {
    ps_type pstype;           /**< A pixelsort típusa */
    ps_option_type treshold;  /**< A treshold kiválasztásának típusa. Ha ran akkor alsó és felsó korlát között választ egy véletlenszerű értéket. Ha man akkor csak az alsó korlát számít. */
    int treshold_bottom_min;  /**< Az alsó treshold alsó határa. */
    int treshold_bottom_max;  /**< Az alsó treshold felső határa. */
    int treshold_top_min;     /**< Az felső treshold alsó határa. */
    int treshold_top_max;     /**< Az felső treshold felső határa. */
    ps_option_type interval; /**< a rendezési környezet kiválasztásának típusa. Ha ran akkor alsó és felsó korlát között választ egy véletlenszerű értéket. Ha man akkor csak az alsó korlát számít. */
    int interval_min; /**< a rendezési környezet alsó határa. Az értékét úgy érdemes megválasztani, hogy igazodjon a képen található objektumok méretéhez. Egy tájképnél például lehet nagy értéket választani, mivel ott nem fontosak a részletek, míg egy részletes képnél minnél kisebbre kell választani, hogy minden felismerhető legyen.*/
    int interval_max; /**< a rendezési környezet felső határa */
    double merge; /**< minél kisebb a merge mérete annál kisebbet ugrik a ciklus, ennek megfelelően annál nagyobb lesz az átfedés a környezetek között. Ha ez 0, az azt jelenti hogy minden pixelt megvizsgál, így kellően nagy treshold tartományban majdnem minden pixel bekerül és a kép el fog csúszni a rendezés irányának megfelelően, mivel a legvilágosabb pixelek a kép szélére sodródnak. Ez azt is jelenti, hogy sokkal lassabb lesz a program (1080x1080-as képen akár 500 ezer - 1 millió rendezést is el kell végezni.). */
} PsOptions;

void setfilter(Filter *filter, int *filt, double mult, int size_x, int size_y);
void freefilter(Filter filter);
int **allocatefilter(int size_x, int size_y);

HSL rgb2hsl(unsigned char pixel[]);

double hsl2rgbcolor(double temp1, double temp2, double temp3);

RGB hsl2rgb(HSL pixel);

void invert(unsigned char pixel[]);

void contrast(unsigned char pixel[], double level);

void grayscale(unsigned char pixel[]);
void sharp_grayscale(unsigned char pixel[]);
void set_black(unsigned char pixel[], int treshold);

unsigned char ***detect_edges(unsigned char ***image, int size_x, int size_y);

void change_light(unsigned char pixel[], int percent);

void mirror_diagonal(unsigned char ***matrix, int size_x, int size_y);
void mirror_vertical(unsigned char ***matrix, int size_x, int size_y);
void mirror_horizontal(unsigned char ***matrix, int size_x, int size_y);

void convolve(unsigned char ***original, int size_x, int size_y, Filter filter, int times);

void sortcopy(Sort partline[], unsigned char ***image, int line, int start, int elem, int dir);

void pixelsort(unsigned char ***image, int size_x, int size_y, PsOptions options);

void hue_shift(unsigned char pixel[], double value);
void sinecolor_shift(unsigned char pixel[], double amplifier, double freq, double phase, double bias);

void rgb_shift(PPM_Image *image, RGB_SHIFT options);

void anaglyph3d(PPM_Image *image);

void corrupt(PPM_Image *image);

#endif
