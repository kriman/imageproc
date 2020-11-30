#include "addmath.h"
/**
 * @file
 * @brief Kiegészítő matematikai függvények
 */

/**
 * @brief: egy double tömb legkisebb elemét veszi
 * @param[in] list a tömb amiben a legkisebbet keressük
 * @param[in] size a tömb mérete
 * @param[out] minimum a legkisebb érték
*/
double min(double list[], int size) {
    double minimum = list[0];
    for (int i = 1; i < size; i++) {
        if (list[i] < minimum)
            minimum = list[i];
    }
    return minimum;
}

/**
 * @brief: egy double tömb legnagyobb elemét veszi
 * @param[in] list a tömb amiben a legnagyobbat keressük
 * @param[in] size a tömb mérete
 * @param[out] maximum a legnagyobb érték
*/
double max(double list[], int size) {
    double maximum = list[0];
    for (int i = 1; i < size; i++) {
        if (list[i] > maximum)
            maximum = list[i];
    }
    return maximum;
}

/**
 * @brief: minimum és maximum értékek közé szorítja a megadott számot
 * @param[in] value a szám amit korlátozni akarunk
 * @param[in] minval minimum
 * @param[in] maxval maximum
*/
double clamp(double value, double minval, double maxval) {
    if (value < minval)
        return minval;
    if (value > maxval)
        return maxval;
    return value;
}

/**
 * @brief A quicksort segédfüggvénye
 * @param[in] *list a rendezni kívánt tömb
 * @param[in] lo a legkisebb index
 * @param[in] hi a legnagyobb index
 */
int partition(Sort *list, int lo, int hi) {
    double pivot = list[hi].value;
    int i = lo;
    for (int j = lo; j < hi; j++){
        if (list[j].value < pivot) {
            Sort temp = list[i];
            list[i] = list[j];
            list[j] = temp;
            i++;
        }
    }
    Sort temp = list[i];
    list[i] = list[hi];
    list[hi] = temp;
    return i;
}

/**
 * @brief Quicksort alapú rendezés
 * @param[in] *list a rendezni kívánt tömb
 * @param[in] lo a legkisebb index
 * @param[in] hi a legnagyobb index
 * @see pszeudokód itt: https://en.wikipedia.org/wiki/Quicksort#Lomuto_partition_scheme
 * @see partition
 */
void quicksort(Sort *list, int lo, int hi) {
    if (lo < hi) {
        int p = partition(list, lo, hi);
        quicksort(list, lo, p - 1);
        quicksort(list, p + 1, hi);
    }
}
