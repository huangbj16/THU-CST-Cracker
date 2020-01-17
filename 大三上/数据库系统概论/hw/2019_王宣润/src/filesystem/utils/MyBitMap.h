#ifndef MY_BIT_MAP
#define MY_BIT_MAP
typedef unsigned int uint;

#include <iostream>
using namespace std;

#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
#define BIAS 5

#ifndef DATABASE_MYBITMAP1_H
#define DATABASE_MYBITMAP1_H

class MyBitMap {
protected:
    static unsigned char h[61];
    static unsigned int getMask(int k) {
        unsigned int s = 0;
        for (int i = 0; i < k; ++ i) {
            s += (1 << i);
        }
        return s;
    }
    unsigned int * data;
    int size;
    int rootBit;
    int rootLevel;
    int rootIndex;
    unsigned int inner[MAX_INNER_NUM];
    unsigned int innerMask;
    unsigned int rootMask;
    //virtual
    unsigned int getLeafData(int index) {
        return data[index];
    }
    //virtual
    void setLeafData(int index, unsigned int v) {
        data[index] = v;
    }
    int setLeafBit(int index, unsigned int k) {
        int pos, bit;
        getPos(index, pos, bit);
        unsigned int umask = (1 << bit);
        unsigned int mask = (~umask);
        if (k == 0) {
            umask = 0;
        }
        unsigned int w = ((getLeafData(pos) & mask) | umask);
        setLeafData(pos, w);
        return pos;
    }
    unsigned int childWord(int start, int bitNum, int i, int j) {
        //cout << start << " " << bitNum << " " << i << " " << j << endl;
        int index = (i << BIAS) + j;
        if (start == 0) {
            return getLeafData(index);
        } else {
            //cout << start - bitNum + index << endl;
            return inner[start - bitNum + index];
        }
    }
    void init() {
        rootLevel = 0;
        int s = size;
        rootIndex = 0;
        while (s > LEAF_BIT) {
            int wordNum = (s >> BIAS);
            //cout << rootIndex << " " << s << " " << wordNum << endl;
            for (int i = 0; i < wordNum; ++ i) {
                unsigned int w = 0;
                //cout << "---------------------------------------" << endl;
                for (int j = 0; j < LEAF_BIT; ++ j) {
                    //cout << i << endl;
                    unsigned int k = (1 << j);
                    unsigned int c = childWord(rootIndex, s, i, j);
                    if (c != 0) {
                        w += k;
                    }
                }
                inner[rootIndex + i] = w;
            }
            rootLevel ++;
            rootIndex += wordNum;
            s = wordNum;
        }
        rootBit = s;
        int i = 0;
        unsigned int w = 0;
        for (int j = 0; j < rootBit; ++ j) {
            unsigned int k = (1 << j);
            unsigned int c = childWord(rootIndex, s, i, j);
            if (c != 0) {
                w += k;
            }
        }
        inner[rootIndex] = w;
        innerMask = getMask(BIAS);
        rootMask = getMask(s);
    }
    int _setBit(unsigned int * start, int index, unsigned int k) {
        int pos, bit;
        getPos(index, pos, bit);
        unsigned int umask = (1 << bit);
        unsigned int mask = (~umask);
        if (k == 0) {
            umask = 0;
        }
        start[pos] = ((start[pos] & mask) | umask);
        return pos;
    }
    void updateInner(int level, int offset, int index, int levelCap, unsigned int k) {
        //cout << level << " " << rootLevel << endl;
        unsigned int * start = (&inner[offset]);
        int pos = _setBit(start, index, k);
        if (level == rootLevel) {
            return;
        }
        unsigned int c = 1;
        if (start[pos] == 0) {
            c = 0;
        }
        /*
        if (level == 1) {
            cout << "level1:" << start[index] << " " << c << endl;
        }*/
        updateInner(level + 1, offset + levelCap, pos, (levelCap >> BIAS), c);
    }
    int _findLeftOne(int level, int offset, int pos, int prevLevelCap) {
        unsigned int lb = lowbit(inner[offset + pos]);
        int index = h[_hash(lb)];
        /*if (level == 0) {
            cout << "level0:" << index << " " << pos << endl;
        }*/
        int nPos = (pos << BIAS) + index;
        if (level == 0) {
            //	cout << "npos " << nPos << endl;
            return nPos;
        }
        return _findLeftOne(level - 1, offset - prevLevelCap, nPos, (prevLevelCap << BIAS));
    }
public:
//	static const int BIAS;/* = 5;*/
//	static void initConst();
    /* {
        for (int i = 0; i < 32; ++ i) {
            unsigned int k = (1 << i);
            MyBitMap::h[MyBitMap::_hash(k)] = i;
        }
    }
    */
    static int _hash(unsigned int i) {
        return i % 61;
    }
    static void initConst() {
        for (int i = 0; i < 32; ++ i) {
            unsigned int k = (1 << i);
            h[_hash(k)] = i;
        }
    }
    static int getIndex(unsigned int k)
    {
        return h[_hash(k)];
    }
    static unsigned int lowbit(unsigned int k) {
        return (k & (-k));
    }
    static void getPos(int index, int& pos, int& bit) {
        pos = (index >> BIAS);
        bit = index - (pos << BIAS);
    }
    unsigned int data0(){
        return data[0];
    }
    void setBit(int index, unsigned int k) {
        //cout << data[0]<<endl;
        int p = setLeafBit(index, k);
        //cout <<"seting "<<data[0]<<endl;
        //cout << "ok" << endl;
        unsigned int c = 1;
        if (getLeafData(p) == 0) {
            c = 0;
        }
        //cout << p << " " << c << endl;
        updateInner(0, 0, p, (size >> BIAS), c);
    }
    int findLeftOne() {
        int i = _findLeftOne(rootLevel, rootIndex, 0, rootBit);
        /*
        for (i = 0; i < size;++i){
            if (data[i] !=0)break;
        }*/
        //cout << "nPosi " << i << " " << getLeafData(i) << endl;
        //cout << i << endl;
        //cout << data[0] << endl;
        unsigned int lb = lowbit(getLeafData(i));
        int index = h[_hash(lb)];
        return (i << BIAS) + index;
    }
    MyBitMap(int cap, unsigned int k) {
        size = (cap >> BIAS);
        data = new unsigned int[size];
        unsigned int fill = 0;
        if (k == 1) {
            fill = 0xffffffff;
        }
        for (int i = 0; i < size; ++ i) {
            data[i] = fill;
        }
        init();
    }
    MyBitMap(int cap, unsigned int * da) {
        data = da;
        size = (cap >> BIAS);
        init();
    }
    void reLoad(unsigned int * da) {
        data = da;
    }
};

#include "MyBitMap.h"

#endif //DATABASE_MYBITMAP1_H


#endif
