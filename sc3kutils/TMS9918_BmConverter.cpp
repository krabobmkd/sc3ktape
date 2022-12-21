#include "TMS9918_BmConverter.h"

using namespace std;
using namespace vchip;

TMS_BmConverter::TMS_BmConverter(TMS9918State &tms)
    :_tms(tms)
{

}

typedef uint16_t errt;

inline errt colorDif(const uint8_t *pRgb,uint32_t palettec )
{
    int16_t rp = (int16_t) pRgb[0];
    int16_t gp = (int16_t) pRgb[1];
    int16_t bp = (int16_t) pRgb[2];
    int16_t rr = rp - (palettec & 255);
    int16_t gg = gp - ((palettec>>8) & 255);
    int16_t bb = bp - ((palettec>>16) & 255);
    if(rr<0) rr=-rr;
    if(gg<0) gg=-gg;
    if(bb<0) bb=-bb;
    return (uint16_t)rr+gg+bb;
}

void TMS_BmConverter::map8PixelsToTMS(const uint8_t *pRgba,uint8_t &bm,uint8_t &cl, uint16_t &errorRate)
{

    const errt maxErr = 256*4;
    // map to palette
    vector<uint8_t> directRemap(8);
    vector<uint16_t> directRemapError(8);
    uint16_t directRemapErrorTotal = 0;

    const vector<uint32_t> &palette = _tms.paletteRGBA();
    vector<uint16_t> paletteScore(palette.size(),0);

    int mostUsedA=0;
    errt mostUsedAerr = maxErr;
    int mostUsedB=0;
    for(size_t i=0;i<8;i++)
    {                
        int bestc=0; // can't be 0 with TMS.
        errt besterr = 256*4;
        // 0 is transpa
        for(int ic=1;ic<(int)palette.size();ic++)
        {
            errt err = colorDif(pRgba,palette[ic]);
            if(err<besterr) bestc=ic;
        }
        directRemap[i]=bestc;
        directRemapError[i]=besterr;
        directRemapErrorTotal += besterr;

        // note: another approach would be find average color of region,...


        // sort out 2 most used colors
        paletteScore[bestc]++;

        pRgba+=4;
    }

    // get 2 dominants


    // map bm and color


}

void TMS_BmConverter::mode2Map(const BmRGBA &bm)
{

}


