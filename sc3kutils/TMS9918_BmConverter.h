#ifndef _TMS9918CONV_H_
#define _TMS9918CONV_H_

#include <inttypes.h>
#include <vector>
#include "TMS9918State.h"


struct BmRGBA {
    std::vector<uint8_t> _v;
    int _width,_height;
};

class TMSBmConverter {
public:
    TMSBmConverter(TMS9918State &tms);

    void mode2Map(const BmRGBA &bm);

    void map8PixelsToTMS(const uint8_t *pRgba,uint8_t &bm,uint8_t &cl, uint16_t &errorRate);

    TMS9918State &_tms;
};


#endif
