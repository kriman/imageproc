#ifndef ADDMATH
#define ADDMATH

typedef struct Sort {
    double value; /**< a rendezés alapjául szolgáló érték */
    int idx; /**< az értékhez tartozó index az eredeti tömbben */
} Sort;

double min(double list[], int size);

double max(double list[], int size);

double clamp(double value, double minval, double maxval);

int partition(Sort *list, int lo, int hi);

void quicksort(Sort *list, int lo, int hi);

#endif
