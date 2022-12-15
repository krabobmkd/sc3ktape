#include "TMS9918_BmConverter.h"

using namespace std;

TMSBmConverter::TMSBmConverter(TMS9918State &tms)
    :_tms(tms)
{

}

inline uint16_t colorDif(uint8_t *pRgb,uint32_t c )
{

}

void TMSBmConverter::map8PixelsToTMS(const uint8_t *pRgba,uint8_t &bm,uint8_t &cl, uint16_t &errorRate)
{
    // map to palette
    vector<uint8_t> p8(8);

    const vector<uint32_t> &palette = _tms.paletteRGBA();
    const vector<uint16_t> paletteUse(palette.size(),0);
    for(size_t i=0;i<8;i++)
    {

    }

    // get 2 dominants


    // map bm and color


}

void TMSBmConverter::mode2Map(const BmRGBA &bm)
{

}


