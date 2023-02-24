#ifndef _TMS9918CONV_H_
#define _TMS9918CONV_H_

#include <inttypes.h>
#include <vector>
#include "TMS9918State.h"

/*
class BmRGBA {
    std::vector<uint8_t> _v;
    int _width,_height;
};*/

class BmIndexPix {
public:
    virtual uint8_t pix(int x, int y) const=0;
    virtual uint16_t width() const =0;
    virtual uint16_t height() const =0;
};


class TMS_BmConverter {
public:
    TMS_BmConverter(vchip::TMS9918State &tms);

    // spritecolors in sprite mode
    void mode2MapIndexedPixelBm(const BmIndexPix &bmi, const std::vector<uint8_t> &spriteColors);

    //void mode2Map(const BmRGBA &bm);
    //void map8PixelsToTMS(const uint8_t *pRgba,uint8_t &bm,uint8_t &cl, uint16_t &errorRate);

    vchip::TMS9918State &_tms;
};


#endif
