#ifndef DATABASE_UTILS_H
#define DATABASE_UTILS_H

#include <assert.h>

struct RMBitMap {
    char *pData;
    int length;

    RMBitMap(char *pData, int length) {
        this->pData = pData;
        this->length = length;
    }

    int get(int i) {
        assert(i < length);
        return (pData[i >> 3] >> (i & 7)) & 1;
    }

    void set(int i, int d) {
        assert(i < length);
        if (d)
            pData[i >> 3] |= 1 << (i & 7);
        else
            pData[i >> 3] &= 255 - (1 << (i & 7));
    }
};

#endif //DATABASE_UTILS_H
