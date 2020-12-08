#ifndef CEXT_UTILS_H
#define CEXT_UTILS_H

static inline double fclamp(double x, double min, double max){
    if(x < min) return min;
    if(x > max) return max;
    return x;
}

#endif //CEXT_UTILS_H
